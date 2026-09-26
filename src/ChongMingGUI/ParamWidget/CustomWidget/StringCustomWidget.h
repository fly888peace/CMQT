#ifndef STRINGCUSTOMWIDGET_H
#define STRINGCUSTOMWIDGET_H
/**************************************************
 * String型参数的编辑控件：QLineEdit
 **************************************************/

#include "../OneCustomWidget.h"
#include <QLineEdit>

class StringCustomWidget : public OneCustomWidget {
public:
    explicit StringCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent = nullptr);

    void setParam(CameraParam& param) override;
    CameraParam getParam() override;

protected:
    void addEditLayout(QHBoxLayout* layout) override;

private slots:
    void onValueChanged();

private:
    QLineEdit* m_pLineEdit;
};

#endif // STRINGCUSTOMWIDGET_H
