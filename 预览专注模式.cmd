@echo off
chcp 65001 >nul
rem 日常启动读取正式数据，不传入隔离预览参数。
set "timerDir=%~dp0dist\NiumaTimer-0.2.0-windows-x64"
if not exist "%timerDir%\NiumaTimer.exe" set "timerDir=%~dp0dist\NiumaTimer-widget-r4-windows-x64"
if not exist "%timerDir%\NiumaTimer.exe" set "timerDir=%~dp0dist\NiumaTimer-widget-r3-windows-x64"
if not exist "%timerDir%\NiumaTimer.exe" set "timerDir=%~dp0dist\NiumaTimer-focus-audio-r2-windows-x64"
if not exist "%timerDir%\NiumaTimer.exe" set "timerDir=%~dp0dist\NiumaTimer-focus-preview-windows-x64"
if not exist "%timerDir%\NiumaTimer.exe" set "timerDir=%~dp0dist\NiumaTimer-0.1.0-windows-x64"
if not exist "%timerDir%\NiumaTimer.exe" (
  echo 未找到完整发布包。请从 GitHub Releases 下载并完整解压。
  echo https://github.com/871099/NiumaTimer/releases
  pause
  exit /b 1
)
if not exist "%timerDir%\Qt6Core.dll" (
  echo 缺少 Qt 运行库，请重新完整解压发布包，不要只复制 EXE。
  pause
  exit /b 1
)
if not exist "%timerDir%\platforms\qwindows.dll" (
  echo 缺少 Windows 平台插件，请重新完整解压发布包。
  pause
  exit /b 1
)
start "" /D "%timerDir%" "%timerDir%\NiumaTimer.exe"
