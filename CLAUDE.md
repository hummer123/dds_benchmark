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

# Original benchmark tools (examples/listener/)
./build/deploy/bin/raw_listener   # Terminal 1: Subscriber
./build/deploy/bin/raw_talker     # Terminal 2: Publisher

# New benchmark tool (utils/bcli/)
./build/deploy/bin/bcli pub 1000 256  # Publish at 1kHz, 256B payload
./build/deploy/bin/bcli sub -c        # Subscribe with CPU monitoring
```

## Architecture

### Two Benchmark Tools

1. **Original tools** (`examples/listener/`): Simple publish/subscribe using CycloneDDS C++ API with listener callbacks
2. **bcli tool** (`utils/bcli/`): Modular benchmark client with transport abstraction layer for measuring latency, throughput, CPU/memory

### Message Flow (raw_listener/raw_talker)

```
raw_talker                    raw_listener
    │                              │
    ├── DomainParticipant(0) ─────┼── DomainParticipant(0)
    ├── Topic("BM_TalkData") ──────┼── Topic("BM_TalkData")
    ├── Publisher ──────────────────┼── Subscriber
    ├── DataWriter ─────────────────┼── DataReader + TalkListener
    │                              │
    └─ waits for publication_matched──► on_data_available()
```

**Key Patterns**:
- `TalkListener` extends `NoOpDataReaderListener<bm_test::TestData>` with `on_data_available()`
- Signal handling: `g_running` atomic bool controlled by SIGINT/SIGTERM handlers
- DDS QoS: uses default datareader_qos()

### bcli Transport Abstraction

```
bcli_main.cpp
    ├── CycloneTransport (adapter_cyclonedds.cpp)
    └── ITransport interface (bcli_transport.hpp)
            ├── create_participant()
            ├── create_topic()
            ├── create_writer() / create_reader()
            └── write() / read()
```

## Code Style

- C++17 with CycloneDDS C++ API
- Namespace: `org::eclipse::cyclonedds`
- Build: Debug (`-O0 -g`) / Release (`-O3`)
- Compiler warnings: `-Wall -Wextra`

## Key Files

| File | Purpose |
|------|---------|
| `msg/test_data.idl` | IDL message definitions (TestData, TestFixedData) |
| `msg/test_data.hpp` | Generated C++ header (auto-generated, do not edit) |
| `config/cyclonedds.xml` | CycloneDDS configuration (Domain 0, localhost only) |
| `utils/bcli/bcli_transport.hpp` | ITransport interface for middleware abstraction |
| `utils/bcli/adapters/adapter_cyclonedds.cpp` | CycloneDDS implementation of ITransport |

## IDL Message Types

```idl
module bm_test {
  struct TestData {
    uint64 seq;
    uint64 timestamp;
    string baggage;
  };

  struct TestFixedData {
    uint64 seq;
    uint64 timestamp;
    octet baggage[1024];
  };
};
```
