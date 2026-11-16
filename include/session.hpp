#pragma once
#include <memory>

#include "file_descriptor.hpp"

namespace aether {
void handle_echo(const std::shared_ptr<FileDescriptor>& client_fd);
}