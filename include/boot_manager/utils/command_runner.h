#pragma once

#include <QString>
#include <QStringList>

struct CommandResult {
    bool ok = false;
    QString output;
    QString error;
};

CommandResult run_command(const QString &program, const QStringList &arguments, int timeout_ms = 10000);
