# Inventory

| File | Main class/function | Purpose | Called by | Tag |
|---|---|---|---|---|
| file_descriptor.hpp | FileDescriptor | RAII wrapper for POSIX fd | tcp_server.hpp, client.hpp, test_fd.cpp | [OK] |
| frame.hpp | encode_frame, FrameDecoder | Frame encode/decode | client.cpp, aether_frame_torture.cpp, test_frame.cpp | [OK] |
| logger.hpp | Logger | Thread-safe logging | main.cpp, client.cpp, connection_guard.hpp, aether_cli.cpp | [OK] |
| net_io.hpp | send_all, recv_some, recv_exact | Byte send/recv | client.cpp, test_net_io.cpp | [OK] |
| connection_guard.hpp | ConnectionGuard | RAII log connect/disconnect | client.cpp | [OK] |
| thread_safe_queue.hpp | ThreadSafeQueue | Mutex + condvar queue | thread_pool.hpp, test_queue.cpp | [OK] |
| thread_pool.hpp | ThreadPool | N worker threads | tcp_server.hpp, test_pool.cpp | [OK] |
| tcp_server.hpp | TcpServer, g_stop | Accept loop, signal handling | main.cpp, net_io.cpp (#2) | [MISPLACED] g_stop |
| client.hpp | AetherClient, handle_echo | Client + server handler (#3) | tcp_server.cpp, aether_cli.cpp | [MISPLACED] handle_echo |
| frame.cpp | encode_frame, FrameDecoder | Frame impl | (link vào aether_core) | [OK] |
| logger.cpp | Logger | Logger impl | (link) | [OK] |
| net_io.cpp | send_all, recv_some, recv_exact | Byte I/O impl | (link) | [MISPLACED] include tcp_server |
| tcp_server.cpp | TcpServer, g_stop | Server impl | (link) | [OK] |
| client.cpp | AetherClient, handle_echo | Client + handler impl | (link) | [MISPLACED] handle_echo |
| main.cpp | main | Entry point | (binary) | [OK] |
| test_fd.cpp | (tests) | Test FileDescriptor | file_descriptor.hpp | [OK] |
| test_frame.cpp | (tests) | Test frame encode/decode | frame.hpp | [OK] |
| test_net_io.cpp | (tests) | Test net_io | net_io.hpp | [OK] |
| test_pool.cpp | (tests) | Test ThreadPool | thread_pool.hpp | [OK] |
| test_queue.cpp | (tests) | Test ThreadSafeQueue | thread_safe_queue.hpp | [OK] |
| aether_cli.cpp | main | CLI client | client.hpp, logger.hpp | [OK] |
| aether_frame_torture.cpp | main | Frame torture test | frame.hpp | [OK] |