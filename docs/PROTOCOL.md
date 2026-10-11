# Aether Protocol Specification (Framing)

This document defines the wire protocol contract between the `aether` client and broker/server. Because TCP is a continuous **byte stream** with no inherent message boundaries, a custom framing mechanism is required to split and reconstruct discrete messages safely.

## 1. Frame Structure

Every message transmitted over the TCP socket follows this exact binary layout:

| Field | Size (Bytes) | Type / Endianness | Description |
| :--- | :--- | :--- | :--- |
| **`Length`** | 4 Bytes | `uint32_t` / **Big-Endian** | Total byte length of the remaining frame (from `Command` to the end of `Payload`). Max size is 1 MiB. |
| **`Command`** | 1 Byte | `uint8_t` / Enum | Command code (e.g., `1` for `PRODUCE`, `2` for `CONSUME`, etc.). |
| **`TopicLength`** | 2 Bytes | `uint16_t` / **Big-Endian** | Length of the subsequent `Topic` string. |
| **`Topic`** | Variable | UTF-8 String | The target topic name (e.g., `"orders"`). |
| **`Payload`** | Variable | Raw Bytes / UTF-8 | The actual message content or body. |

---

## 2. Byte Order & Endianness
- All multi-byte integer fields (`Length`, `TopicLength`) are transmitted in **Network Byte Order (Big-Endian)**. 
- Both client and broker must explicitly convert integers using `htons()` / `htonl()` on transmission and `ntohs()` / `ntohl()` on reception.

---

## 3. Safety Limits
- **Maximum Frame Size:** Restricted to **1 MiB** ($1,048,576$ bytes) to prevent Denial-of-Service (DoS) attacks via memory exhaustion from malformed or malicious huge length prefixes.