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
### Quick Start

1. **Build**
```bash
   cmake -B build
   cmake --build build
```

2. **Run Echo Server & Client**
```bash
   # Terminal 1: launch server
   ./build/aether_server 8080

   # Terminal 2: run client
   ./build/aether_cli 127.0.0.1 8080 "hello aether"
```

### Diagnostics & Testing

- **Unit tests**
```bash
  ctest --test-dir build --output-on-failure
```

- **Network torture test** (TCP coalescing & fragmentation)
```bash
  ./build/aether_frame_torture
```

- **Sanitizers**
```bash
  # AddressSanitizer (ASan)
  cmake -B build-asan -DAETHER_SANITIZE=address && cmake --build build-asan
  ctest --test-dir build-asan --output-on-failure

  # ThreadSanitizer (TSan)
  cmake -B build-tsan -DAETHER_SANITIZE=thread && cmake --build build-tsan
  ctest --test-dir build-tsan --output-on-failure
```

- **System call tracing**
```bash
  strace -f -o trace.txt ./build/aether_server 8080
```

---

## Documentation & Notes

For detailed technical notes, architecture decision records (ADR), and phase specifications, refer to:
- [Phase 1: Echo Server & Basic Networking](docs/phase-1.md)
- [Phase 2: Message Broker & Protocol Design](docs/phase-2.md)
- [Protocol Specification (Framing)](docs/PROTOCOL.md)