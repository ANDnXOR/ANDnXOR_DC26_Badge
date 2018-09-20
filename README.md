# AND!XOR DC26 Indie Badge

All files related to the badge are posted here. For now we are only releasing the SD card image as well as any updates to the .bin file (for updates to the core badge itself).

To create a new SD card, format the card with FAT32 then copy the contents of SD/ to the SD card.

*Note: due to the large size of some files, git lfs is required: https://git-lfs.github.com/ *

## LULZCODE Support for Atom Editor ##

LULZCODE is supported by Atom using our plugin: https://atom.io/packages/atom-lulzcode

It is only tested to work on Linux as Windows does not play well with YMODEM from the commandline. At AND!XOR we've been using Linux as our daily-driver since the year of the Linux Desktop anyways. And so should you.
 
## Firmware Updates ##

This year's badge is capable of flashing itself over the air (OTA) or from SD card. OTAs will be available at DEF CON at select locations, times and locations TBD.

To update your firmware manually, place `mrmeeseeks.enc.bin` and `mrmeeseeks.enc.bin.sha256` in the root directory of your SD Card. Place the SD Card back into the badge. The badge may reboot itself or you will need to manually restart it to start the OTA process.

The badge's eye will _very slowly_ blink as it flashes itself. When complete, the badge will restart and the files will be deleted. The process should take 1 to 2 minutes. 

To check the firmware version, go to *System --> About*

*NOTE: It is pointless to check the hash in the `.sha256` file. It will not match.*

## Changelog ##

#### v1.3 ####
* Minor tweaks to further improve SD reliability / avoid corruption.

#### v1.2 ####
* Huge SD stability improvements. Bad blocks no longer crash bling, but will drop frames. Bad blocks are still not readable but we no longer crash.
* Updated to latest ESP-IDF 3.1 beta
* Improved usability of menus based on user feedback
* Fixed ADC not-calibrated (battery voltage was incorrect)
* Added BLE peer hello for Trans Ionospheric badge

#### v1.1 ####
* Stability improvements to the SD Card driver at the expense of some FPS
* Minor improvements to usability of the Pick Name UI
* Thank you to the beta testers for the honest and plentiful feedback of SD card issues. Quality of the SD cards we provided with the badges is poor :( but this firmware update should ensure that good SD cards work properly

#### v1.0 ####
* Initial Release, all Kickstarters received this version

## Known Issues ##

* Voltage displayed on main menu title bar is not accurate
* SD Cards with many bad sectors crash badge (reboot) or exit bling to menu
* Some CHIP8 games do not run
