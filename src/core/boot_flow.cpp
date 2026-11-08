#include "boot_manager/core/boot_flow.h"

#include <QString>

OperationResult prepare_next_boot(const BootBackend &backend,
                                  const BootEntry &entry)
{
    const auto set_result = backend.set_next_boot(entry);
    if (!set_result.ok)
    {
        return set_result;
    }

    const auto verify_result = backend.verify_next_boot(entry);
    if (!verify_result.ok)
    {
        return {false,
                QStringLiteral("下一次開機項目讀回不一致：%1")
                    .arg(verify_result.message)};
    }
    return {true, QStringLiteral("下一次開機項目已設定並驗證")};
}
