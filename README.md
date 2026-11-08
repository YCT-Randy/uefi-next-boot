# Boot Manager

[Traditional Chinese version](READMD-CN.md)

Boot Manager is a C++17 and Qt 6 desktop tool for selecting a one-time UEFI boot target. It lists EFI boot entries visible from the current operating system, sets the next boot target, reads the setting back for verification, and reboots only after another confirmation. It does not change the permanent BootOrder.

## Source layout

```text
include/boot_manager/       Headers grouped by core, gui, platform, and utils
src/main.cpp                Application entry point
src/core/                   Shared workflow and output parsers
src/gui/                    Qt window implementation
src/platform/windows/       bcdedit backend
src/platform/linux/         efibootmgr backend
src/utils/                  Command execution utilities
```

`BootBackend` defines the platform interface; the application selects the Windows or Linux backend at startup. On Windows, the application runs with administrator privileges through UAC. On Ubuntu, the GUI runs as a regular user, while setting the next boot target and rebooting are performed by a helper authorized through polkit.

## Prerequisites

### Windows

1. Install a stable Windows x64 release from the [CMake download page](https://cmake.org/download/) and add it to `PATH`.
2. Follow the [Qt installation instructions](https://doc.qt.io/qt-6/qt-online-installation.html) to install Qt 6 for desktop, a matching compiler kit, and the CMake/Ninja tools. Qt and the compiler must use the same toolchain, such as MinGW or MSVC 2022.
3. Open a new PowerShell window and check `cmake --version` and `Get-Command windeployqt.exe`. If Qt's `bin` directory is not in `PATH`, pass the Qt kit directory with `-QtDir` when packaging.

If several Qt versions are installed, specify the Qt 6 kit with `-QtDir` so that the packaging script uses the correct `windeployqt`.

### Ubuntu

On Ubuntu 22.04 or 24.04, install the distribution packages:

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools \
    libgl1-mesa-dev efibootmgr policykit-1 dpkg-dev
```

`qt6-base-dev` provides Qt Widgets and Concurrent. If package names differ for your Ubuntu release, check with `apt search`. See the [Qt Linux installation guide](https://doc.qt.io/qt-6/linux.html) and [package manager instructions](https://doc.qt.io/qt-6.10/linux-alternative-installers.html).

## Build and package

In Windows PowerShell, run:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\packaging\windows\package.ps1 -QtDir 'C:\Qt\<version>\mingw_64'
```

Replace `<version>` with your installed Qt 6 version. The executable is placed at `dist/bin/boot-manager.exe`, alongside the Qt files collected by `windeployqt`. `-QtDir` must point to a Qt 6 kit containing `bin/windeployqt.exe`. Run the command in an environment configured for the matching compiler. The `-ExecutionPolicy Bypass` option applies only to that PowerShell process; it does not change the machine's persistent execution policy.

On Ubuntu, run:

```bash
bash packaging/linux/package.sh
```

The `.deb` package is written to `dist/` and includes the desktop launcher, polkit policy, and privileged helper. Build separately on each platform; the source code is shared.

## Qt licensing information

The repository includes [Qt license terms and distribution notes](licenses/qt/README.md). The Windows packaging script copies these files into the output folder; the Ubuntu package uses the system Qt packages. Before distributing binaries, review the licenses of the DLLs, plugins, and toolchain runtime components actually included, and arrange access to the applicable Qt source code. The license for this project's own source code still needs to be chosen separately.

## Safety and tests

An EFI entry shows only that firmware has a record of a boot target; it does not guarantee that the device or operating system will boot. The application checks that the selected entry still exists and has an EFI path. It does not reboot if setting the target fails or the read-back value differs. If the final reboot confirmation is canceled, firmware may still retain the one-time boot selection that was already set.

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Unit tests use fixed input and a fake backend; they do not change boot settings or reboot the computer. The project has built successfully on Windows with Qt 6.12.0, Qt's CMake 3.30.5, and MinGW 13.1, and the single CTest test passed. It has not yet been built on Ubuntu or tested with a real operating-system switch.
