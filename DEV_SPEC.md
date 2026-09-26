<!-- Dev specification for ChongMingGUI (重明). 按骨架七章填写。 -->
# Developer Specification (DEV_SPEC)

> 版本：1.0 — 重明项目复刻版
> 项目：**ChongMingGUI（重明）** —— 海康工业相机二次开发上位机
> 课程来源：周旋机器视觉《重明：海康工业相机二次开发》 https://www.roundvision.cc
> 参照源工程：`D:\QT6\000workspace\chongming-V2.1.0.0`（只读，勿改动）
> 本文档定位：**既是复刻目标规格，也是本机环境适配记录**

## 目录

- 项目概述
- 核心特点
- 技术选型
- 测试方案
- 系统架构与模块设计
- 项目排期
- 可扩展性与未来展望

---

## 1. 项目概述

本项目是一个完整的工业相机二次开发应用：**Qt 上位机 + 可扩展的相机框架**。
它把不同厂商相机的共性能力——**枚举、连接、拉流、断连、参数读写、配置导入导出**——抽象成统一接口，
基于海康 MVS SDK 实现了网口（GigE）与 USB 口工业相机的完整控制，并附带一个**虚拟相机**用于无硬件开发调试。

从工程视角看，它是一次「框架设计」练习，覆盖：

| 层次 | 涉及内容 |
|------|---------|
| 工程组织 | 目录分层、qmake 构建、三类资源（qrc）管理 |
| C++ 设计模式 | 抽象工厂（品牌扩展）、单例（上下文/工厂/JSON 解析/观察者）、生产者-消费者（图像队列） |
| Qt 开发 | .ui 文件与 uic 链路、QSS 全局样式、Model/View/Delegate 参数树、QGraphicsView 视觉窗口、多线程 |
| 相机 SDK | 海康 MVS SDK 封装：句柄生命周期、回调注册、六种参数类型读写、访问模式判断 |
| 图像链路 | SDK 回调 → 线程安全队列 → 取图线程 → cv::Mat↔QImage → 屏幕 |

### 设计理念 (Design Philosophy)

> **核心定位：以复刻驱动学习 (Learn by Replication)**
>
> 本项目的目标是**不照抄、而是理解后重建**周旋课程中的重明上位机。
> 复刻的每一步都要求能回答三个问题：这段代码解决什么问题？为什么用这种设计？
> 如果换一种写法会怎样？因此本文档不仅是"要做什么"的清单，
> 也是"为什么这么做"的依据，同时记录了本机环境与课程环境的全部差异。

---

## 2. 核心特点

### 相机框架：抽象接口 + 工厂 + 上下文三层解耦

整个相机能力被拆成三层，UI 只跟最上层打交道：

```
UI (ControlWidget/ParamWidget/ViewWidget)
        │  只认序列号 QString
        ▼
CameraContext (单例)  ── QMap<序列号, CameraInterface*> ── 按序列号路由
        │  只认纯虚接口
        ▼
CameraInterface (17 个纯虚函数)
        ▲                    ▲
   HikCamera           VirtualCamera
 (MVS SDK 实现)       (无硬件模拟，随机出图)
```

`CameraInterface` 定义 17 个纯虚函数（`CameraInterface.h:37-69`），覆盖相机全生命周期：
`acquire/release`（句柄）、`connect/disconnect`（连接）、`creatStream/destroyStream`（流通道）、
`startGrabbing/stopGrabbing`（拉流）、`readParam/writeParam`（参数）、`loadConfig/saveConfig`（配置）等。
新增一个品牌相机 = 继承接口 + `CameraFactory::registerCamera<T>` 注册一行，UI 零改动。

### 生产者-消费者图像队列

相机出图和界面刷新天然是两个节奏，中间用 `CameraImageQueue` 缓冲：

- **双队列设计**：`freeImageQueue` 预分配 10 个空 `cv::Mat`（复用内存），`workImageQueue` 存待显示帧（`CameraImageQueue.cpp:8-10`）；
- **深度 10、超时 5s**：`ImageQueueSize 10` / `TIME_OUT_MS 5000`（`CameraImageQueue.h:18-19`）；
- **满则丢最旧帧**：`Put` 不阻塞 SDK 回调线程（`CameraImageQueue.cpp:34-35`）；
- **消费者阻塞等待**：`Take` 用 `std::condition_variable::wait_for` + 谓词（`CameraImageQueue.cpp:50`）。

> ⚠️ 源工程队列的 Mat 复用存在跨线程覆写隐患，见附录隐患清单 #3。

### 参数系统：JSON 描述 → Model/View/Delegate 动态生成

参数面板不是写死的表单，而是数据驱动的：

