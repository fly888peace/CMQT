#include "CameraFactory.h"
#include "HikCamera.h"
#include "VirtualCamera.h"
#include <QMutex>
#include <QMutexLocker>

// 静态成员初始化
CameraFactory* CameraFactory::m_instance = nullptr;
QMutex CameraFactory::m_mutex;

CameraFactory* CameraFactory::instance()
{
    if (!m_instance) {
        QMutexLocker locker(&m_mutex);
        if (!m_instance) {
            m_instance = new CameraFactory();
            // 复刻修正（DEV_SPEC 附录 #8）：海康的厂商常量已改名 HIK_CAMERA_VENDER，
            // 源工程叫 VIRTUAL_CAMERA_VENDER，读这里会误以为注册错了对象
            CameraFactory::instance()->registerCamera<HikCamera>(
                HikCamera::HIK_CAMERA_VENDER);
            // 老固件别名（实测 MV-CA060-11GM 上报 "Hikvision"）
            CameraFactory::instance()->registerCamera<HikCamera>(
                HikCamera::HIK_CAMERA_VENDER_LEGACY);
            CameraFactory::instance()->registerCamera<VirtualCamera>(
                VirtualCamera::VIRTUAL_CAMERA_VENDER);
        }
    }
    return m_instance;
}

CameraInterface* CameraFactory::createCamera(const CameraMetaInfo& info)
{
    QString venderName = info.VenderName;

    if (!m_creatorMap.contains(venderName)) {
        qWarning() << "不支持的相机厂商:" << venderName;
        return nullptr;
    }

    return m_creatorMap[venderName](info);
}

QStringList CameraFactory::getSupportedVenders() const
{
    return m_creatorMap.keys();
}

bool CameraFactory::isVenderSupported(const QString& venderName) const
{
    return m_creatorMap.contains(venderName);
}
