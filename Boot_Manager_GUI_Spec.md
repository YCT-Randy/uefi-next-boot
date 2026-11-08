# Boot Manager GUI — 跨平台 UEFI 系統切換工具規格（MVP）

> 此文件保留早期 Python／PySide6 方案作為設計紀錄；目前實作與建置方式請以 README.md 的 C++17／Qt 6 說明為準。

## 1. 專案目標

開發一個可在 **Windows 與 Ubuntu** 執行的桌面應用程式，讓使用者在目前作業系統中選擇**下一次重新啟動要進入 Windows 或 Ubuntu**，不必手動進入 BIOS/UEFI 開機選單。

核心原則：使用 **UEFI 的一次性開機選項（BootNext）**，**不永久修改 BootOrder**、不修改 GRUB 設定、不重灌作業系統。

## 2. 已知環境與實測結果

- 一台 UEFI 電腦。
- **Windows** 安裝在內建 SSD。
- **Ubuntu** 安裝在 USB 連接的攜帶式 SSD。
- 已實測成功：
  - Windows → Ubuntu：透過 `bcdedit /set "{fwbootmgr}" bootsequence "{ubuntu-guid}"` 設定後重新啟動。
  - Ubuntu → Windows：透過 `sudo efibootmgr -n <Windows BootXXXX>` 設定後重新啟動。
- Windows 上 `bcdedit /enum firmware` 曾顯示：

  ```text
  Windows Boot Manager
      identifier  {bootmgr}
      path        \EFI\MICROSOFT\BOOT\BOOTMGFW.EFI

  ubuntu
      identifier  {<本機 Ubuntu 啟動項目 GUID>}
      device      partition=\Device\HarddiskVolumeN
      path        \EFI\UBUNTU\SHIMX64.EFI
  ```

- 上述 Ubuntu GUID 是示意占位符；每台電腦的實際值不同。Linux 的 BootXXXX 編號也不應寫死。
- EFI 分割區的 `HarddiskVolumeN` 卷號不固定；GUI 不應假設裝置路徑一定固定。

## 3. 使用者操作流程

1. 開啟 **Boot Manager** GUI。
2. GUI 顯示目前作業系統，並列出可選的開機目標：**Windows**、**Ubuntu**。
3. 選擇目標，按下「重新啟動到所選系統」。
4. 跳出確認對話框，提醒使用者先儲存工作。
5. 使用必要權限設定一次性啟動目標。
6. **重新讀取/驗證**一次性啟動設定；僅在成功時執行重新啟動。
7. 若失敗，顯示明確錯誤訊息，**不要重新啟動**。

建議 UI 草稿：

```text
┌────────────────────────────────────┐
│ Boot Manager                       │
│ 目前系統：Windows / Ubuntu          │
│                                    │
│  [ Windows（內建 SSD） ]            │
│  [ Ubuntu（USB SSD）   ]            │
│                                    │
│  [ 重新啟動到選取的系統 ]           │
│                                    │
│ 提示：僅設定下一次開機，不改預設順序 │
└────────────────────────────────────┘
```

若目標系統就是目前系統，仍可允許一般重啟，但 UI 應清楚標示此行為。**MVP 可先只提供已辨識的 Windows/Ubuntu 項目**，不要將任意 UEFI 裝置項目當作可靠的作業系統。

## 4. 技術方案

- GUI：**Python + PySide6（Qt）**，Windows / Ubuntu 共用 UI 與大部分商業邏輯。
- 平台偵測：`platform.system()`。
- Windows 後端：`bcdedit.exe` 列舉韌體啟動項目，使用 `bcdedit /set "{fwbootmgr}" bootsequence "{target-guid}"` 設定下次啟動。
- Ubuntu 後端：`efibootmgr` 列出 UEFI BootXXXX，使用 `efibootmgr -n XXXX` 設定 BootNext。
- 重新啟動：Windows `shutdown.exe /r /t 0`；Ubuntu 透過授權的 `systemctl reboot`。
- 外部命令使用 `subprocess.run([...], shell=False, capture_output=True, text=True, timeout=...)`；勿直接拼接未驗證的使用者輸入當 shell 指令。
- 權限：
  - Windows：需要提升管理員權限（UAC）；可設計為啟動時提升，或僅執行系統操作時提升。
  - Ubuntu：以 **polkit / pkexec + 最小權限 helper** 或明確的 sudo 授權機制執行設定操作；**不應直接讓整個 GUI 以 root 執行**。

### 4.1 啟動項目偵測

**Windows：**

1. 執行 `bcdedit /enum firmware`。
2. 從韌體項目找出描述為 `ubuntu`、且 EFI 路徑符合 Ubuntu shim/grub 預期的項目。
3. Windows 可使用 `{bootmgr}` 作為 Windows Boot Manager 識別碼；仍應確認該項目存在。
4. 對無法唯一辨識的目標不要猜測，顯示錯誤並要求使用者確認。
5. `bcdedit` 輸出可能依 Windows 顯示語言不同而變化，解析不可只依靠中文標題。

**Ubuntu：**

