mcxboot
=======

This is a memory card bootloader for AVR XMEGA devices, using
[Petit FatFs](http://elm-chan.org/fsw/ff/00index_p.html) to support FAT32
formatted filesystems. This bootloader compiles to less than 4K on parts
tested so far.

Installation
------------

To compile and install the bootloader, do the following:

* Edit `config.h` and `Makefile` to match your project and programmer.
* Run `make` to build the binary.
* Run `make flash` to install the bootloader (by default, this will also
  perform a chip erase in advance).
* Run `make fuses` to instruct the AVR to start up using the bootloader.

Usage
-----

The bootloader starts each time the device resets. If the reset was caused by
the device being powered-on *or* the PDI interface, the bootloader performs the
following steps. Otherwise the application is invoked. No external buttons are
needed in this process.

These steps are performed by the bootloader:

1) The memory card is initialized and mounted. If no memory card is present,
   the bootloader resets and the application is invoked.
2) The firmware file specified in `config.h` is opened. If the file cannot be
   found, the bootloader resets and the application is invoked.
3) The firmware file is read into memory and flashed into application space.
   Each page is read back to verify flash integrity. The device LED flickers
   during this process to indicate activity.
4) Once done, the device shows a LED flash-code to report status to the user.

The flash codes are as follows.

* **3**: Application flash was successful!
* **4**: Error: read-after-write verification failed.
* **5**: Error: a memory card read error occurred after the file was opened.

This bootloader works with raw binary files. To make a compatible `.bin` file
containing the application, use `avr-objcopy` with the *binary* output type.
After being made, simply copy the file to a FAT32 memory card for flashing.

Some other important notes:

* Only FAT32 formatted cards are supported.
* No long filenames are supported.
* The file must be placed in the root of the volume to be found.

Status
------

This is beta-quality code and pull requests are welcome. The following parts
have been tested:

* x64a3u

If you try this on other devices, or if you encounter problems, please open a
new issue.

License
-------

Except where otherwise noted, all files in this repository are available under
the terms of the GNU General Public License, version 3, available in the
LICENSE document. There is NO WARRANTY, not even for MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. For details, refer to the license.
