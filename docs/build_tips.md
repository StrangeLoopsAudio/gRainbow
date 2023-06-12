## Fast Debug Mode

In order to quickly debug UI flows, there are a series of macros that can be used to quickly improve iteration speed

These can be set manually or in Projucer `Preprocessor Definitions` section in the exporter configs

- `FDB_LOAD_FILE`
    - Starts loading a file on start up instead of finding the file in file explorer each time
    - Example: `FDB_LOAD_FILE="/home/Music/test.wav"`

## Debugging VST3 in Ableton Live on Windows

1. Generate solution with: cmake -B cmake-build-vs2022 -G "Visual Studio 17 2022" -A x64
2. Open the solution, right click on it, Properties->Debugging
3. Build the vst3 in Debug mode, copy it to wherever Ableton looks for VST3s
3. Set the Command to Ableton's exe, and Attach to Yes
4. Open Ableton
5. Launch the debugger, then open the plugin in Ableton