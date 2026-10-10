# 实施进度

最后更新：2026-10-10（Asia/Shanghai）

## 当前阶段

2026-10-10 Git交付：用户确认新建私有仓库871099/NiumaTimer-Private，已创建并核查PRIVATE，功能提交b2d3d7a已推送main。新增private远程，main跟踪private/main；origin仍指向原公开仓库，公开仓库与v0.1.0发布未修改。编译包和本机数据未纳入Git。

2026-10-10 记录入口与穿透修正：只读确认原history有10个日期而隔离目录0个，两根启动脚本统一读取原正式数据且优先r4；未改动历史文件。穿透改为展示区原生输入样式，按钮区恢复输入。Release16/16成功，QML静态检查未报错误；实际交互待验收。包dist/NiumaTimer-widget-r4-windows-x64。

2026-10-10 挂件透明度：桌面陪伴新增0–80%透明度滑块，角色/文字/背景整体即时淡化，偏好保存至 Desktop/miniOpacity，默认保持原效果；不影响主窗口。实际透明显示与重启回读待验收。

2026-10-10 声音修正：音频生成与设备操作移至独立线程，按FIFO提交6个缓冲，淡出后自然排空再关闭；设备错误完整清理，音量偏好合并写盘。Release增量6/6成功，实际听感待用户确认。包 dist/NiumaTimer-focus-audio-r2-windows-x64；根目录启动脚本优先使用修正版。

2026-10-10：新增专注与声音首版：独立 FocusController/SoundController、FocusPage、活动清单/今日计划、番茄与等待/打断记录、复盘、Windows 合成声音和挂件入口。Release 构建成功，修正版启动日志确认 QML 主窗口已加载；Computer Use 未完成页面观察，用户按 Escape 停止界面自动操作。完整交互与试听待验收；Android 音频未接入。详见 docs/07-focus-and-sound.md。

2026-10-10：按用户确认开始桌面陪伴首批升级；新增托盘/迷你挂件、隐私显示、提醒、开机启动选项、未结算日期例外和收入目标，Windows Release 已编译。首个预览主窗口和挂件已实际显示；完整交互待验收。范围及后续阶段见 `docs/06-desktop-companion.md`。GitHub 仓库已由用户改为 Public；线上发布仍为 v0.1.0，本批源码未提交推送。

已完成 Windows Debug 和 Release 构建；已有共享导航、首页、任务、统计、工资/排班、成就、换装和关于页。2026-10-08 法定假期停班/补班和旧历史修正已完成，v0.1.0 Windows x64便携包已发布至 `https://github.com/871099/NiumaTimer/releases/tag/v0.1.0`。工程位于 `K:\dev\Game\worker-timer-v2`；旧原型 `K:\dev\Game\worker-timer` 保留，不作为本工程代码基础。

产品范围以用户提供的完整跨平台产品图和已确认的 `docs/04-product-implementation-plan.md` 为准。原文“独立页面尚未实现、方案待确认”与当前代码及用户确认冲突，已修正。页面代码已存在，不能将构建/启动成功视为全部视觉和交互完成验收。

## 已完成

- 2026-10-10 新增独立 DesktopController、MiniWidget、DesktopSettingsPage、DateExceptionsCard 和 IncomeGoalsCard；日期来源、例外班次、工资/经验与历史读取共用业务计算。
- 新增启动日志、Windows 单实例唤回和根目录启动脚本；本批不覆盖线上 v0.1.0。

- 源码已推送至GitHub仓库（发布时私有，用户后续改为公开） `871099/NiumaTimer` 的main分支，发布标签v0.1.0对应源码提交 `3c6e5bd`；Release包含Windows x64下载包和SHA256SUMS.txt。

- 修正旧版国庆误结算迁移：历史仓库备份原文件、归档原班次并清零假期工作记录，保留任务和外观；展示/统计/写入入口统一拦截假期班次。实际本机6条已修正，非假期2条保留。

