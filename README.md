# Bootloader
### Description
A bootloader that allows you to load a user program from a specified address. It supports a number of commands, which, in particular, provide the ability to load a user program via the UART interface.

### Launch
* ```make``` - building a production version of the code for target;
* ```make -f MakefileTest.mk``` - building a test version for development system.

### Structure
Since the bootloader is inextricably linked to the hardware, its functionality was separated. The most important part, responsible for loading the user application (start_application_code function) is located in the [main](https://github.com/MatveyMelnikov/Bootloader/blob/master/Core/Src/main.c). 
A separate [bootloader_cmd](https://github.com/MatveyMelnikov/Bootloader/tree/master/External/bootloader) module contains the code responsible for processing the command (this operating mode is enabled if pin PB12 pin is connected to power when the microcontroller is started).
Tests using the [Unity library]([https://github.com/MatveyMelnikov/EEPROM_Driver/tree/master/External/Unity-2.5.2](https://github.com/MatveyMelnikov/Bootloader/tree/master/External/Unity-2.5.2)) 
are implemented [here]([https://github.com/MatveyMelnikov/EEPROM_Driver/tree/master/Tests](https://github.com/MatveyMelnikov/Bootloader/tree/master/Tests)https://github.com/MatveyMelnikov/Bootloader/tree/master/Tests).

### Commands
0. Help - lists all available bootloader commands;
1. Get id - displays microcontroller ID;
2. Get bootloader version - displays the current version of the bootloader (current - 0.1);
3. Write to flash - Starts cyclic writing to flash memory (The first page into which data can be written is the 10th). The cycle can be described as follows (from the STM32 side):
```send ACK; read address / end sequence (32 bit); read data (16 bits); program flash; send ACK```. If any of the transmissions is late or an error occurs, the cycle is interrupted.
Wherein: ACK = 0x55, NACK = 0xAA, end sequence = 0xCC33;
4. Erase flash - clears specified pages. Before writing to flash memory, it must be cleared. How the erasing process occurs (from the STM32 side): ```send ACK; first erase page address (32 bit); num of pages (8 bits); erase flash; send ACK```;
5. Read - displays the contents of a flash memory page.

Running user code is only allowed from the 'app start address' ([APP_START_ADDRESS](https://github.com/MatveyMelnikov/Bootloader/blob/master/External/bootloader/Inc/bootloader_defs.h)). To load it you need to change the addresses in the linker script:
```
...
/* Specify the memory areas */
MEMORY
{
RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 20K
FLASH (rx)      : ORIGIN = 0x8002800, LENGTH = 64K - 10K
}
...
```

![sheme_bootloader](https://github.com/MatveyMelnikov/Bootloader/assets/55649891/d02f3fb2-c2aa-4f95-845a-af110fa38f6e)