```
参数元信息 (CameraParamMetaInfo)
  ├─ HikCamera：38 条硬编码静态表，5 个分组（HikCamera.cpp:184-222）
  └─ VirtualCamera：:/VirtualCameraParam.json 由 ParseUiJson 解析（VirtualCamera.cpp:29-31）
        ▼
CameraParamModel (QAbstractItemModel，按 group 建二级树)
        ▼
CameraParamDelegate::createEditor 按 6 种参数类型动态创建编辑控件
  Int→QSpinBox  Double→QDoubleSpinBox  Enum→QComboBox
  Bool→QCheckBox  Cmd→QPushButton  String→QLineEdit
```

参数可写性由 SDK 访问模式（`MV_XML_GetNodeAccessMode`）决定，只读参数不给 `ItemIsEditable`。

### 观察者模式驱动的控件解耦（与烛照同源）

`Listener` + `ListenerManger` 与烛照的 `ZZListener` 是同一套设计，但**消息枚举修正为 2 的幂**：
6 个消息 `0x01~0x20`（`Listener.h:22-29`），支持按位或组合注册，烛照版的位运算隐患在这里不存在。
控件间不直接通信，统一 `notify` 广播，由注册了对应消息的控件各自响应。

### QSS 全局样式 + .ui 文件（与烛照的关键差异）

烛照全程手写布局；重明反过来：**5 个 .ui 文件** 走 uic 链路，样式集中在
`AppStyle/chongming.qss`（15.7KB 全局样式表）+ Fusion 风格 + 全量 QPalette（`AppStyle.cpp:14-48`），
`MainWindow` 构造时一次性 `AppStyle::Polish()`。

---

## 3. 技术选型

### 3.1 课程环境 vs 本机环境（**复刻必读**）

| 项目 | 课程环境 | 本机环境 | 处理方式 |
|------|---------|---------|---------|
| C++ 标准 | C++11 | **C++17**（Qt 6 最低要求） | `.pro` 改 `CONFIG += c++17` |
| Qt | **5.14.2** | **6.11.2** | 见 3.4「Qt5→Qt6 适配点」（好消息：本工程几乎没有阻塞性 API） |
| 编译器 | VS2019 | **MSVC2022 Kit**（实际工具链 VS2026 / MSVC 14.51） | 沿用 msvc2022_64 Kit |
| OpenCV | 4.5.5（`D:/Opencv/build`，本机无此路径） | **4.6.0**，位于 `D:\QT6\opencv-4.6.0\opencv\build` | 改 `INCLUDEPATH` 与 `LIBS` |
| 海康 MVS SDK | 随工程 `depends/HikCamera`（头文件 + .lib） | **本机已装 MVS**：`D:\Program Files\MVS\Development` | 直接引用安装目录，不拷贝 depends |
| 随附 exe | Qt5 版本（`Qt5Core.dll` 等，源工程 `bin/`） | 需自编译 | — |

### 3.2 本机 MVS SDK 路径（已验证）

| 用途 | 路径 |
|------|------|
| 头文件 | `D:\Program Files\MVS\Development\Includes`（`MvCameraControl.h` 等） |
| 导入库 | `D:\Program Files\MVS\Development\Libraries\win64\MvCameraControl.lib` |
| 运行时 DLL | `C:\Program Files (x86)\Common Files\MVS\Runtime\Win64_x64\MvCameraControl.dll`（MVS 安装器注册的系统运行环境） |
| 环境变量 | `MVCAM_COMMON_RUNENV=D:\Program Files\MVS\Development` |

> ⚠️ MVS 安装目录下**没有** `MvCameraControl.dll`，只有 `.lib`；运行时 DLL 在 `Common Files` 里。
> 若运行时报缺 DLL，从上述 Runtime 目录拷入 `src/bin/`。

### 3.3 界面与控制层设计

- **5 个 .ui 文件**：`mainwindow.ui`（QSplitter 三占位）、`ControlWidget.ui`（4 按钮+相机列表）、
  `ViewWidget.ui`（视图+拉流按钮）、`ParamWidget.ui`（刷新按钮+参数树+描述框）、`LoadingDialog.ui`（GIF 加载框）；
- **MainWindow 组装方式**：三个子界面 `new` 出来后塞进 .ui 三个占位 QWidget 的 layout（`mainwindow.cpp:16-18`）；
- **错误上报**：三路 `SigUpdateErrorInfo` 信号汇集到 MainWindow 弹 `QMessageBox`（`mainwindow.cpp:23-25`）。

### 3.4 Qt5→Qt6 适配点（全工程排查结果，实测很少）

1. **`CONFIG += c++11` → `c++17`**：Qt 6 硬性要求，唯一必改项。
2. **`QT += concurrent` 可删**：代码未用任何 QtConcurrent API（仅 `LoadingDialog.h:13` 残留 include）。
3. **`Q_NULLPTR` → `nullptr`**：散见 `CameraContext.cpp:15-29`、`LoadingDialog.cpp:5,32,46` 等处，Qt6 仍能编译但建议替换。
4. **`BoolCustomWidget.cpp:20` 用了 `checkStateChanged`**：这是 **Qt 6.7+ 的信号**，源工程此文件已被部分 Qt6 化，本机直接可用。
5. **未发现** `QRegExp`、`qrand()`、`QTextStream::setCodec`、`QFontMetrics::width()`、`QDesktopWidget`、`Q_FOREACH` 等典型坑——本工程没有。
6. `.ui` 文件枚举写法新旧混用（`QFrame::StyledPanel` vs `QFrame::Shape::StyledPanel`），Qt6 uic 两种都兼容，无需改。
7. `CMCameraParam.h:227-234` 的 7 个 `Q_DECLARE_METATYPE` 在 Qt6 下可删（自动识别），保留无害。

