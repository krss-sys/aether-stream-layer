#include "session.hpp"

#include <cerrno>
#include <cstring>

#include "config.hpp"
#include "connection_guard.hpp"
#include "frame.hpp"
#include "logger.hpp"
#include "net_io.hpp"
#include "stop_flag.hpp"

namespace aether {
void handle_echo(std::shared_ptr<FileDescriptor> client_fd) {
    ConnectionGuard guard;
    FrameDecoder decoder;
    uint8_t buffer[aether::config::kRecvBufferSize];

    while (true) {
        ssize_t bytes_received = recv_some(client_fd->get(), buffer, sizeof(buffer));

        if (bytes_received > 0) {
            decoder.feed(buffer, static_cast<size_t>(bytes_received));
            while (auto frame = decoder.next()) {
                // Tạo preview: tối đa 64 byte, thay ký tự điều khiển bằng '?'
                std::string preview;
                size_t preview_len = std::min<size_t>(frame->size(), 64);
                for (size_t i = 0; i < preview_len; ++i) {
                    char c = static_cast<char>((*frame)[i]);
                    preview += (c >= 32 && c < 127) ? c : '?';
                }

                Logger::log(Logger::Level::Info,
                            "Received frame, size = " + std::to_string(frame->size()) +
                                ", preview = " + preview);

                auto wire_bytes = encode_frame(*frame);
                bool ok = send_all(client_fd->get(), wire_bytes.data(), wire_bytes.size());
                if (!ok) {
                    Logger::log(Logger::Level::Error, "Send error, closing connection");
                    return;
                }
            }
            if (decoder.has_error()) {
                Logger::log(Logger::Level::Warn, "Frame exceeds size limit. Closing connection");
                return;
            }
        } else if (bytes_received == 0) {
            break;
        } else {
            if (!aether::g_stop.load()) {
                Logger::log(Logger::Level::Error,
                            "Recv error: " + std::string(std::strerror(errno)));
            }
            break;
        }
    }
}
}  // namespace aether