1. 執行 `efibootmgr -v`。
2. 找到描述包含 `Windows Boot Manager` 的 BootXXXX，並可透過 EFI 路徑（例如 `\EFI\Microsoft\Boot\bootmgfw.efi`）做輔助確認。
3. 找到 Ubuntu 的 BootXXXX，用於顯示目前/可用目標；必要時比對 `BootCurrent`。
4. 若多個 Windows 或 Ubuntu 項目同名，不任意選第一個，應要求選擇或顯示明確提示。

### 4.2 操作安全與驗證

- **不能只因為設定命令 return code = 0 就假設一定能啟動**：需要重新讀取 BootNext / firmware bootsequence 確認設定目標正確。
- Windows 可再次執行 `bcdedit /enum "{fwbootmgr}"` 檢查 `bootsequence`。
- Linux 可再次執行 `efibootmgr` 檢查 `BootNext: XXXX`。
- 使用者取消權限提升、目標消失、外接 SSD 拔除、命令失敗或逾時：不得自動重啟。
- 外接 SSD 是否存在可做**額外的最佳努力檢查**，但 EFI 啟動項目存在 ≠ 實體 SSD 必然可用；不承諾百分之百保證可開機。
- 如 Windows 啟用了 BitLocker / 裝置加密，應提醒使用者事先保存復原金鑰；此提醒與 Ubuntu 數量無關。
- 顯示明確的「取消」及「確認重新啟動」按鈕。
- 預設不變更 `BootOrder`、不刪除或建立 EFI 啟動項目、不改動磁碟分割區。

## 5. 建議專案架構

```text
boot-manager/
├── main.py
├── gui/
│   └── main_window.py
├── core/
│   ├── boot_manager.py        # 共通模型/接口與控制流程
│   ├── windows_boot.py        # bcdedit 後端
│   └── linux_boot.py          # efibootmgr 後端
├── helpers/
│   └── README.md              # 平台權限 helper 設計說明
├── tests/
│   ├── test_windows_parser.py
│   ├── test_linux_parser.py
│   └── test_boot_flow.py
├── requirements.txt
└── README.md
```

可先使用後端抽象介面：

```python
class BootBackend:
    def list_targets(self): ...
    def get_current_system(self): ...
    def set_next_boot(self, target): ...
    def verify_next_boot(self, target): ...
    def reboot(self): ...
```

## 6. MVP 範圍

**必做：**

- Windows / Ubuntu 相同風格的 GUI。
- 列出可辨識的開機系統，並標示目前執行的系統。
- 一次性啟動目標設定、讀回驗證、再重啟。
- 管理員權限/授權流程。
- 確認對話框與錯誤訊息。
- 找不到指定啟動項目時不允許操作。
- 解析器及流程單元測試（使用 mock；單元測試不能真實重啟電腦）。
- Windows 桌面捷徑，Ubuntu `.desktop` 啟動器。

**暫不做：**

- 修改永久開機順序（BootOrder）。
- 修復 GRUB/EFI 分割區。
- 遠端控制或網路服務。
- 支援 Legacy BIOS。
- 跨電腦同步啟動項目 ID。
- 自動安裝或重新安裝 Ubuntu。

## 7. 打包與發佈

- Windows：預計用 **PyInstaller** 產生 `.exe`（在 Windows 上建置）。
- Ubuntu：先提供 Python 專案 + `.desktop`，後續再評估 AppImage / `.deb`（在 Linux 上建置）。
- **PyInstaller 不是跨作業系統交叉編譯器**；Windows 和 Ubuntu 必須分別建置/測試。
- 不要把這台機器的 UEFI GUID 寫死在發佈版程式裡；只可用於本機測試設定或可編輯的本機設定檔。

## 8. 驗收條件

1. Windows 開啟 GUI，選擇 Ubuntu → 使用者確認並授權 → 重新啟動成功進入 Ubuntu。
2. Ubuntu 開啟 GUI，選擇 Windows → 使用者確認並授權 → 重新啟動成功進入 Windows。
3. 再次正常開機時，原本的 UEFI 預設開機順序保持不變。
4. 使用者取消或無權限時，**不重啟**。
5. 外接 Ubuntu SSD 未連接、目標項目不存在、目標有歧義時，顯示警告且**不重啟**。
6. 模擬外部命令失敗、讀回驗證不一致、命令逾時，應正確中止。

## 9. 希望 Codex 協助討論的問題

1. Windows `bcdedit` 的**多語系輸出解析**怎樣設計最穩定？是否有更可靠的 Windows API 可直接使用？
2. Ubuntu 最合適的 **polkit 最小權限 helper** 實作與打包方式是什麼？
3. Windows UAC 如何兼顧 GUI 使用體驗與最小權限？
4. 如何可靠處理多筆同名 UEFI 啟動項目、以及 USB SSD 暫時不可用？
5. 如何安排 mock 測試，確保開發時不會真的重啟電腦？
6. MVP 是否先優先支援當前已驗證的這一台電腦，再逐步擴展通用硬體相容性？

---

**開發建議：先實作只讀掃描與 GUI，接著 mock 測試，最後再加入需授權的「設定 BootNext + 確認 + 重啟」功能。**
