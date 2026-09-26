#include "BoolCustomWidget.h"

BoolCustomWidget::BoolCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent)
    : OneCustomWidget(param, index, parent)
    , m_pCheckBox(new QCheckBox(this))
{
    m_pCheckBox->setStyleSheet(QString("QCheckBox{background-color: white;}"));
}

void BoolCustomWidget::setParam(CameraParam& param)
{
    disconnect(m_pCheckBox, &QCheckBox::checkStateChanged,
        this, &BoolCustomWidget::onValueChanged);

    OneCustomWidget::setParam(param);
    BoolParam varParam = m_param.GetValue().value<BoolParam>();
    m_pCheckBox->setChecked(varParam.value);
    m_pCheckBox->setText(m_param.displayText());

    // 复刻修正（DEV_SPEC 附录 #5 之一）：源工程只 disconnect 从不 connect，
    // 勾选永远不触发回写；这里真正建立连接（checkStateChanged 是 Qt 6.7+ 的信号）
    connect(m_pCheckBox, &QCheckBox::checkStateChanged,
        this, &BoolCustomWidget::onValueChanged);
}

CameraParam BoolCustomWidget::getParam()
{
    return OneCustomWidget::getParam();
}

void BoolCustomWidget::addEditLayout(QHBoxLayout* layout)
{
    layout->addWidget(m_pCheckBox);
}

void BoolCustomWidget::onValueChanged(Qt::CheckState state)
{
    // 复刻修正（DEV_SPEC 附录 #5 之二）：BOOL 参数就该取 BoolParam，
    // 源工程取了 IntParam，类型完全对不上
    BoolParam varParam = getParam().GetValue().value<BoolParam>();
    varParam.value = (state == Qt::Checked);
    m_param.SetValue(QVariant::fromValue(varParam));
    emit sigValueChanged(m_param, m_index);
}
