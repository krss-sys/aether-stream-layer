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