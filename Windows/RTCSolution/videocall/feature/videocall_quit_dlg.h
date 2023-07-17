#pragma once

#include <QDialog>

namespace Ui {
class VideoCallQuitDlg;
}
/** {zh}
 * 结束通话的再次确认页面
 */

/** {en}
* Reconfirmation page for ending a call
*/
class VideoCallQuitDlg : public QDialog
{
    Q_OBJECT
public:
    explicit VideoCallQuitDlg(QWidget *parent = nullptr);
    ~VideoCallQuitDlg();
    void initView();
private:
    Ui::VideoCallQuitDlg*ui;
};
