#include "boot_manager/core/boot_flow.h"
#include "boot_manager/core/boot_parser.h"

#define CHECK(condition)     \
    do                       \
    {                        \
        if (!(condition))    \
        {                    \
            return __LINE__; \
        }                    \
    } while (false)

class FakeBackend final : public BootBackend
{
public:
    bool set_ok = true;
    bool verify_ok = true;
    mutable int set_calls = 0;
    mutable int verify_calls = 0;
    mutable int reboot_calls = 0;

    QVector<BootEntry> list_entries(QString &) const override
    {
        return {};
    }

    OperationResult set_next_boot(const BootEntry &) const override
    {
        ++set_calls;
        return {set_ok, "set"};
    }

    OperationResult verify_next_boot(const BootEntry &) const override
    {
        ++verify_calls;
        return {verify_ok, "mismatch"};
    }

    OperationResult reboot() const override
    {
        ++reboot_calls;
        return {true, "reboot"};
    }

    QString current_system() const override
    {
        return "mock";
    }
};

int main()
{
    const auto linux = parse_linux_entries(
        "Boot0007*Windows Boot Manager HD(1,GPT,abc)/File(\\EFI\\Microsoft\\Boot\\bootmgfw.efi)\n"
        "Boot0008 ubuntu HD(2,GPT,def) File(\\EFI\\ubuntu\\shimx64.efi)\n"
        "Boot0009 device-only HD(3,GPT,ghi)\n");
    CHECK(linux.size() == 2);
    CHECK(linux[1].id == "0008");
    CHECK(linux[1].efi_path.contains("EFI"));
    CHECK(verify_linux_bootnext("BootNext: 0008\n", "0008"));
    CHECK(verify_windows_bootsequence(
        "Firmware Boot Manager\n---------------------\n"
        "bootsequence            {12345678-1234-1234-1234-123456789abc}\n",
        "{12345678-1234-1234-1234-123456789abc}"));

    const auto windows = parse_windows_entries(
        "ubuntu\nidentifier              "
        "{12345678-1234-1234-1234-123456789abc}\n"
        "path                    \\EFI\\ubuntu\\shimx64.efi\n");
    CHECK(windows.size() == 1);
    CHECK(windows[0].name.contains("Ubuntu"));

    const auto generic_windows = parse_windows_entries(
        "新增應用程式 (101ffff)\n"
        "identifier              {12345678-1234-1234-1234-123456789abc}\n"
        "description             新增應用程式 (101ffff)\n"
        "path                    \\EFI\\UBUNTU\\SHIMX64.EFI\n");
    CHECK(generic_windows.size() == 1);
    CHECK(generic_windows[0].name.contains("Ubuntu"));
    CHECK(generic_windows[0].name.contains("版本未知"));

    const auto versioned_windows = parse_windows_entries(
        "新增應用程式 (101ffff)\n"
        "identifier              {12345678-1234-1234-1234-123456789abc}\n"
        "description             Ubuntu 20.04\n"
        "path                    \\EFI\\UBUNTU\\SHIMX64.EFI\n");
    CHECK(versioned_windows.size() == 1);
    CHECK(versioned_windows[0].name == "Ubuntu 20.04");

    FakeBackend backend;
    BootEntry entry{"0008", "ubuntu", "\\EFI\\ubuntu\\shimx64.efi", {}};
    auto result = prepare_next_boot(backend, entry);
    CHECK(result.ok && backend.set_calls == 1 && backend.verify_calls == 1 &&
          backend.reboot_calls == 0);

    backend.verify_ok = false;
    result = prepare_next_boot(backend, entry);
    CHECK(!result.ok && backend.reboot_calls == 0);

    backend.set_ok = false;
    result = prepare_next_boot(backend, entry);
    CHECK(!result.ok && backend.reboot_calls == 0);
}
