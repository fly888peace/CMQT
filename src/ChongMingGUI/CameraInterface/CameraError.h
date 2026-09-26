#ifndef CAMERAERROR_H
#define CAMERAERROR_H
/**************************************************
 * 重明错误码文件，定义了大部分情况下的报错，以及
 * 对应的报错信息
 *
 * 复刻说明（相对源工程的有意差异）：
 * - `getErrorInfoEn` 由 `static` 改 `inline`：源工程定义在头文件里，
 *   多个 .cpp 包含时各持一份实体（虽能链接过，但属 ODR 灰色地带），
 *   inline 是「头文件共享函数」的正确写法（DEV_SPEC 附录 隐患关联项）。
 * - 错误码是「互斥序数」不是位掩码，不可像 Listener 消息那样用 | 组合。
 **************************************************/
#include <QString>

namespace CAMERAERROR {
// 注意：#define 不受 namespace 约束，这里写 namespace 只是视觉分组
#define CHONGMING_OK 0x0000 // 运行成功
#define CONNECT_ERROR 0X0001 // 连接相机失败
#define DISCONNECT_ERROR 0X0002 // 断开相机失败
#define STARTGRAB_ERROR 0x0003 // 开启拉流失败
#define STOPGRAB_ERROR 0x0004 // 停止拉流失败
#define NOCAMERA_ERROR 0x0005 // 未找到相机
#define INVALID_INPUT 0x0006 // 输入参数无效
#define INVALID_CAMERA_HANDLE 0x0007 // 相机句柄无效
#define WRITE_PARAM_FAILED 0x0008 // 参数设置失败
#define READ_PARAM_FAILED 0x0009 // 参数读取失败
#define CAMERA_NOT_CONNECTED 0x000A // 相机未连接
#define CAMERA_ACQUIRE_FAILED 0x000B // 相机创建失败
#define CAMERA_CONFIG_SAVE_FAILED 0x000C // 相机配置文件导出失败
#define CAMERA_CONFIG_LOAD_FAILED 0x000D // 相机配置文件导入失败
#define GETIAMGE_TIMEOUT 0x000E // 采图超时
#define DEVICE_NOT_ACCESSIBLE 0x000F // 设备被占用不可达
}

inline QString getErrorInfoEn(unsigned int error)
{
    QString info {};
    switch (error) {
    case CHONGMING_OK: {
        info = "Run Success";
        break;
    }
    case CONNECT_ERROR: {
        info = "Camera Connect failed";
        break;
    }
    case DISCONNECT_ERROR: {
        info = "Camera DisConnect failed";
        break;
    }
    case STARTGRAB_ERROR: {
        info = "Camera StartGrabing failed";
        break;
    }
    case STOPGRAB_ERROR: {
        info = "Camera StopGrabing failed";
        break;
    }
    case NOCAMERA_ERROR: {
        info = "Not Find Any Camera";
        break;
    }
    case INVALID_INPUT: {
        info = "Invalid Input Parameter";
        break;
    }
    case INVALID_CAMERA_HANDLE: {
        info = "The camera handle is invalid";
        break;
    }
    case WRITE_PARAM_FAILED: {
        info = "Write Param failed";
        break;
    }
    case READ_PARAM_FAILED: {
        info = "Read Param failed";
        break;
    }
    case CAMERA_NOT_CONNECTED: {
        info = "Camera not connected";
        break;
    }
    case CAMERA_ACQUIRE_FAILED: {
        info = "Camera creation failure";
        break;
    }
    case CAMERA_CONFIG_SAVE_FAILED: {
        info = "Failed to export the camera configuration file";
        break;
    }
    case CAMERA_CONFIG_LOAD_FAILED: {
        info = "Failed to import the camera configuration file";
        break;
    }
    case GETIAMGE_TIMEOUT: {
        info = "Get Image TimeOut";
        break;
    }
    case DEVICE_NOT_ACCESSIBLE: {
        info = "The device is  UnAccessible";
        break;
    }
    default: {
        info = "Unkonw Error";
        break;
    }
    }
    return info;
}

#endif // CAMERAERROR_H
