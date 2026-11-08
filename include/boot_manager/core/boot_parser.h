#pragma once

#include "boot_manager/core/boot_backend.h"

QVector<BootEntry> parse_windows_entries(const QString &output);
QVector<BootEntry> parse_linux_entries(const QString &output);
bool verify_windows_bootsequence(const QString &output, const QString &id);
bool verify_linux_bootnext(const QString &output, const QString &id);
