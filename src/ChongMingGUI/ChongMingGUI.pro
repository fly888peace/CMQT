# ==========================================================
# 重明 ChongMingGUI 复刻工程（本机环境适配版）
# 相对源工程 D:\QT6\000workspace\chongming-V2.1.0.0 的改动见 DEV_SPEC.md §3.5
# ==========================================================

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Qt 6 最低要求 C++17（源工程是 c++11）
CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

# 源文件：复刻约定——只列已存在的文件，写完一个补一个
SOURCES += \
    main.cpp \
    mainwindow.cpp

# 头文件：同上（源工程这里的 CMCamraMetaInfo.h 是拼写错误，复刻时修正）
HEADERS += \
    Listener.h \
    mainwindow.h

# 界面ui文件
FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 配置生成产物的目录文件夹（源工程是 $$PWD/../../bin 指向源仓库根，本工作区层级为 src/ChongMingGUI）
# 层级不可挪：$$PWD/../bin 与 src/bin/.gitignore 一一对应
DESTDIR = $$PWD/../bin

# 配置opencv库（本机 4.6.0，源工程是 D:/Opencv/build 下的 4.5.5）
OPENCV_ROOT = D:/QT6/opencv-4.6.0/opencv/build
INCLUDEPATH += $$OPENCV_ROOT/include
Debug: {
LIBS += -L$$OPENCV_ROOT/x64/vc15/lib -lopencv_world460d
}
Release: {
LIBS += -L$$OPENCV_ROOT/x64/vc15/lib -lopencv_world460
}

# 配置海康工业相机SDK二次开发库（本机已装 MVS，直接引用安装目录；路径含空格必须引号包裹）
# 头文件:  D:\Program Files\MVS\Development\Includes
# 导入库:  D:\Program Files\MVS\Development\Libraries\win64\MvCameraControl.lib
# 运行时:  C:\Program Files (x86)\Common Files\MVS\Runtime\Win64_x64\MvCameraControl.dll
INCLUDEPATH += "D:/Program Files/MVS/Development/Includes"
LIBS += -L"D:/Program Files/MVS/Development/Libraries/win64" -lMvCameraControl

# 设置图标
RC_ICONS = favicon.ico

RESOURCES += \
    AppStyle/style.qrc \
    Icon/Icon.qrc \
    Resource/resource.qrc
