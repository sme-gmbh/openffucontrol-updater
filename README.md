# Firmware updater for openFFUcontrol
Commandline tool for updating the firmware in openFFUcontrol OCU and PTS via ModBus.
The openFFUcontrol bootloader needs to be installed on the target in order
to use in system update. 

## Building and installing
First make sure to have Qt5 and openffucontrol-qtmodbus installed on your system.

Create a directory for the build
```
mkdir bin
```

Create the Makefile
```
cd bin
qmake ../src
```

Compile the application wih a number of concurrent jobs of your choice

```
make -j 8
```

Install the application as root user
```
sudo make install
```

Run the updater and show available commands
```
openffucontrol-updater --help
```