- 本次新增独立 `ChinaHolidayCalendar`、官方2026年年历资源和 `HolidayCountdownCard`：网络时间推进、北京时间边界、下一假期/当前假期剩余时间、年度详情和调休日期、跨年未公布状态。法定放假日覆盖周排班并停止计时、工资与经验；官方调休工作日按个人班次执行。
- 此前已落入代码的共享 AppShell、任务/统计/工资/排班/成就/外观/关于页及 WorkHistoryStore，在本轮核对并保留；完整交互验收仍待完成。
- 参考成熟项目和 Qt 官方资料，形成技术选择记录：`docs/01-reference-research.md`。
- 建立 CMake/Qt QML 工程骨架与 `WorkTimer` C++ 业务层。
- 实现月薪、日薪、时薪估算、工作日设置、上下班时间、休息区间、跨午夜班次和本地设置保存。
- 增加大小周排班模式，大周/小周分别配置工作日，并可把本周设为轮换锚点。
- 按参考图重排主页：新增原创夜班办公室像素横幅，突出倒计时与工资主卡，并整理进度和班次摘要区。
- 新增透明背景六帧牛马疲劳度图组和精力血条；血条由 100% 逐渐降到 0%，颜色由绿转黄再转红，角色帧和疲劳文案同步变化。
- 新增打工经验和等级：每完成一个排班工作日获得 20 XP，每 500 XP 升一级；当前班次按有效工时预览增长，累计天数和处理日期保存在本机。
- 设置页可编辑既往累计上班天数；保存后重算等级经验，进行中的班次保持待完成状态，完成排班班次后继续自动累计。
- 新增异步 `NetworkClock`：启动、恢复前台及每小时发起 NTP 校时；以单调时钟推进已校准时间，失败时沿用本次运行内的最近校准，首次失败使用设备时间。设置页显示状态并可手动同步。
- 修复设置页保存：点击“保存设置”时先校验并提交工资、天数、上下班与休息时间输入，再检查 `QSettings::sync()` 状态；上下班时间可成对更新，保存失败会给出提示。
- Windows 应用顶栏新增置顶图钉，可切换 `Qt.WindowStaysOnTopHint`；状态写入 `QSettings`，窗口重启后读取。按钮仅在 Windows 显示，Android 不启用跨应用悬浮窗权限。
- Android 自定义清单基于 Qt 6.8 模板，增加 `INTERNET` 权限；安卓构建工具链仍不可用。
- 完成适配宽屏与窄屏的 QML 首页/设置页、主题和原创像素风吉祥物。
- 编写架构、计薪口径和 UML：`docs/02-architecture.md`、`docs/03-uml.md`。
- QML 静态检查完成：Qt 6.8.3 `qmllint`，0 errors、0 warnings。
- 使用 Qt `windeployqt` 部署运行依赖，已打开正式构建出的 Windows 窗口供验收。

## 当前问题与环境限制

- 首批新增交互未全部验收；首次运行托盘图标 URI 问题已修正并编译，修正版托盘恢复尚待确认。日期例外只支持未结算日期，已结算历史与旧经验账本迁移仍待实施。
- 当前只有姿态切换及轻微呼吸动效，完整逐帧动作未制作；目标编辑、备份恢复、CSV、年历联网更新与 Android APK 未完成。

- 旧版经验总数混合手填基数与自动奖励，没有逐日来源；旧历史修正不据工资记录反推扣减累计天数。后续自动经验已按假期规则停算。

- 官方年历当前收录至2026年；2027年只有元旦法定日期提示，连休和调休待正式公布后更新。年历随应用版本发布，没有自动联网年历更新。
- Windows 编译器问题已解决：PATH 原先优先命中旧版 `J:/AutoMod/bin/cl.exe`；改用 `L:/Program Files/Microsoft Visual Studio/2022/Community/VC/Auxiliary/Build/vcvars64.bat` 后，CMake 正确识别 MSVC 19.44。
- Android Qt Kit 与常见位置的 Android SDK 尚未安装；APK 构建和手机交互验收未执行。
- 当前运行的是首个本地桌面开发预览 PID35712；修正版已打包在 dist/NiumaTimer-desktop-preview-r2-windows-x64，需退出已有预览后启动。首版窗口/挂件已显示，修正版完整交互与其他电脑部署仍待验收。
- 已初始化Git main分支，当前公开远程仓库为 `https://github.com/871099/NiumaTimer`；编译目录、发布包和本机日志已忽略。

