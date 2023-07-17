#include "videocall_setting.h"
#include "ui_videocall_setting.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QItemDelegate>
#include <QListView>
#include <QPainter>
#include <QTimer>

#include "videocall/core/videocall_rtc_wrap.h"
#include "videocall/core/data_mgr.h"

static constexpr char* kQss =
    "QListView{"
    "background:#fff;"
    "border:1px solid #E5E6EB;"
    "border-radius : 4px;"
    "padding:5px;"
    "color:#1D2129"
    "}"
    "QListView::item{ "
    "color:#1D2129;"
    " height : 32px;"
    "}"
    "QListView::item:hover{"
    "color:#1D2129;"
    "background:#F2F3F8;"
    "}"
    "QListView::item:selected{"
    "color:#1D2129;"
    "background:#F2F3F8;"
    "}";

static std::vector<videocall::VideoResolution> video_resolutions{
    {320, 180}, {640, 360}, {960, 540}, {1280, 720}  };

Q_DECLARE_METATYPE(videocall::VideoResolution)

VideoCallSetting::VideoCallSetting(QWidget* parent)
    : QDialog(parent), ui(new Ui::VideoCallSetting) {

    ui->setupUi(this);
    initUITranslations();
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    auto set_combobox = [](QComboBox* cmb) {
        auto list_view = new QListView(cmb);
        list_view->setStyleSheet(kQss);
        cmb->setView(list_view);
        cmb->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint |
            Qt::NoDropShadowWindowHint);
        cmb->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    };

    set_combobox(ui->cmb_quality);
    set_combobox(ui->cmb_resolution);

    auto set_resoultion_data = [](QComboBox* cmb) {
        for (auto item : video_resolutions) {
            QVariant var;
            var.setValue(item);
            cmb->addItem(QString("%1*%2").arg(item.width).arg(item.height), var);
        }
    };

    auto set_quality_data = [](QComboBox* cmb) {
        std::vector<QString> vec_fps{ QObject::tr("clarity"), QObject::tr("high_definition"), QObject::tr("extreme")};
        for (auto item : vec_fps) {
            QVariant var;
            var.setValue(item);
            cmb->addItem(QString("%1").arg(item), var);
        }
    };

    set_resoultion_data(ui->cmb_resolution);
    set_quality_data(ui->cmb_quality);
    initConnect();
}

void VideoCallSetting::onConfirm() {
    videocall::DataMgr::instance().setSetting(setting_);
    code_ = QDialog::Accepted;
    close();
}

void VideoCallSetting::onClose() {
    code_ = QDialog::Rejected;
    close();
}

void VideoCallSetting::onCancel() {
    code_ = QDialog::Rejected;
    close();
}

void VideoCallSetting::initConnect() {
    connect(
        ui->cmb_resolution, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [=](int idx) {
            setting_.camera.resolution =
                ui->cmb_resolution->currentData().value<videocall::VideoResolution>();
        });

    connect(ui->cmb_quality, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [=](int idx) {
            setting_.audio_quality = 
                static_cast<videocall::AudioQuality>(ui->cmb_quality->currentIndex());
        });

    connect(ui->mirror_camera_btn, &CheckButton::sigChecked, this,
        [=](bool checked) { setting_.enable_camera_mirror = checked; });
}

void VideoCallSetting::initUITranslations() {
    ui->lbl_title->setText(QObject::tr("settings"));
    ui->btn_confirm->setText(QObject::tr("ok"));
    ui->btn_cancel->setText(QObject::tr("cancel"));
    ui->lbl_resolutions->setText(QObject::tr("resolution"));
    ui->lbl_quality->setText(QObject::tr("call_sound_quality"));
    ui->lbl_mirror_camera->setText(QObject::tr("local_mirror"));
}

void VideoCallSetting::initView() {
    setting_ = videocall::DataMgr::instance().setting();
    ui->cmb_quality->setCurrentIndex(static_cast<int>(setting_.audio_quality));
    ui->mirror_camera_btn->setChecked(setting_.enable_camera_mirror);
    ui->cmb_resolution->setCurrentIndex(getIdxFromResolution(setting_.camera.resolution));
}

VideoCallSetting::~VideoCallSetting() { 
    delete ui; 
}

int VideoCallSetting::getIdxFromResolution(const videocall::VideoResolution& resolution) {
    int idx = -1;
    for (const auto& iter : video_resolutions) {
        idx++;
        if (resolution.width == iter.width &&
            resolution.height == iter.height)
            return idx;
    }
    return idx;
}

void VideoCallSetting::showEvent(QShowEvent* e) {}

bool VideoCallSetting::eventFilter(QObject* o, QEvent* e) { return false; }

void VideoCallSetting::closeEvent(QCloseEvent* e) {
    e->ignore();
    if (code_ == QDialog::Accepted) {
        this->accept();
    }
    else {
        reject();
    }
}
