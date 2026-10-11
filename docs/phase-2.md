# Phase 2: Message Broker & Protocol Design

---

## Day 14 - Protocol Design Rationale & Framing

### 1. Why Big-Endian (Network Byte Order)?
- **Standardization:** Network protocols (like TCP/IP, IP headers, DNS) universally adopt Big-Endian as the standard Network Byte Order. 
- **Consistency across platforms:** Different CPU architectures (e.g., x86/x64 are Little-Endian, while some ARM or network processors can be Big-Endian) store multi-byte integers in memory differently. By enforcing Big-Endian on the wire, any client and server running on completely different hardware architectures can communicate without misinterpreting integer values (like `Length` or `TopicLength`).

### 2. Why a Maximum Frame Size Limit of 1 MiB?
- **Denial-of-Service (DoS) Prevention:** If there is no upper limit on the `Length` field, a malicious or buggy client could send a packet specifying a length of 4 GB. A naive server might try to allocate a massive buffer in RAM (`std::vector` or string) to read this payload, instantly exhausting server memory and crashing the broker.
- **Resource Predictability:** Restricting individual messages to 1 MiB ($1,048,576$ bytes) ensures that memory allocations remain bounded, predictable, and safe for high-throughput streaming environments.

### 3. Hex Dump Exercise Example (`PRODUCE` command)
For a message producing to topic `"orders"` with payload `"a"`:

| Field | Size | Type / Value | Hex Stream (Big-Endian) | Calculation / Notes |
| :--- | :---: | :--- | :--- | :--- |
| **`Length`** | 4 Bytes | `uint32_t` (= 10) | `00 00 00 0a` | Total bytes following Length field ($1 + 2 + 6 + 1 = 10$) |
| **`Command`** | 1 Byte | `PRODUCE` (`1`) | `01` | Fixed command code for `PRODUCE` |
| **`TopicLength`** | 2 Bytes | `uint16_t` (= 6) | `00 06` | Length of the topic string `"orders"` (6 characters) |
| **`Topic`** | 6 Bytes | `"orders"` | `6f 72 64 65 72 73` | ASCII hex encoding for each character of `"orders"` |
| **`Payload`** | 1 Byte | `"a"` | `61` | ASCII hex encoding for character `"a"` |

**Final Wire Hex Stream:**
`00 00 00 0a 01 00 06 6f 72 64 65 72 73 61`