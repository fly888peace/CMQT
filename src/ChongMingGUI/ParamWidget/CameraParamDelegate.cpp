#include "CameraParamDelegate.h"
#include "CameraParamModel.h"
#include "CustomWidget/BoolCustomWidget.h"
#include "CustomWidget/CmdCustomWidget.h"
#include "CustomWidget/DoubleCustomWidget.h"
#include "CustomWidget/EnumCustomWidget.h"
#include "CustomWidget/IntCustomWidget.h"
#include "CustomWidget/StringCustomWidget.h"
#include "OneCustomWidget.h"

CameraParamDelegate::CameraParamDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

CameraParamDelegate::~CameraParamDelegate()
{
}

// 根据传入的QModelIndex来决定创建何种编辑器
QWidget* CameraParamDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)

    auto createWidget = [](CameraParam& cameraParam, const QModelIndex& index, QWidget* parent) -> OneCustomWidget* {
        switch (cameraParam.type()) {
        case STRING:
            return new StringCustomWidget(cameraParam, index, parent);
        case CMD:
            return new CmdCustomWidget(cameraParam, index, parent);
        case INT:
            return new IntCustomWidget(cameraParam, index, parent);
        case DOUBLE:
            return new DoubleCustomWidget(cameraParam, index, parent);
        case BOOL:
            return new BoolCustomWidget(cameraParam, index, parent);
        case ENUM:
            return new EnumCustomWidget(cameraParam, index, parent);
        default:
            return nullptr;
        }
    };

    if (index.column() == CameraParamModel::ColType::VALUE) {
        // 只为第2列数据列做特殊处理
        const QVariant varParam = index.data(CameraParamModel::ParamRole);
        CameraParam cameraParam = varParam.value<CameraParam>();

        auto* oneCustomWidget = createWidget(cameraParam, index, parent);
        if (oneCustomWidget) {
            oneCustomWidget->InitWidget();
            connect(oneCustomWidget, &OneCustomWidget::sigValueChanged, this,
                &CameraParamDelegate::onValueChanged, Qt::UniqueConnection);
        }
        return oneCustomWidget;
    }
    return nullptr;
}

// 通过传入的QModelIndex来设置页面的值
void CameraParamDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    // 只为第2列数据列做特殊处理
    if (index.column() == CameraParamModel::ColType::VALUE) {
        OneCustomWidget* pCustomEdit = qobject_cast<OneCustomWidget*>(editor);

        const QVariant varParam = index.data(CameraParamModel::ParamRole);
        CameraParam cameraParam = varParam.value<CameraParam>();

        pCustomEdit->setParam(cameraParam);
    }
}

// 通过界面的传值设置model模型里的值
void CameraParamDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    // 只为第2列数据列做特殊处理
    if (index.column() == CameraParamModel::ColType::VALUE) {
        OneCustomWidget* pCustomEdit = qobject_cast<OneCustomWidget*>(editor);
        CameraParam cameraParam = pCustomEdit->getParam();
        model->setData(index, QVariant::fromValue(cameraParam), CameraParamModel::ParamRole);
    }
}

void CameraParamDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

void CameraParamDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    painter->save();

    const bool enabled = option.state & QStyle::State_Enabled;

    // 1. 绘制选中背景（复刻修正：半透明高亮替代不透明填充——
    // 源工程禁用态下 palette.highlight 是不透明深色，拉流时整格变黑框盖住文字，
    // 用户看不到自己之前选的是哪个参数）
    if (option.state & QStyle::State_Selected) {
        QColor hlColor = option.palette.highlight().color();
        hlColor.setAlpha(enabled ? 90 : 45); // 正常态 35% 透明，禁用态再淡一点
        painter->fillRect(option.rect, hlColor);
    }

    // 2. 绘制文本内容（不需要缩进处理；颜色按可用/禁用状态取，保证选中底上可读）
    QString text = index.data().toString();
    QColor textColor = enabled
        ? option.palette.color(QPalette::Active, QPalette::Text)
        : option.palette.color(QPalette::Disabled, QPalette::Text);
    painter->setPen(textColor);
    painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, text);

    // 3. 绘制网格线
    QPen pen(QColor(220, 220, 220), 1, Qt::SolidLine);
    painter->setPen(pen);

    // 绘制右边框（列分割线）
    painter->drawLine(option.rect.topRight(), option.rect.bottomRight());

    // 绘制下边框（行分割线）
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

    painter->restore();
}

QSize CameraParamDelegate::sizeHint(const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);

    // 增加高度，保持宽度不变
    size.setHeight(20);
    return size;
}

void CameraParamDelegate::onValueChanged(const CameraParam& param, const QModelIndex& index)
{
    OneCustomWidget* pCustomEdit = qobject_cast<OneCustomWidget*>(sender());
    if (pCustomEdit) {
        if (index.isValid()) {
            // 更新Model数据
            QAbstractItemModel* model = const_cast<QAbstractItemModel*>(index.model());
            if (model) {
                model->setData(index, QVariant::fromValue(param), CameraParamModel::ParamRole);
            }
        }
    }
}
