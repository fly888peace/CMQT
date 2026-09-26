#ifndef INTCUSTOMWIDGET_H
#define INTCUSTOMWIDGET_H
/**************************************************
 * Int型参数的编辑控件：QSpinBox
 **************************************************/

#include "../OneCustomWidget.h"
#include <QSpinBox>

class IntCustomWidget : public OneCustomWidget {
public:
    explicit IntCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent = nullptr);

    void setParam(CameraParam& param) override;
    CameraParam getParam() override;

protected:
    void addEditLayout(QHBoxLayout* layout) override;

private slots:
    void onValueChanged(int value);

private:
    QSpinBox* m_SpinBox;
};

#endif // INTCUSTOMWIDGET_H
