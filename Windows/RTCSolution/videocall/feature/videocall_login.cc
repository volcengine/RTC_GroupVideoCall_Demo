#include "videocall_login.h"
#include <Windows.h>
#include <QTimer>

#include "core/util_tip.h"
#include "core/component/toast.h"
#include "videocall/core/data_mgr.h"
#include "videocall/core/videocall_rtc_wrap.h"
#include "videocall/core/videocall_session.h"
#include "videocall/core/videocall_manager.h"

static constexpr char* kEdtError =
"font-family : 'Microsoft YaHei UI';"
"font-size : 14px;"
"padding-left : 16px;"
"color : #fff;"
"background : #1C222D;"
"border : 1px solid #F53F3F;"
"border-radius : 8px;";


VideoCallLoginWidget::VideoCallLoginWidget(QWidget* parent) : QWidget(parent) {
    ui.setupUi(this);
	initUi();
    initConnections();
}

void VideoCallLoginWidget::setMicState(bool on) {
    ui.micBtn->setIcon(on ? QIcon(":/img/videocall_mic_on") : QIcon(":/img/videocall_mic_off"));
    ui.micLabel->setText(on ? tr("microphone_enabled") : tr("microphone_disabled"));
}


void VideoCallLoginWidget::setCameraState(bool on) {
    ui.cameraBtn->setIcon(on ? QIcon(":/img/videocall_camera_on") : QIcon(":/img/videocall_camera_off"));
    ui.cameraLabel->setText(on ? tr("camera_enabled") : tr("camera_disabled"));
}

void VideoCallLoginWidget::showEvent(QShowEvent*) {
    if (!VideoCallRtcEngineWrap::audioRecordDevicesTest()) {
        vrd::util::showToastInfo(QObject::tr("microphone_permission_disabled").toStdString());
    }
    else {
        VideoCallRtcEngineWrap::muteLocalAudio(false);
        VideoCallRtcEngineWrap::enableLocalAudio(true);
        setMicState(true);
        videocall::DataMgr::instance().setMuteAudio(false);
    }

    std::vector<RtcDevice> devices;
    VideoCallRtcEngineWrap::getVideoCaptureDevices(devices);
    if (devices.empty()) {
        vrd::util::showToastInfo(QObject::tr("camera_permission_disabled").toStdString());
    }
    else {
        VideoCallRtcEngineWrap::muteLocalVideo(false);
        VideoCallRtcEngineWrap::enableLocalVideo(true);
        setCameraState(true);
        videocall::DataMgr::instance().setMuteVideo(false);
    }

    ui.edt_user_name->setText(videocall::DataMgr::instance().user_name().c_str());
     ui.lbl_demo_ver->setText(QObject::tr("app_version_vxxx").arg("12.0.8"));

	ui.lbl_sdk_ver->setText(QObject::tr("sdk_version_vxxx").arg(RtcEngineWrap::getSDKVersion().c_str()));
    videocall::DataMgr::instance().setUserName(videocall::DataMgr::instance().user_name());
}

void VideoCallLoginWidget::closeEvent(QCloseEvent*) {
	videocall::DataMgr::instance().setSetting(videocall::VideoCallSettingModel());
	emit sigClose();
}

void VideoCallLoginWidget::initUi() {
    setWindowTitle(QObject::tr("log_in"));
    resize(QSize(960, 700));
    ui.btn_login->setEnabled(false);
    ui.edt_room_id->clear();
    ui.edt_user_name->clear();
    ui.txt_msg->setText("");
    ui.txt_msg2->setText("");
}

