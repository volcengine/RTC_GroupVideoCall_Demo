#include "videocall_main_page.h"
#include "ui_videocall_main_page.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QTimer>
#include <QDialog>
#include <QPointer>
#include <QIcon>
#include <QToolButton>
#include <QRadioButton>
#include <QPushButton>

#include <algorithm>

#include "videocall/core/popup_arrow_widget.h"
#include "videocall/core/videocall_video_widget.h"
#include "core/util_tip.h"
#include "videocall/core/videocall_rtc_wrap.h"
#include "videocall/core/videocall_manager.h"
#include "videocall/core/data_mgr.h"
#include "videocall/feature/normal_video_view.h"
#include "videocall/feature/focus_video_view.h"

static constexpr char* optionBtnQss =
"QPushButton{background: transparent;"
" border-radius: 2px;} "
"QPushButton:hover {background: rgba(255, 255, 255, 0.1);}";

static constexpr char* radioBtnQss =
"QRadioButton{ font-weight: 400; font-size: 12px; color: #FFFFFF;}"
"QRadioButton::indicator{width: 12px;height: 12px; border-radius: 6px;}"
"QRadioButton::indicator:checked{image: url(:/img/videocall_radio_button_checked)}"
"QRadioButton::indicator:unchecked{image: url(:/img/videocall_radio_button_unchecked)}";

VideoCallMainPage::VideoCallMainPage(QWidget *parent) 
    : QWidget(parent)
    , ui(new Ui::VideoCallMainPage) {

    ui->setupUi(this);
    initUi();
    initConnections();
}

VideoCallMainPage::~VideoCallMainPage() {
    delete ui;
}

void VideoCallMainPage::updateVideoWidget() {
    auto vec = videocall::DataMgr::instance().users();
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i].user_id == videocall::DataMgr::instance().user_id()) {
            videocall::VideoCallManager::setLocalVideoWidget(vec[i], i);
        }
        else {
            videocall::VideoCallManager::setRemoteVideoWidget(vec[i], i);
        }
    }
    showWidget(vec.size());
}

void VideoCallMainPage::showWidget(int cnt) {
    if (current_page_ == kNormalPage) {
        static_cast<NormalVideoView*>(ui->stackedWidget->widget(current_page_))
            ->showWidget(cnt);
    }
    else {
        static_cast<FocusVideoView*>(ui->stackedWidget->widget(current_page_))
            ->showWidget(cnt);
    }
}

void VideoCallMainPage::init() {
    setDefaultProfiles();

    setCameraState(!videocall::DataMgr::instance().mute_video());
    setMicState(!videocall::DataMgr::instance().mute_audio());
    setBasicBeauty(true);

    tick_count_ = videocall::DataMgr::instance().room().duration;
    const auto& roomId = videocall::DataMgr::instance().room_id();
    auto find_pos = roomId.find("call_");
    if (find_pos != std::string::npos) {
        // {zh} 5只得是前缀"call_"的长度
        // {en} 5 means the length of string prefix "call_"
        ui->lbl_room_id->setText(roomId.substr(find_pos + 5).c_str());
    }
    else {
        ui->lbl_room_id->setText(roomId.c_str());
    }
    ui->lbl_time->setText("00:00");
    main_timer_->start(1000);

    static_cast<FocusVideoView*>(ui->stackedWidget->widget(VideoCallMainPage::kFocusPage))->init();
    static_cast<NormalVideoView*>(ui->stackedWidget->widget(VideoCallMainPage::kNormalPage))->init();
    changeViewMode(0);
    froce_close_ = false;
}

void VideoCallMainPage::setCameraState(bool on) {
    ui->cameraBtn->setIcon(on ? QIcon(":/img/videocall_camera_on")
        : QIcon(":/img/videocall_camera_off"));
}

void VideoCallMainPage::setMicState(bool on){
    ui->micBtn->setIcon(on ? QIcon(":/img/videocall_mic_on")
        : QIcon(":/img/videocall_mic_off"));
}

void VideoCallMainPage::setBasicBeauty(bool enabled) {
    beauty_enabled_ = enabled;
    VideoCallRtcEngineWrap::setBasicBeauty(enabled);
    ui->beautyBtn->setIcon(enabled ? QIcon(":/img/videocall_beauty")
        : QIcon(":/img/videocall_beauty_off"));
}

