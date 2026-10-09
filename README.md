# Aether Stream Layer

A lightweight, high-throughput message streaming component built in C++20 for Linux environments. It serves as the low-latency ingestion backbone for the **Aether Platform** ecosystem.

---

## Overview

`aether-stream-layer` is an AP-oriented (Availability / Partition tolerance) message streaming engine. Designed around an in-memory architecture, it prioritizes immediate availability and fast write acknowledgement (`ACK`) over synchronous disk persistence.

### Core Architecture Concepts
- **Concurrency & Safety**: In-memory message buffers using Modern C++ thread synchronization primitives (`std::mutex`, `std::condition_variable`).
- **POSIX Networking**: Non-blocking/blocking TCP socket communication handling Producer-Consumer network traffic.
- **Data Ingestion**: High-speed message queuing designed to isolate network ingestion from downstream processing.

---

## Build & Execution

### Prerequisites
- **OS**: Linux (Ubuntu / WSL2)
- **Toolchain**: `cmake` (>= 3.16), `g++` with C++20 support

### Quick Start
```bash
# 1. Configure & build project
cmake -B build
cmake --build build

# 2. Run Echo Server & Client
./build/aether_server 8080
./build/aether_cli 127.0.0.1 8080 "hello aether"

# 3. Run unit tests
ctest --test-dir build --output-on-failure

# 4. Run tests with ThreadSanitizer (TSan)
cmake -B build-tsan -DAETHER_SANITIZE=thread
cmake --build build-tsan
ctest --test-dir build-tsan --output-on-failure

# 5. Trace server system calls (verify multi-threaded execution)
strace -f -o trace.txt ./build/aether_server 8080
```

---

## Documentation & Notes

For detailed technical notes, architecture decision records (ADR), and sequence diagrams, refer to:
- [Technical Notes & Architecture Diagrams](docs/notes.md)