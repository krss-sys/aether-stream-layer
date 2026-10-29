# 📨 Phase 2: Message Broker & Protocol Design

> **TL;DR - What does this phase cover?**
> - 🔢 Multi-byte integers go on the wire in **Big-Endian** (Network Byte Order)
> - 🛑 Every frame is capped at **1 MiB** so a bad `Length` can never exhaust memory
> - 🧮 Worked example: the `PRODUCE` frame for topic `"orders"` + payload `"a"`, byte by byte
> - 📥 `recv_exact` loops until it has exactly `n` bytes, unlike `recv_some` which returns whatever is available
> - 🧱 Buffers are only allocated **after** `Length` is validated against `kMaxFrameSize`
> - 🧵 Concurrency = a fixed-size **blocking ThreadPool**: 1 worker per connection, so max concurrent clients = number of workers

---

## 📐 Day 14 - Protocol Design Rationale & Framing

### 1. Why Big-Endian (Network Byte Order)?

- **Standardization:** Network protocols (TCP/IP, IP headers, DNS) universally adopt Big-Endian as the standard Network Byte Order.
- **Consistency across platforms:** Different CPU architectures store multi-byte integers in memory differently. x86/x64 are Little-Endian, while some processors (e.g., certain ARM configurations or network processors) are Big-Endian. By enforcing Big-Endian on the wire, any client and server running on completely different hardware architectures can communicate without misinterpreting integer values (like `Length` or `TopicLength`).

The same value, two byte orders:

```text
Value 10 as uint32_t

Big-Endian (on the wire):      00 00 00 0a    <- most significant byte first
Little-Endian (x86 memory):    0a 00 00 00    <- least significant byte first
```

A reader that assumed the wrong order would see `0a 00 00 00` as 167,772,160 instead of 10.

### 2. Why a Maximum Frame Size Limit of 1 MiB?

- **Denial-of-Service (DoS) prevention:** If there is no upper limit on the `Length` field, a malicious or buggy client could send a packet specifying a length of about 4 GiB (the `uint32_t` maximum). A naive server might try to allocate a massive buffer in RAM (`std::vector` or string) to read this payload, instantly exhausting server memory and crashing the broker.
- **Resource predictability:** Restricting individual messages to 1 MiB (1,048,576 bytes = 2^20) ensures that memory allocations remain bounded, predictable, and safe for high-throughput streaming environments.

```mermaid
flowchart LR
    subgraph nolimit ["❌ No upper limit"]
        direction TB
        N1["Header says<br/>Length = ~4 GiB"] --> N2["Server allocates<br/>a ~4 GiB buffer"]
        N2 --> N3["Memory exhausted<br/>→ broker crashes"]
    end
    subgraph limit ["✅ 1 MiB limit"]
        direction TB
        L1["Header says<br/>Length = ~4 GiB"] --> L2{"Length ><br/>1 MiB?"}
        L2 -- "Yes" --> L3["Connection closed,<br/>nothing allocated"]
    end
    nolimit ~~~ limit
```

### 3. Hex Dump Exercise Example (`PRODUCE` command)

For a message producing to topic `"orders"` with payload `"a"`:

| Field | Offset | Size | Type / Value | Hex Stream (Big-Endian) | Calculation / Notes |
| :--- | :---: | :---: | :--- | :--- | :--- |
| **`Length`** | 0 | 4 Bytes | `uint32_t` (= 10) | `00 00 00 0a` | Total bytes following the Length field (1 + 2 + 6 + 1 = 10) |
| **`Command`** | 4 | 1 Byte | `PRODUCE` (`1`) | `01` | Fixed command code for `PRODUCE` |
| **`TopicLength`** | 5 | 2 Bytes | `uint16_t` (= 6) | `00 06` | Length of the topic string `"orders"` (6 characters) |
| **`Topic`** | 7 | 6 Bytes | `"orders"` | `6f 72 64 65 72 73` | ASCII hex encoding for each character of `"orders"` |
| **`Payload`** | 13 | 1 Byte | `"a"` | `61` | ASCII hex encoding for character `"a"` |

**Final wire hex stream** (14 bytes in total: 4 for `Length` + 10 that follow it):

`00 00 00 0a 01 00 06 6f 72 64 65 72 73 61`

The same frame as a layout (byte offsets on the first line):

```text
 0             4          5               7                   13
┌─────────────┬──────────┬───────────────┬───────────────────┬──────────────┐
│ Length (4B) │ Cmd (1B) │ TopicLen (2B) │    Topic (6B)     │ Payload (1B) │
├─────────────┼──────────┼───────────────┼───────────────────┼──────────────┤
│ 00 00 00 0a │    01    │     00 06     │ 6f 72 64 65 72 73 │      61      │
└─────────────┴──────────┴───────────────┴───────────────────┴──────────────┘
                                          o  r  d  e  r  s       a
```

