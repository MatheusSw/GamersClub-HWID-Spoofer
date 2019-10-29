# Gamersclub HWID Spoofer

## Patched
This is currently patched, also known as, **not working**, and I don't intend to update this whatsoever, so please don't contact me regarding this.

## About
Just a simple HWID Spoofer for the GamersClub anti-cheat, that doesn't hook any disk.sys major functions, because they have checks for those in their driver.

The JSONLogger basically just logs the number of IOCTL requests between the Launcher (GCLauncher.exe) and the Driver (GCSecure.sys) for now.

## Explanation

### What they do

**They use the standard/easiest way to get a HWID**
1. They call CreateFileA/W on your main HDD, so they can retrieve its size.
2. Call the DeviceIoControl function, but this time they want the STORAGE_DEVICE_DESCRIPTOR structure.
3. Sum the device descriptor serial offset to the struct base (STORAGE_DEVICE_DESCRIPTOR) and then use it on their anti-cheat.

### What we do
1. Intercept the second call to get the pointer to the buffer, they'll fill the output buffer, we generate a new serial and replace it on the output buffer.
