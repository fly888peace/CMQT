#include "VirtualCamera.h"
#include "../ParseUiJson/ParseUiJson.h"
#include <chrono>
#include <ctime>
#include <thread>

const QString VirtualCamera::VIRTUAL_CAMERA_NAME = "VirtualCamera";
const QString VirtualCamera::VIRTUAL_CAMERA_SERIAL = "Vir123456";
const QString VirtualCamera::VIRTUAL_CAMERA_VENDER = "Virtual";

VirtualCamera::VirtualCamera(const CameraMetaInfo& info)
    : CameraInterface(info)
{
}

VirtualCamera::~VirtualCamera()
{
    // 复刻修正：析构前确保出图线程已退出（源工程 detached 线程在此之后仍访问 this）
    if (m_starGrabbing) {
        stopGrabbing();
    }
}

uint32_t VirtualCamera::EnumCamera(QVector<CameraMetaInfo>& cameraInfos)
{
    // 枚举设备,插入一个虚拟相机
    // 复刻修正（DEV_SPEC 附录 #12）：聚合初始化按声明顺序赋值 {Serial, UserDefineID, VenderName}，
    // 源工程按 {名字, 序列号, 厂商} 塞，Serial 实际被赋成了 "VirtualCamera"——语义颠倒
    cameraInfos.push_back(CameraMetaInfo { VIRTUAL_CAMERA_SERIAL, VIRTUAL_CAMERA_NAME, VIRTUAL_CAMERA_VENDER });
    return CHONGMING_OK;
}

uint32_t VirtualCamera::getParamList(QVector<CameraParam>& paramList)
{
    ParseUiJson* parser = ParseUiJson::instance();
    parser->loadFromFile(":/VirtualCameraParam.json");
    QList<CameraParamMetaInfo> paramMetaInfoList = parser->getParamList();

    for (auto var : paramMetaInfoList) {
        paramList.push_back(CameraParam(var));
    }

    return CHONGMING_OK;
}

bool VirtualCamera::isConnect()
{
    return m_connect;
}

bool VirtualCamera::isGrabbing()
{
    return m_starGrabbing;
}

uint32_t VirtualCamera::acquire()
{
    return CHONGMING_OK;
}

uint32_t VirtualCamera::release()
{
    return CHONGMING_OK;
}

uint32_t VirtualCamera::connect()
{
    m_connect = true;
    return CHONGMING_OK;
}

uint32_t VirtualCamera::disconnect()
{
    m_connect = false;
    return CHONGMING_OK;
}

uint32_t VirtualCamera::creatStream()
{
    return CHONGMING_OK;
}

uint32_t VirtualCamera::destroyStream()
{
    return CHONGMING_OK;
}

uint32_t VirtualCamera::startGrabbing()
{
    auto CreateImage = [this]() -> void {
        cv::RNG rng((uint64)time(nullptr)); // 随机数发生器只播种一次（源工程每帧以秒播种，同秒内序列相同）
        while (this->isGrabbing()) {
            // 生成随机颜色图像
            cv::Mat canvas = cv::Mat::zeros(cv::Size(512, 512), CV_8UC3);
            int b = rng.uniform(0, 255);
            int g = rng.uniform(0, 255);
            int r = rng.uniform(0, 255);
            canvas.setTo(cv::Scalar(b, g, r));

            // 将图像加入到图像队列，模拟相机出图
            this->getImageQueue().Put(canvas);
            // 休眠
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    };

    m_imageQueue.Restart(); // 复位队列停止标志（配合 stopGrabbing 里的 Stop）
    m_starGrabbing = true;
    // 开启出图子线程（成员线程，stopGrabbing 时 join，替代源工程的 detach）
    m_grabThread = std::thread(CreateImage);

    return CHONGMING_OK;
}

uint32_t VirtualCamera::stopGrabbing()
{
    m_starGrabbing = false;
    m_imageQueue.Stop(); // 唤醒可能阻塞在 Take 的消费者
    if (m_grabThread.joinable()) {
        m_grabThread.join(); // 等出图线程真正退出，杜绝 use-after-free
    }
    return CHONGMING_OK;
}

uint32_t VirtualCamera::loadConfig(const QString path)
{
    return CHONGMING_OK;
}

uint32_t VirtualCamera::saveConfig(const QString path)
{
    return CHONGMING_OK;
}

QString VirtualCamera::configFormat()
{
    return "xml";
}

uint32_t VirtualCamera::readParam(CameraParam& param)
{
    param.setValid(true);
    param.setReadable(true);
    param.setWriteable(true);

    // 读取参数
    if (param.type() == INT) {
        IntParam varParam = param.GetValue().value<IntParam>();
        varParam.value = 30;
        varParam.increment = 1;
        varParam.min = 0;
        varParam.max = 100;
        param.SetValue(QVariant::fromValue(varParam));
    } else if (param.type() == DOUBLE) {
        DoubleParam varParam = param.GetValue().value<DoubleParam>();
        varParam.value = 55.6;
        varParam.min = 0;
        varParam.max = 1000;
        param.SetValue(QVariant::fromValue(varParam));
    } else if (param.type() == ENUM) {
        // 获取当前枚举值的条目
        EnumParam varParam = param.GetValue().value<EnumParam>();
        varParam.value = "item1";
        varParam.valueInt = 0;
        varParam.availableInt = QVector<int> { 0, 1, 2 };
        varParam.availableValue = QVector<QString> { "item1", "item2", "item3" };
        param.SetValue(QVariant::fromValue(varParam));
    } else if (param.type() == BOOL) {
        BoolParam varParam = param.GetValue().value<BoolParam>();
        varParam.value = false;
        param.SetValue(QVariant::fromValue(varParam));
    } else if (param.type() == CMD) {
        // CMD参数不需要任何读取操作
    } else if (param.type() == STRING) {
        StringParam varParam = param.GetValue().value<StringParam>();
        varParam.value = "string_value";
        varParam.nMaxLength = 16;
        param.SetValue(QVariant::fromValue(varParam));
    }

    return CHONGMING_OK;
}

uint32_t VirtualCamera::writeParam(CameraParam& param)
{
    return CHONGMING_OK;
}

uint32_t VirtualCamera::getImageLast(cv::Mat& image)
{
    // 从图像队列中取到最前的一帧图像
    cv::Mat srcImage;
    // 复刻修正：透传队列全部错误码（源工程只认 GETIAMGE_TIMEOUT，其余失败当成功继续 copyTo 空 Mat）
    auto ret = m_imageQueue.Take(srcImage);
    if (ret != CHONGMING_OK) {
        return ret;
    }
    srcImage.copyTo(image);

    return CHONGMING_OK;
}
