#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H
/**************************************************
 * LoadingDialog加载提示
 * 可以弹窗来阻塞主线程，等待后台执行某一费时操作结束，例如相机枚举
 *
 * 复刻说明：去掉源工程残留的 `#include <QtConcurrent>`（全工程未用 QtConcurrent，
 * Qt6 下 concurrent 模块已从 .pro 移除）；Q_NULLPTR 改 nullptr。
 **************************************************/

#include <QDialog>
#include <QMovie>
#include <QThread>
#include <QTimer>

namespace Ui {
class LoadingDialog;
}

class LoadingDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoadingDialog(QWidget* parent = nullptr);
    ~LoadingDialog();

    // 显示loading窗
    static void Loading(QWidget* parent = nullptr);
    // 隐藏loading窗
    static void HideLoading();

private slots:
    void on_Close_Button_clicked();

private:
    Ui::LoadingDialog* ui;
    QMovie* m_pLoadingMovie;
};

#endif // LOADINGDIALOG_H
