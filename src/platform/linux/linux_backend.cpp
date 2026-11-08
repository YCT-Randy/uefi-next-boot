#include "boot_manager/platform/linux/linux_backend.h"
#include "boot_manager/core/boot_parser.h"
#include "boot_manager/utils/command_runner.h"

#include <algorithm>

QVector<BootEntry> LinuxBackend::list_entries(QString &error) const
{
    const auto result = run_command("efibootmgr", {"-v"});
    error = result.error;
    return result.ok ? parse_linux_entries(result.output) : QVector<BootEntry>{};
}

OperationResult LinuxBackend::set_next_boot(const BootEntry &entry) const
{
    QString error;
    const auto entries = list_entries(error);
    const bool exists = std::any_of(
        entries.cbegin(), entries.cend(), [&entry](const BootEntry &candidate)
        { return candidate.id.compare(entry.id, Qt::CaseInsensitive) == 0 &&
                 !candidate.efi_path.isEmpty(); });
    if (!error.isEmpty() || !exists)
    {
        return {false, "所選項目已不存在或 EFI 路徑無效"};
    }

    const auto result = run_command(
        "pkexec", {"/usr/lib/boot-manager/boot-manager-helper", "set-next",
                   entry.id});
    return {result.ok, result.ok ? "已設定下一次開機" : result.error};
}

OperationResult LinuxBackend::verify_next_boot(const BootEntry &entry) const
{
    const auto result = run_command("efibootmgr", {});
    if (!result.ok)
    {
        return {false, result.error};
    }
    if (!verify_linux_bootnext(result.output, entry.id))
    {
        return {false, "讀回的 BootNext 與所選項目不一致"};
    }
    return {true, "讀回 BootNext 驗證完成"};
}

OperationResult LinuxBackend::reboot() const
{
    const auto result = run_command(
        "pkexec", {"/usr/lib/boot-manager/boot-manager-helper", "reboot"});
    return {result.ok, result.ok ? "正在重新啟動" : result.error};
}

QString LinuxBackend::current_system() const
{
    return QStringLiteral("Ubuntu");
}
