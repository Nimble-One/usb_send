# USB payload sender

Asks for, then sends a 16-bit payload to the connected EVB-USB7216 board.

This code is derived from [this GitHub repo](https://github.com/MicrochipTech/USB-Hub-Linux-Examples/tree/master/USB%20Hub%20Feature%20Examples/USB70xx/FlexConnect).

Tested on Ubuntu 22.04.2 LTS.

## Compile and run

```bash
# install prerequisites
sudo apt install libusb-1.0.0-dev build-essential

# compile
make

# run (needs to be root)
sudo ./usb_send
```
