#include "EnumCustomWidget.h"

EnumCustomWidget::EnumCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent)
    : OneCustomWidget(param, index, parent)
    , m_pCombox(new QComboBox(this))
{
}

void EnumCustomWidget::setParam(CameraParam& param)
{
    disconnect(m_pCombox, &QComboBox::currentIndexChanged,
        this, &EnumCustomWidget::onValueChanged);

    OneCustomWidget::setParam(param);
    EnumParam varParam = m_param.GetValue().value<EnumParam>();

    // 复刻修正：先清空再填充，源工程重复 setParam 会不停 addItems 越堆越多
    m_pCombox->clear();
    QStringList valueList(varParam.availableValue.begin(), varParam.availableValue.end());
    m_pCombox->addItems(valueList);
    m_pCombox->setCurrentText(varParam.value);

    connect(m_pCombox, &QComboBox::currentIndexChanged,
        this, &EnumCustomWidget::onValueChanged);
}

CameraParam EnumCustomWidget::getParam()
{
    return OneCustomWidget::getParam();
}

void EnumCustomWidget::addEditLayout(QHBoxLayout* layout)
{
    layout->addWidget(m_pCombox);
}

void EnumCustomWidget::onValueChanged(int index)
{
    EnumParam varParam = getParam().GetValue().value<EnumParam>();
    varParam.value = m_pCombox->currentText();
    // 复刻修正（DEV_SPEC 附录 #7）：枚举的真实值在 availableInt 里，索引≠值，
    // 源工程直接把 currentIndex 当枚举值写相机，会写错参数
    if (index >= 0 && index < varParam.availableInt.size()) {
        varParam.valueInt = varParam.availableInt.at(index);
    }
    m_param.SetValue(QVariant::fromValue(varParam));
    // 复刻修正：与 Cmd 控件行为对齐，选择后立即回写相机
    emit sigValueChanged(m_param, m_index);
}
