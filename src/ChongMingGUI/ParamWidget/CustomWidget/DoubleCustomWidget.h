#ifndef DOUBLECUSTOMWIDGET_H
#define DOUBLECUSTOMWIDGET_H
/**************************************************
 * Double型参数的编辑控件：QDoubleSpinBox
 **************************************************/

#include "../OneCustomWidget.h"
#include <QDoubleSpinBox>

class DoubleCustomWidget : public OneCustomWidget {
public:
    explicit DoubleCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent = nullptr);

    void setParam(CameraParam& param) override;
    CameraParam getParam() override;

protected:
    void addEditLayout(QHBoxLayout* layout) override;

private slots:
    void onValueChanged(double value);

private:
    QDoubleSpinBox* m_SpinBox;
};

#endif // DOUBLECUSTOMWIDGET_H
