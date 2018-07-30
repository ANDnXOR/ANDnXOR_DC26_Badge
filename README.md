# AND!XOR DC26 Indie Badge

All files related to the badge are posted here. For now we are only releasing the SD card image as well as any updates to the .bin file (for updates to the core badge itself).

To create a new SD card, format the card with FAT32 then copy the contents of SD/ to the SD card.

*Note: due to the large size of some files, git lfs is required: https://git-lfs.github.com/ *

## Firmware Updates ##

This year's badge is capable of flashing itself over the air (OTA) or from SD card. OTAs will be available at DEF CON at select locations, times and locations TBD.

To update your firmware manually, place `mrmeeseeks.enc.bin` and `mrmeeseeks.enc.bin.sha256` in the root directory of your SD Card. Place the SD Card back into the badge. The badge may reboot itself or you will need to manually restart it to start the OTA process.

The badge's eye will _very slowly_ blink as it flashes itself. When complete, the badge will restart and the files will be deleted. The process should take 1 to 2 minutes. 

To check the firmware version, go to *System --> About*

*NOTE: It is pointless to check the hash in the `.sha256` file. It will not match.*
