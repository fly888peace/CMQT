#ifndef VIRTUALCAMERA_H
#define VIRTUALCAMERA_H
/**************************************************
 * 虚拟相机类，实现了虚拟相机的功能
 * 该类继承自相机接口类CameraInterface，相机接口类具体作用
 * 参见CameraInterface类的注释
 *
 * 复刻修正（DEV_SPEC 附录 #11 #12）：
 * - detached std::thread 改为成员线程 + stopGrabbing 时 join，
 *   消除对象析构后线程仍访问 this 的 use-after-free；
 * - 状态标志改 std::atomic<bool>（出图线程读、UI 线程写）；
 * - 停流时 Stop 图像队列唤醒 Take，开流时 Restart 复位。
 **************************************************/

#include "../CameraInterface/CameraInterface.h"
#include <QObject>
#include <atomic>
#include <thread>

class VirtualCamera
    : public CameraInterface {
public:
    static const QString VIRTUAL_CAMERA_NAME;
    static const QString VIRTUAL_CAMERA_SERIAL;
    static const QString VIRTUAL_CAMERA_VENDER;

    explicit VirtualCamera(const CameraMetaInfo& info);
    ~VirtualCamera();
    // 枚举相机
    static uint32_t EnumCamera(QVector<CameraMetaInfo>& cameraInfos);
    // 获取相机参数列表
    uint32_t getParamList(QVector<CameraParam>& paramList) override;
    // 判断相机是否连接
    bool isConnect() override;
    // 判断相机是否拉流
    bool isGrabbing() override;
    // 获取相机
    uint32_t acquire() override;
    // 释放相机
    uint32_t release() override;
    // 连接相机
    uint32_t connect() override;
    // 断开连接
    uint32_t disconnect() override;
    // 创建拉流资源
    uint32_t creatStream() override;
    // 销毁资源
    uint32_t destroyStream() override;
    // 开启拉流
    uint32_t startGrabbing() override;
    // 停止拉流
    uint32_t stopGrabbing() override;
    // 加载配置文件
    uint32_t loadConfig(const QString path) override;
    // 保存配置文件
    uint32_t saveConfig(const QString path) override;
    // 获取配置文件格式
    QString configFormat() override;
    // 读取参数
    uint32_t readParam(CameraParam& param) override;
    // 写入参数
    uint32_t writeParam(CameraParam& param) override;
    // 获取图像
    uint32_t getImageLast(cv::Mat& image) override;
    // 获取图像队列
    CameraImageQueue& getImageQueue()
    {
        return m_imageQueue;
    }

private:
    std::atomic<bool> m_connect { false };
    std::atomic<bool> m_starGrabbing { false };
    std::thread m_grabThread; // 出图线程（成员对象，可 join，替代源工程的 detached 临时线程）
};

#endif // VIRTUALCAMERA_H
