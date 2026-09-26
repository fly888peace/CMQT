#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/**************************************************
 * MainWindow主界面
 * 主界面仅需要做子界面的加载和布局即可，功能实现都集成在子界面中。
 * 所以主界面MainWindow基本没有什么代码。
 *
 * 阶段 A：仅骨架（.ui 三占位 + 状态栏错误标签）。
 * 子界面 ControlWidget / ParamWidget / ViewWidget 在阶段 E 接入。
 **************************************************/

#include <QLabel>
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void OnUpdateErrorInfo(QString strErrorInfo);

private:
    Ui::MainWindow* ui;
    QLabel* m_pErrorInfoLabel; // 错误信息显示
};
#endif // MAINWINDOW_H