> 🧠 `Length` counts everything **after** itself, which is why it is `10` and not `14`.

---

## 📥 Day 21 - `recv_exact` vs `recv_some` (Network I/O Stream Guarantees)

### 1. Functional Divergence

| | `recv_some` (Day 08) | `recv_exact` (Day 21) |
| :--- | :--- | :--- |
| **Behavior** | Thin wrapper around the POSIX `recv` system call. Returns whatever chunk is currently available in the TCP socket receive buffer (1 byte up to the requested capacity). | Enforces strict stream framing by looping over `recv` calls until exactly `n` bytes are accumulated in the destination buffer. |
| **Result** | Number of bytes read (`> 0`), EOF (`== 0`), or error (`< 0`). | Success once `n` bytes are read. Premature EOF (`received == 0`) is a failure state (`false`). |
| **Bookkeeping** | None needed. | Pointer arithmetic (`ptr += received`), remaining length (`n_left -= received`), and signal interruption (`EINTR`) handling. |
| **Use case** | General stream reading where message boundaries are not predetermined or partial chunk processing is acceptable. | Protocol frame decoding, e.g. reading a fixed-size 4-byte header to learn the payload length, then an exact read of the payload bytes. |

### 2. How `recv_exact` Loops

```mermaid
flowchart TD
    S["n_left = n"] --> Q{"n_left > 0?"}
    Q -- "No" --> OK["Return success<br/>(exactly n bytes read)"]
    Q -- "Yes" --> R["received = recv(ptr, n_left)"]
    R --> C{received}
    C -- "> 0" --> U["ptr += received<br/>n_left -= received"]
    U --> Q
    C -- "== 0" --> F1["Premature EOF<br/>→ return false"]
    C -- "< 0" --> E{"errno == EINTR?"}
    E -- "Yes" --> R
    E -- "No" --> F2["Error"]
```

### 3. Example: A Header That Arrives in Pieces (illustrative)

Say the 4-byte `Length` header from the Day 14 example (`00 00 00 0a`) has only partly arrived when the read happens:

| Call | What happens | Outcome |
| :--- | :--- | :--- |
| `recv_some(buf, 4)` | One `recv()` returns what is buffered right now. | Returns `2` (`00 00`). The caller now holds a **partial** header. |
| `recv_exact(buf, 4)` | `recv()` returns 2 bytes, loops, `recv()` returns the other 2. | Success with all 4 bytes: `00 00 00 0a`. |

And how a two-step frame read fits together:

```mermaid
sequenceDiagram
    autonumber
    participant Caller as Caller (frame decoding)
    participant RE as recv_exact
    participant Sock as Socket

    Caller->>RE: recv_exact(4), the header
    RE->>Sock: recv() repeatedly until 4 bytes
    RE-->>Caller: Length
    Caller->>RE: recv_exact(Length), the body
    RE->>Sock: recv() repeatedly until Length bytes
    RE-->>Caller: complete body bytes
```

### 4. Architectural Rationale

TCP delivers a byte stream, so the bytes of one frame can arrive split across several reads. Using `recv_some` directly for structured framing therefore leads to partial reads and protocol desynchronization. `recv_exact` guarantees that a read either returns exactly the requested number of bytes or fails, ensuring that upper-layer protocol parsers (`FrameDecoder`) only receive complete frames.

---

## 🧠 Day 23 — Buffer Management

### 1. Buffer Allocation Follows the Frame, Not the Socket

TCP gives no message boundaries (Day 21 §4), so the broker cannot "read a message" in one call. Instead, buffer allocation is driven by the **frame structure** defined in Day 14:

1. **Read the 4-byte header** into a fixed-size buffer. This size never depends on input — it is always 4 bytes.
2. **Validate `Length`** against `kMaxFrameSize` (1 MiB, Day 14 §2) *before* allocating anything. This is a comparison only — zero bytes allocated.
3. **Allocate exactly `Length` bytes** for the body, then `recv_exact(Length)` into it (Day 21 §2).

| Step | Buffer size | Constraint |
| :--- | :--- | :--- |
| Read header | 4 bytes, fixed | independent of input |
| Validate `Length` | 0 bytes (compare only) | `Length <= kMaxFrameSize` (1 MiB) |
| Read body | exactly `Length` bytes | only reached after validation passes |

The key property: **no allocation happens before validation**. A header claiming ~4 GiB is rejected at step 2 with nothing allocated — the same DoS mitigation described in Day 14 §2, now expressed as an allocation-ordering rule rather than just a size check.

### 2. Why the Fixed Header Buffer Matters

The header read is the only place where the broker touches the socket without knowing the frame size. Keeping it at a fixed 4 bytes means:

- A malicious peer cannot influence the first allocation at all — it is a compile-time constant.
- The `Length` value is only trusted *after* it has been read in full by `recv_exact(4)` (Day 21 §3), so a partially-arrived header can never be misinterpreted as a smaller/larger frame.

