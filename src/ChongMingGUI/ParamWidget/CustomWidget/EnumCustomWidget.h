#ifndef ENUMCUSTOMWIDGET_H
#define ENUMCUSTOMWIDGET_H
/**************************************************
 * Enum型参数的编辑控件：QComboBox
 **************************************************/

#include "../OneCustomWidget.h"
#include <QComboBox>

class EnumCustomWidget : public OneCustomWidget {
public:
    explicit EnumCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent = nullptr);

    void setParam(CameraParam& param) override;
    CameraParam getParam() override;

protected:
    void addEditLayout(QHBoxLayout* layout) override;

private slots:
    void onValueChanged(int index);

private:
    QComboBox* m_pCombox;
};

#endif // ENUMCUSTOMWIDGET_H
