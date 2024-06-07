![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)
![Harmony logo small](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_mplab_harmony_logo_small.png)

# MPLAB® Harmony 3 MiWi

MPLAB® Harmony 3 is an extension of the MPLAB® ecosystem for creating embedded firmware solutions for Microchip 32-bit SAM and PIC® microcontroller and microprocessor devices.  Refer to the following links for more information.

- [Microchip 32-bit MCUs](https://www.microchip.com/design-centers/32-bit)
- [Microchip 32-bit MPUs](https://www.microchip.com/design-centers/32-bit-mpus)
- [Microchip MPLAB X IDE](https://www.microchip.com/mplab/mplab-x-ide)
- [Microchip MPLAB® Harmony](https://www.microchip.com/mplab/mplab-harmony)
- [Microchip MPLAB® Harmony Pages](https://microchip-mplab-harmony.github.io/)

This repository contains the MPLAB® Harmony 3 MiWi for PIC32CX-BZ2 platform devices. 
The MiWi protocol stack supports MiWi's own proprietary application(MiApp) & mac layer(MiMAC) and IEEE 802.15.4 Standalone PHY Layer of devices like WBZ451, RF233, RF212B & RF215. 
Refer to the following links for release notes, training materials, and interface reference information.

As a first step, the [MiWi](https://github.com/MicrochipTech/MiWi) repo must be cloned to the User's Local Harmony repo folder location and follow the steps mentioned in the follwoing [link](https://github.com/MicrochipTech/MiWi/blob/main/docs/GUID-32628D58-8B41-490F-8DA4-520C34856980.md) to generate the harmony project application.
User must have atleast a Standalone PHY component that supports [WBZ451](https://github.com/Microchip-MPLAB-Harmony/wireless_15_4_phy) or Standalone Transceivers like [RF233](https://github.com/MicrochipTech/wireless_15_4_phy_trx) / [RF212B]( https://github.com/MicrochipTech/wireless_15_4_phy_trx ).

- [Release Notes](./release_notes.md)
- [MPLAB® Harmony License](mplab_harmony_license.md)
- [MPLAB® Harmony 3 MiWi API Help](docs/index.md)

# Contents Summary

| Folder     | Description                                                       |
| -----------| ------------------------------------------------------------------|
| config     | MiWi module configuration file |
| docs| [MiWi User Guide documentation](docs/index.md)|
| drivers    | MiWi Protocol files and dependent files for pic32cxbz2 products     |


____

[![License](https://img.shields.io/badge/license-Harmony%20license-orange.svg)](https://github.com/Microchip-MPLAB-Harmony/replaceme/blob/master/mplab_harmony_license.md)
[![Latest release](https://img.shields.io/github/release/Microchip-MPLAB-Harmony/replaceme.svg)](https://github.com/Microchip-MPLAB-Harmony/replaceme/releases/latest)
[![Latest release date](https://img.shields.io/github/release-date/Microchip-MPLAB-Harmony/replaceme.svg)](https://github.com/Microchip-MPLAB-Harmony/replaceme/releases/latest)
[![Commit activity](https://img.shields.io/github/commit-activity/y/Microchip-MPLAB-Harmony/replaceme.svg)](https://github.com/Microchip-MPLAB-Harmony/replaceme/graphs/commit-activity)
[![Contributors](https://img.shields.io/github/contributors-anon/Microchip-MPLAB-Harmony/replaceme.svg)]()

____

[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/user/MicrochipTechnology)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/microchip-technology)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/microchiptechnology/)
[![Follow us on Twitter](https://img.shields.io/twitter/follow/MicrochipTech.svg?style=social)](https://twitter.com/MicrochipTech)

[![](https://img.shields.io/github/stars/Microchip-MPLAB-Harmony/replaceme.svg?style=social)]()
[![](https://img.shields.io/github/watchers/Microchip-MPLAB-Harmony/replaceme.svg?style=social)]()


