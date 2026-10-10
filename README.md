# 牛马打工计时器

一个面向 Windows 与 Android 的 Qt 6 工具应用，包含下班倒计时、工资估算、任务、历史统计和成长装扮。界面采用深蓝夜色、霓虹色块和原创像素风牛马插画。

## Windows 下载

在 [私有仓库 Releases](https://github.com/871099/NiumaTimer-Private/releases) 下载 `NiumaTimer-0.2.0-windows-x64.zip`（需有访问权限的 GitHub 账号），完整解压后运行 `NiumaTimer.exe`。Qt 动态库和插件随包提供；若缺 MSVC 运行库，可运行包内 `vc_redist.x64.exe`。本版本是第二个发布版本，详见 [v0.2.0 发布说明](docs/releases/v0.2.0.md)。

## 本地开发预览（2026-10-10）

首批新增：Windows 迷你挂件/托盘、缩放/锁定/点击穿透、关闭收起、开机启动选项、工资隐私模式、下班及久坐提醒、收入目标、六周日期例外预览，以及启动日志和单实例保护。详见 [实施范围与待验收项](docs/06-desktop-companion.md)。线上 v0.1.0 尚不含本批功能。

本机可双击根目录 `启动牛马计时器.cmd`，优先使用本地完整预览包。请保留整个运行目录，不要单独复制 EXE。

## 当前功能

- 中国内地法定节假日倒计时：自动选择下一个假期，假期中显示剩余时间；法定放假日按休息日处理，暂停上班计时、工资和经验，官方调休上班日按个人班次执行；详情列出全年安排与官方来源，按北京时间计算。
- 已收录官方 2026 年放假安排；2027 年暂仅显示元旦法定日期，并标记连休/调休待公布。年历随应用更新，未收录日期显示待更新提示，详见 `docs/05-china-holiday-countdown.md`。
- 共享桌面侧栏与手机底栏；独立首页、任务、统计、工资、工作时间、成就、外观和关于页面。
- 首页按产品图采用明亮像素卡片、霓虹边框、实时点阵倒计时、中央疲劳牛马和办公室场景；状态/时间线/班次快捷设置在宽屏并排显示，手机纵向重排并使用五项底栏。
- 当日自定义任务可添加、编辑、删除和标记完成；日/周/月统计使用本地结算历史，成就和装扮根据工作天数、等级解锁。
- 优先使用网络时间推进上班/下班倒计时；断网时沿用本次运行最近一次校准，首次校时失败时回退到设备时间。支持跨午夜班次。
- 按月薪、日薪或时薪估算今日已赚金额，使用整数“分”显示结果。
- 可配置工资、上/下班时间、休息区间，并选择固定周排班或大小周轮换；大周、小周的上班日可分别设置。
- 首页显示牛马精力血条：随有效工时从 100% 逐渐降到 0%，颜色由绿转黄再转红；角色姿态和疲劳文案同步变化。
- 蓝色经验条随累计排班上班日增长：每完成一个工作日获得 20 XP，每 500 XP 升一级；当前班次会按有效工时预览经验增长，累计数据保存在本机。
- 设置页可录入既往累计上班天数；之后按完成的排班日继续累计，并可手动触发网络校时。
- Windows 应用顶栏图钉可切换窗口置顶，置顶状态保存在本机设置中。
- 休息时段继续计算墙钟倒计时，但冻结已赚工资与有效工时进度。
- 设置使用 Qt `QSettings` 保存在本机；应用恢复前台后会刷新班次并重新进行网络校时。

金额是税前个人估算，不计算个税、社保、公积金、法定节假日或加班倍率。Android 首版不承诺锁屏常驻通知；恢复应用后会重新计算。

## 工程结构

```text
worker-timer-v2/
├── CMakeLists.txt
├── assets/                  # 原创像素风吉祥物与素材说明
├── docs/                    # 技术参考、架构与 UML
├── src/
│   ├── app/WorkTimer.*      # Qt 属性、设置持久化与计时快照
│   ├── app/NetworkClock.*   # 异步 NTP 校时与时间推进
│   ├── app/ChinaHolidayCalendar.* # 官方年历与北京时间假期快照
│   ├── app/WorkHistoryStore.* # 版本化历史、任务与外观数据
│   ├── ui/                  # QML 主界面、主题和设置页
│   └── main.cpp
├── progress.md              # 当前实施与实际验收状态
└── progress_history.md     # 追加式历史记录
```

## 构建要求

- Qt 6.8 或更新版本，包含 Qt Quick、Qt Quick Controls 2 和对应目标平台 Kit。
- Windows：MSVC 2022 x64、CMake、Ninja。
- Android：Qt for Android Kit、JDK 17、Android SDK/NDK；Qt Creator 可生成 APK。

### Windows

在 Visual Studio 2022 x64 开发者终端中（确保 PATH 使用 VS 2022 的 `cl.exe`）：

```powershell
cmake -S . -B build-msvc -G Ninja `
  -DCMAKE_PREFIX_PATH="I:/soft/qt/6.8.3/msvc2022_64" `
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-msvc
```

打包运行目录：

```powershell
I:/soft/qt/6.8.3/msvc2022_64/bin/windeployqt.exe `
  --qmldir src/ui build-msvc/NiumaTimer.exe
```

### Android

在 Qt Creator 中选择 Android Kit，配置 JDK、SDK 和 NDK 后构建 APK。也可以用对应 Qt Android Kit 的 `qt-cmake` 单独配置 Android 构建目录；桌面和 Android 产物必须分别构建。

## 研究与设计

- [开源项目与 Qt 官方资料](docs/01-reference-research.md)
- [架构与计算口径](docs/02-architecture.md)
- [UML 设计](docs/03-uml.md)
- [已确认的完整产品实施方案](docs/04-product-implementation-plan.md)
- [中国法定节假日倒计时与年度数据更新](docs/05-china-holiday-countdown.md)
- [实施进度与验收记录](progress.md)

## 本地专注开发预览（2026-10-10）

新增番茄专注、任务估算、打断/等待、复盘及 Windows 原创合成声音。双击 `预览专注模式.cmd` 使用独立业务数据目录；正式使用先退出旧版，再双击 `启动牛马计时器.cmd`。共享桌面/声音偏好仍使用本机设置，业务工资/历史/专注账本在隔离目录。完整交互和听感待验收，Android 音频未接入。见 `docs/07-focus-and-sound.md`。

### 2026-10-10 r4 入口修正

两个根目录启动脚本均使用正式数据，不再默认传入隔离预览参数。需要先退出已有各个牛马实例，再打开脚本，防止单实例唤回旧版。原history未删除；显式开发隔离参数仍保留并在窗口标题标明。r4穿透仅作用展示区，顶部和底部按钮保留输入，实际交互待验收。
