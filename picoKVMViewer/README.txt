# Dependencies
qtmultimedia5-dev libqt5serialport5-dev

# Build
cd src
cmake
make

# Open Points
 * Keycode mapping only tested on one linux keyboard
 * Keyboard autorepeat not implemented/understood
 * might collide with gstreamer1.0-vaapi1.0-0

# Notes

## udev device
udevadm info --name=/dev/ttyACM1 --attribute-walk
SUBSYSTEM=="tty", ATTRS{idVendor}=="1b4f", ATTRS{idProduct}=="9206", SYMLINK+="ttyProMicro"
 

