#include "HikCamera.h"
#include "opencv2/opencv.hpp"
#include <QDebug>

const QString HikCamera::HIK_CAMERA_VENDER = "Hikrobot";
const QString HikCamera::HIK_CAMERA_VENDER_LEGACY = "Hikvision";

// 判断像素格式是否为彩色
bool IsColor(MvGvspPixelType enType)
{
    switch (enType) {
    case PixelType_Gvsp_BGR8_Packed:
    case PixelType_Gvsp_YUV422_Packed:
    case PixelType_Gvsp_YUV422_YUYV_Packed:
    case PixelType_Gvsp_BayerGR8:
    case PixelType_Gvsp_BayerRG8:
    case PixelType_Gvsp_BayerGB8:
    case PixelType_Gvsp_BayerBG8:
    case PixelType_Gvsp_BayerGB10:
    case PixelType_Gvsp_BayerGB10_Packed:
    case PixelType_Gvsp_BayerBG10:
    case PixelType_Gvsp_BayerBG10_Packed:
    case PixelType_Gvsp_BayerRG10:
    case PixelType_Gvsp_BayerRG10_Packed:
    case PixelType_Gvsp_BayerGR10:
    case PixelType_Gvsp_BayerGR10_Packed:
    case PixelType_Gvsp_BayerGB12:
    case PixelType_Gvsp_BayerGB12_Packed:
    case PixelType_Gvsp_BayerBG12:
    case PixelType_Gvsp_BayerBG12_Packed:
    case PixelType_Gvsp_BayerRG12:
    case PixelType_Gvsp_BayerRG12_Packed:
    case PixelType_Gvsp_BayerGR12:
    case PixelType_Gvsp_BayerGR12_Packed:
    case PixelType_Gvsp_BayerRBGG8:
    case PixelType_Gvsp_BayerGR16:
    case PixelType_Gvsp_BayerRG16:
    case PixelType_Gvsp_BayerGB16:
    case PixelType_Gvsp_BayerBG16:
        return true;
    default:
        return false;
    }
}

// 判断像素格式是否为黑白Mono
bool IsMono(MvGvspPixelType enType)
{
    switch (enType) {
    case PixelType_Gvsp_Mono8:
    case PixelType_Gvsp_Mono10:
    case PixelType_Gvsp_Mono10_Packed:
    case PixelType_Gvsp_Mono12:
    case PixelType_Gvsp_Mono12_Packed:
    case PixelType_Gvsp_Mono14:
    case PixelType_Gvsp_Mono16:
        return true;
    default:
        return false;
    }
}

// 帧数据转换为Mat格式图片并保存
bool HikConvert2Mat(void* handle, MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, cv::Mat& dstImage)
{
    if (NULL == pstImageInfo || NULL == pData) {
        printf("NULL info or data.\n");
        return false;
    }
    // 添加图像格式转换，如果是彩色类型，就转成RGB8，如果是黑白，就转成Mono8
    MvGvspPixelType enDstPixelType = PixelType_Gvsp_Undefined;
    int nChannelNum = 1;
    // Mono8类型
    if (IsMono(pstImageInfo->enPixelType)) {
        enDstPixelType = PixelType_Gvsp_Mono8;
        nChannelNum = 1;
    }
    // RGB8类型
    else if (IsColor(pstImageInfo->enPixelType)) {
        enDstPixelType = PixelType_Gvsp_RGB8_Packed;
        nChannelNum = 3;
    }

    if (enDstPixelType == PixelType_Gvsp_Undefined) {
        qDebug() << "Unsupported pixel format!";
        return false;
    }

    // 复刻修正（DEV_SPEC 附录 #4）：让 cv::Mat 自己持有转换缓冲，
    // SDK 直接往 Mat 的数据区里写——替代源工程「每帧 malloc 从不 free」的泄漏写法
    dstImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth,
        nChannelNum == 1 ? CV_8UC1 : CV_8UC3);

    // ch:像素格式转换 | en:Convert pixel format
    MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };

    stConvertParam.nWidth = pstImageInfo->nWidth; // 图像宽
    stConvertParam.nHeight = pstImageInfo->nHeight; // 图像高
    stConvertParam.pSrcData = pData; // 输入数据缓存
    stConvertParam.nSrcDataLen = pstImageInfo->nFrameLen; // 输入数据大小
    stConvertParam.enSrcPixelType = pstImageInfo->enPixelType; // 输入像素格式
    stConvertParam.enDstPixelType = enDstPixelType; // 输出像素格式
    stConvertParam.pDstBuffer = dstImage.data; // 输出数据缓存（Mat 自持内存）
    stConvertParam.nDstBufferSize = (unsigned int)dstImage.total() * (unsigned int)dstImage.elemSize(); // 输出缓存大小
    auto nRet = MV_CC_ConvertPixelType(handle, &stConvertParam);
    if (MV_OK != nRet) {
        qDebug() << "Convert Pixel Type fail!";
        dstImage.release();
        return false;
    }

    return true;
}