This is the buffer-level counterpart of the `recv_exact` guarantee: `recv_exact` ensures the 4 header bytes are complete before parsing; the fixed buffer ensures parsing never reads past them.

### 3. Cross-References

- `recv_some` vs `recv_exact` — see Day 21 §1.
- `kMaxFrameSize` rationale — see Day 14 §2.
- TCP stream model / partial reads — see Day 21 §4.

### 4. Reflection

The through-line of Phase 2 is that **every buffer the broker allocates is bounded by a value it has already validated**. The header buffer is bounded by the protocol (4 bytes). The body buffer is bounded by `Length`, which is bounded by `kMaxFrameSize`. Nothing the peer sends can cause an allocation larger than 1 MiB, and nothing is allocated before the bound is checked.

---

## 🧵 Day 29 — Threading Model & Concurrency Design (Blocking ThreadPool)

### 1. Task-Queue ThreadPool Architecture

Concurrency is handled by a **Task-Queue ThreadPool**: a fixed number of worker threads, each running an infinite loop that pulls tasks from a thread-safe queue (`ThreadSafeQueue`).

- **Task submission:** when a client connects (`TcpServer`), a task that handles the connection lifecycle (`handle_echo`) is `submit`-ted to the `ThreadPool`.
- **Blocking execution:** the worker that picks up the task owns the **entire** connection lifetime — read a frame, process it, echo it back — in a loop until the client disconnects. The frame reading inside that loop uses the `recv_exact` approach from Day 21 and the validation-before-allocation rule from Day 23.

```mermaid
flowchart LR
    C["Client connects"] --> S["TcpServer<br/>submit(handle_echo)"]
    S --> Q[("ThreadSafeQueue")]
    Q --> W1["Worker 1<br/>serves connection A"]
    Q --> W2["Worker 2<br/>serves connection B"]
    Q --> Wn["Worker N<br/>serves connection ..."]
    W1 -. "client disconnects<br/>→ worker free → takes next task" .-> Q
```

| Property | How it works | Architectural consequence |
| :--- | :--- | :--- |
| **Worker assignment** | 1 worker serves exactly 1 connection for its whole lifetime | A worker is occupied (blocked) for as long as the client keeps the connection open |
| **Concurrency limit** | Bounded by the number of workers in the pool | Max simultaneously *served* clients = number of workers |
| **Queue behavior** | Tasks beyond the worker count wait in `ThreadSafeQueue` | A waiting task is not served until some worker finishes its connection |

A side benefit worth noting: since only N connections are served at once, the memory held by frame bodies is also bounded. With one body buffer per active connection and the 1 MiB cap from Day 14 §2, that is at most N × 1 MiB at any moment. This is a consequence of the design, not something that was measured.

### 2. Practical Test: Worker Exhaustion

To confirm the limit of the blocking model, the pool was run with a deliberately small number of workers:

1. **Fix the worker count** to a small value (`4` workers).
2. **Open persistent connections:** a client script opens **5 connections** at the same time and keeps them open (no immediate disconnect).
3. **Observed behavior:**

| Connection | Result |
| :--- | :--- |
| **1st – 4th** | Picked up immediately by the 4 workers, logged as connected and served normally |
| **5th** | Not served and not logged: every worker is occupied, so no thread is available to run its handler |
| **After one of the first 4 disconnects** | That worker returns to the pool, takes the 5th connection's task from the queue, and serves it |

**Where the 5th connection waits:** in `ThreadSafeQueue`, not in the OS listen backlog. `TcpServer::run()` loops `accept_one()` → `pool_.submit(...)`, and the accept loop runs on its own thread, independent of the workers. So the 5th connection is already `accept()`-ed and its `FileDescriptor` is held inside the queued task, but no worker has run `handle_echo` for it yet. That is why there is no "Client connected" log. From the client's side the connection looks established, and any bytes it sends just sit unread in the kernel socket receive buffer until a worker picks the task up.

### 3. Architectural Takeaway

- **Core weakness:** each connection pins one thread, and that thread spends most of its time blocked in `recv` waiting for data. Threads are expensive, so this model does not scale to a large number of mostly-idle clients (the classic C10K problem).
- **Why the limit is by design:** the pool size is a hard ceiling on concurrent clients, which is predictable, but one idle client can occupy a worker that other clients are waiting for.
- **Direction:** this motivates moving toward event-driven, non-blocking I/O multiplexing (e.g. `epoll`), where one thread watches many sockets and only works on those that are ready. This is a planned direction, not something implemented in this phase.

### 4. Cross-References

- **Day 14 §2** — `kMaxFrameSize` (1 MiB), which bounds per-connection memory.
- **Day 21 §1–§2** — `recv_exact`, used by the per-connection loop to read frames.
- **Day 23 §1** — validation-before-allocation, applied on every frame a worker reads.