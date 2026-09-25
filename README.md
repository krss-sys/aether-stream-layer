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

# 2. Run main application
./build/aether_broker
./build/aether_server

# 3. Run unit tests
ctest --test-dir build --output-on-failure

# 4. Run tests with ThreadSanitizer (TSan)
cmake -B build-tsan -DAETHER_SANITIZE=thread
cmake --build build-tsan
ctest --test-dir build-tsan --output-on-failure

# 5. Trace system calls (verify file descriptor lifecycle with Linux strace)
strace -e trace=close,pipe2,pipe ./build/aether_tests
```

---

## Engineering Log

<details>
<summary><b>Phase 0: Project Infrastructure & Setup (Completed)</b></summary>

* Integrated build system verification with CMake, optimized execution workflow via terminal, and finalized core README documentation structure.
* Integrated GoogleTest using CMake `FetchContent` and configured ThreadSanitizer to catch data races early.
* Writing unit tests alongside thread-safe data structures like `ThreadSafeQueue` provided clear visibility into race conditions and mutex locking behavior.
* Standardized the build and test pipeline with CMake and CTest for a more reliable development workflow.

</details>

### Phase 1: Networking & Linux Systems Core
* Implemented RAII `FileDescriptor` wrapper to safely manage OS resources with move semantics and strict copy prevention.
* Verified `FileDescriptor` move semantics and exception safety using GoogleTest framework[cite: 8].
* Confirmed non-leaking descriptor destruction and prevented double-close bugs by analyzing system calls with `strace`[cite: 8].
* Designed `TcpServer` core socket initialization sequence (`socket`, `setsockopt`, `bind`, `listen`) under `aether` namespace[cite: 4, 8].
* Configured `SO_REUSEADDR` socket option to handle immediate socket address rebinding upon server restarts[cite: 8].
* Verified active server socket listening state on port `8080` via Linux network utility `ss -ltnp`[cite: 8].