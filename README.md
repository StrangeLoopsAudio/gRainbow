# gRainbow

A synthesizer that uses pitch detection to choose candidates for granular synthesis or sampling.

## Windows

Use the Visual Studio 2019 exporter, or create your own exporter for your IDE in the Projucer

## Linux

How to build from Projucer exporter

```bash
# Debug
CONFIG=Debug make -j10 -C Builds/LinuxMakefile/
./Builds/LinuxMakefile/build/gRainbow_debug

# Release
CONFIG=Release make -j10 -C Builds/LinuxMakefile/
./Builds/LinuxMakefile/build/gRainbow
```

## Fast Debug Mode

In order to quickly debug UI flows, there are a series of macros that can be used to quickly improve iteration speed

These can be set manually or in Projucer `Preprocessor Definitions` section in the exporter configs

- `FDB_LOAD_FILE`
    - Starts loading a file on start up instead of finding the file in file explorer each time
    - Example: `FDB_LOAD_FILE="/home/Music/test.wav"`