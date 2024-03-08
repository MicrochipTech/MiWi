# Network Startup

The PAN coordinator organizes the wireless network automatically. Upon starting the network, every node informs the network of its role. When the PAN coordinator is powered on, it switches to an active state even though no child node is present. This behavior is normal. It indicates that the PAN coordinator is ready, and the child nodes can join the network with the coordinator’s PAN ID. By default, the coordinator uses PAN ID 0x1234, which is recognized by all the coordinators. The PAN ID can be modified by the user through the application’s configuration file.

If the PAN coordinator is absent or has not been turned on, the coordinators and end devices remain in the Network Search mode. In this mode, the coordinators scan the channels specified in the channel mask in search of a network. By default, the channel mask in the application provided with the release contains a single channel. On rare occasions, if the frequency corresponding to the radio channel is busy, the coordinator node may stay in the network search mode. If this happens, it may become necessary to change the application’s channel mask to select another channel by changing the application’s configuration file and recompiling the application.

**Parent topic:**[MiWi Demo Reference Application](GUID-32628D58-8B41-490F-8DA4-520C34856980.md)