// 海康取图回调函数（跑在 SDK 内部线程，不是 UI 线程）
void __stdcall ImageCallBack(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    if (!pFrameInfo || !pData)
        return;
    // 类对象的指针
    HikCamera* pCamera = static_cast<HikCamera*>(pUser);
    if (!pCamera)
        return;

    // 将图像数据转换至cv::Mat
    cv::Mat cvImage;
    if (!HikConvert2Mat(pCamera->CameraHandle(), pFrameInfo, pData, cvImage)) {
        return; // 转换失败直接丢帧，不入队（源工程失败也照 Put 空 Mat）
    }

    // 将图像保存到相机的图像队列中
    pCamera->ImageQueue().Put(cvImage);
}

HikCamera::HikCamera(const CameraMetaInfo& info)
    : CameraInterface(info)
{
}

HikCamera::~HikCamera()
{
}

uint32_t HikCamera::EnumCamera(QVector<CameraMetaInfo>& cameraInfos)
{
    // 枚举设备
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    auto nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != nRet) {
        // 复刻修正（DEV_SPEC 附录 #16）：枚举失败返回真实错误码，
        // 源工程返回 CHONGMING_OK，调用方无法区分「无相机」与「枚举失败」
        return NOCAMERA_ERROR;
    }

    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* cameraInfo = stDeviceList.pDeviceInfo[i];
        if (cameraInfo->nTLayerType == MV_GIGE_DEVICE) {
            CameraMetaInfo info;
            info.Serial = (char*)cameraInfo->SpecialInfo.stGigEInfo.chSerialNumber;
            info.UserDefineID = (char*)cameraInfo->SpecialInfo.stGigEInfo.chUserDefinedName;
            info.VenderName = (char*)cameraInfo->SpecialInfo.stGigEInfo.chManufacturerName;
            cameraInfos.push_back(info);
        } else if (cameraInfo->nTLayerType == MV_USB_DEVICE) {
            CameraMetaInfo info;
            info.Serial = (char*)cameraInfo->SpecialInfo.stUsb3VInfo.chSerialNumber;
            info.UserDefineID = (char*)cameraInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName;
            info.VenderName = (char*)cameraInfo->SpecialInfo.stUsb3VInfo.chManufacturerName;
            cameraInfos.push_back(info);
        }
    }

    return CHONGMING_OK;
}

