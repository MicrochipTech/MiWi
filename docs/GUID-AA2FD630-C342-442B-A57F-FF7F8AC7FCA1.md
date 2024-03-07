# Security Abstraction Layer\(SAL\)

The SAL \(Security Abstraction Layer\) provides an API that allows access to low level AES<br /> engine functions abstraction to encrypt and decrypt frames.

These functions are actually implemented dependent on the underlying hardware, for<br /> example, the AES engine of the transceiver/ Microcontroller. The API provides functions<br /> to set up the proper security key, security scheme \(ECB or CBC\), and direction<br /> \(encryption or decryption\).

SAL is responsible for using the Hardware accelerators for Encryption and<br /> authentication.

CCM Encryption mode is used for doing Encryption and authentication.

Wolf Crypto Library provides following function for encryption/decryption. This can be<br /> used for different Security Levels like, with/without encryption and with/without<br /> Authentication\(MIC\) and various MIC lengths

**Parent topic:**[Other Stack Components](GUID-25E87729-19EF-46AC-A69C-DB0025F4D8BE.md)

