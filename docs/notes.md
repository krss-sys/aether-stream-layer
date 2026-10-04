# Aether Network Engine - Technical Notes

---

## Day 07 - `recv()` Return Values & Semantics

The `recv(fd, buf, count, flags)` system call handles incoming socket data. Its return value `n` indicates the socket status:

| Return Value | Meaning | Action Required |
| :--- | :--- | :--- |
| **`n > 0`** | Successfully received `n` bytes of data. | Process buffer contents. |
| **`n == 0`** | Graceful shutdown by peer (TCP FIN packet / EOF). | Close the client socket. |
| **`n < 0`** | System/Network error occurred during read. | Log error via `errno` / `strerror` and close socket. |

---

## Day 09 - Thread-per-Connection Architecture

Transitioned from a single-threaded blocking model to a multi-threaded architecture (`std::thread`). The main accept loop delegates client connections to dedicated background worker threads.

### Execution Flow

```mermaid
sequenceDiagram
    autonumber
    actor Client
    participant Main as Main Thread
    participant Worker as Worker Thread

    loop Continuous Accept Loop
        Main->>Main: Wait for connection (accept)
        Client->>Main: Connect (TCP Handshake)
        Main->>Worker: Spawn thread & move(client_fd)
        activate Worker
        Note over Main: Immediately resume accept loop
    end

    Worker->>Client: Handle Echo I/O independently
    Client->>Worker: Disconnect (FIN)
    Worker->>Worker: Close Socket & Exit Thread
    deactivate Worker