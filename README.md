## How to Use the Library

### Understanding the Library - Files on the Programming Guide Folder
- **API Documentation:** Provides details about the library's functions. It's essential to get acquainted with these to effectively utilize the library.
- **Programming Guide:** Explains the design of the library and offers guidance on how to use it. Please read the files in the programming guide section to gain a comprehensive understanding.

### About This Library
This repository contains the Realtek Unmanaged Switch Library Version 1.12. 
- **Staying Updated:** If you're using or have a newer version of this library, please contact me at [your email/contact information] to discuss updating this repository.
- **Original Library Files:** For those who need them, the original library files are included in the `api_source` folder.

### Enhanced C++ Library for Arduino
I have developed a C++ library with classes to enhance the usability of this library. The enhanced library is designed to be more user-friendly and intuitive. It also provides additional features and functions.
- **Find the Enhanced Library Here:** [Realtek Unmanaged Switch Class Wrapper for Arduino](https://github.com/shiroichiheisen/Realtek-Unmanaged-Switch-Class-Wrapper-Arduino-Library)

In addition to the main library, there's a EEPROM file designed for the RTL8367S IC. In my experience, initializing the switch with this EEPROM file before using the library for further configuration proves to be more effective because you dont need to initialize with your IC (you will need to wait about 3 seconds to initialize to make the configurations). This file is just 4KB in size, making it compatible with smaller EEPROMs like the 24C04 or larger. Although I'm currently using a 24C32 due to availability, the 24C04 is more than sufficient to store the EEPROM file. Please read the RTL8367S datasheet, this IC use the pin EEPROM_MOD/LAN4LED0 to select the eeprom size, if low - eeprom size less or equal to 16kb (24C02 - 24C16), high - eeprom size greater than 16kb (24C32 - 24C256).

### Acknowledgments and Enhanced Arduino Library Access
My heartfelt thanks go to the entire community and all contributors. Your invaluable support and feedback played a crucial role in the development of this library, making it a useful resource for Arduino enthusiasts.

- **Origin of Realtek Files:** This project builds upon the foundational work found in the repository [Realtek_switch_hacking](https://github.com/libc0607/Realtek_switch_hacking). The original library files provided there served as a key resource in the creation of this library.