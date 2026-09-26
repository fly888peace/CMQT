#ifndef ACQUIREIMAGEPROCESS_H
#define ACQUIREIMAGEPROCESS_H
/**************************************************
 * 取图线程，当开启拉流时，ViewWidget会开启该子线程进行取图
 * 通过CameraContext::Instance()单例来获取图像，并将图像发送给主线程来更新
 * 视觉窗口。
 * 采用子线程采图的原因是，如果你将采图线程放到主线程，则一旦开启拉流采图，
 * 出图功能就会将主线程卡住，从而在出图时，重明软件的主界面将无法响应其它事件，等同卡死
 *
 * 复刻修正（DEV_SPEC 附录 #1）：run() 是 while(true) 且没跑 exec()，
 * 源工程的 quit() 根本叫不停它（quit 只退事件循环，而这里没有事件循环）。
 * 正确做法：原子停止标志 + 队列 Stop 唤醒 Take + 调用方 wait()。
 **************************************************/

#include <QImage>
#include <QObject>
#include <QString>
#include <QThread>
#include <atomic>

class AcquireImageProcess : public QThread {
    Q_OBJECT
public:
    explicit AcquireImageProcess(QObject* parent = nullptr);
    ~AcquireImageProcess();

    // 设置取图相机序列号
    void setSerial(QString serial);
    // 请求线程退出（需配合 wait() 等待真正退出）
    void requestStop();

signals:
    void sigUpdateImage(const QImage& iamge);
    // 连续取图超时报错（如拉流中拔网线），由 ViewWidget 转发给 MainWindow 弹窗
    void sigErrorInfo(QString info);

protected:
    void run() override;

private:
    QString m_serial {};
    std::atomic<bool> m_needStop { false };
};

#endif // ACQUIREIMAGEPROCESS_H