### 3.5 本机适配清单（`.pro` 相对源工程的改动）

| 原内容 | 现内容 |
|--------|--------|
| `CONFIG += c++11` | `CONFIG += c++17` |
| `QT += core gui concurrent` | `QT += core gui`（删 concurrent） |
| `INCLUDEPATH += D:/Opencv/build/include` | `OPENCV_ROOT = D:/QT6/opencv-4.6.0/opencv/build` → `INCLUDEPATH += $$OPENCV_ROOT/include` |
| `LIBS += -lD:/Opencv/.../opencv_world455d` | `LIBS += -L$$OPENCV_ROOT/x64/vc15/lib -lopencv_world460d`（Debug） |
| `LIBS += -lD:/Opencv/.../opencv_world455` | `LIBS += -L$$OPENCV_ROOT/x64/vc15/lib -lopencv_world460`（Release） |
| `INCLUDEPATH += $$PWD/../../depends/HikCamera/Includes` | `INCLUDEPATH += "D:/Program Files/MVS/Development/Includes"`（含空格路径必须引号包裹） |
| `LIBS += -l$$PWD/../../depends/HikCamera/Libraries/MvCameraControl` | `LIBS += -L"D:/Program Files/MVS/Development/Libraries/win64" -lMvCameraControl` |
| HEADERS 里 `CMCamraMetaInfo.h`（拼写错误，文件不存在） | 修正为 `CMCameraMetaInfo.h` |
| `SOURCES/HEADERS` 一次列全 46 个文件 | **只列已存在的文件，写完一个补一个**（复刻约定） |

其他约定（沿用烛照已验证经验）：

- **DESTDIR = `$$PWD/../bin`**：源工程是 `$$PWD/../../bin`（指向源仓库根的 bin），本工作区目录层级为 `src/ChongMingGUI`，改为 `$$PWD/../bin`；**层级不可挪**；
- 运行时 DLL 放入 `src/bin/`：`opencv_world460d.dll`（Debug）/ `opencv_world460.dll`（Release）。
  **Debug / Release 的 CRT 不通用，两套产物必须各自配对的 DLL**；
- 不需要手写 `QMAKE_CXXFLAGS += /utf-8`：**Qt 6 的 win32-msvc mkspec 默认就会加 `-utf-8`**；
- 无 `TRANSLATIONS`（源工程本来就没有，与烛照不同）。

### 3.6 Qt Creator 侧的三个"非代码"坑（本机实测，同烛照）

1. `.pro` 之外的工程配置写在 `.qtcreator/` 子目录，不是 `.pro.user` 旁边；
2. 「构建和运行」页勾选 Kit 后**必须点页面右下角的 `Configure Project`** 才算提交；
3. 必须先取消 `Hide unsuitable kits`，否则看不到 `Desktop Qt 6.11.2 MSVC2022 64bit`。

> ⚠️ **Kit 只能选 MSVC，不能选 MinGW。**
> 链接的 `.lib`（`opencv_world460(d).lib`、`MvCameraControl.lib`）都是 MSVC 格式。

---

## 4. 测试方案

本项目无自动化测试，验收以**手工验证清单**为主。核心手段是 **VirtualCamera 无相机全流程验收**——
这正是框架自带虚拟相机的价值：没有海康硬件也能把 UI、队列、参数树全部跑通。

### 4.1 分阶段手工验收

每个阶段做完，都要能在 Qt Creator 里构建成功并观察到预期现象（详见第 6 章各步「验收标准」）。
统一前提：构建后 `src/bin/ChongMingGUI.exe` 生成；Debug 构建使用 `opencv_world460d.dll`。

### 4.2 VirtualCamera 验收链路（无硬件）

```
枚举 → 列表出现 "VirtualCamera"（Serial=Vir123456）
连接 → 参数树加载 JSON 描述的参数（:/VirtualCameraParam.json）
拉流 → 视觉窗口出现 512×512 随机纯色图，约 300ms 一帧（VirtualCamera.cpp:84-99）
改参 → 树编辑控件可编辑（假值，readParam 返回硬编码假数据）
断连 → 出图停止，无崩溃无报错弹窗
```

### 4.3 真相机实测（有硬件时，阶段 F）

GigE / USB 海康相机各测一遍：枚举出真实序列号 → 连接 → 拉流看真实画面 →
改曝光/增益立即生效 → 保存配置 `.mfs` → 断连重连 → 导入配置还原。

