#ifndef PARSEUIJSON_H
#define PARSEUIJSON_H
/**************************************************
 * ParseUiJson是一个单例类， 他会读取我们本地的相机面板描述json文件，通过预定的规则解析
 * json文件，反序列化得到QList<CameraParamMetaInfo>，然后我们再根据CameraParamMetaInfo
 * 生成相机参数，并进一步的生成我们的参数面板。
 * 整个上面的过程都是框架性的、自动化的，当我们扩展一个新的相机型号时，如果通过上面我们的框架流程，则我们只需要
 * 按照预定的格式写一个json，就可以完成我们新的相机型号的参数面板的自动生成。
 * 扩展性强，功能内聚，模块松耦合。
 *
 **************************************************/

#include "../CameraInterface/CMCameraParam.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QMutex>
#include <QObject>

// 相机参数解析器单例类
class ParseUiJson : public QObject {
    Q_OBJECT

public:
    // 获取单例实例
    static ParseUiJson* instance();

    // 从文件解析JSON
    bool loadFromFile(const QString& filePath);

    // 从字符串解析JSON
    bool loadFromString(const QString& jsonString);

    // 从QByteArray解析JSON
    bool loadFromByteArray(const QByteArray& jsonData);

    // 获取解析后的参数列表
    QList<CameraParamMetaInfo> getParamList() const;

    // 获取指定分组的参数列表
    QList<CameraParamMetaInfo> getParamListByGroup(const QString& group) const;

    // 获取所有分组名称
    QStringList getAllGroups() const;

    // 清空解析的数据
    void clear();

    // 获取最后的错误信息
    QString getLastError() const { return m_lastError; }

    // 检查解析是否成功
    bool isValid() const { return m_isValid; }

signals:
    void parseFinished(bool success, const QString& error);
    void parseProgress(int current, int total); // 解析进度信号

private:
    // 私有构造函数（单例模式）
    explicit ParseUiJson(QObject* parent = nullptr);
    ~ParseUiJson();

    // 禁止拷贝构造和赋值
    ParseUiJson(const ParseUiJson&) = delete;
    ParseUiJson& operator=(const ParseUiJson&) = delete;

    // 解析JSON的核心方法
    bool parseJson(const QByteArray& jsonData);

    // 辅助函数：将字符串转换为枚举类型
    CMParamType stringToParamType(const QString& typeStr) const;

    // 辅助函数：验证参数字段完整性
    bool validateParamObject(const QJsonObject& paramObj, const QString& groupName, int index);

private:
    static ParseUiJson* m_instance;
    static QMutex m_mutex;

    QList<CameraParamMetaInfo> m_paramList;
    QString m_lastError;
    bool m_isValid;
};

#endif // PARSEUIJSON_H
