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
# Configure & build project
cmake -B build
cmake --build build

# Run application
./build/aether_broker

```

---

## Engineering Log

* Integrated build system verification with CMake, optimized execution workflow via terminal, and finalized core README documentation structure.