/*
©  [2018] Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software and
any derivatives exclusively with Microchip products. It is your responsibility
to comply with third party license terms applicable to your use of third party
software (including open source software) that may accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER EXPRESS,
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES
OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE. IN
NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN
ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST
EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU
HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

USB70xx_FlexConnect.c - Example code on how to issue the FlexConnect Command to the
USB70xx hub
Author: Connor Chilton <connor.chilton@microchip.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <errno.h>

/* the buffer sizes can exceed the USB MTU */
#define MAX_CTL_XFER	64
#define MAX_BULK_XFER	512

/*
 * VENDOR_ID is always 0x0424 for Microchip/SMSC Hubs
 * Note: FlexConnect Command is not issued to Hub but rather the Hub Feature Controller
*/

#define VENDOR_ID	0x0424


// product id of the down7 USB2 hub that connects to HFC
// #define HFC_HUB_ID 	0x4216

// product id of the Hub Feature Controller on internal USB2 port
#define HFC_ID 		0x7240


/**
 * struct my_usb_device - struct that ties USB related stuff
 * @dev: pointer libusb devic
 * @dev_handle: device handle for USB devices
 * @ctx: context of the current session
 * @device_desc: structure that holds the device descriptor fields
 * @inbuf: buffer for USB IN communication
 * @outbut: buffer for USB OUT communication
 */

struct libusb_session {
	libusb_device **dev;
	libusb_device_handle *dev_handle;
	libusb_context *ctx;
	struct libusb_device_descriptor device_desc;
	unsigned char inbuf[MAX_CTL_XFER];
	unsigned char outbuf[MAX_BULK_XFER];
	uint16_t wValue;
	int port_num;
};
static struct libusb_session session;



int main(int argc, char **argv) {
	ssize_t cnt;

	int r = libusb_init(&session.ctx);
	if (r < 0) {
		printf("Init Error %i occourred.\n", r);
		return -EIO;
	}

	printf("Searching USB devices...");
	cnt = libusb_get_device_list(session.ctx, &session.dev);
	if (cnt < 0) {
		printf("no device found\n");
		libusb_exit(session.ctx);
		return -ENODEV;
	}

	session.dev_handle = libusb_open_device_with_vid_pid(session.ctx, VENDOR_ID, HFC_ID);
	if (session.dev_handle){
		printf("Opened HFC with ProductID 0x%04x\n", HFC_ID);
	}
	else {
		printf("Failed to open HFC\n");
		libusb_exit(session.ctx);
		return -ENODEV;
	}
	/* free the list, unref the devices in it */
	libusb_free_device_list(session.dev, 1);

	/* find out if a kernel driver is attached */
	if (libusb_kernel_driver_active(session.dev_handle, 0) == 1) {
		printf("Device has kernel driver attached.\n");
		/* detach it */
		if (!libusb_detach_kernel_driver(session.dev_handle, 0))
			printf("Kernel Driver Detached!\n");
	}

	/* claim interface 0 (the first) of device (mine had jsut 1) */
	printf("Claiming interface 0...");
	r = libusb_claim_interface(session.dev_handle, 0);
	if (r < 0) {
		printf("failed\n");
		libusb_close(session.dev_handle);
		libusb_exit(session.ctx);
		return -EIO;
	}
	printf("ok\n");

	uint8_t bmRequestType = 0x41;	/* Always 0x41					*/
	uint8_t bRequest = 0x90;		/* Always 0x90 for SET_ROLE_SWITCH 		*/
	uint16_t wIndex = 0x0000;		/* Always 0x0000				*/
	unsigned char *data = 0;		/* Always zero data length control xfer		*/
	uint16_t wLength = 0x0000;		/* Always 0x0000				*/
	unsigned int timeout_ = 50000000;

	uint16_t payload = 0;

	printf("Enter payload as hex word (without '0x'): ");
	scanf("%x", (unsigned int *)&payload);
	printf("Sending payload: 0x%04x...\n", payload);

	/* Send Flexconnect control transfer */
	r = libusb_control_transfer(session.dev_handle,
								bmRequestType,
								bRequest,
								payload,
								wIndex,
								data,
								wLength,
								timeout_);
	if (!r){
		printf("Flexconnect Control transfer successful!\n");
	}
	else {
		printf("Flexconnect Control transfer failed. Error: %d\n", r);
	}

	/* release interface */
	printf("Releasing interface...");
	r = libusb_release_interface(session.dev_handle, 0);
	if (r) {
		printf("failed\n");
		return -EIO;
	}
	printf("ok\n");

	/* close the device we opened */
	libusb_close(session.dev_handle);
	libusb_exit(session.ctx);
	return 0;
}