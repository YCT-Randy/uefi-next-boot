# Qt 授權與發佈說明

[English version](README.md)

本專案使用 Qt 6 建置桌面介面。`LICENSE` 複製自 Qt Online Installer 安裝的 `C:\Qt\Licenses\LICENSE`，包含 GNU GPL v3 與 GNU LGPL v3 的完整條文；它是 **Qt 的授權文件，不是本專案原始碼的授權宣告**。

## 使用的 Qt 元件

- 原始碼直接連結：Qt Core、Qt GUI、Qt Widgets、Qt Concurrent。
- 目前 Windows 的 `windeployqt` 輸出也包含 Qt Network、Qt SVG、平台及其他外掛。實際清單會隨 Qt 版本和打包結果改變。
- Windows 使用 DLL 動態連結；Ubuntu `.deb` 目前使用發行版的 Qt 套件，並未把 Qt 函式庫複製進 `.deb`。

Qt 的[授權總覽](https://doc.qt.io/qt-6/licensing.html)說明各模組可能有不同授權；應以實際發佈的版本與檔案核對。Qt DLL 和外掛可能含有其他著作權人的程式碼，詳見 [Qt 6.12 第三方元件清單](https://doc.qt.io/qt-6/licenses-used-in-qt.html)與 Qt 安裝目錄內的 `sbom/`。發佈包應只列出並附上實際隨附元件需要的聲明與授權文字。

## 發佈執行檔前

Windows 打包腳本會將本目錄的 `LICENSE` 與兩種語言的說明複製到輸出資料夾的 `licenses/qt/`。這處理了 Qt 授權條文的隨附，**尚未代表整個發佈包已完成授權義務**。若使用 Qt 開放原始碼授權發佈，還應依 [Qt 官方義務說明](https://www.qt.io/development/open-source-lgpl-obligations) 核對：

1. 實際隨附的 Qt DLL、外掛與第三方執行階段元件，逐項提供所需授權及著作權聲明。
2. 準備與發佈版本對應的 Qt 完整原始碼及任何修改，或依適用條款提供有效的取得方式；單獨指向 Qt 官方下載頁可能不足。
3. 確保使用者能以相容的修改版 Qt 函式庫替換 DLL 並執行程式。

若使用 Qt 商業授權，應改依該授權契約檢查要求。每次升級 Qt 或更改打包內容後，都需重新核對以上項目。
