#include "AcquireImageProcess.h"
#include "CameraInterface/CameraContext.h"
#include "CameraInterface/CameraError.h"

AcquireImageProcess::AcquireImageProcess(QObject* parent)
    : QThread(parent)
{
}

AcquireImageProcess::~AcquireImageProcess()
{
    // 兜底：析构前确保线程已退出
    requestStop();
    wait(3000);
}

void AcquireImageProcess::setSerial(QString serial)
{
    m_serial = serial;
}

void AcquireImageProcess::requestStop()
{
    m_needStop = true;
}

void AcquireImageProcess::run()
{
    m_needStop = false;
    int timeoutCount = 0; // 连续超时计数（拉流中拔网线场景：SDK 不再出图，队列一直空）
    bool timeoutReported = false;
    while (!m_needStop) {
        QImage image;
        auto ret = CameraContext::Instance()->getImageLast(m_serial, image);
        if (ret == GETIAMGE_TIMEOUT) { // 采图超时
            // 复刻增补：连续 3 次超时（约 15 秒）向 UI 报一次错，
            // 源工程静默 continue，用户拔了网线都毫无感知
            if (++timeoutCount >= 3 && !timeoutReported) {
                timeoutReported = true;
                emit sigErrorInfo(getErrorInfoEn(GETIAMGE_TIMEOUT));
            }
            continue;
        }
        if (ret == CAMERA_QUEUE_STOPPED) // 队列被停（停流/断连），退出线程
            break;
        if (ret != CHONGMING_OK) // 其他错误（如相机已被释放），退出线程防止持失效指针空转
            break;

        // 取到图就复位超时状态（恢复供电/插回网线后能再次报错）
        timeoutCount = 0;
        timeoutReported = false;

        // 将图像通过信号槽发送给视觉窗口（跨线程自动走 QueuedConnection）
        emit sigUpdateImage(image);
    }
}