uint32_t HikCamera::getParamList(QVector<CameraParam>& paramList)
{
    static QList<CameraParamMetaInfo> paramMetaInfoList = {
        { "DeviceControl", "DeviceVendorName", STRING, "", QStringLiteral("设备制造商名称") },
        { "DeviceControl", "DeviceUserID", STRING, "", QStringLiteral("设备名称，默认为空，可自行设置") },
        { "DeviceControl", "DeviceSerialNumber", STRING, "", QStringLiteral("设备序列号") },
        { "ImageFormatControl", "WidthMax", INT, "", QStringLiteral("最大宽度") },
        { "ImageFormatControl", "HeightMax", INT, "", QStringLiteral("最大高度") },
        { "ImageFormatControl", "Width", INT, "", QStringLiteral("ROI 区域横向的分辨率") },
        { "ImageFormatControl", "Height", INT, "", QStringLiteral("ROI 区域纵向的分辨率") },
        { "ImageFormatControl", "OffsetX", INT, "", QStringLiteral("ROI 区域左上角起点位置的横坐标") },
        { "ImageFormatControl", "OffsetY", INT, "", QStringLiteral("ROI 区域左上角起点位置的纵坐标") },
        { "ImageFormatControl", "ReverseX", BOOL, "", QStringLiteral("相机图像左右翻转") },
        { "ImageFormatControl", "ReverseY", BOOL, "", QStringLiteral("相机图像上下翻转") },
        { "ImageFormatControl", "PixelFormat", ENUM, "", QStringLiteral("相机支持多种像素格式，用户可根据需要自行设置像素格式") },
        { "AcquisitionControl", "AcquisitionMode", ENUM, "", QStringLiteral("采集模式") },
        { "AcquisitionControl", "AcquisitionStart", CMD, "", QStringLiteral("开始采集") },
        { "AcquisitionControl", "AcquisitionStop", CMD, "", QStringLiteral("停止采集") },
        { "AcquisitionControl", "AcquisitionFrameRateEnable", BOOL, "", QStringLiteral("帧率使能") },
        { "AcquisitionControl", "AcquisitionFrameRate", DOUBLE, "", QStringLiteral("需求帧率") },
        { "AcquisitionControl", "TriggerSelector", ENUM, "", QStringLiteral("触发选项") },
        { "AcquisitionControl", "TriggerMode", ENUM, "", QStringLiteral("触发模式") },
        { "AcquisitionControl", "TriggerSoftware", CMD, "", QStringLiteral("软触发") },
        { "AcquisitionControl", "TriggerSource", ENUM, "", QStringLiteral("触发源设置") },
        { "AcquisitionControl", "ExposureTime", DOUBLE, "", QStringLiteral("曝光设置") },
        { "AnalogControl", "Gain", DOUBLE, "", QStringLiteral("增益设置") },
        { "AnalogControl", "BlackLevel", INT, "", QStringLiteral("黑电平设置") },
        { "AnalogControl", "BlackLevelEnable", BOOL, "", QStringLiteral("黑电平设置使能") },
        { "AnalogControl", "BalanceWhiteAuto", BOOL, "", QStringLiteral("自动白平衡") },
        { "AnalogControl", "Gamma", DOUBLE, "", QStringLiteral("Gamma校正") },
        { "AnalogControl", "GammaSelector", ENUM, "", QStringLiteral("Gamma校正") },
        { "AnalogControl", "GammaEnable", BOOL, "", QStringLiteral("Gamma校正使能") },
        { "AnalogControl", "Sharpness", INT, "", QStringLiteral("锐度设置") },
        { "AnalogControl", "SharpnessEnable", BOOL, "", QStringLiteral("锐度设置使能") },
        { "AnalogControl", "ContrastRatio", DOUBLE, "", QStringLiteral("对比度设置") },
        { "AnalogControl", "ContrastRatioEnable", BOOL, "", QStringLiteral("对比度设置使能") },
        { "UserSetControl ", "UserSetSelector", ENUM, "", QStringLiteral("用户参数组选择") },
        { "UserSetControl ", "UserSetLoad", CMD, "", QStringLiteral("参数组加载") },
        { "UserSetControl ", "UserSetSave", CMD, "", QStringLiteral("参数组保存") },
        { "UserSetControl ", "UserSetDefault", ENUM, "", QStringLiteral("默认用户参数组设置") }
    };

    for (auto var : paramMetaInfoList) {
        paramList.push_back(CameraParam(var));
    }

    return CHONGMING_OK;
}

bool HikCamera::isConnect()
{
    if (m_cameraHandle == NULL) {
        return false;
    }

    return MV_CC_IsDeviceConnected(m_cameraHandle);
}

bool HikCamera::isGrabbing()
{
    if (m_cameraHandle == NULL) {
        return false;
    }

    return isStartGrabbing;
}

uint32_t HikCamera::acquire()
{
    // 枚举设备，找到和当前相机序列号对应的相机信息
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    auto nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != nRet) {
        return CAMERA_ACQUIRE_FAILED;
    }

    // 复刻修正（DEV_SPEC 附录 #13）：按设备类型分支取序列号，
    // 源工程无条件读 GigE 分支的 chSerialNumber，USB 相机永远匹配失败
    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* cameraInfo = stDeviceList.pDeviceInfo[i];
        QString serial;
        if (cameraInfo->nTLayerType == MV_GIGE_DEVICE) {
            serial = (char*)cameraInfo->SpecialInfo.stGigEInfo.chSerialNumber;
        } else if (cameraInfo->nTLayerType == MV_USB_DEVICE) {
            serial = (char*)cameraInfo->SpecialInfo.stUsb3VInfo.chSerialNumber;
        } else {
            continue;
        }
        if (serial == Serial()) {
            m_pDeviceInfo = cameraInfo;
            break;
        }
    }

    if (m_pDeviceInfo == NULL)
        return CAMERA_ACQUIRE_FAILED;

    // 创建相机句柄
    nRet = MV_CC_CreateHandle(&m_cameraHandle, m_pDeviceInfo);
    if (MV_OK != nRet) {
        return INVALID_CAMERA_HANDLE;
    }

    return CHONGMING_OK;
}

