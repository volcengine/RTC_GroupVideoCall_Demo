#pragma once

#include <QWidget>
#include <QPoint>
#include "push_button_warp.h"

namespace Ui {
    class ShareButtonBar;
}
/** {zh}
 * 本地内容共享的控制条,用于本地共享的时候控制音频，视频，结束共享等操作
 */

/** {en}
* The control bar for local content sharing is used to control audio, 
* video, end sharing and other operations during local sharing
*/
class ShareButtonBar : public QWidget {
    Q_OBJECT

public:
    explicit ShareButtonBar(QWidget* parent = nullptr);
    ~ShareButtonBar();
    void setMicState(bool isOn);
    void setCameraState(bool isOn);
    void setEventFilter(QWidget* w);
    void unSetEventFilter(QWidget* w);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent*) override;
    bool eventFilter(QObject* watched, QEvent* e) override;

private:
    void initConnections();
    void initCameraOption();
    void initMicOption();
    void initShareOption();

signals:
    void sigShareStateChanged(bool is_share);
    void sigOpenSetting();

private:
    Ui::ShareButtonBar* ui;
    QWidget* listener_ = nullptr;
    bool move_enabled_ = false;
    QPoint point_;
};
