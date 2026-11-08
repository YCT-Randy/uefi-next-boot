param([string]$BuildDir = "", [string]$OutputDir = "", [string]$QtDir = "")
$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
if ([string]::IsNullOrWhiteSpace($BuildDir)) { $BuildDir = Join-Path $repoRoot "build" }
if ([string]::IsNullOrWhiteSpace($OutputDir)) { $OutputDir = Join-Path $repoRoot "dist" }

if ($QtDir) {
    $QtDir = (Resolve-Path -LiteralPath $QtDir).Path
    $deployToolPath = Join-Path $QtDir "bin/windeployqt.exe"
    if (!(Test-Path -LiteralPath $deployToolPath)) { throw "QtDir 內找不到 windeployqt.exe：$QtDir" }
} else {
    $deployTool = Get-Command windeployqt.exe -ErrorAction SilentlyContinue
    if (!$deployTool) { throw "找不到 Qt 6 的 windeployqt.exe；請指定 -QtDir。" }
    $deployToolPath = $deployTool.Source
}

$cmakeArgs = @("-S", $repoRoot, "-B", $BuildDir, "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_INSTALL_PREFIX=$OutputDir")
if ($QtDir) { $cmakeArgs += "-DCMAKE_PREFIX_PATH=$QtDir" }
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw "CMake 設定失敗" }
& cmake --build $BuildDir --config Release --parallel
if ($LASTEXITCODE -ne 0) { throw "CMake 編譯失敗" }
& cmake --install $BuildDir --config Release
if ($LASTEXITCODE -ne 0) { throw "CMake 安裝失敗" }

$exe = Join-Path $OutputDir "bin/boot-manager.exe"
if (!(Test-Path -LiteralPath $exe)) { throw "找不到輸出執行檔：$exe" }
& $deployToolPath $exe
if ($LASTEXITCODE -ne 0) { throw "windeployqt 部署失敗" }
$qtNoticeDir = Join-Path $OutputDir "licenses/qt"
New-Item -ItemType Directory -Path $qtNoticeDir -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot "licenses/qt/LICENSE") -Destination (Join-Path $qtNoticeDir "LICENSE") -Force
Copy-Item -LiteralPath (Join-Path $repoRoot "licenses/qt/README.md") -Destination (Join-Path $qtNoticeDir "README.md") -Force
Copy-Item -LiteralPath (Join-Path $repoRoot "licenses/qt/READMD-CN.md") -Destination (Join-Path $qtNoticeDir "READMD-CN.md") -Force
Write-Host "Windows 部署資料夾已產生：$OutputDir"
