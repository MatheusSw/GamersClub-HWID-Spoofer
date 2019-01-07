### Gamersclub HWID Spoofer
Just a simple HWID spoofer for GamersClub without hooking disk.sys major functions which they check for in their driver.

The JSONLogger basically just logs the number of IOCTL requests between the Launcher (GCLauncher.exe) and the Driver (GCSecure.sys) for now.

# What they do

They use the standard/easiest way to get a HWID.
1. CreateFileA/W on your main disk to retrieve the size of the storage device, so they can allocate memory for a later call to DeviceIO requesting the actual information on the disk. Check out STORAGE_DESCRIPTOR_HEADER on MSDN.
2.With the size in hands and the buffer allocated, they make a second call via DeviceIoControl but this time they are looking to get the STORAGE_DEVICE_DESCRIPTOR structure.
3. Then they get the serial by suming output_buffer and SerialOffset (that comes from the STORAGE_DEVICE_DESCRIPTOR structure) and using it.

# What we do
Simply intercept that second call so we can get the pointer to the buffer that we'll later on call the original DeviceIoControl function and let they fill the structure, then we generate a new serial and simply replace it on the output buffer.
