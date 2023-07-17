#include "share_button_bar.h"
#include "ui_share_button_bar.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QRadioButton>
#include <unordered_map>

#include "core/util_tip.h"
#include "videocall/core/videocall_rtc_wrap.h"
#include "videocall/core/data_mgr.h"
#include "videocall/core/popup_arrow_widget.h"

static constexpr char* optionBtnQss =
"QPushButton{background: transparent;"
" border-radius: 2px;} "
"QPushButton:hover {background: rgba(255, 255, 255, 0.1);}";

static constexpr char* radioBtnQss =
"QRadioButton{ font-weight: 400; font-size: 12px; color: #FFFFFF;}"
"QRadioButton::indicator{width: 12px;height: 12px; border-radius: 6px;}"
"QRadioButton::indicator:checked{image: url(:/img/videocall_radio_button_checked)}"
"QRadioButton::indicator:unchecked{image: url(:/img/videocall_radio_button_unchecked)}";

ShareButtonBar::ShareButtonBar(QWidget *parent)
    : QWidget(parent), ui(new Ui::ShareButtonBar) {
    ui->setupUi(this);
    this->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint |
                        Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->content_widget->setAttribute(Qt::WA_TranslucentBackground);
    ui->btn_end->setText(QObject::tr("end_sharing"));

    initConnections();
    initCameraOption();
    initShareOption();
    initMicOption();
}

ShareButtonBar::~ShareButtonBar() { 
    delete ui; 
}

void ShareButtonBar::setMicState(bool isOn) {
    ui->btn_mic->setIcon(isOn ? QIcon(":/img/videocall_mic_on")
        : QIcon(":/img/videocall_mic_off"));
}

void ShareButtonBar::setCameraState(bool isOn) {
    ui->btn_camera->setIcon(isOn ? QIcon(":/img/videocall_camera_on")
        : QIcon(":/img/videocall_camera_off"));
}

void ShareButtonBar::setEventFilter(QWidget *w) {
    w->installEventFilter(this);
    listener_ = w;
}

void ShareButtonBar::unSetEventFilter(QWidget *w) {
    w->removeEventFilter(this);
}

void ShareButtonBar::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        point_ = e->pos();
    }
    QWidget::mousePressEvent(e);
}

void ShareButtonBar::mouseMoveEvent(QMouseEvent *e) {
    QPoint newLeftPos;
    newLeftPos = e->globalPos() - point_;
    this->move(newLeftPos);
    QWidget::mouseMoveEvent(e);
}

void ShareButtonBar::paintEvent(QPaintEvent *e) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QBrush(QColor(0x1d, 0x21, 0x29)));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 30, 30);
    QWidget::paintEvent(e);
}

bool ShareButtonBar::eventFilter(QObject *watched, QEvent *e) {
    auto w = static_cast<QWidget*>(watched);
    if (listener_ != w) return false;
    switch (e->type()) {
    case QEvent::Show:
        show();
    case QEvent::Resize:
    case QEvent::Move: {
        QPoint p = w->pos() + QPoint((w->frameGeometry().width() - width()) / 2,
            w->frameGeometry().height() - height() - 8);
        this->move(p);
    } break;
    case QEvent::Hide:
        hide();
        break;
    }
    return false;
}

