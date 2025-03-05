## Prerequisites:
1. Setup as described in D5.4:
	1. RPI configured, connected to power and Network.
	2. Both P6 and P9 ports and connected to RPI.
	3. Jumper wires on nRST and J10
	4. LPC is zeroed and bootloader is disabled
2. 2 terminal windows, side by side, with ssh connection to RPI
	1. 1 window should have connection via minicom to `/dev/ttyACM0`
	   
## Notes:
Zeroing LPC:
1. Get board memory info `lpc55 info`

		flash_size: 646656,
		flash_page_size: 512,
		flash_sector_size: 32768,

		646656 / 512 = 1263 blocks of size 512

2. Create zero binary the size of memory of LPC:
	   `dd if=/dev/zero of=emptyfile.bin bs=512 count=1263`
3. Flash the binary

## Plan

1. Show `tree .`
	1. It shows that we have prepared 2 binaries:
		1. `flasz_zero.bin`
		2. `zephyr-helloworld.bin`
	2. And 3 scripts:
		1. `board-reset.sh` - sends signal to nRST to reset the board.
		   `sudo pinctrl set 24 op pu && sleep 1 && sudo pinctrl set 24 ip`
		2. `disable-bootloader.sh` - Disables pull down on J10. No bootloader on reset.
		   `sudo pinctrl set 23 ip`
		3. `enable-bootloader.sh` - Enables pull down on J10. Board will boot in bootloader mode on reset.
		   `sudo pinctrl set 23 op pd`
2. Show `cat` for each script
	1. `cat ./scripts/board-reset.sh`
	2. `cat ./scripts/disable-bootloader.sh`
	3. `cat ./scripts/enable-bootloader.sh`

3. Reset board (empty app)
   `./scripts/board-reset.sh`
2. Show that board produces no output on reset (minicom)
3. Enable bootloader
   `./scripts/enable-bootloader.sh`
4. Reset
   `./scripts/board-reset.sh`
5. Show detected bootloaders
   `lpc55 ls`
6. Flash "hello world"
   `lpc55 write-flash ./binaries/zephyr-helloworld.bin`
7. Disable bootloader
   `./scripts/disable-bootloader.sh`
8. Reset
   `./scripts/board-reset.sh`
9. Show that "Hello World" is displayed in minicom (success)