uint32_t HikCamera::release()
{
    // 销毁相机句柄资源（复刻修正：判空后再调 SDK）
    if (m_cameraHandle != NULL) {
        MV_CC_DestroyHandle(m_cameraHandle);
        m_cameraHandle = NULL;
    }

    // delete m_pDeviceInfo;  // 设备信息结构体归 SDK 设备列表所有，不在这里释放
    m_pDeviceInfo = NULL;

    return CHONGMING_OK;
}

uint32_t HikCamera::connect()
{
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }
    // 先判断设备是否可达
    if (MV_CC_IsDeviceAccessible(m_pDeviceInfo, MV_ACCESS_Exclusive) == false) {
        return DEVICE_NOT_ACCESSIBLE;
    }

    // 打开设备
    auto nRet = MV_CC_OpenDevice(m_cameraHandle);
    if (MV_OK != nRet)
        return CONNECT_ERROR;

    // 注册回调函数
    nRet = MV_CC_RegisterImageCallBackEx(m_cameraHandle, ImageCallBack, this);
    if (MV_OK != nRet)
        return CONNECT_ERROR;

    return CHONGMING_OK;
}

uint32_t HikCamera::disconnect()
{
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }

    // 取消回调回调函数注册
    auto nRet = MV_CC_RegisterImageCallBackEx(m_cameraHandle, NULL, NULL);
    if (MV_OK != nRet)
        return DISCONNECT_ERROR;

    // 关闭设备
    nRet = MV_CC_CloseDevice(m_cameraHandle);
    if (MV_OK != nRet)
        return DISCONNECT_ERROR;

    return CHONGMING_OK;
}

uint32_t HikCamera::creatStream()
{
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }

    // 设置拉流策略
    MV_CC_SetGrabStrategy(m_cameraHandle, MV_GRAB_STRATEGY::MV_GrabStrategy_OneByOne);
    // 设置SDK内部缓存队列节点数量
    MV_CC_SetImageNodeNum(m_cameraHandle, ImageQueueSize);

    // 复位队列停止标志（配合 stopGrabbing 里的 Stop）
    m_imageQueue.Restart();

    return CHONGMING_OK;
}

uint32_t HikCamera::destroyStream()
{
    return CHONGMING_OK;
}

uint32_t HikCamera::startGrabbing()
{
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }

    // 开启拉流
    auto nRet = MV_CC_StartGrabbing(m_cameraHandle);
    if (MV_OK != nRet) {
        return STARTGRAB_ERROR;
    }
    isStartGrabbing = true;

    return CHONGMING_OK;
}

uint32_t HikCamera::stopGrabbing()
{
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }

    // 停止拉流
    auto nRet = MV_CC_StopGrabbing(m_cameraHandle);
    if (MV_OK != nRet) {
        return STOPGRAB_ERROR;
    }
    isStartGrabbing = false;
    // 唤醒可能阻塞在 Take 的消费者（复刻新增，配合 CameraImageQueue::Stop）
    m_imageQueue.Stop();

    return CHONGMING_OK;
}

uint32_t HikCamera::loadConfig(const QString path)
{
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }
    // 导入相机配置文件
    auto nRet = MV_CC_FeatureLoad(m_cameraHandle, path.toLocal8Bit().data());
    if (MV_OK != nRet) {
        return CAMERA_CONFIG_LOAD_FAILED;
    }

    return CHONGMING_OK;
}

uint32_t HikCamera::saveConfig(const QString path)
{
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }
    // 导出相机配置文件
    auto nRet = MV_CC_FeatureSave(m_cameraHandle, path.toLocal8Bit().data());
    if (MV_OK != nRet) {
        return CAMERA_CONFIG_SAVE_FAILED;
    }

    return CHONGMING_OK;
}

