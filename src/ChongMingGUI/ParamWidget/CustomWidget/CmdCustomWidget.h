#ifndef CMDCUSTOMWIDGET_H
#define CMDCUSTOMWIDGET_H
/**************************************************
 * Cmd型参数的编辑控件：QPushButton（点击即向相机下发命令）
 **************************************************/

#include "../OneCustomWidget.h"
#include <QPushButton>

class CmdCustomWidget : public OneCustomWidget {
public:
    explicit CmdCustomWidget(CameraParam param, const QModelIndex& index, QWidget* parent = nullptr);

    void setParam(CameraParam& param) override;
    CameraParam getParam() override;

protected:
    void addEditLayout(QHBoxLayout* layout) override;

private slots:
    void onCmdButtonClicked();

private:
    QPushButton* m_pCmdButton;
};

#endif // CMDCUSTOMWIDGET_H
