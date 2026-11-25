# Aether Protocol Specification

This document defines the wire protocol contract between the `aether` client and server. TCP is a continuous byte stream with no inherent message boundaries, so a framing mechanism splits and reconstructs discrete messages.

> **At a glance**
> - Every message is one **frame**: `Length | Payload`
> - Multi-byte integers are **Big-Endian**
> - `Length` is capped at **1 MiB** to stop memory-exhaustion attacks; a violation closes the connection
> - Protocol **v1** is the structure **inside** `Payload`; the framing layer is unchanged

---

## 1. Frame Structure (v0, unchanged)

Every message on the TCP socket follows this binary layout:

| Field | Size | Type / Endianness | Description |
|:--|:--|:--|:--|
| `Length` | 4 bytes | `uint32_t` / Big-Endian | Byte length of `Payload`. Must not exceed 1 MiB (1,048,576 bytes). |
| `Payload` | variable | raw bytes | The message content. For v1, this is a structured request or response (see §3). |

`Length` counts only the payload, so a frame is at most 1 MiB + 4 bytes in total.

---

## 2. Byte Order & Endianness

- `Length` is transmitted in **Network Byte Order (Big-Endian)**.
- Convert explicitly with `htonl()` on transmission and `ntohl()` on reception.

| Field | Size | On transmission | On reception |
|:--|:-:|:-:|:-:|
| `Length` | 4 bytes | `htonl()` | `ntohl()` |

`Payload` is a raw byte sequence; its internal integers (see §3) are also Big-Endian, converted with `htobe16/32/64` / `be16/32/64toh`.

---

## 3. Protocol v1 (inside `Payload`)

### 3.1 Envelopes

**Request payload:**

```
Command(1) | TopicLength(2 BE) | Topic | Body
```

**Response payload:**

```
Status(1) | Body
```

Responses are returned **in the same order as requests** on a given connection (pipelining is allowed).

### 3.2 Commands

| Command | Code | Request body | Response body |
|:--|:-:|:--|:--|
| `PING` | 4 | empty (empty topic allowed) | empty |
| `PRODUCE` | 1 | message bytes | empty. `kOk` = "accepted into broker memory" |
| `FETCH` | 2 | `GroupLength(2 BE) \| Group \| MaxMessages(4 BE)` | `FirstOffset(8 BE) \| Count(4 BE) \| Count × (MsgLength(4 BE) \| bytes)` |
| `COMMIT` | 3 | `GroupLength(2 BE) \| Group \| Offset(8 BE)` | empty |

### 3.3 Status codes

| Status | Code | Meaning |
|:--|:-:|:--|
| `kOk` | 0 | success |
| `kBusy` | 1 | ingest queue full (retry later) |
| `kBadRequest` | 2 | malformed, unknown command, bad name, too large |

### 3.4 Semantics

- **(a)** Topic offsets start at `0` and increase by `1` per message.
- **(b)** `COMMIT(k)` means "messages `0..k-1` are processed; the next `FETCH` starts at `k`". Reading it as "processed up to `k`" is the classic off-by-one.
- **(c)** `FETCH` reads from the committed offset of `(topic, group)` and **does not commit**. Consumers are therefore **at-least-once**.
- **(d)** `FETCH` on a topic that does not exist returns `Count = 0`, not an error (a consumer may start before a producer).
- **(e)** `COMMIT` only moves forward; it never moves backward and never exceeds `next_offset`.
- **(f)** Topic and group names match `[A-Za-z0-9._-]{1,64}` (prevents log injection; `/` is reserved as a key separator).
- **(g)** `MaxMessages` is clamped to `kMaxFetchMessages = 1000`; a value of `0` is `kBadRequest`.
- **(h)** `kOk` on `PRODUCE` does **not** mean the message is immediately visible to `FETCH`. The ACK is returned before the ingest worker appends to the log, so a message may appear a few milliseconds later. Consumers must poll; tests and e2e scripts must wait with a bounded loop, not a fixed `sleep`.
- **(i)** `FETCH` and `COMMIT` never create a topic. Only `PRODUCE` creates a topic. This prevents an untrusted client from creating topics just by asking.

### 3.5 Size limits

`kMaxMessageSize != kMaxFrameSize`. A `FETCH` response adds 17 bytes (`Status 1 + FirstOffset 8 + Count 4 + MsgLength 4`), and a `PRODUCE` request adds `3 + len(topic)`. Both must fit inside the 1 MiB frame.

- `kMaxMessageSize = kMaxFrameSize - 128` (largest header today is `3 + 64 = 67` bytes; 128 leaves headroom).
- Broker limits: `kMaxTopics = 64` (hard limit, enforced at `PRODUCE` in Day 7), `kMaxGroups = 1024`.
- `kIngestCapacity` (ingest queue size) is a broker constant defined on Day 6, not a protocol constant.

`static_assert` in `config.hpp` enforces:

- `kMaxMessageSize + 17 <= kMaxFrameSize`
- `kMaxMessageSize + 3 + 64 <= kMaxFrameSize`

If a future header grows, `static_assert` fails at compile time.

### 3.6 Worked examples (hex)

**`PING`**

Request: `00 00 00 03 04 00 00`
- `00 00 00 03` — frame length = 3
- `04` — `Command = PING`
- `00 00` — `TopicLength = 0`

Response: `00 00 00 01 00`
- `00 00 00 01` — frame length = 1
- `00` — `Status = kOk`

**`PRODUCE` topic `t`, message `hi`**

Request: `00 00 00 06 01 00 01 74 68 69`
- `00 00 00 06` — frame length = 6
- `01` — `Command = PRODUCE`
- `00 01` — `TopicLength = 1`
- `74` — `t`
- `68 69` — `hi`

Response: `00 00 00 01 00`
- `00` — `Status = kOk`

---

## 4. Example: `"hello"` payload (v0 framing only)

| Field | Value | Bytes (hex) |
|:--|:--|:--|
| `Length` | `5` | `00 00 00 05` |
| `Payload` | `"hello"` | `68 65 6c 6c 6f` |

Full frame on the wire (9 bytes):

```
00 00 00 05 68 65 6c 6c 6f
```

> This example is v0 framing only. Under v1, `Payload` would be a structured request/response (see §3).

---

## 5. Safety Limits

- **Maximum `Length`:** 1 MiB (1,048,576 bytes), to prevent memory-exhaustion DoS.
- **On the wire:** `Length` does not include its own 4 bytes, so a frame is at most 1 MiB + 4 bytes.
- **On violation:** the server logs a warning and closes the connection (RAII `FileDescriptor`). Once `Length` cannot be trusted, there is no reliable way to find where the next frame begins.

---

## 6. Compatibility

- **v0 (raw echo):** removed. The server no longer echoes raw payloads.
- **v1 (current):** structured commands inside `Payload`, as described in §3.
