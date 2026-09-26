#ifndef CMCAMERAMETAINFO_H
#define CMCAMERAMETAINFO_H
/**************************************************
 * CameraMetaInfo相机信息类，该类包括了相机的比较重要的、
 * 可以标识相机的 基础信息
 *
 * 复刻说明：`operator==` 补 `const`，源工程是非 const 成员函数，
 * 对 const 对象调用会编译失败（且语义上比较操作不该修改对象）。
 **************************************************/

#include <QString>

struct CameraMetaInfo {
    QString Serial {}; // 串口号
    QString UserDefineID {}; // 相机用户名
    QString VenderName {}; // 相机厂商名

    // 只比序列号：Serial 是相机的全局唯一标识，重枚举去重依赖这个判断
    bool operator==(const CameraMetaInfo& info) const
    {
        if (Serial == info.Serial) {
            return true;
        }
        return false;
    }
};

#endif // CMCAMERAMETAINFO_H