void VideoCallLoginWidget::initConnections() {
    connect(ui.edt_room_id, &QLineEdit::textChanged, this,
        &VideoCallLoginWidget::validateRoomId);
    connect(ui.edt_user_name, &QLineEdit::textChanged, this,
        &VideoCallLoginWidget::validateUserId);

    connect(ui.btn_login, &QPushButton::clicked, this,
        [=] {
            if (login_) return;
            login_ = true;
            videocall::DataMgr::instance().setUserName(std::string(ui.edt_user_name->text().toUtf8()));
            // {zh} 清除可能在其他房间的相同用户
            // {en} Clear the same user who may be in another room
            vrd::VideoCallSession::instance().cleanUser(videocall::DataMgr::instance().user_id(), 
                [=](int code) {
                    auto roomId = QString("call_").append(ui.edt_room_id->text());
                    vrd::VideoCallSession::instance().joinCall(
                        videocall::DataMgr::instance().user_id(),
                        roomId.toStdString(), [=](int code) {
                            if (code == 200) {
                                vrd::VideoCallSession::instance().setRoomId(videocall::DataMgr::instance().room_id());
                                hide();
                                videocall::VideoCallManager::initRoom();
                                VideoCallRtcEngineWrap::login(
                                    videocall::DataMgr::instance().room_id(),
                                    videocall::DataMgr::instance().user_id(),
                                    videocall::DataMgr::instance().token());
                                // {zh} 适配无摄像头权限进房后移动端头像画面初始化失败
                                // {en} Solve the issue that the avatar of the mobile app fails to initialize after entering the room without camera permission
                                VideoCallRtcEngineWrap::muteLocalVideo(videocall::DataMgr::instance().mute_video());
                                VideoCallRtcEngineWrap::enableLocalVideo(!videocall::DataMgr::instance().mute_video());
                                VideoCallRtcEngineWrap::muteLocalAudio(videocall::DataMgr::instance().mute_audio());
                                VideoCallRtcEngineWrap::enableLocalAudio(!videocall::DataMgr::instance().mute_audio());
                            }
                            login_ = false;
                        });
                });
        });

    connect(ui.micBtn, &QPushButton::clicked,
        this, [=] {
            if (!VideoCallRtcEngineWrap::audioRecordDevicesTest()) {
                vrd::util::showToastInfo(QObject::tr("microphone_permission_disabled").toStdString());
                return;
            }
            bool enabled = !videocall::DataMgr::instance().mute_audio();
            if (!VideoCallRtcEngineWrap::muteLocalAudio(enabled)) {
                setMicState(!enabled);
                videocall::DataMgr::instance().setMuteAudio(enabled);
            }
        });

    connect(ui.cameraBtn, &QPushButton::clicked,
        this, [=] {
            std::vector<RtcDevice> devices;
            VideoCallRtcEngineWrap::getVideoCaptureDevices(devices);
            if (devices.empty()) {
                vrd::util::showToastInfo(QObject::tr("camera_permission_disabled").toStdString());
                return;
            }
            bool enabled = !videocall::DataMgr::instance().mute_video();
            if (!VideoCallRtcEngineWrap::muteLocalVideo(enabled)) {
                VideoCallRtcEngineWrap::enableLocalVideo(!enabled);
                setCameraState(!enabled);
                videocall::DataMgr::instance().setMuteVideo(enabled);
            }
        });
}

void VideoCallLoginWidget::validateUserId(QString str) {
    user_name_error_ = false;
    auto is_overflow = false;
    if (str.size() > 128) {
        ui.edt_user_name->setText(str.left(128));
        is_overflow = true;
    }
    auto tmp = ui.edt_user_name->text();
    for (auto& ch : tmp) {
        ushort uNum = ch.unicode();
        if (!(ch.isDigit() || ch.isUpper() || ch.isLower() || (uNum >= 0x4E00 && uNum <= 0x9FA5))) {
            user_name_error_ = true;
            break;
        }
    }

    if (user_name_error_ || is_overflow) {
        ui.txt_msg2->setText(tr("name_error"));
        ui.edt_user_name->setState(LineEditState::kError);
        ui.edt_user_name->setStyleSheet(kEdtError);
        if (is_overflow) {
            ui.txt_msg2->setText(tr("name_limit_error"));
            QTimer* timer = new QTimer(this);
            connect(timer, &QTimer::timeout, [=] {
                ui.txt_msg2->setText("");
                ui.edt_user_name->setStyleSheet("");
                timer->stop();
                });
            timer->start(2000);
        }
    }
    else {
        ui.txt_msg2->setText("");
        ui.edt_user_name->setState(LineEditState::kNormal);
        ui.edt_user_name->setStyleSheet("");
    }

    if (!room_id_error_ && !user_name_error_ && !ui.edt_room_id->text().isEmpty() &&
        !ui.edt_user_name->text().isEmpty()) {
        ui.btn_login->setEnabled(true);
    }
    else {
        ui.btn_login->setEnabled(false);
    }
}

void VideoCallLoginWidget::validateRoomId(QString str) {
    room_id_error_ = false;
    auto is_overflow = false;
    if (str.size() > 18) {
        ui.edt_room_id->setText(str.left(18));
        is_overflow = true;
    }

    auto tmp = ui.edt_room_id->text();
    for (auto& ch : tmp) {
        if (!ch.isDigit()) room_id_error_ = true;
    }

    if (room_id_error_ || is_overflow) {
        ui.txt_msg->setText(tr("room_number_error_content_limit"));
        ui.edt_room_id->setState(LineEditState::kError);
        ui.edt_room_id->setStyleSheet(kEdtError);
        
        if (is_overflow) {
            QTimer* timer = new QTimer(this);
            connect(timer, &QTimer::timeout, [=] {
                ui.txt_msg->setText("");
                ui.edt_room_id->setStyleSheet("");
                timer->stop();
                });
            timer->start(2000);
        }
    }
    else {
        ui.txt_msg->setText("");
        ui.edt_room_id->setState(LineEditState::kNormal);
        ui.edt_room_id->setStyleSheet("");
    }

    if (!room_id_error_ && !user_name_error_ && !ui.edt_room_id->text().isEmpty() &&
        !ui.edt_user_name->text().isEmpty()) {
        ui.btn_login->setEnabled(true);
    }
    else {
        ui.btn_login->setEnabled(false);
    }
}