
This is a driver that was used during NVIDIA driver installation (`NVI2SystemService32.sys` and `NVI2SystemService64.sys`) that has an interesting vulnerability. This driver uses a pointer to a kernel object that it receives from the user and passes it to `ObQueryNameString()` to get the name of a `FILE_OBJECT`. The code in `PoC.c` demonstrates how such vulnerability be exploited by passing a pointer to fake object.

This right way of doing it is to only accept handles from the user and get the address of the object `ObReferenceObjectByHandle()`. You should never accept a direct pointer.

I contacted NVIDIA regarding this driver and they informed me that it was no longer being used post R385. It it was no longer being shipped after R396.
