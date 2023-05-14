## Fast Debug Mode

In order to quickly debug UI flows, there are a series of macros that can be used to quickly improve iteration speed

These can be set manually or in Projucer `Preprocessor Definitions` section in the exporter configs

- `FDB_LOAD_FILE`
    - Starts loading a file on start up instead of finding the file in file explorer each time
    - Example: `FDB_LOAD_FILE="/home/Music/test.wav"`