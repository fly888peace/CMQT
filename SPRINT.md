# 重明 ChongMingGUI 复刻 · 迭代执行清单

> 与 `DEV_SPEC.md` §6 阶段排期一一对应。每步标注：输入（参照源工程位置）→ 产物 → 验证动作。
> 源工程：`D:\QT6\000workspace\chongming-V2.1.0.0\src\ChongMingGUI\`（只读）。
> 铁律：**做完一步就构建一次**（`Ctrl+B`），不要攒着写。
> Git：每步一个 commit，conventional commits（参照烛照风格），建议信息见各步末尾 `git:` 行。

---

## 阶段 A：工程骨架与构建基座 ✅（2026-09-26 完成）

| 步 | 任务 | 产物 | 验证 | git |
|---|------|------|------|-----|
| A1 | 目录层级 + .gitignore | `src/ChongMingGUI/`、`src/bin/`、两个 `.gitignore` | 目录就位 | `chore: 添加 .gitignore 与目录骨架` |
| A2 | `.pro` 本机适配 | `ChongMingGUI.pro`（C++17/OpenCV4.6.0/MVS SDK/DESTDIR） | qmake 无报错 | （并入 A4） |
| A3 | 拷贝资源 | `favicon.ico`、`AppStyle/`、`Icon/`、`Resource/` | 3 个 qrc 引用文件齐全 | （并入 A4） |
| A4 | main + mainwindow 骨架 | `main.cpp`、`mainwindow.h/.cpp/.ui` → `src/bin/ChongMingGUI.exe` | ✅ 已实测：编译通过、空窗口跑起来（windeployqt 部署 Qt6 运行时后） | `feat: 按课程布局搭建 src/ 骨架并适配本机环境` |

---

## 阶段 B：前端地基（观察者 + 样式 + 图像转换）

| 步 | 任务 | 输入（参照） | 产物 | 验证 | git |
|---|------|------------|------|------|-----|
| B1 | Listener.h | `Listener.h:22-35` | `Listener.h`：6 消息位枚举 + 抽象基类 | 编译过 | `feat(observer): 定义消息枚举与 Listener 抽象基类` |
| B2 | Listener.cpp | `Listener.cpp:4-58` | `ListenerManger` 单例 + 注册/广播（**补 `CAMERA_CAMERASWICH` 分支**） | 注册+notify 自测 | `feat(observer): 实现 ListenerManger 事件总管` |
| B3 | AppStyle | `AppStyle/`（已拷） | `AppStyle.cpp` 接入 `MainWindow` | 窗口套用 QSS | `feat(style): 接入 Fusion+QSS 全局样式` |
| B4 | ImageConver.h | `Utils/ImageConver.h` | cv::Mat↔QImage 双向转换 | 单测一张图往返 | `feat(utils): 新增 cv::Mat 与 QImage 互转工具` |

---

## 阶段 C：相机抽象层

| 步 | 任务 | 输入（参照） | 产物 | 验证 | git |
|---|------|------------|------|------|-----|
| C1 | CameraError.h | `CameraInterface/CameraError.h:10-103` | 16 错误码宏 + `getErrorInfoEn()`（**加 inline**） | 编译过 | `feat(camera): 定义相机错误码与翻译函数` |
| C2 | 元信息+参数抽象 | `CMCameraMetaInfo.h`、`CMCameraParam.h` | 三元组 + 6 类型值包装 | 编译过 | `feat(camera): 新增相机元信息与参数值包装` |
| C3 | CameraImageQueue | `CameraImageQueue.h/.cpp` | 双队列生产者消费者（**修超时返回码 + Mat 覆写 + 补 stop**） | 多线程 Put/Take 自测 | `feat(camera): 实现线程安全图像队列` |
| C4 | CameraInterface.h | `CameraInterface.h:37-79` | 17 纯虚函数 + 3 默认实现 | 编译过 | `feat(camera): 定义相机抽象接口` |
| C5 | CameraContext | `CameraContext.h/.cpp` | 单例 + 序列号路由 + 接口转发 | 编译过 | `feat(camera): 实现 CameraContext 单例路由` |
| C6 | 抽象层自测 | — | 临时 main + 假实现类（测完删） | Enumeration→connect→getParamList 走通 | （临时，不提交） |

---

## 阶段 D：相机实现层

| 步 | 任务 | 输入（参照） | 产物 | 验证 | git |
|---|------|------------|------|------|-----|
| D1 | ParseUiJson | `ParseUiJson/` + `:/VirtualCameraParam.json` | JSON→CameraParamMetaInfo 单例 | 解析出全部参数项 | `feat(camera): 实现参数 JSON 解析器` |
| D2 | VirtualCamera | `CameraFactory/VirtualCamera.cpp:23-172` | 假相机（**detached 线程改成员线程+atomic 标志**） | 枚举出 Vir123456、出随机图 | `feat(camera): 实现 VirtualCamera 模拟相机` |
| D3 | HikCamera 前半 | `HikCamera.cpp:144-330` | 枚举/acquire/connect/disconnect（**修 USB 序列号**） | 真相机枚举出序列号（无硬件跳过） | `feat(camera): 实现海康相机枚举与连接` |
| D4 | HikCamera 后半 | `HikCamera.cpp:331-585` | 拉流/回调/6 类型参数读写/配置导入导出 | 编译过+句柄判空齐全 | `feat(camera): 实现海康拉流与参数读写` |
| D5 | CameraFactory | `CameraFactory.h/.cpp` | 注册表工厂（**改 `HIK_CAMERA_VENDER` 命名**） | createCamera 建出两种相机 | `feat(camera): 实现相机品牌工厂` |

---

## 阶段 E：界面层

| 步 | 任务 | 输入（参照） | 产物 | 验证 | git |
|---|------|------------|------|------|-----|
| E1 | ControlWidget | `ControlWidget/`（.ui + cpp:38-146） | 控制条 4 按钮 + 相机列表 | 枚举按钮刷新列表 | `feat(gui): 实现相机控制条` |
| E2 | 参数 MVD 三件套 | `ParamWidget/CameraParam{Model,Item,Delegate}.*` | 二级树 Model + 按类型 createEditor | 编译过 | `feat(gui): 实现参数树 Model/View/Delegate` |
| E3 | 6 种 CustomWidget | `ParamWidget/CustomWidget/` | Int/Double/Enum/Bool/Cmd/String（**修 Bool 双 bug、Enum 索引当值**） | 每种控件能编辑回写 | `feat(gui): 实现 6 种参数编辑控件` |
| E4 | ParamWidget | `ParamWidget/`（.ui + cpp:58-137） | 参数树面板（**修空选择崩溃**） | 连接后树加载参数 | `feat(gui): 实现参数树面板` |
| E5 | GraphicsView+ImageItem | `ViewWidget/GraphicsView.*`、`ImageItem.*` | 缩放/居中/棋盘格/悬停 RGB | 显示一张测试图 | `feat(gui): 实现视觉窗口三件套` |
| E6 | ViewWidget+取图线程 | `ViewWidget/`、`AcquireImageProcess.*` | 拉流按钮 + 取图线程（**修线程永不退出**） | 拉流出图、停流线程退 | `feat(gui): 打通取图显示链路` |
| E7 | MainWindow+LoadingDialog | `mainwindow.cpp:6-46`、`LoadingDialog/` | 三子界面组装 + 错误弹窗 + 加载框 | 完整界面出现 | `feat(gui): 组装主窗口与加载对话框` |

---

## 阶段 F：端到端联调

| 步 | 任务 | 验证 | git |
|---|------|------|-----|
| F1 | VirtualCamera 全流程走查 | DEV_SPEC §4.2 五项全过 | `test: VirtualCamera 全流程验收` |
| F2 | 真相机实测 | DEV_SPEC §4.3（无硬件则跳过并在此标注） | `test: 真相机 GigE/USB 实测` |
| F3 | 异常路径手测 | 拔线/拉流中断连/重复点按钮无崩溃 | `fix: 联调暴露的问题修复` |

---

## 当前进度

- ✅ 阶段 A（4/4）
- ⬜ 阶段 B（0/4）→ **下一步：B1**
- ⬜ 阶段 C（0/6）
- ⬜ 阶段 D（0/5）
- ⬜ 阶段 E（0/7）
- ⬜ 阶段 F（0/3）
