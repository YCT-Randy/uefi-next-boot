#pragma once

#include <QString>
#include <QVector>
#include <memory>

struct BootEntry {
    QString id;
    QString name;
    QString efi_path;
    QString device;
};

struct OperationResult {
    bool ok = false;
    QString message;
};

class BootBackend {
public:
    virtual ~BootBackend() = default;
    virtual QVector<BootEntry> list_entries(QString &error) const = 0;
    virtual OperationResult set_next_boot(const BootEntry &entry) const = 0;
    virtual OperationResult verify_next_boot(const BootEntry &entry) const = 0;
    virtual OperationResult reboot() const = 0;
    virtual QString current_system() const = 0;
};

std::unique_ptr<BootBackend> create_boot_backend();
