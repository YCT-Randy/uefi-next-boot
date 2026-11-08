#include "boot_manager/utils/command_runner.h"

#include <QProcess>

CommandResult run_command(const QString &program, const QStringList &arguments,
                          const int timeout_ms)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.start();

    if (!process.waitForStarted(timeout_ms))
    {
        return {false, {}, QStringLiteral("無法啟動指令：%1").arg(process.errorString())};
    }
    if (!process.waitForFinished(timeout_ms))
    {
        process.kill();
        process.waitForFinished();
        return {false, {}, QStringLiteral("指令逾時：%1").arg(program)};
    }

    const QString output =
        QString::fromLocal8Bit(process.readAllStandardOutput());
    const QString error =
        QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
    if (process.exitStatus() != QProcess::NormalExit ||
        process.exitCode() != 0)
    {
        const QString message =
            error.isEmpty() ? QStringLiteral("指令失敗：%1").arg(program)
                            : error;
        return {false, output, message};
    }
    return {true, output, {}};
}
