#ifndef CAMERAINTERFACE_H
#define CAMERAINTERFACE_H
/**************************************************
 * CameraInterface是相机接口类，它是一个纯虚接口类，所有品牌的相机的具体实现类
 * 均继承该虚类，并必须实现该类定义的若干个接口
 * 这些接口对相机的行为进行了抽象，我们只需要基于对应品牌相机的SDK，来填充这些
 * 接口，实现对应的功能即可。
 *
 * 复刻说明：去掉源工程多余的 `#include <QtPlugin>`（纯抽象接口不涉及 Qt 插件体系）。
 * 17 个纯虚函数覆盖相机全生命周期三对状态机：
 *   acquire/release（句柄）、connect/disconnect（连接）、creatStream/destroyStream（流通道）
 *   外加拉流启停、参数读写、配置导入导出、取图。
 **************************************************/

#include "CMCameraMetaInfo.h"
#include "CMCameraParam.h"
#include "CameraError.h"
#include "CameraImageQueue.h"
#include "opencv2/core.hpp"

// 相机接口类
class CameraInterface {
public:
    CameraInterface(const CameraMetaInfo& info)
    {
        m_cameraInfo = info;
    }
    virtual ~CameraInterface() { }
    // 获取相机用户定义名称
    virtual QString UserName()
    {
        return m_cameraInfo.UserDefineID;
    }
    // 获取相机序列号
    virtual QString Serial()
    {
        return m_cameraInfo.Serial;
    }
    // 获取相机参数列表
    virtual uint32_t getParamList(QVector<CameraParam>& paramList) = 0;
    // 判断相机是否连接
    virtual bool isConnect() = 0;
    // 判断相机是否拉流
    virtual bool isGrabbing() = 0;
    // 初始化相机对象
    virtual uint32_t acquire() = 0;
    // 释放相机
    virtual uint32_t release() = 0;
    // 连接相机
    virtual uint32_t connect() = 0;
    // 断开连接
    virtual uint32_t disconnect() = 0;
    // 创建拉流资源
    virtual uint32_t creatStream() = 0;
    // 销毁拉流资源
    virtual uint32_t destroyStream() = 0;
    // 开启拉流
    virtual uint32_t startGrabbing() = 0;
    // 停止拉流
    virtual uint32_t stopGrabbing() = 0;
    // 导入配置文件
    virtual uint32_t loadConfig(const QString path) = 0;
    // 导出配置文件
    virtual uint32_t saveConfig(const QString path) = 0;
    // 获取配置文件格式
    virtual QString configFormat() = 0;
    // 读取相机参数
    virtual uint32_t readParam(CameraParam& param) = 0;
    // 写入相机参数
    virtual uint32_t writeParam(CameraParam& param) = 0;
    // 获取实时图像
    virtual uint32_t getImageLast(cv::Mat& image) = 0;
    // 获取图像队列
    virtual CameraImageQueue& ImageQueue()
    {
        return m_imageQueue;
    }

protected:
    CameraImageQueue m_imageQueue; // 图像队列
    QVector<CameraParam> m_cameraParams; // 相机参数列表
    CameraMetaInfo m_cameraInfo; // 相机元信息
};

#endif
