#include "CameraContext.h"
#include "../CameraFactory/CameraFactory.h"
#include "../CameraFactory/HikCamera.h"
#include "../CameraFactory/VirtualCamera.h"
#include "../Utils/ImageConver.h"
#include "CMCameraMetaInfo.h"
#include "CMCameraParam.h"
#include "CameraImageQueue.h"
#include "CameraInterface.h"
#include <QApplication>
#include <QDebug>
#include <QDir>

// 单例类对象定义
CameraContext* CameraContext::m_pContext = nullptr;

CameraContext* CameraContext::Instance()
{
    if (nullptr == m_pContext) {
        m_pContext = new CameraContext();
    }
    return m_pContext;
}

void CameraContext::Release()
{
    if (nullptr != m_pContext) {
        delete m_pContext;
        m_pContext = nullptr;
    }
}

CameraContext::CameraContext()
{
}

CameraContext::~CameraContext()
{
    clearAllCameras();
}

void CameraContext::clearAllCameras()
{
    // 复刻修正（DEV_SPEC 附录 #10）：源工程只停流断连就 clear，
    // 相机对象全部泄漏；这里先走完整下电流程，再 delete 相机对象本体
    QMap<QString, CameraInterface*>::iterator iter;
    for (iter = m_serialCamMap.begin(); iter != m_serialCamMap.end(); ++iter) {
        iter.value()->stopGrabbing();
        iter.value()->disconnect();
        iter.value()->release();
        delete iter.value();
    }
    m_serialCamMap.clear();
}

uint32_t CameraContext::EnumerationCamera(QVector<CameraMetaInfo>& cameraInfos)
{
    // 先将所有旧相机停流、断连并释放
    clearAllCameras();

    // 重新枚举获取相机列表
    QVector<CameraMetaInfo> infos;
    VirtualCamera::EnumCamera(infos);
    HikCamera::EnumCamera(infos);

    for (auto info : infos) {
        QVector<CameraMetaInfo>::iterator it = std::find(cameraInfos.begin(), cameraInfos.end(), info);
        // 如果枚举到的是新相机
        if (it == cameraInfos.end()) {
            cameraInfos.push_back(info);
            QString serial = info.Serial;
            // 使用工厂创建相机实例
            CameraInterface* camera = CameraFactory::instance()->createCamera(info);
            if (camera) {
                m_serialCamMap[info.Serial] = camera;
                qDebug() << "创建相机成功:" << info.VenderName << info.Serial;
            } else {
                qWarning() << "创建相机失败，不支持的厂商:" << info.VenderName;
            }
        }
    }
    return CHONGMING_OK;
}

uint32_t CameraContext::getParamList(const QString serial, QVector<CameraParam>& paramList)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    QVector<CameraParam> paramListTemp;
    auto camera = m_serialCamMap[serial];
    camera->getParamList(paramListTemp);

    for (auto var : paramListTemp) {
        camera->readParam(var);
        paramList.push_back(var);
    }

    return CHONGMING_OK;
}

uint32_t CameraContext::isConnect(const QString serial, bool& state)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    auto camera = m_serialCamMap[serial];
    state = camera->isConnect();

    return CHONGMING_OK;
}

uint32_t CameraContext::isGrabbing(const QString serial, bool& state)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    auto camera = m_serialCamMap[serial];
    state = camera->isGrabbing();

    return CHONGMING_OK;
}

uint32_t CameraContext::connect(const QString serial)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    auto camera = m_serialCamMap[serial];
    // 初始化相机
    auto ret = camera->acquire();
    if (ret != CHONGMING_OK)
        return ret;
    // 连接相机
    ret = camera->connect();
    if (ret != CHONGMING_OK)
        return ret;

    return CHONGMING_OK;
}

uint32_t CameraContext::disconnect(const QString serial)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    auto camera = m_serialCamMap[serial];
    // 断连相机
    auto ret = camera->disconnect();
    if (ret != CHONGMING_OK)
        return ret;

    // 解初始化
    ret = camera->release();
    if (ret != CHONGMING_OK)
        return ret;

    return CHONGMING_OK;
}

uint32_t CameraContext::startGrabbing(const QString serial)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    auto camera = m_serialCamMap[serial];

    if (camera->isConnect() != true)
        return CAMERA_NOT_CONNECTED;

    if (camera->isGrabbing() == true)
        return CHONGMING_OK;

    // 创建拉流资源
    auto ret = camera->creatStream();
    if (ret != CHONGMING_OK)
        return ret;

    // 开启拉流（复刻修正：源工程这里检查的是 creatStream 的旧 ret，startGrabbing 的失败被吞掉）
    ret = camera->startGrabbing();
    if (ret != CHONGMING_OK)
        return ret;

    return CHONGMING_OK;
}

uint32_t CameraContext::stopGrabbing(const QString serial)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    auto camera = m_serialCamMap[serial];

    if (camera->isConnect() == false || camera->isGrabbing() == false)
        return CHONGMING_OK;

    // 停止拉流
    auto ret = camera->stopGrabbing();
    if (ret != CHONGMING_OK)
        return ret;

    // 销毁数据流资源（复刻修正：同上，检查 destroyStream 自己的返回值）
    ret = camera->destroyStream();
    if (ret != CHONGMING_OK)
        return ret;

    return CHONGMING_OK;
}

uint32_t CameraContext::loadConfig(const QString serial, const QString path)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    auto camera = m_serialCamMap[serial];

    if (camera->isConnect() == false || camera->isGrabbing() == true)
        return CAMERA_NOT_CONNECTED;

    // 加载配置文件
    auto ret = camera->loadConfig(path);
    if (ret != CHONGMING_OK)
        return ret;

    return CHONGMING_OK;
}

uint32_t CameraContext::saveConfig(const QString serial, const QString path)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    auto camera = m_serialCamMap[serial];

    if (camera->isConnect() == false || camera->isGrabbing() == true)
        return CAMERA_NOT_CONNECTED;

    // 导出配置文件
    auto ret = camera->saveConfig(path);
    if (ret != CHONGMING_OK)
        return ret;

    return CHONGMING_OK;
}

QString CameraContext::getConfigFormat(const QString serial)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return "";

    CameraInterface* camera = m_serialCamMap[serial];

    return camera->configFormat();
}

uint32_t CameraContext::readParam(const QString serial, CameraParam& param)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    CameraInterface* camera = m_serialCamMap[serial];
    auto ret = camera->readParam(param);
    if (ret != CHONGMING_OK)
        return ret;

    return CHONGMING_OK;
}

uint32_t CameraContext::writeParam(const QString serial, CameraParam& param)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    CameraInterface* camera = m_serialCamMap[serial];
    auto ret = camera->writeParam(param);
    if (ret != CHONGMING_OK)
        return ret;

    return CHONGMING_OK;
}

uint32_t CameraContext::getImageLast(const QString serial, QImage& image)
{
    if (m_serialCamMap.find(serial) == m_serialCamMap.end())
        return NOCAMERA_ERROR;

    CameraInterface* camera = m_serialCamMap[serial];
    cv::Mat cvImage;
    // 复刻修正：直接透传底层错误码（队列现在能返回 GETIAMGE_TIMEOUT / CAMERA_QUEUE_STOPPED），
    // 源工程把所有失败一律翻译成 GETIAMGE_TIMEOUT，掩盖了真实原因
    auto ret = camera->getImageLast(cvImage);
    if (ret != CHONGMING_OK)
        return ret;

    // 成功取到图像，将cv::Mat格式转换为QImage，传递给视觉窗口显示
    image = ImageConver::cvMat2QImage(cvImage);

    return CHONGMING_OK;
}