QString HikCamera::configFormat()
{
    return "mfs";
}

uint32_t HikCamera::readParam(CameraParam& param)
{
    // 复刻修正（DEV_SPEC 附录 #18）：入口判空，源工程句柄为空时直接调 SDK 会崩
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }

    // 获取参数的访问模式
    getFeatureAccessMode(param);
    if (param.isReadable() == false) {
        return CHONGMING_OK;
    }
    // 读取参数（复刻修正：读取失败返回 READ_PARAM_FAILED，源工程笔误成 WRITE_PARAM_FAILED）
    if (param.type() == INT) {
        QString name = param.name();
        MVCC_INTVALUE value {};
        auto nRet = MV_CC_GetIntValue(m_cameraHandle, name.toLocal8Bit().data(), &value);
        if (nRet != MV_OK)
            return READ_PARAM_FAILED;

        IntParam varParam = param.GetValue().value<IntParam>();
        varParam.value = value.nCurValue;
        varParam.increment = value.nInc;
        varParam.min = value.nMin;
        varParam.max = value.nMax;
        param.SetValue(QVariant::fromValue(varParam));
    } else if (param.type() == DOUBLE) {
        QString name = param.name();
        MVCC_FLOATVALUE value {};
        auto nRet = MV_CC_GetFloatValue(m_cameraHandle, name.toLocal8Bit().data(), &value);
        if (nRet != MV_OK)
            return READ_PARAM_FAILED;

        DoubleParam varParam = param.GetValue().value<DoubleParam>();
        varParam.value = value.fCurValue;
        varParam.min = value.fMin;
        varParam.max = value.fMax;
        param.SetValue(QVariant::fromValue(varParam));
    } else if (param.type() == ENUM) {
        QString name = param.name();

        MVCC_ENUMVALUE value {};
        memset(&value, 0, sizeof(MVCC_ENUMVALUE));

        auto nRet = MV_CC_GetEnumValue(m_cameraHandle, name.toLocal8Bit().data(), &value);
        if (nRet != MV_OK)
            return READ_PARAM_FAILED;

        // 获取当前枚举值的条目
        MVCC_ENUMENTRY entryValue {};
        memset(&entryValue, 0, sizeof(MVCC_ENUMENTRY));
        entryValue.nValue = value.nCurValue;
        MV_CC_GetEnumEntrySymbolic(m_cameraHandle, name.toLocal8Bit().data(), &entryValue);

        // 获取所有条目
        QVector<int> intlist {};
        QVector<QString> strlist {};
        for (unsigned int i = 0; i < value.nSupportedNum; i++) {
            int curInt = value.nSupportValue[i];
            intlist.push_back(curInt);

            // 获取当前枚举值的条目
            MVCC_ENUMENTRY entryValue {};
            memset(&entryValue, 0, sizeof(MVCC_ENUMENTRY));
            entryValue.nValue = curInt;
            MV_CC_GetEnumEntrySymbolic(m_cameraHandle, name.toLocal8Bit().data(), &entryValue);
            QString curStr = entryValue.chSymbolic;
            strlist.push_back(curStr);
        }

        EnumParam varParam = param.GetValue().value<EnumParam>();
        varParam.value = entryValue.chSymbolic;
        varParam.valueInt = value.nCurValue;
        varParam.availableInt = intlist;
        varParam.availableValue = strlist;
        param.SetValue(QVariant::fromValue(varParam));
    } else if (param.type() == BOOL) {
        QString name = param.name();
        bool bValue {};
        auto nRet = MV_CC_GetBoolValue(m_cameraHandle, name.toLocal8Bit().data(), &bValue);
        if (nRet != MV_OK)
            return READ_PARAM_FAILED;

        BoolParam varParam = param.GetValue().value<BoolParam>();
        varParam.value = bValue;
        param.SetValue(QVariant::fromValue(varParam));
    } else if (param.type() == CMD) {
        // CMD参数不需要任何读取操作
    } else if (param.type() == STRING) {
        QString name = param.name();
        MVCC_STRINGVALUE value {};
        auto nRet = MV_CC_GetStringValue(m_cameraHandle, name.toLocal8Bit().data(), &value);
        if (nRet != MV_OK)
            return READ_PARAM_FAILED;

        StringParam varParam = param.GetValue().value<StringParam>();
        varParam.value = value.chCurValue;
        varParam.nMaxLength = value.nMaxLength;
        param.SetValue(QVariant::fromValue(varParam));
    }

    return CHONGMING_OK;
}

