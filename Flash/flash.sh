#!/bin/bash
DEV=/dev/ttyUSB0
BAUD=921600
esptool --chip esp32 --port $DEV --baud $BAUD --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 bootloader.bin 0x10000 boot_stub.bin 0x90000 mrmeeseeks.bin 0x8000 partitions.bin
