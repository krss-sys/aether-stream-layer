#pragma once
#include <memory>

#include "file_descriptor.hpp"

namespace aether {
void handle_echo(std::shared_ptr<FileDescriptor> client_fd);
}