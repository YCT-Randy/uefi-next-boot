#include "boot_manager/platform/windows/windows_backend.h"
#include "boot_manager/core/boot_parser.h"
#include "boot_manager/utils/command_runner.h"

#include <algorithm>

QVector<BootEntry> WindowsBackend::list_entries(QString &error) const
{
    const auto result = run_command("bcdedit.exe", {"/enum", "firmware"});
    error = result.error;
    return result.ok ? parse_windows_entries(result.output)
                     : QVector<BootEntry>{};
}

OperationResult WindowsBackend::set_next_boot(const BootEntry &entry) const
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
        "bcdedit.exe", {"/set", "{fwbootmgr}", "bootsequence", entry.id});
    return {result.ok, result.ok ? "已設定下一次開機" : result.error};
}

OperationResult WindowsBackend::verify_next_boot(const BootEntry &entry) const
{
    const auto result = run_command("bcdedit.exe", {"/enum", "{fwbootmgr}"});
    if (!result.ok)
    {
        return {false, result.error};
    }
    if (!verify_windows_bootsequence(result.output, entry.id))
    {
        return {false, "讀回的 BootSequence 與所選項目不一致"};
    }
    return {true, "讀回 BootSequence 驗證完成"};
}

OperationResult WindowsBackend::reboot() const
{
    const auto result = run_command("shutdown.exe", {"/r", "/t", "0"});
    return {result.ok, result.ok ? "正在重新啟動" : result.error};
}

QString WindowsBackend::current_system() const
{
    return QStringLiteral("Windows");
}
