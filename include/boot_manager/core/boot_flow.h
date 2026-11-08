#pragma once

#include "boot_manager/core/boot_backend.h"

OperationResult prepare_next_boot(const BootBackend &backend,
                                  const BootEntry &entry);
