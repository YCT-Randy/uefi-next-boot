#include "boot_manager/core/boot_backend.h"
#include "boot_manager/platform/linux/linux_backend.h"
#include "boot_manager/platform/windows/windows_backend.h"

#include <QtGlobal>

std::unique_ptr<BootBackend> create_boot_backend()
{
#if defined(Q_OS_WIN)
    return std::make_unique<WindowsBackend>();
#elif defined(Q_OS_LINUX)
    return std::make_unique<LinuxBackend>();
#else
    return nullptr;
#endif
}