void ShareButtonBar::initConnections() {
    connect(ui->btn_share, &QPushButton::clicked, this, [=] {
        auto cur_share_uid =
            videocall::DataMgr::instance().room().screen_shared_uid;
        if (!cur_share_uid.empty() ) {
            vrd::util::showToastInfo(QObject::tr("switch_sharing").toStdString());
            return;
        }
        emit sigShareStateChanged(true);
    });

    connect(ui->btn_end, &QPushButton::clicked, this,
        [=] { emit sigShareStateChanged(false); });

    connect(ui->btn_setting, &QPushButton::clicked, this, [=] {
        if (videocall::DataMgr::instance().room().screen_shared_uid ==
            videocall::DataMgr::instance().user_id()) {
            vrd::util::showToastInfo(QObject::tr("sharing_enter_settings").toStdString());
            return;
        }
        emit sigOpenSetting();
        });

    connect(ui->btn_mic, &QPushButton::clicked, this, [=] {
        auto mute = videocall::DataMgr::instance().mute_audio();
        if (mute) {
            if (!VideoCallRtcEngineWrap::audioRecordDevicesTest()) {
                vrd::util::showToastInfo(QObject::tr("microphone_permission_disabled").toStdString());
                return;
            }
        }
        VideoCallRtcEngineWrap::muteLocalAudio(!mute);
        VideoCallRtcEngineWrap::enableLocalAudio(mute);
        videocall::DataMgr::instance().setMuteAudio(!mute);
        setMicState(mute);
        });

    connect(ui->btn_camera, &QPushButton::clicked, this, [=] {
        auto mute = videocall::DataMgr::instance().mute_video();
        if (mute) {
            std::vector<RtcDevice> devices;
            VideoCallRtcEngineWrap::getVideoCaptureDevices(devices);
            if (devices.empty()) {
                vrd::util::showToastInfo(QObject::tr("camera_permission_disabled").toStdString());
                return;
            }
        }
        VideoCallRtcEngineWrap::muteLocalVideo(!mute);
        VideoCallRtcEngineWrap::enableLocalVideo(mute);
        videocall::DataMgr::instance().setMuteVideo(!mute);
        setCameraState(mute);
        });

}

void ShareButtonBar::initCameraOption() {
    // {zh} 摄像头选项按钮
    // {en} camera option button
    auto camera_option_btn = new QPushButton(ui->btn_camera);
    camera_option_btn->setFixedSize(QSize(12, 12));
    camera_option_btn->setIconSize(QSize(8, 8));
    camera_option_btn->setStyleSheet(optionBtnQss);
    camera_option_btn->setIcon(QIcon(":img/videocall_down_arrow"));
    camera_option_btn->move(QPoint(ui->btn_camera->width() - camera_option_btn->width() - 6, 8));
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
        int idx = 0;
        int currentIndex = RtcEngineWrap::instance().getCurrentVideoCaptureDeviceIndex();
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
                ui->btn_camera->update();
            });
        camera_option_popup->setArrowPosition(PopupArrowWidget::ArrowPosition::bottom);
        camera_option_popup->show();
        camera_option_popup->setPopupPosition();
        });
}

void ShareButtonBar::initMicOption() {
    // {zh} 麦克风选项按钮
    // {en} mic option button
    auto audio_option_btn = new QPushButton(ui->btn_mic);
    audio_option_btn->setFixedSize(QSize(12, 12));
    audio_option_btn->setIconSize(QSize(8, 8));
    audio_option_btn->setCheckable(false);
    audio_option_btn->setStyleSheet(optionBtnQss);
    audio_option_btn->setIcon(QIcon(":img/videocall_down_arrow"));
    audio_option_btn->move(QPoint(ui->btn_mic->width() - audio_option_btn->width() - 6, 8));
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
        int idx = 0;
        int currentIndex = RtcEngineWrap::instance().getCurrentAudioInputDeviceIndex();
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
                ui->btn_mic->update();
            });

        audio_option_popup->show();
        audio_option_popup->setPopupPosition();
    });
}

void ShareButtonBar::initShareOption() {
    // {zh} 共享内容选项按钮
    // {en} share option button
    auto share_option_btn = new QPushButton(ui->btn_share);
    share_option_btn->setFixedSize(QSize(12, 12));
    share_option_btn->setIconSize(QSize(8, 8));
    share_option_btn->setStyleSheet(optionBtnQss);
    share_option_btn->setIcon(QIcon(":img/videocall_down_arrow"));
    share_option_btn->move(QPoint(ui->btn_share->width() - share_option_btn->width() - 6, 8));
    share_option_btn->show();

    connect(share_option_btn, &QPushButton::clicked, [this, share_option_btn]() {
        auto share_option_popup = new PopupArrowWidget(share_option_btn);
        auto optionWidget = new QWidget(share_option_popup);
        optionWidget->setStyleSheet(radioBtnQss);
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 12, 16, 12);
        layout->setSpacing(8);

        std::vector<RtcDevice> audio_input_devices;
        VideoCallRtcEngineWrap::getAudioInputDevices(audio_input_devices);

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
                ui->btn_share->update();
            });
        });
}
