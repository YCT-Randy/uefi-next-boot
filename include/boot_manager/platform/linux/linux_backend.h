#pragma once
#include "boot_manager/core/boot_backend.h"
class LinuxBackend final : public BootBackend {
public:
 QVector<BootEntry> list_entries(QString &error) const override;
 OperationResult set_next_boot(const BootEntry &entry) const override;
 OperationResult verify_next_boot(const BootEntry &entry) const override;
 OperationResult reboot() const override;
 QString current_system() const override;
};
