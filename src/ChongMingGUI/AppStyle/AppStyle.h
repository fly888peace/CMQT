#ifndef APPSTYLE_H
#define APPSTYLE_H
/**************************************************
 * 样式表皮肤类，该类设置了重明软件的调色板和样式表qss
 * 皮肤的实现，通过设置应用程序的 QPalette 调色板以及
 * 控件的qss样式表，来实现界面的美化。
 *
 * 不了解qt调色板和qss的，自行百度
 **************************************************/

#include <QApplication>

class AppStyle {
public:
    AppStyle() { };
    // 应用样式
    static void Polish();

private:
    // 设置调色板
    static void setPalette();
    // 设置qss
    static void setQss();
};

#endif // APPSTYLE_H
