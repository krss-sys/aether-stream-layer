## Day 07 - `recv()` Return Values & Semantics

The `recv(fd, buf, count, flags)` system call returns byte count `n`:
* `n > 0`: Successfully received `n` bytes of data from client.
* `n == 0`: Peer gracefully closed connection (EOF / TCP FIN packet). Server must close socket.
* `n < 0`: System error occurred during read (check `errno` or `strerror(errno)` for details).