## 验证记录

- 2026-10-10 挂件透明度：Windows Release构建返回0，QML静态检查未报错误，仍有既有未限定访问警告；无新增/运行测试。已打包dist/NiumaTimer-widget-r3-windows-x64并更新启动脚本；未操作用户运行中的窗口，实际视觉与重启回读待验收。

- 2026-10-10 声音修正：Release6/6构建成功，git diff --check 无空白错误；未新增/运行测试、未操作当前运行窗口。声音听感及设备错误重试行为待验收。日志 build-release/sound-fix-build.log；包哈希见 build-release/sound-fix-evidence.json。

- 2026-10-10 专注首版：Windows Qt6.8.3/MSVC2022 Release 增量7/7链接成功；首次启动暴露 Flow 只读 implicitHeight 赋值错误，修正为 Layout.preferredHeight 后隔离预览启动日志出现 Startup complete。页面交互、完整轮次、恢复及实际听感待人工验收；未新增或运行测试。构建日志 build-release/focus-build-final.log；包 dist/NiumaTimer-focus-preview-windows-x64。

- 2026-10-10：Windows Qt6.8.3/MSVC2022 x64 Release 已编译；首个预览 PID35712 响应正常，Computer Use 观察宽/窄屏首页与挂件，倒计时持续推进。工具随后连续检测到用户输入，停止自动点击；日期保存、通知、穿透、托盘恢复等待验收。本轮未新增或运行测试用例。

- 2026-10-08：远程main与v0.1.0标签已核对；ZIP为58,623,064字节，解压后EXE与启动验证的包内EXE哈希一致，未混入用户history.json、日志、编译缓存或调试符号。GitHub Release非草稿，两份附件均已上传，服务端SHA256均与本机一致；证据为 `build-release/release-archive-verification.json` 与 `release-publication-verification.json`。

- 2026-10-08：Qt6.8.3/MSVC2022 x64 Release构建59/59成功；包内附Qt动态库/QML插件、MSVC运行库安装程序、Qt许可证与60份源模块第三方声明。移除开发Qt搜索路径后从包目录启动，PID11528响应正常，15个Qt模块全部来自包目录，标准错误为空；证据为 `build-release/release-startup-verification.json`。

- 2026-10-08：历史迁移回归和日历回归2/2通过；Qt6.8.3/MSVC x64 Debug 44/44构建成功。本机迁移6条国庆班次，假期工时/金额清零且不再结算，9月29日/30日保留，任务2→2，外观保留，备份SHA256与原文件一致。新版PID43300响应正常且标准错误为空；实际统计页视觉复核待用户验收，证据为 `build-msvc/history-migration-verification.json`。