void VideoCallMainPage::changeViewMode(int mode) {
    ui->stackedWidget->setCurrentIndex(mode);
    current_page_ = mode;
    updateVideoWidget();
    if (current_page_ == kNormalPage) {
        static_cast<NormalVideoView*>(ui->stackedWidget->widget(current_page_))
            ->showWidget(videocall::DataMgr::instance().users().size(), true);
    }
}

int VideoCallMainPage::viewMode() { 
    return current_page_; 
}

void VideoCallMainPage::froceClose() {
    froce_close_ = true;
    close();
}

void VideoCallMainPage::closeEvent(QCloseEvent *e) {
    if (!froce_close_ &&
        videocall::VideoCallManager::showCallExpDlg(this) == QDialog::Rejected) {
        e->ignore();
        return;
    }

    main_timer_->stop();
    emit sigClose();
}

void VideoCallMainPage::initUi() {
    tick_count_ = 0;
    ui->stackedWidget->addWidget(new NormalVideoView(this));
    ui->stackedWidget->addWidget(new FocusVideoView(this));
    ui->stackedWidget->setContentsMargins(0, 0, 0, 0);
    main_timer_ = new QTimer(this);

    initCameraOption();
    initMicOption();
    initShareOption();
}

void VideoCallMainPage::setDefaultProfiles() {
    videocall::VideoConfiger camera{ {1280, 720}, 15, -1 };
    VideoCallRtcEngineWrap::setVideoProfiles(camera);

    videocall::VideoConfiger screen{ { 1280, 720 },15, -1 };
    VideoCallRtcEngineWrap::setScreenProfiles(screen);

    VideoCallRtcEngineWrap::setAudioProfiles(videocall::AudioQuality::kAudioQualityStandard);
    VideoCallRtcEngineWrap::setLocalMirrorMode(bytertc::MirrorType::kMirrorTypeRenderAndEncoder);
}

void VideoCallMainPage::initConnections() {
    connect(main_timer_, &QTimer::timeout, this, [=] {
        tick_count_++;
        auto time =
            QString::asprintf("%02lld:%02lld", tick_count_ / 60, tick_count_ % 60);
        ui->lbl_time->setText(time);
    });

    connect(ui->endCallBtn, &QToolButton::clicked, this, &QWidget::close);

    connect(ui->shareBtn, &QToolButton::clicked, this, [=] {
        auto cur_share_uid =
            videocall::DataMgr::instance().room().screen_shared_uid;
        if (!cur_share_uid.empty() &&
                cur_share_uid != videocall::DataMgr::instance().user_id()) {
            vrd::util::showToastInfo(QObject::tr("grab_sharing").toStdString());
            return;
        }
        emit sigShareButtonClicked();
    });

    connect(ui->settingBtn, &QToolButton::clicked, this, [=] {
        emit sigVideoCallSetting();
    });

    connect(ui->dataBtn, &QToolButton::clicked, this, [=] {
        emit sigRealTimeDataClicked();
    });

    connect(ui->micBtn, &QToolButton::clicked, this, [=] {
        auto mute = videocall::DataMgr::instance().mute_audio();
        if (mute) {
            if (!VideoCallRtcEngineWrap::audioRecordDevicesTest()) {
                vrd::util::showToastInfo(QObject::tr("microphone_permission_disabled").toStdString());
                return;
            }
        }
        setMicState(mute);
        VideoCallRtcEngineWrap::muteLocalAudio(!mute);
        VideoCallRtcEngineWrap::enableLocalAudio(mute);
        videocall::DataMgr::instance().setMuteAudio(!mute);
    });

    connect(ui->cameraBtn, &QToolButton::clicked, this, [=] {
        auto mute = videocall::DataMgr::instance().mute_video();
        if (mute) {
            std::vector<RtcDevice> devices;
            VideoCallRtcEngineWrap::getVideoCaptureDevices(devices);
            if (devices.empty()) {
                vrd::util::showToastInfo(QObject::tr("camera_permission_disabled").toStdString());
                return;
            }
        }

        setCameraState(mute);
        VideoCallRtcEngineWrap::muteLocalVideo(!mute);
        VideoCallRtcEngineWrap::enableLocalVideo(mute);
        videocall::DataMgr::instance().setMuteVideo(!mute);
        emit sigCameraEnabled(mute);
    });

    connect(ui->beautyBtn, &QToolButton::clicked, this, [=]() {
        setBasicBeauty(!beauty_enabled_);
    });
}

