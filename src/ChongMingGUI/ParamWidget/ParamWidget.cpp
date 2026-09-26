#include "ParamWidget.h"
#include "CameraInterface/CMCameraMetaInfo.h"
#include "CameraInterface/CMCameraParam.h"
#include "CameraInterface/CameraContext.h"
#include "CameraInterface/CameraError.h"
#include "ControlWidget/ControlWidget.h"
#include "ParamWidget/CameraParamDelegate.h"
#include "ParamWidget/CameraParamItem.h"
#include "ParamWidget/CameraParamModel.h"
#include "ui_ParamWidget.h"
#include <QDebug>

ParamWidget::ParamWidget(ControlWidget* controlWidget, QWidget* parent)
    : QWidget(parent)
    , Listener()
    , ui(new Ui::ParamWidget)
    , m_pCameraParamDelegate(new CameraParamDelegate())
    , m_pControlWidget(controlWidget)
{
    ui->setupUi(this);
    m_pParamTreeView = ui->Param_treeView;
    m_pParamDescript = ui->ParamDescript_textBrowser;
    m_pParamDescript->setFixedHeight(58);

    // 初始化模型数据
    // 添加表头
    QStringList headerList;
    headerList << "Param" << "Value";
    m_pModel = new CameraParamModel(headerList);
    m_pParamTreeView->setModel(m_pModel);
    m_pParamTreeView->setItemDelegate(m_pCameraParamDelegate);
    m_pParamTreeView->expandAll();
    m_pSelectionModel = m_pParamTreeView->selectionModel();

    // 添加相机参数描述信息
    m_pParamDescript->append(QStringLiteral("相机参数注释"));

    // 创建信号槽
    connect(m_pSelectionModel, &QItemSelectionModel::selectionChanged,
        this, &ParamWidget::OnUpdataSelection);
    connect(m_pModel, &CameraParamModel::SigValueChanged, this, &ParamWidget::writeCameraParam);

    // 注册事件监听
    ListenerManger::Instance()->registerMessage(MESSAGE::CAMERA_CONNECT
            | MESSAGE::CAMERA_DISCONNECT
            | MESSAGE::CAMERA_ENUMRTION
            | MESSAGE::CAMERA_STARTGRAB
            | MESSAGE::CAMERA_STOPTGRAB
            | MESSAGE::CAMERA_CAMERASWICH,
        this);
}

ParamWidget::~ParamWidget()
{
    delete ui;
}

void ParamWidget::initParamWidget(QVector<CameraParam> paramList)
{
    CameraMetaInfo currentCameraInfo = m_pControlWidget->GetCurrentCameraInfo();
    QString serial = currentCameraInfo.Serial;

    for (auto param : paramList) {
        // 先读取参数
        auto ret = CameraContext::Instance()->readParam(serial, param);
        if (ret != CHONGMING_OK) {
            QString error = param.name() + QString(" read failed");
            emit SigUpdateErrorInfo(error);
        }

        // 将参数显示在参数窗口
        m_pModel->addCameraParam(param);
    }
}

void ParamWidget::clearParamWidget()
{
    m_pModel->clear();
    m_pParamDescript->clear();
    // 复刻后 CameraParamModel::clear 内部已发 begin/endResetModel，
    // 不再需要源工程 m_pParamTreeView->reset() 这种补救调用
}

void ParamWidget::writeCameraParam(const QModelIndex& index)
{
    QVariant dataValue = m_pModel->data(index, CameraParamModel::ItemRoles::ParamRole);
    CameraParam curCameraParam = dataValue.value<CameraParam>();

    CameraMetaInfo currentCameraInfo = m_pControlWidget->GetCurrentCameraInfo();
    QString serial = currentCameraInfo.Serial;

    CHECK_RETURN(CameraContext::Instance()->writeParam(serial, curCameraParam));
}

void ParamWidget::RespondMessage(int message)
{
    if ((message & MESSAGE::CAMERA_ENUMRTION) == MESSAGE::CAMERA_ENUMRTION) {
        clearParamWidget();
    }
    if ((message & MESSAGE::CAMERA_CONNECT) == MESSAGE::CAMERA_CONNECT) {
        this->setEnabled(true);

        // 相机连接时，读取相机参数
        CameraMetaInfo currentCameraInfo = m_pControlWidget->GetCurrentCameraInfo();
        QVector<CameraParam> paramList;
        CameraContext::Instance()->getParamList(currentCameraInfo.Serial, paramList);
        initParamWidget(paramList); // 显示参数列表
        on_Refresh_Button_clicked();
    }
    if ((message & MESSAGE::CAMERA_DISCONNECT) == MESSAGE::CAMERA_DISCONNECT) {
        clearParamWidget();
    }
    if ((message & MESSAGE::CAMERA_STARTGRAB) == MESSAGE::CAMERA_STARTGRAB) {
        this->setEnabled(false);
    }
    if ((message & MESSAGE::CAMERA_STOPTGRAB) == MESSAGE::CAMERA_STOPTGRAB) {
        this->setEnabled(true);
    }
    if ((message & MESSAGE::CAMERA_CAMERASWICH) == MESSAGE::CAMERA_CAMERASWICH) {
        clearParamWidget();
    }
}

// 同步在参数介绍窗口更新参数的描述
void ParamWidget::OnUpdataSelection(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)
    m_pParamDescript->clear();

    // 复刻修正（DEV_SPEC 附录 #19）：清空选择时 indexes() 为空，
    // 源工程直接 first() 越界崩溃
    if (selected.indexes().isEmpty()) {
        return;
    }

    QModelIndex index = selected.indexes().first();
    QVariant varValue = m_pParamTreeView->model()->data(index, CameraParamModel::ParamDescriptionRole);
    QString strDescript = varValue.toString();
    m_pParamDescript->append(strDescript);
}

void ParamWidget::on_Refresh_Button_clicked()
{
    m_pParamTreeView->reset();
    m_pParamTreeView->expandAll();
}
