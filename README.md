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
- **Compiler**: `g++` with C++20 support

### Quick Start
```bash
# Compile with C++20 standard, include path, and POSIX threads
g++ -std=c++20 src/main.cpp -Iinclude -pthread -o main

# Run executable
./main
```
---

## Engineering Log

* **2026-09-20**: Initialized project architecture (`include/`, `src/`) and implemented core thread-safe message queue (`ThreadSafeQueue.hpp`).