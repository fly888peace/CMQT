#ifndef BOOLCUSTOMWIDGET_H
#define BOOLCUSTOMWIDGET_H
/**************************************************
 * Bool型参数的编辑控件：QCheckBox
 **************************************************/

#include "../OneCustomWidget.h"
#include <QCheckBox>

class BoolCustomWidget : public OneCustomWidget {
public:
    explicit BoolCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent = nullptr);

    void setParam(CameraParam& param) override;
    CameraParam getParam() override;

protected:
    void addEditLayout(QHBoxLayout* layout) override;

private slots:
    void onValueChanged(Qt::CheckState state);

private:
    QCheckBox* m_pCheckBox;
};

#endif // BOOLCUSTOMWIDGET_H
