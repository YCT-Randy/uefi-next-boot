# Boot Manager

[English README](README.md)

以 C++17 與 Qt 6 製作的 UEFI 單次開機選擇工具。介面列出目前系統可讀到的 EFI 開機項目；設定後讀回驗證，使用者再次確認後才重新啟動。程式不修改永久 BootOrder。

## 原始碼結構

```text
include/boot_manager/       標頭檔，依 core/gui/platform/utils 分類
src/main.cpp                程式進入點
src/core/                   共用流程與輸出解析實作
src/gui/                    Qt 視窗實作
src/platform/windows/       bcdedit 後端實作
src/platform/linux/         efibootmgr 後端實作
src/utils/                  命令執行工具實作
```

`BootBackend` 是平台策略介面；啟動時選擇 Windows 或 Linux 後端。Windows 以 UAC 管理員權限執行；Ubuntu 的 GUI 維持一般使用者權限，設定與重啟則由 polkit 授權的 helper 執行。

## 安裝建置工具

### Windows

1. 從 [CMake 官方下載頁](https://cmake.org/download/)安裝 Windows x64 的穩定版，安裝時加入 `PATH`。
2. 從 [Qt 官方安裝說明](https://doc.qt.io/qt-6/qt-online-installation.html)下載 Qt Online Installer。選擇 Qt 6 桌面版、對應的編譯器套件，以及 CMake／Ninja 工具。Qt 套件與編譯器必須使用相同的工具鏈，例如都選 MinGW，或都選 MSVC 2022。
3. 在新的 PowerShell 視窗確認 `cmake --version`、`Get-Command windeployqt.exe`。若 Qt 的 `bin` 不在 `PATH`，打包時以 `-QtDir` 指定 Qt kit 根目錄。

若電腦上有多個 Qt 版本，打包時應指定 Qt 6 的 `-QtDir`，避免誤用其他版本的 `windeployqt`。

### Ubuntu

在 Ubuntu 22.04／24.04 可先安裝發行版套件：

```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools \
    libgl1-mesa-dev efibootmgr policykit-1 dpkg-dev
```

Qt Widgets 與 Concurrent 由 `qt6-base-dev` 提供。若套件名稱因 Ubuntu 版本而異，先用 `apt search` 確認。參考 [Qt 的 Linux 安裝說明](https://doc.qt.io/qt-6/linux.html)與 [Qt 的套件管理器說明](https://doc.qt.io/qt-6.10/linux-alternative-installers.html)。

## 建置與打包

在 Windows PowerShell 執行：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\packaging\windows\package.ps1 -QtDir 'C:\Qt\<版本>\mingw_64'
```

請將 `<版本>` 改成已安裝的 Qt 6 版本。輸出在 `dist/bin/boot-manager.exe`，同資料夾會有 `windeployqt` 收集的 Qt 檔案。`-QtDir` 指向包含 `bin/windeployqt.exe` 的 Qt 6 kit 目錄。請在對應編譯器的開發環境執行。上例的 `-ExecutionPolicy Bypass` 僅套用到該次 PowerShell 行程，不修改電腦的永久執行原則。

在 Ubuntu 執行：

```bash
bash packaging/linux/package.sh
```

`.deb` 產生於 `dist/`，安裝時包含桌面捷徑、polkit policy 與最小權限 helper。兩個平台必須各自編譯，來源碼共用。

## Qt 授權資料

[Qt 授權條文與發佈說明](licenses/qt/README.md)已收入原始碼。Windows 打包腳本會將它們複製到發佈資料夾；Ubuntu 版本使用系統 Qt 套件。公開發佈執行檔前，仍需依實際附帶的 DLL、外掛及工具鏈執行階段元件完成第三方授權核對，並安排適用的 Qt 原始碼提供方式。本專案自己的原始碼授權則需另行決定。

## 安全與測試

掃描到 EFI 項目只表示韌體有紀錄，無法保證其裝置或作業系統一定能開機。程式會重新確認所選項目仍存在且有 EFI 路徑；設定失敗或讀回不一致時不會重啟。取消最後的重啟確認後，已設定的一次性開機選項仍可能由韌體保留。

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

單元測試使用固定輸入與假後端，不會改開機設定或重新啟動。Windows 已使用 Qt 6.12.0、Qt 附的 CMake 3.30.5 與 MinGW 13.1 成功建置，`ctest` 1/1 通過；尚未在 Ubuntu 建置，也未進行實機開機切換。
