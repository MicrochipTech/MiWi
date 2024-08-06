![Microchip logo](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_logo.png)
![Harmony logo small](https://raw.githubusercontent.com/wiki/Microchip-MPLAB-Harmony/Microchip-MPLAB-Harmony.github.io/images/microchip_mplab_harmony_logo_small.png)

# Microchip Wireless (MiWi) Release Notes

## Release v1.1.0

MiWi is a Microchip proprietary protocol developed to support a wide range of wireless applications. 
The backbone of MiWi is MiMAC and MiApp interfaces, which links the support of multiple RF transceivers and wireless communication protocols together as a well-defined, simple but robust Microchip proprietary wireless development environment.
With MiWi, application developers can switch between RF transceivers and wireless protocols with little or no modification in the application layer. Such migration capability in MiWi DE reduces the firmware development risk to a very minimum level. MiWi DE is defined by three layers:
• Application layer
• Protocol layer
• RF transceiver layer
The three layers are linked together by MiMAC and MiApp interfaces. The Application layer uses MiApp interfaces to communicate to the protocol layer. In the protocol layer, there are implementations of MiWi Mesh, MiWi P2P, MiWi Star wireless communication protocols. 
The drivers for WBZ451 and other Standalone Microchip RF transceivers (AT86RF233, AT86RF212B, AT86RF215) are called by protocol layers via MiMAC interfaces.

## New Features

-   Introduced Address Filtering fearute during network formation, activated by enabling the macro ADDRESS_CHECK.

## Known Issues / Limitations

-	OTAU is not supported.
-   MiWi depends on the Standalone PHY layer for lower functionality and any not supported feature/issues persist on that component will have an impact of the MiWi stack functionality.

## Development Tools
-	MPLAB X v6.20
-	MPLAB® XC32 C/C++ Compiler v4.40
-	MPLAB® X IDE plug-ins: MPLAB® Code Configurator (MCC) v5.5.0 and above
-	Device Pack: PIC32CX-BZ2-DFP (1.4.243)

## Notes
-	None