void VideoCallMainPage::initCameraOption() {
    // {zh} 摄像头选项按钮
    // {en} camera option button
    auto camera_option_btn = new QPushButton(ui->cameraBtn);
    camera_option_btn->setFixedSize(QSize(16, 16));
    camera_option_btn->setIconSize(QSize(12, 12));
    camera_option_btn->setStyleSheet(optionBtnQss);
    camera_option_btn->setIcon(QIcon(":img/videocall_down_arrow"));
    camera_option_btn->move(QPoint(ui->cameraBtn->width() - 25, 8));
    camera_option_btn->show();

    connect(camera_option_btn, &QPushButton::clicked, [this, camera_option_btn]() {
        auto camera_option_popup = new PopupArrowWidget(camera_option_btn);
        auto optionWidget = new QWidget(camera_option_popup);
        optionWidget->setStyleSheet(radioBtnQss);
        QVBoxLayout* layout = new QVBoxLayout(optionWidget);
        layout->setContentsMargins(16, 12, 16, 12);
        layout->setSpacing(8);

        std::vector<RtcDevice> camera_devices;
        VideoCallRtcEngineWrap::getVideoCaptureDevices(camera_devices);
        int currentIndex = RtcEngineWrap::instance().getCurrentVideoCaptureDeviceIndex();
        int idx = 0;
        for (auto& dc : camera_devices) {
            auto radioBtn = new QRadioButton(dc.name.c_str());
            radioBtn->setCheckable(true);
            connect(radioBtn, &QRadioButton::clicked, [idx, radioBtn, this]() {
                radioBtn->setChecked(true);
                VideoCallRtcEngineWrap::setVideoCaptureDevice(idx);
                });

            if (currentIndex == idx) {
                radioBtn->setChecked(true);
            }
            layout->addWidget(radioBtn);
            idx++;
        }

        optionWidget->setLayout(layout);
        camera_option_popup->addCustomWidget(optionWidget);

        connect(&VideoCallRtcEngineWrap::instance(), &VideoCallRtcEngineWrap::sigUpdateVideoDevices,
            camera_option_popup, [camera_option_popup]() {
                if (camera_option_popup && camera_option_popup->isVisible()) {
                    camera_option_popup->setParent(nullptr);
                    camera_option_popup->close();
                }
        });

        connect(camera_option_popup, &PopupArrowWidget::widgetVisiblilityChanged,
            [this, camera_option_btn](bool isVisibled) {
                camera_option_btn->setIcon(isVisibled
                    ? QIcon(":img/videocall_up_arrow") : QIcon(":img/videocall_down_arrow"));
                ui->cameraBtn->update();
        });
        camera_option_popup->setArrowPosition(PopupArrowWidget::ArrowPosition::bottom);
        camera_option_popup->show();
        camera_option_popup->setPopupPosition();
    });
}

