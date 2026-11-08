#include "boot_manager/core/boot_parser.h"

#include <QRegularExpression>

namespace {

QString windows_entry_name(const QString &block, const QString &efi_path)
{
    const QRegularExpression description_pattern(
        R"((?:^|\r?\n)\s*description\s+([^\r\n]+))",
        QRegularExpression::CaseInsensitiveOption);
    const auto description_match = description_pattern.match(block);
    const QString description = description_match.hasMatch()
                                    ? description_match.captured(1).trimmed()
                                    : QString{};

    // NOTE: 韌體項目只記錄啟動程式，EFI 路徑無法判定作業系統版本。
    if (efi_path.contains(QStringLiteral("\\EFI\\UBUNTU\\"),
                          Qt::CaseInsensitive))
    {
        if (description.contains(QStringLiteral("ubuntu"), Qt::CaseInsensitive))
        {
            return description;
        }
        return QStringLiteral("Ubuntu 啟動程式（版本未知）");
    }
    if (efi_path.contains(QStringLiteral("\\EFI\\MICROSOFT\\BOOT\\BOOTMGFW.EFI"),
                          Qt::CaseInsensitive))
    {
        if (description.contains(QStringLiteral("Windows"), Qt::CaseInsensitive))
        {
            return description;
        }
        return QStringLiteral("Windows Boot Manager");
    }

    if (!description.isEmpty())
    {
        return description;
    }

    const QStringList lines = block.split(
        QRegularExpression(R"(\r?\n)"), Qt::SkipEmptyParts);
    return lines.isEmpty() ? QStringLiteral("未命名的 UEFI 項目")
                           : lines.first().trimmed();
}

} // namespace

QVector<BootEntry> parse_windows_entries(const QString &output)
{
    QVector<BootEntry> entries;
    const QRegularExpression id_pattern(
        R"(\{(?:[0-9a-fA-F-]{36}|bootmgr)\})",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression path_pattern(
        R"((\\EFI\\[^\r\n\s]+))",
        QRegularExpression::CaseInsensitiveOption);
    const QStringList blocks = output.split(
        QRegularExpression(R"(\r?\n\s*\r?\n)"), Qt::SkipEmptyParts);
    for (const QString &block : blocks)
    {
        const auto id_match = id_pattern.match(block);
        const auto path_match = path_pattern.match(block);
        if (!id_match.hasMatch() || !path_match.hasMatch())
        {
            continue;
        }
        const QString id = id_match.captured().toLower();
        if (id == QStringLiteral("{fwbootmgr}"))
        {
            continue;
        }
        const QString efi_path = path_match.captured(1);
        const QString name = windows_entry_name(block, efi_path);
        entries.append({id, name, efi_path, {}});
    }
    return entries;
}

QVector<BootEntry> parse_linux_entries(const QString &output)
{
    QVector<BootEntry> entries;
    const QRegularExpression line_pattern(
        R"(^Boot([0-9a-fA-F]{4})(\*?)\s*(.+?)\s+((?:HD|File)\(.*)$)",
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression file_pattern(
        R"(File\(([^)]+)\))", QRegularExpression::CaseInsensitiveOption);
    const QStringList lines = output.split(
        QRegularExpression(R"(\r?\n)"), Qt::SkipEmptyParts);
    for (const QString &line : lines)
    {
        const auto match = line_pattern.match(line);
        if (!match.hasMatch())
        {
            continue;
        }
        const auto file_match = file_pattern.match(match.captured(4));
        if (!file_match.hasMatch())
        {
            continue;
        }
        entries.append({match.captured(1).toUpper(), match.captured(3).trimmed(),
                        file_match.captured(1), match.captured(4)});
    }
    return entries;
}

bool verify_windows_bootsequence(const QString &output, const QString &id)
{
    const QRegularExpression sequence_pattern(
        R"((?:^|\r?\n)\s*bootsequence\s+([^\r\n]+))",
        QRegularExpression::CaseInsensitiveOption);
    const auto match = sequence_pattern.match(output);
    return match.hasMatch() && match.captured(1).contains(id, Qt::CaseInsensitive);
}

bool verify_linux_bootnext(const QString &output, const QString &id)
{
    const QRegularExpression pattern(R"((?:^|\r?\n)BootNext:\s*([0-9a-fA-F]{4})\b)");
    const auto match = pattern.match(output);
    return match.hasMatch() && match.captured(1).compare(id, Qt::CaseInsensitive) == 0;
}