### 4.4 环境问题排查表

| 现象 | 原因 | 处理 |
|------|------|------|
| 编译通过但 exe 起不来，提示「应用程序控制策略已阻止此文件」 | 本机开启**智能应用控制 (SAC)**，拦截未签名 exe | 事件日志 CodeIntegrity EventID 3077；需放行或关闭 SAC |
| 启动即退出，提示缺 `opencv_world460(d).dll` | `src/bin/` 缺 OpenCV 运行时库 | 从 `D:\QT6\opencv-4.6.0\opencv\build\x64\vc15\bin` 拷对应版本 |
| 启动即退出，提示缺 `MvCameraControl.dll` | 系统运行环境未注册或被清理 | 从 `C:\Program Files (x86)\Common Files\MVS\Runtime\Win64_x64\` 拷入 `src/bin/` |
| 链接报 `cannot find -lMvCameraControl` | 路径含空格未加引号，或误选 MinGW Kit | `.pro` 用引号包裹路径；换回 MSVC Kit |
| 枚举不到真相机 | 网口相机 IP 不在同网段 / USB 驱动未装 | 用 MVS 客户端先确认相机可见 |
| 中文变成乱码 | 源文件编码与编译器解析不一致 | Qt 6 mkspec 已自带 `/utf-8`；若自定义编译参数需自行确认 |

---

## 5. 系统架构与模块设计

### 5.1 整体架构

```
┌──────────────────────────── ChongMingGUI.exe (qmake / Qt6) ────────────────────────────┐
│                                                                                        │
│   MainWindow (.ui + QSplitter)                                                         │
│     ├── ControlWidget ── 枚举/连接/配置导入导出 ──┐                                    │
│     ├── ParamWidget  ── 参数树 (MVD+6种编辑控件) ─┼── ListenerManger (单例广播, 6消息)   │
│     └── ViewWidget   ── GraphicsView + 取图线程 ──┘                                    │
│                          │                                                             │
│                          ▼  只认序列号 QString                                          │
│                  CameraContext (单例, QMap<序列号, CameraInterface*>)                   │
│                          │                                                             │
│              ┌───────────┴────────────┐                                                │
│              ▼                        ▼                                                │
│        HikCamera (MVS SDK)      VirtualCamera (JSON+d随机图)                            │
│              │                        ▲                                                │
│              │                   ParseUiJson (:/VirtualCameraParam.json)                │
│              ▼                                                                         │
│   CameraImageQueue (双队列, mutex+CV, 深度10)  ◀── SDK 回调线程 Put                      │
│              ▲                                                                         │
│   AcquireImageProcess (QThread) ──Take──► cvMat2QImage ──emit──► GraphicsView          │
└────────────────────────────────────────────────────────────────────────────────────────┘
                          │
                          ▼
        MvCameraControl.dll (海康 MVS SDK, 方案C: 引用 D:\Program Files\MVS)
```

**唯一的跨进程边界**是 SDK 句柄 `void* m_cameraHandle`；UI 完全不感知 SDK 类型。

### 5.2 目录结构

```
<repo root>/
├─ .gitignore
├─ DEV_SPEC.md                     ← 本文档
├─ SPRINT.md                       ← 迭代执行清单
├─ .skills/                        ← project-learner（重明版学习教练）
└─ src/
   ├─ ChongMingGUI/                ← 前端工程（qmake）
   │  ├─ ChongMingGUI.pro
   │  ├─ main.cpp
   │  ├─ mainwindow.h / .cpp / .ui
   │  ├─ favicon.ico               ← exe 与窗口图标（RC_ICONS）
   │  ├─ Listener.h / .cpp         ← 观察者：MESSAGE 枚举 + ListenerManger
   │  ├─ AppStyle/                 ← AppStyle.h/.cpp + chongming.qss + style.qrc
   │  ├─ Icon/                     ← 13 个图标/图片 + Icon.qrc
   │  ├─ Resource/                 ← VirtualCameraParam.json + resource.qrc
   │  ├─ Utils/                    ← ImageConver.h（cv::Mat↔QImage）
   │  ├─ CameraInterface/          ← 抽象层：CameraInterface / CameraImageQueue /
   │  │                              CameraContext / CameraError / CMCameraMetaInfo / CMCameraParam
   │  ├─ CameraFactory/            ← CameraFactory / HikCamera / VirtualCamera
   │  ├─ ParseUiJson/              ← 参数 JSON 解析单例
   │  ├─ ControlWidget/            ← 控制条（.ui）
   │  ├─ ParamWidget/              ← 参数树（.ui）+ Model/Item/Delegate + CustomWidget/ ×6
   │  ├─ ViewWidget/               ← 视觉窗口（.ui）+ GraphicsView + ImageItem + AcquireImageProcess
   │  └─ LoadingDialog/            ← 加载动画框（.ui）
   └─ bin/                         ← DESTDIR：编译产物与运行资源
      └─ opencv_world460(d).dll
