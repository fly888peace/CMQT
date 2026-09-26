#include "IntCustomWidget.h"

IntCustomWidget::IntCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent)
    : OneCustomWidget(param, index, parent)
    , m_SpinBox(new QSpinBox(this))
{
    m_SpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void IntCustomWidget::setParam(CameraParam& param)
{
    disconnect(m_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &IntCustomWidget::onValueChanged);

    OneCustomWidget::setParam(param);
    IntParam varParam = m_param.GetValue().value<IntParam>();
    // 复刻修正（DEV_SPEC 附录 #6）：QSpinBox 是 int32，海康 INT 节点是 int64，
    // 超出 int32 范围直接传入会溢出——这里做钳制（更大范围的参数建议换 QLineEdit 方案）
    auto clamp32 = [](int64_t v) -> int {
        if (v < INT32_MIN)
            return INT32_MIN;
        if (v > INT32_MAX)
            return INT32_MAX;
        return (int)v;
    };
    m_SpinBox->setMinimum(clamp32(varParam.min));
    m_SpinBox->setMaximum(clamp32(varParam.max));
    m_SpinBox->setSingleStep(clamp32(varParam.increment > 0 ? varParam.increment : 1));
    m_SpinBox->setValue(clamp32(varParam.value));

    connect(m_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
        this, &IntCustomWidget::onValueChanged);
}

CameraParam IntCustomWidget::getParam()
{
    return OneCustomWidget::getParam();
}

void IntCustomWidget::addEditLayout(QHBoxLayout* layout)
{
    layout->addWidget(m_SpinBox);
}

void IntCustomWidget::onValueChanged(int value)
{
    IntParam varParam = getParam().GetValue().value<IntParam>();
    varParam.value = value;
    m_param.SetValue(QVariant::fromValue(varParam));
    // 复刻修正：与 Cmd 控件行为对齐，值一变立即通知 model 回写相机
    // （源工程只靠编辑器关闭时的 setModelData，点别处才生效）
    emit sigValueChanged(m_param, m_index);
}
