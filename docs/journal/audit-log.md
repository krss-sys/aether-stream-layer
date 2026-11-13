# Audit Log

## Day 1 — 2026-11-04
- Read map 1 (big picture) and map 2 (journey).
- Understood: 1 main thread accepts, 8 workers handle connections.
- No code changes.

## Day 2 — 2026-11-05
- Read map 3 (file links). Created docs/inventory.md.
- Verified 5 critical links with grep: g_stop (#2), handle_echo (#3), kMaxFrameSize (#5), setup_signals (#1), pop_for (#8).
- No code changes.

## Day 3 — 2026-11-06
- #9: removed Zone.Identifier, updated .gitignore.
- #10: added historical note to phase-1.md.
- #15: removed broken demo.gif link.
- #19: renamed project to aether-stream-layer.
- #17: removed asm-generic include.
- #7: fixed docs phase-2/3 (recv_some, cv_, q_, [&]).
- #6: aligned PROTOCOL.md with v0 implementation.
- #18: moved handle_signal to anonymous namespace.
- #16: added comment on manual join.
- #14: fixed include in test_queue.cpp.
- Tests: pass. Build: clean.

## Day 4 — 2026-11-07
- #5: unified kMaxFrameSize (removed from client.hpp).
- #11: created config.hpp, replaced hardcoded values (port, workers, buffer, timeouts).
- Tests: pass. Build: clean.

## Day 5 — 2026-11-08
- #2: extracted g_stop to stop_flag.hpp (decouple net_io from tcp_server).
- #29: changed volatile sig_atomic_t to std::atomic<bool>.
- #1: called setup_signals() in main(). Ctrl+C now exits 0 (was 130).
- Tests: pass. Build: clean.
- Manual test: kill -INT server → exit 0, clean log.

## Day 6 — 2026-11-09
- #21: caught exceptions in worker loop. Server no longer aborts on bad task.
- #22: log frame size (and safe preview) only, not full payload. Prevents log injection.
- Tests: pass. Build: clean.

## Day 7 — 2026-11-10
- #12: accept errors logged, loop continues (sleep 100ms on EMFILE).
- #28: poll errors logged, throws instead of silent break.
- #20: FileDescriptor wraps fd immediately after accept (RAII).
- #23: log peer ip:port on connect.
- Tests: pass. Build: clean.

## Day 8 — 2026-11-11
- #3: extracted handle_echo to session.hpp/.cpp. client.cpp now only AetherClient.
- Cleaned unused includes.
- Tests: pass. Build: clean.

## Day 9 — 2026-11-12
- #4: AetherClient uses FileDescriptor (RAII) instead of int m_sockfd.
- #25: connect() calls disconnect() first. No fd leak on reconnect.
- Added test_client_reconnect.cpp.
- Tests: pass. Build: clean.

## Day 10 — 2026-11-13
- #24: cli validates port (1-65535), no abort on bad input.
- #26: fragmentation test now sleeps 50ms between bytes, sets TCP_NODELAY, reads echo.
- #27, #6, #13, #8: recorded in DECISIONS.md.
- Tests: pass. Build: clean.
- Manual test: torture test PASS, cli rejects bad port.