```

### 5.3 模块说明

| 模块 | 类 | 职责 | 关键点（源工程参照位置） |
|------|----|------|------------------------|
| 程序入口 | `main.cpp` | QApplication + MainWindow，极简 | `main.cpp:6-9` |
| 主窗口 | `MainWindow` | 组装三子界面、错误弹窗、样式装载 | `mainwindow.cpp:6-32` |
| 观察者 | `Listener` / `ListenerManger` | 6 消息位掩码注册与广播 | `Listener.h:22-29`（2 的幂，无烛照版隐患） |
| 样式 | `AppStyle` | Fusion + QPalette + 全局 QSS | `AppStyle.cpp:14-59` |
| 抽象接口 | `CameraInterface` | 17 个纯虚函数定义相机全能力 | `CameraInterface.h:37-69` |
| 图像队列 | `CameraImageQueue` | 双队列生产者-消费者缓冲 | `CameraImageQueue.cpp:24-60` |
| 上下文 | `CameraContext` | 单例，序列号→相机路由，接口转发 | `CameraContext.cpp:48-292` |
| 错误码 | `CAMERAERROR` 命名空间 | 16 个宏错误码 + 英文翻译 | `CameraError.h:10-27` |
| 参数抽象 | `CameraParam` + CMParam 族 | 6 种参数类型的值包装 | `CMCameraParam.h` |
| 工厂 | `CameraFactory` | 单例 + `registerCamera<T>` 注册表 | `CameraFactory.cpp:11-36` |
| 海康实现 | `HikCamera` | MVS SDK 全封装：枚举/连接/回调/参数 | `HikCamera.cpp:157-585` |
| 虚拟相机 | `VirtualCamera` | 无硬件模拟：随机图 + JSON 参数 | `VirtualCamera.cpp:23-172` |
| JSON 解析 | `ParseUiJson` | 参数描述 JSON → CameraParamMetaInfo | `ParseUiJson.cpp:5-223` |
| 控制条 | `ControlWidget` | 枚举/连接/切相机/配置导入导出 | `ControlWidget.cpp:38-146` |
| 参数树 | `ParamWidget` + Model/Item/Delegate | 数据驱动参数面板 | `CameraParamDelegate.cpp:25-42` |
| 自定义控件 | OneCustomWidget ×6 | Int/Double/Enum/Bool/Cmd/String 编辑器 | `ParamWidget/CustomWidget/` |
| 视觉窗口 | `GraphicsView` / `ImageItem` | 缩放/居中/棋盘格/悬停 RGB | `GraphicsView.cpp:74-195` |
| 取图线程 | `AcquireImageProcess` | QThread 循环 Take → emit QImage | `AcquireImageProcess.cpp:19-30` |
| 加载框 | `LoadingDialog` | GIF 模态加载动画 | `LoadingDialog.cpp:30-48` |

### 5.4 消息流与数据流

**主链路 a（枚举→连接→拉流→显示）：**

```
ControlWidget::on_Enumeration_Button_clicked
  → CameraContext::EnumerationCamera → VirtualCamera::EnumCamera + HikCamera::EnumCamera
  → CameraFactory::createCamera（按 VenderName 建对象入 m_serialCamMap）
  → notify(CAMERA_ENUMRTION)
ControlWidget::on_Connect_Button_toggled
  → CameraContext::connect（内部先 acquire 建句柄，再 connect 开设备）
  → notify(CAMERA_CONNECT)
ViewWidget::on_Grabbing_Button_toggled
  → CameraContext::startGrabbing（内部先 creatStream 注册回调，再 startGrabbing）
  → AcquireImageProcess::start()
  → notify(CAMERA_STARTGRAB)
```

**主链路 b（参数树读/写）：**

```
读：ParamWidget::RespondMessage(CAMERA_CONNECT)
  → CameraContext::getParamList（拿元信息表 + 逐项 readParam 刷当前值）
  → CameraParamModel::addCameraParam（按 group 建二级树）
写：编辑器值变 → OneCustomWidget::sigValueChanged
  → CameraParamDelegate::onValueChanged → model->setData(ParamRole)
  → model 发 SigValueChanged → ParamWidget::writeCameraParam
  → CameraContext::writeParam → HikCamera::writeParam（按 6 类型分发 MV_CC_Set*）
```

**主链路 c（SDK 回调 → 屏幕，跨三线程）：**

```
[SDK 回调线程] MV_CC_RegisterImageCallBackEx → ImageCallBack
  → HikConvert2Mat（MV_CC_ConvertPixelType 像素转换）→ m_imageQueue.Put
[取图线程]     AcquireImageProcess::run
  → CameraContext::getImageLast（Take + cvMat2QImage）→ emit sigUpdateImage(QImage)
