#ifndef HIKCAMERA_H
#define HIKCAMERA_H
/**************************************************
 * 海康相机类，基于海康的SDK实现了海康相机的功能
 * 该类继承自相机接口类CameraInterface，相机接口类具体作用
 * 参见CameraInterface类的注释
 *
 * 复刻修正（DEV_SPEC 附录 #8 #18）：
 * - 厂商常量改名 HIK_CAMERA_VENDER（源工程叫 VIRTUAL_CAMERA_VENDER，
 *   与 VirtualCamera 的同名常量混淆，读起来像工厂注册错对象）；
 * - isStartGrabbing 改 std::atomic<bool>；
 * - readParam/writeParam/getFeatureAccessMode 补句柄判空（与 connect 等函数风格统一）。
 **************************************************/

#include "../CameraInterface/CameraInterface.h"
#include "MvCameraControl.h"
#include <atomic>

class HikCamera
    : public CameraInterface {
public:
    static const QString HIK_CAMERA_VENDER;
    HikCamera(const CameraMetaInfo& info);
    ~HikCamera();
    // 枚举相机
    static uint32_t EnumCamera(QVector<CameraMetaInfo>& cameraInfos);
    // 获取相机参数列表
    uint32_t getParamList(QVector<CameraParam>& paramList) override;
    // 判断相机是否连接
    bool isConnect() override;
    // 判断相机是否拉流
    bool isGrabbing() override;
    // 初始化相机对象
    uint32_t acquire() override;
    // 释放相机
    uint32_t release() override;
    // 连接相机
    uint32_t connect() override;
    // 断开连接
    uint32_t disconnect() override;
    // 创建拉流资源
    uint32_t creatStream() override;
    // 销毁拉流资源
    uint32_t destroyStream() override;
    // 开启拉流
    uint32_t startGrabbing() override;
    // 停止拉流
    uint32_t stopGrabbing() override;
    // 导入配置文件
    uint32_t loadConfig(const QString path) override;
    // 导出配置文件
    uint32_t saveConfig(const QString path) override;
    // 获取配置文件格式
    QString configFormat() override;
    // 读取相机参数
    uint32_t readParam(CameraParam& param) override;
    // 写入相机参数
    uint32_t writeParam(CameraParam& param) override;
    // 获取实时图像
    uint32_t getImageLast(cv::Mat& image) override;

    // 获取海康相机句柄
    void* CameraHandle()
    {
        return m_cameraHandle;
    }

private:
    // 获取参数访问模式
    uint32_t getFeatureAccessMode(CameraParam& param);

private:
    void* m_cameraHandle = NULL;
    MV_CC_DEVICE_INFO* m_pDeviceInfo = NULL;

    std::atomic<bool> isStartGrabbing { false };
};

#endif // HIKCAMERA_H
