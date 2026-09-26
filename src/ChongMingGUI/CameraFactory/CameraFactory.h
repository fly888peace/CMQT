#ifndef CAMERAFACTORY_H
#define CAMERAFACTORY_H
/**************************************************
 *相机工厂类，在instance函数内部完成相机类型的注册，然后根据相机的厂商名，完成相机对象指针的创建。
 *相机的厂商名是固定的，例如海康是Hikrobot，我们的虚拟相机的厂商名，就是自定义的叫Virtual
 *
 * 扩展新品牌相机的全部成本：
 *   1. 写一个继承 CameraInterface 的实现类（如 DaHuaCamera）；
 *   2. 在 CameraFactory::instance() 里加一行 registerCamera<DaHuaCamera>(DAHUA_VENDER)。
 **************************************************/

#include "../CameraInterface/CameraInterface.h"
#include <QMap>
#include <QMutex>
#include <QString>
#include <QVector>

// 相机工厂类 - 单例模式
class CameraFactory {
public:
    // 获取单例实例
    static CameraFactory* instance();

    // 注册相机创建器
    template <typename T>
    void registerCamera(const QString& venderName)
    {
        m_creatorMap[venderName] = [](const CameraMetaInfo& info) -> CameraInterface* {
            return new T(info);
        };
    }

    // 创建相机实例
    CameraInterface* createCamera(const CameraMetaInfo& info);

    // 获取支持的厂商列表
    QStringList getSupportedVenders() const;

    // 检查是否支持某个厂商
    bool isVenderSupported(const QString& venderName) const;

private:
    CameraFactory() = default;
    ~CameraFactory() = default;

    // 禁止拷贝
    CameraFactory(const CameraFactory&) = delete;
    CameraFactory& operator=(const CameraFactory&) = delete;

private:
    using CameraCreator = std::function<CameraInterface*(const CameraMetaInfo&)>;
    QMap<QString, CameraCreator> m_creatorMap;
    static CameraFactory* m_instance;
    static QMutex m_mutex;
};

#endif // CAMERAFACTORY_H