[UI 线程]      GraphicsView::SetImage → QPixmap::fromImage → setPixmap → fitFrame
```

### 5.5 资源与国际化

| 资源 | qrc | 内容 |
|------|-----|------|
| `AppStyle/style.qrc` | prefix `/` | `chongming.qss`（15.7KB 全局样式表） |
| `Icon/Icon.qrc` | prefix `/` | 13 项：枚举/连接/断连/拉流/停流/刷新/导入/导出/关闭图标 + loading.gif + zhouxuan.jpg + favicon.ico + icon.png |
| `Resource/resource.qrc` | prefix `/` | `VirtualCameraParam.json`（虚拟相机参数面板描述） |
| `favicon.ico` | 工程根 | `RC_ICONS` 作 exe 图标；qrc 内副本 `:/favicon.ico` 作窗口图标 |

本工程**无国际化**（无 .ts/.qm、无 tr() 包裹要求），与烛照不同。

---

## 6. 项目排期

### 阶段总览

| 阶段 | 目的 | 步骤数 | 状态 |
|------|------|--------|------|
| **A** 工程骨架与构建基座 | 先能编译、能跑出空窗口 | 4 | ✅ 已完成 |
| **B** 前端地基：观察者 + 样式 + 图像转换 | 建立全局通信、样式系统与工具函数 | 4 | ✅ 已完成 |
| **C** 相机抽象层 | 接口/队列/上下文/错误码/参数抽象 | 6 | ⬜ 待开始 |
| **D** 相机实现层 | JSON 解析 + 虚拟相机 + 海康封装 + 工厂 | 5 | ⬜ 待开始 |
| **E** 界面层 | 参数树 + 视觉窗口 + 控制条 + 主窗口组装 | 7 | ⬜ 待开始 |
| **F** 端到端联调 | VirtualCamera 全流程 + 真相机实测 | 3 | ⬜ 待开始 |

> 总步数 29。每一步都可在 30~90 分钟内完成并单独验证——**做完一步就构建一次**，
> 不要攒着写，否则排错成本会成倍上升。
>
> **依赖说明**：`VirtualCamera` 依赖 `ParseUiJson`（读参数 JSON），故 ParseUiJson 提前到阶段 D 开头；
> `HikCamera` 编译依赖 MVS SDK 头文件与 .lib（阶段 A 已在 .pro 配好，随时可写）。

### 📊 进度跟踪表

#### 阶段 A：工程骨架与构建基座 ✅

- [x] **A1** 建立目录层级（`src/ChongMingGUI` + `src/bin`）与 `.gitignore`
- [x] **A2** `.pro` 适配本机环境（C++17 / OpenCV 4.6.0 / MVS SDK 路径 / DESTDIR / 删 concurrent / 修 `CMCamraMetaInfo` 拼写）
- [x] **A3** 拷贝资源（favicon.ico、AppStyle/、Icon/、Resource/ 三个 qrc 全部内容）
- [x] **A4** `main.cpp` + `mainwindow` 骨架，首次编译产出 `src/bin/ChongMingGUI.exe`，跑出空窗口

> 验收标准：Qt Creator 配好 MSVC Kit 后 `Ctrl+B` 成功，运行出现空白主窗口（带 favicon 图标）。

#### 阶段 B：前端地基：观察者 + 样式 + 图像转换 ✅

- [x] **B1** `Listener.h` —— `MESSAGE` 枚举 6 个 2 的幂位值 + `Listener` 抽象基类
- [x] **B2** `Listener.cpp` —— `ListenerManger` 单例（改 Meyers 实现修泄漏）、`registerMessage()` 位拆包注册（已补 `CAMERA_CAMERASWICH` 分支 + 注册去重）、`notify()` 广播
- [x] **B3** `AppStyle` —— Fusion + QPalette + 读 `:/chongming.qss` 全局装载，`MainWindow` 接入
- [x] **B4** `Utils/ImageConver.h` —— `cvMat2QImage` / `QImage2cvMat`（8UC1/8UC3/8UC4 三分支，`static`→`inline`，去热路径日志）

> 验收标准：主窗口套用 chongming.qss 全局样式；`main.cpp` 里 `qDebug` 一条消息能被广播链路接收。

#### 阶段 C：相机抽象层

- [ ] **C1** `CameraError.h` —— 16 个错误码宏 + `getErrorInfoEn()`（**加 `inline` 或拆 .cpp**，修源工程 ODR 隐患）
- [ ] **C2** `CMCameraMetaInfo.h` + `CMCameraParam.h` —— 元信息三元组与 6 类型参数值包装
- [ ] **C3** `CameraImageQueue` —— 双队列 + mutex/CV + 满丢最旧帧 + Take 超时（**修复源工程超时返回成功码、Mat 覆写两个隐患**）
- [ ] **C4** `CameraInterface.h` —— 17 个纯虚函数 + `ImageQueue()`/`UserName()`/`Serial()` 默认实现
- [ ] **C5** `CameraContext` —— 单例 + `m_serialCamMap` 路由 + 全部接口转发 + `CHECK_RETURN` 宏
- [ ] **C6** 抽象层自测：写一个栈上假实现类跑通 `CameraContext` 路由（临时代码，测完即删）

> 验收标准：抽象层独立编译通过；自测 main 能对假相机完成 Enumeration→connect→getParamList 调用序列。

#### 阶段 D：相机实现层

- [ ] **D1** `ParseUiJson` —— 单例解析 `:/VirtualCameraParam.json` → `QList<CameraParamMetaInfo>`（group/params 二级结构）
- [ ] **D2** `VirtualCamera` —— 固定 1 台假相机、出图线程（**修复 detached 线程隐患：改为可 join 的成员线程 + 原子停止标志**）、假参数读写
- [ ] **D3** `HikCamera` 前半 —— 枚举（GigE/USB 双分支）、acquire/release、connect/disconnect（**修复 USB 相机取错序列号隐患**）
- [ ] **D4** `HikCamera` 后半 —— creatStream/destroyStream（回调注册）、拉流启停、6 类型参数读写、`MV_CC_FeatureLoad/Save` 配置
- [ ] **D5** `CameraFactory` —— 单例 + 注册表，`HikCamera`→"Hikrobot"、`VirtualCamera`→"Virtual"

> 验收标准：临时 main 调 `CameraFactory::instance()->createCamera` 能建出两种相机；VirtualCamera 枚举/连接/拉流出随机图；有真相机时 HikCamera 枚举出真实序列号。

#### 阶段 E：界面层

- [ ] **E1** `ControlWidget` —— .ui + 4 按钮 + 相机列表 + 配置导入导出（QFileDialog + `configFormat` 过滤）
- [ ] **E2** `CameraParamModel/Item/Delegate` —— 二级树 Model、角色定义、按类型 createEditor、自绘网格线
- [ ] **E3** 6 种 CustomWidget —— Int/Double/Enum/Bool/Cmd/String（**修复源工程 Bool 控件双 bug、Enum 用索引当值两个隐患**）
- [ ] **E4** `ParamWidget` —— .ui + 树 + 描述框 + 读写串联（**修复空选择崩溃隐患**）
- [ ] **E5** `GraphicsView` + `ImageItem` —— 缩放/双击居中/棋盘格/悬停 RGB
- [ ] **E6** `ViewWidget` + `AcquireImageProcess` —— 拉流按钮 + 取图线程（**修复线程永不退出隐患：加退出标志 + 队列唤醒**）
- [ ] **E7** `MainWindow` + `LoadingDialog` —— 三子界面组装、错误弹窗、`AppStyle::Polish()`

> 验收标准：完整界面出现；VirtualCamera 全链路（枚举→连接→参数树加载→拉流出图→改参→断连）在 UI 上跑通。

#### 阶段 F：端到端联调

- [ ] **F1** VirtualCamera 全流程走查（按 §4.2 清单逐项过）
- [ ] **F2** 真相机实测（按 §4.3 清单逐项过，无硬件则跳过并在 SPRINT 标注）
- [ ] **F3** 断连/重连/切相机/异常路径压力手测（拔线、拉流中断连、重复点按钮）

> 验收标准：UI 全流程与源工程行为一致；无崩溃、无报错弹窗误报；日志/错误码路径符合预期。

---

## 7. 可扩展性与未来展望

| 方向 | 说明 |
|------|------|
| **新品牌相机** | 框架的立身之本：继承 `CameraInterface` + `registerCamera<T>` 一行注册，可接大华/巴斯勒/迈德威视 |
| **参数表外置** | HikCamera 的 38 条参数目前硬编码，可像 VirtualCamera 一样 JSON 化，甚至直接解析相机 XML 节点树全量生成 |
| **图像录制/回放** | 在 `CameraImageQueue` 消费侧加落盘分支，支持录像与离线回放调试 |
| **图像算法挂接** | `AcquireImageProcess` 是天然算法挂点：取图后先过 OpenCV 处理再显示（对接烛照的光度立体） |
| **性能优化** | 队列零拷贝（Mat 池引用计数）、回调线程直转 QImage、显示帧率限制 |
| **多相机同时拉流** | `CameraContext` 的 map 结构已支持多相机，UI 需加多视图与相机切换逻辑 |
| **工程化** | 引入 gtest 对队列/参数解析做单元测试；CI 构建；SDK 依赖包管理 |

---

## 附：源工程已知代码隐患清单（复刻时建议修正）

复刻不是照抄，以下问题在源工程中真实存在（均已定位 file:line），写自己的版本时避开。

| # | 位置 | 问题 | 建议 |
|---|------|------|------|
| 1 | `AcquireImageProcess.cpp:21` | 取图线程 `while(true)` 无退出标志，`quit()` 无效、`wait()` 被注释；停流后线程阻塞在 Take 死转，断连后仍持失效相机指针 | 加原子退出标志 + 队列 `notify_all` 唤醒；断连时先停线程再释放相机 |
| 2 | `CameraImageQueue.cpp:54-56` | Take 超时/空队列返回 `CHONGMING_OK` 且不写 m，上层拿不到 `GETIAMGE_TIMEOUT`，把空 Mat 转图继续显示；`m_needStop` 定义了却无接口置位（死代码） | 超时返回超时错误码；补 stop() 接口置 `m_needStop` 并 `notify_all` |
| 3 | `CameraImageQueue.cpp:29,58-60` | Take 把刚 pop 的 Mat 又 push 回 freeImageQueue，cv::Mat 浅拷贝，之后 Put 的 `temp = m` 可能覆写已交付消费者的图像——跨线程数据竞争 | 交付前 `clone()`，或改为帧对象所有权转移 |
| 4 | `HikCamera.cpp:85,104-106` | `HikConvert2Mat` 每帧 `malloc` 从不 free，失败路径也漏 free——每帧泄漏 W×H×3 字节 | 用 RAII（unique_ptr/vector）管理转换缓冲，或复用预分配缓冲 |
| 5 | `BoolCustomWidget.cpp:12,20,36` | 双重 bug：disconnect 了两个信号但**从未 connect**，勾选不触发回写；`onValueChanged` 对 BOOL 参数取了 `IntParam` | 正确 connect；按参数类型取对应值 |
| 6 | `IntCustomWidget.cpp:17-20` | QSpinBox 是 int32，`int64_t` 的 min/max/value 直接传入，海康 INT 节点超 int32 范围溢出 | 用 QLineEdit+校验器，或范围钳制并提示 |
| 7 | `EnumCustomWidget.cpp:39` + `HikCamera.cpp:541-542` | 枚举写入用 `currentIndex` 当值，真实值在 `availableInt`，索引≠值时写错参数 | 存取都用枚举项真实值 |
| 8 | `HikCamera.h:16` + `CameraFactory.cpp:17-18` | `HikCamera::VIRTUAL_CAMERA_VENDER`（值 "Hikrobot"）常量名与 VirtualCamera 同名常量混淆，读起来像注册错对象 | 改名 `HIK_CAMERA_VENDER` |
| 9 | `Listener.cpp:43-57` | `registerMessage` 漏了 `CAMERA_CAMERASWICH` 分支，注册永远落空；全工程也无人 notify——死事件 | 补分支 + 在切换相机处 notify，或删除该消息 |
| 10 | `Listener.cpp:4`、`CameraContext.cpp:37-46` | `ListenerManger` 单例 new 后永不 delete；`~CameraContext` 不 delete 相机指针，相机对象全泄漏 | 单例用静态局部变量；上下文析构遍历 delete |
| 11 | `VirtualCamera.cpp:103-104` | detached 线程持 `this`，对象析构后线程仍访问——use-after-free；`m_starGrabbing` 普通 bool 跨线程读写 | 成员 `std::thread` + `stopGrabbing` join；标志改 `std::atomic<bool>` |
| 12 | `VirtualCamera.cpp:23` + `CMCameraMetaInfo.h:11-14` | 聚合初始化 `{NAME, SERIAL, VENDER}` 与声明顺序错位，Serial 被赋成 "VirtualCamera"——语义颠倒但各处混用反而"自洽" | 显式指定初始化（C++20）或按声明顺序赋值 |
| 13 | `HikCamera.cpp:259-261` | `acquire` 无条件读 GigE 分支序列号，USB 相机永远匹配失败；还有 int/unsigned 比较 | 按设备类型分支取序列号 |
| 14 | `CameraParamModel.cpp:174-180` | `clear()` 直接 delete 根节点不发 `begin/endResetModel`，视图 QModelIndex 悬空 | 用 resetModel 包裹 |
| 15 | `CameraContext.h:24-29` | `CHECK_RETURN` 宏内含 `emit`，定义在非 QObject 的头文件里，语义耦合；三个控件各自定义同名信号迁就它 | 宏改为返回错误码，emit 挪回调用处 |
| 16 | `HikCamera.cpp:158-160` | EnumCamera 失败返回 `CHONGMING_OK`，调用方无法区分"无相机"与"枚举失败" | 失败返回真实错误码 |
| 17 | `GraphicsView.cpp:34-35,41` | 析构先 `deleteLater` scene 再 `delete` ImageItem，而 Item 已 addItem 进 scene——双重所有权 | 统一交给 scene 父子树管理 |
| 18 | `HikCamera.cpp:429` 等 | `readParam/writeParam/getFeatureAccessMode` 不做 `m_cameraHandle` 判空（与 connect 等函数风格不一致），句柄为空时调 SDK 会崩 | 统一入口判空 |
| 19 | `ParamWidget.cpp:129` | `selected.indexes().first()` 未判空，清空选择时越界崩溃 | 先判 `isEmpty()` |
| 20 | `CameraError.h:12-26` | 16 个错误码是序数（1,2,3…）非位掩码，命名风格易误导为可组合 | 注释明确"互斥序数"，或改枚举类 |
