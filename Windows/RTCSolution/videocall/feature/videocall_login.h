#pragma once
#include "ui_videocall_login.h"
/** {zh}
 * 音视频通话登录页面，可输入用户名，房间号，控制音频，视频，然后进入通话
 */

/** {en}
* Video call login page, you can enter the user name, room number, 
* control audio, control video, and then enter the call
*/
class VideoCallLoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoCallLoginWidget(QWidget* parent = nullptr);
    void setMicState(bool on);
    void setCameraState(bool on);

signals:
    void sigClose();

protected:
    void showEvent(QShowEvent*)override;
    void closeEvent(QCloseEvent*)override;

private:
    void initUi();
    void initConnections();
    void validateUserId(QString str);
    void validateRoomId(QString str);

private:
    bool login_ = false;
    bool user_name_error_{ false };
    bool room_id_error_{ false };
    Ui::VideoCallLoginWidget ui;
};
