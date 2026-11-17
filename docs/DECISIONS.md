# Design Decisions

## #27 Idle timeout
Status: deferred to phase 4.
Current: 8 workers, 1 connection per worker, blocking recv.
Long-term: epoll for C10K.
Reason: audit scope is cleanup, not redesign.

## #6 PROTOCOL.md v1 (Future work)
v0 (current code): Length (4 bytes BE) | Payload.
v1 (planned): Length | Command(1) | TopicLength(2 BE) | Topic | Payload.
Status: PROTOCOL.md now describes v0 (current code). The v1 layout is documented in PROTOCOL.md section "Future Work" only.
No code changes for v1 yet.

## #13 EAGAIN loop in recv_some/recv_exact
Intent: EAGAIN means SO_RCVTIMEO expired (1s). We check g_stop then retry.
No behavior change.

## #8 ThreadSafeQueue::pop_for
Status: keep.
Reason: reserved for future phase (timeout handling). Currently only used in tests.
If unused after 2 phases, consider removing.

## #20 RAII for client_fd in accept_one
Status: fixed.
Details: FileDescriptor now wraps client_fd immediately after accept, before setsockopt. If setsockopt throws, fd is closed automatically.

## #2 g_stop extracted to stop_flag.hpp
Status: fixed.
Details: g_stop is now in stop_flag.hpp (inline std::atomic<bool>). net_io.cpp no longer includes tcp_server.hpp.

## #3 handle_echo moved to session.cpp
Status: fixed.
Details: handle_echo is now in session.hpp/.cpp, separate from client. client.cpp only contains AetherClient.

## #4 AetherClient uses FileDescriptor
Status: fixed.
Details: AetherClient now uses FileDescriptor (RAII) instead of raw int. Move-only.

## #25 connect() calls disconnect() first
Status: fixed.
Details: AetherClient::connect() now calls disconnect() if already connected, preventing fd leak on reconnect.