- 2026-10-08：Windows Debug 完成新增年历及QML的编译、链接；windeployqt 返回0。相关6份QML的qmllint返回0，无警告输出。独立CTest `china-holiday-calendar` 1/1通过（14项边界/时区断言，0.10秒）。
- 2026-10-08：新版PID30392，窗口“牛马打工计时器”响应正常；通过computer-use实际观察首页显示2027元旦法定日期和待公布提示，秒数持续减少，标准错误为空。详情点击被窗口用户输入打断，弹层和窄屏完整交互未验收；未更改用户工资/排班输入。
- 2026-09-29：`qmllint` 对 `Theme.qml`、`Main.qml`、`SettingsPage.qml` 检查通过（0 errors、0 warnings）。
- 2026-09-29：VS 2022 x64 开发环境中的 CMake 配置成功，识别 MSVC 19.44.35228.0；使用 Ninja 完成 24/24 构建步骤。
- 2026-09-29：`windeployqt` 部署 Qt Quick 运行依赖；启动 `build-msvc/NiumaTimer.exe` 后，窗口标题为“牛马下班计时器”，进程响应正常。
- 2026-09-29：大小周改动后，三份 QML 的 `qmllint` 检查通过（0 errors、0 warnings），C++/QML 增量构建及链接成功；更新窗口重新启动且响应正常。
- 2026-09-29：主页重排及新增场景图片后，QML 静态检查通过，CMake 自动重新生成并完成 Windows Debug 构建，`windeployqt` 更新依赖；新窗口标题正确且进程响应正常。
- 2026-09-29：疲劳度精灵图为 1536×1024、3列×2行、每格512×512，背景像素 Alpha=0；QML 静态检查通过，Windows Debug 增量构建/链接成功，更新窗口已重新启动。
- 2026-09-29：新增牛马精力血条，绑定现有有效工时进度并使用渐变状态颜色；Qt 6.8.3 `qmllint` 0 errors、0 warnings，Windows Debug 构建 7/7 步骤成功，`windeployqt` 返回成功；窗口标题正确、进程响应正常。
- 2026-09-29：新增按排班工作日累计的经验与等级条；Qt 6.8.3 `qmllint` 0 errors、0 warnings，Windows Debug 构建 19/19 步骤成功，VS 2022 x64 环境中的 `windeployqt` 无警告完成；新窗口标题正确且进程响应正常。
- 2026-09-29：加入可编辑历史上班天数和 Qt Network NTP 校时；Qt 6.8.3 `qmllint` 0 errors、0 warnings，Windows Debug 构建 14/14 步骤成功，`windeployqt` 返回 0 且无警告，AndroidManifest XML 解析通过；新版窗口 PID 46028、标题正确、进程响应正常、标准错误为空。PowerShell 对 `time.cloudflare.com:123` 发起单包探测，收到 48 字节、Mode 4、Stratum 3 的响应；应用内同步状态仍待人工确认。没有 Android 构建工具链。
- 2026-09-29：修复设置保存提交链路；点击按钮会显式读取并校验文本输入，且以 QSettings 同步状态反馈结果。上下班时间改为原子成对更新。Qt 6.8.3 `qmllint` 0 errors、0 warnings，Windows Debug 构建 7/7 成功，`windeployqt` 返回 0 且无警告；新版窗口 PID 41632、标题正确、进程响应正常、标准错误为空。需要用户手动点击保存并重启确认设置值保持。
- 2026-09-29：新增 Windows 窗口置顶开关；Qt 6.8.3 `qmllint` 0 errors、0 warnings，Windows Debug 构建 19/19 成功，`windeployqt` 返回 0；新版窗口 PID 43084、标题正确、进程响应正常、标准错误为空。图钉点击切换和重启读取仍待人工验收。
- 2026-09-29：大小周实际交互待用户验收；Android APK 和手机验收待 Android 工具链。

## 下一步

0. 先验收桌面陪伴首批功能（详见 docs/06-desktop-companion.md），再推进经验账本、数据导出、完整角色动画与 Android。

1. 人工验收假期详情打开/关闭、调休列表和窄屏换行；国务院新年度通知公布后更新年历资源。
2. 完成已有页面的保存回读、任务维护、历史统计、成就/外观，以及置顶、大小周和网络校时交互验收；按产品图继续检查视觉效果。
3. v0.1.0已发布；其他电脑部署待独立验收。
4. 安装匹配 Qt 版本的 Android Kit、JDK、SDK/NDK，生成 APK 并在手机上验收竖屏、触控及前后台恢复。

## 最近进展（事实）

- 2026-10-08：初始化Git仓库并发布v0.1.0；源码、原创素材、UML和回归检查均已推送，Windows Release程序、动态依赖、许可资料及校验文件可在私有仓库Releases下载。

- 2026-10-08：补齐上一轮遗漏的旧历史迁移，国庆6条误结算已撤销，统计不再包含其工资和工时；新版窗口PID43300已重新打开。

- 2026-10-08：法定假期停班和官方调休补班已接入排班源头；21项日期/排班断言、Windows Debug构建、`windeployqt` 与假期卡片 `qmllint` 通过，新版窗口 PID 44556 响应正常。根目录路线总览仅维护Unity/Godot，此独立Qt应用不更改两路线状态。
- 新建独立工程，没有改动旧原型。
- 已完成首版代码、设计文档、参考调研和原创界面插画。
- Windows Debug 构建、精力血条/经验条、大小周和窗口置顶更新版启动已完成；Android 工具链和用户视觉交互验收仍待完成。