void VideoCallMainPage::initMicOption() {
    // {zh} 麦克风选项按钮
    // {en} mic option button
    auto audio_option_btn = new QPushButton(ui->micBtn);
    audio_option_btn->setFixedSize(QSize(16, 16));
    audio_option_btn->setIconSize(QSize(12, 12));
    audio_option_btn->setCheckable(false);
    audio_option_btn->setStyleSheet(optionBtnQss);
    audio_option_btn->setIcon(QIcon(":img/videocall_down_arrow"));
    audio_option_btn->move(QPoint(ui->micBtn->width() - 25, 8));
    audio_option_btn->show();

    connect(audio_option_btn, &QPushButton::clicked, [this, audio_option_btn]() {
        auto audio_option_popup = new PopupArrowWidget(audio_option_btn);
        auto optionWidget = new QWidget(audio_option_popup);
        optionWidget->setStyleSheet(radioBtnQss);
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 12, 16, 12);
        layout->setSpacing(8);

        std::vector<RtcDevice> audio_input_devices;
        VideoCallRtcEngineWrap::getAudioInputDevices(audio_input_devices);
        int currentIndex = RtcEngineWrap::instance().getCurrentAudioInputDeviceIndex();
        int idx = 0;
        for (auto& dc : audio_input_devices) {
            auto radioBtn = new QRadioButton(dc.name.c_str());
            connect(radioBtn, &QRadioButton::clicked, [idx, radioBtn, this]() {
                radioBtn->setChecked(true);
                VideoCallRtcEngineWrap::setAudioInputDevice(idx);
            });

            if (currentIndex == idx) {
                radioBtn->setChecked(true);
            }
            layout->addWidget(radioBtn);
            idx++;
        }

        optionWidget->setLayout(layout);
        audio_option_popup->addCustomWidget(optionWidget);
        audio_option_popup->setArrowPosition(PopupArrowWidget::ArrowPosition::bottom);

        connect(&VideoCallRtcEngineWrap::instance(), &VideoCallRtcEngineWrap::sigUpdateAudioDevices,
            audio_option_popup, [audio_option_popup]() {
                if (audio_option_popup && audio_option_popup->isVisible()) {
                    audio_option_popup->setParent(nullptr);
                    audio_option_popup->close();
                }
            });
        
        connect(audio_option_popup, &PopupArrowWidget::widgetVisiblilityChanged,
            [this, audio_option_btn](bool isVisibled) {
                audio_option_btn->setIcon(isVisibled
                    ? QIcon(":img/videocall_up_arrow") : QIcon(":img/videocall_down_arrow"));
                ui->micBtn->update();
            });

        audio_option_popup->show();
        audio_option_popup->setPopupPosition();
    });
}

void VideoCallMainPage::initShareOption() {
    // {zh} 共享内容选项按钮
    // {en} share option button
    auto share_option_btn = new QPushButton(ui->shareBtn);
    share_option_btn->setFixedSize(QSize(16, 16));
    share_option_btn->setIconSize(QSize(12, 12));
    share_option_btn->setStyleSheet(optionBtnQss);
    share_option_btn->setIcon(QIcon(":img/videocall_down_arrow"));
    share_option_btn->move(QPoint(ui->shareBtn->width() - 25, 8));
    share_option_btn->show();

    connect(share_option_btn, &QPushButton::clicked, [this, share_option_btn]() {
        auto share_option_popup = new PopupArrowWidget(share_option_btn);
        auto optionWidget = new QWidget(share_option_popup);
        optionWidget->setStyleSheet(radioBtnQss);
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 12, 16, 12);
        layout->setSpacing(8);

        auto radioBtn1 = new QRadioButton(QObject::tr("clarity_priority"));
        connect(radioBtn1, &QRadioButton::clicked, []() {
            videocall::VideoConfiger screen;
            screen.resolution = videocall::VideoResolution{ 1280, 720 };
            VideoCallRtcEngineWrap::setScreenProfiles(screen);
            videocall::DataMgr::instance().setShareQualityIndex(0);
            });
        if (videocall::DataMgr::instance().share_quality_index() == 0) {
            radioBtn1->setChecked(true);
        }
        layout->addWidget(radioBtn1);

        auto radioBtn2 = new QRadioButton(QObject::tr("fluency_priority"));
        connect(radioBtn2, &QRadioButton::clicked, []() {
            videocall::VideoConfiger screen;
            screen.resolution = videocall::VideoResolution{ 640, 360 };
            VideoCallRtcEngineWrap::setScreenProfiles(screen);
            videocall::DataMgr::instance().setShareQualityIndex(1);
            });
        if (videocall::DataMgr::instance().share_quality_index() == 1) {
            radioBtn2->setChecked(true);
        }
        layout->addWidget(radioBtn2);

        optionWidget->setLayout(layout);
        share_option_popup->addCustomWidget(optionWidget);
        share_option_popup->setArrowPosition(PopupArrowWidget::ArrowPosition::bottom);
        share_option_popup->show();
        share_option_popup->setPopupPosition();

        connect(share_option_popup, &PopupArrowWidget::widgetVisiblilityChanged,
            [this, share_option_btn](bool isVisibled) {
                share_option_btn->setIcon(isVisibled
                    ? QIcon(":img/videocall_up_arrow") : QIcon(":img/videocall_down_arrow"));
                ui->shareBtn->update();
            });
    });
}
