# Nyancat Character Device

Nyancat rendered locally in your terminal.

[![Nyancat](http://i.imgur.com/VR3Ab7W.png)](http://i.imgur.com/VR3Ab7W.png)

## Build

You will need to install the needed tools and libraries to build:

First build the C application (these 2 are needed on ubuntu/debian):

    sudo apt-get install linux-headers-$(uname -r) build-essential

Compile the module:

    make

Copy the udev rule to the correct place (this creates the device node for you upon module insertion):

  sudo cp etc/udev/rules.d/10-nyancat.rules /etc/udev/rules.d/10-nyancat.rules

Insert the compiled module:

    sudo insmod nyancat_cdev_module.ko

Cat the character device:

    cat /dev/nyancat

Use `cat` and `echo` to view and modify the 2 parameterst that control the output (`90` ms and `xterm` are the defaults):

    cat /proc/nyancat/delay_in_ms
    90
    cat /proc/nyancat/term_type
    xterm
    echo 250 > /proc/nyancat/delay_in_ms
    echo vt220 > /proc/nyancat/term_type

Acceptable values for term_type are: `xterm`, `linux`, `vt220`, `fallback`, `rxvt`, and `default`.

## Licenses, References, etc.

The original source of the Nyancat animation is
[prguitarman](http://www.prguitarman.com/index.php?id=348).

The code provided here is provided under the terms of the
[Apache 2.0 License](http://www.apache.org/licenses/LICENSE-2.0).