uint32_t HikCamera::writeParam(CameraParam& param)
{
    // 复刻修正（DEV_SPEC 附录 #18）：入口判空
    if (m_cameraHandle == NULL) {
        return INVALID_CAMERA_HANDLE;
    }

    // 获取参数的访问模式
    getFeatureAccessMode(param);
    if (param.isWriteable() == false) {
        return CHONGMING_OK;
    }
    // 写入参数
    if (param.type() == INT) {
        QString name = param.name();
        IntParam varValue = param.GetValue().value<IntParam>();
        auto nRet = MV_CC_SetIntValue(m_cameraHandle, name.toLocal8Bit().data(), varValue.value);
        if (nRet != MV_OK)
            return WRITE_PARAM_FAILED;
    } else if (param.type() == DOUBLE) {
        QString name = param.name();
        DoubleParam varValue = param.GetValue().value<DoubleParam>();
        float value = varValue.value;
        auto nRet = MV_CC_SetFloatValue(m_cameraHandle, name.toLocal8Bit().data(), value);
        if (nRet != MV_OK)
            return WRITE_PARAM_FAILED;
    } else if (param.type() == ENUM) {
        QString name = param.name();
        EnumParam varValue = param.GetValue().value<EnumParam>();
        int value = varValue.valueInt;
        auto nRet = MV_CC_SetEnumValue(m_cameraHandle, name.toLocal8Bit().data(), value);
        if (nRet != MV_OK)
            return WRITE_PARAM_FAILED;
    } else if (param.type() == BOOL) {
        QString name = param.name();
        BoolParam varValue = param.GetValue().value<BoolParam>();
        bool value = varValue.value;
        auto nRet = MV_CC_SetBoolValue(m_cameraHandle, name.toLocal8Bit().data(), value);
        if (nRet != MV_OK)
            return WRITE_PARAM_FAILED;
    } else if (param.type() == CMD) {
        QString name = param.name();
        auto nRet = MV_CC_SetCommandValue(m_cameraHandle, name.toLocal8Bit().data());
        if (nRet != MV_OK)
            return WRITE_PARAM_FAILED;
    } else if (param.type() == STRING) {
        QString name = param.name();
        StringParam varValue = param.GetValue().value<StringParam>();
        QString value = varValue.value;
        auto nRet = MV_CC_SetStringValue(m_cameraHandle, name.toLocal8Bit().data(), value.toLocal8Bit().data());
        if (nRet != MV_OK)
            return WRITE_PARAM_FAILED;
    }

    return CHONGMING_OK;
}

uint32_t HikCamera::getImageLast(cv::Mat& image)
{
    // 从图像队列中取到最前的一帧图像
    // 复刻修正：透传队列错误码（源工程一律翻译成 GETIAMGE_TIMEOUT）
    auto ret = m_imageQueue.Take(image);
    if (ret != CHONGMING_OK) {
        return ret;
    }

    return CHONGMING_OK;
}

uint32_t HikCamera::getFeatureAccessMode(CameraParam& param)
{
    // 复刻修正（DEV_SPEC 附录 #18）：入口判空
    if (m_cameraHandle == NULL) {
        param.setValid(false);
        param.setReadable(false);
        param.setWriteable(false);
        return INVALID_CAMERA_HANDLE;
    }

    QString name = param.name();

    MV_XML_AccessMode mode;
    MV_XML_GetNodeAccessMode(m_cameraHandle, name.toLocal8Bit().data(), &mode);
    if (mode == AM_NI || mode == AM_NA || mode == AM_Undefined) {
        param.setValid(false);
        param.setReadable(false);
        param.setWriteable(false);
    } else if (mode == AM_WO) {
        param.setValid(true);
        param.setReadable(false);
        param.setWriteable(true);
    } else if (mode == AM_RO) {
        param.setValid(true);
        param.setReadable(true);
        param.setWriteable(false);
    } else if (mode == AM_RW) {
        param.setValid(true);
        param.setReadable(true);
        param.setWriteable(true);
    }

    return CHONGMING_OK;
}
