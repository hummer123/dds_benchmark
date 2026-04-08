# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

DDS (CycloneDDS) middleware performance benchmarking for robot control systems. Measures CPU usage and message transmission latency in high-frequency data transfer scenarios.

## Build Commands

```bash
# Default build (x86_64 Release)
./build.sh

# Specify architecture and build type
./build.sh x86_64 Debug
./build.sh aarch64 Release

# Clean rebuild
rm -rf build && ./build.sh
```

## Run Commands

**IMPORTANT: Must set CycloneDDS config path before running.**

```bash
export CYCLONEDDS_URI=file://$(pwd)/config/cyclonedds.xml

# Terminal 1: Start subscriber
cd build/deploy/bin
./raw_listener

# Terminal 2: Start publisher
cd build/deploy/bin
./raw_talker
```

## Architecture

```
src/listener/
├── raw_talker.cpp    # Publisher: publishes TestData at high frequency
└── raw_listener.cpp  # Subscriber: receives data using listener callback pattern

msg/
└── test_data.idl      # Message definitions (TestData, TestFixedData)
```

**Message Flow**: raw_talker publishes to "BM_TalkData" topic; raw_listener subscribes and uses `TalkListener` callback to process incoming samples.

**Key Patterns**:
- Listener callback: `TalkListener` extends `NoOpDataReaderListener<bm_test::TestData>` with `on_data_available()`
- Signal handling: `g_running` atomic bool controlled by SIGINT/SIGTERM handlers
- Graceful shutdown via Ctrl+C

## Code Style

- C++17 with CycloneDDS C++ API
- Namespace: `org::eclipse::cyclonedds`
- Build: Debug (`-O0 -g`) / Release (`-O3`)
- Compiler warnings: `-Wall -Wextra`
