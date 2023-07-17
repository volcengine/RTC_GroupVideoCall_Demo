#include "videocall_share_widget.h"
#include "ui_videocall_share_widget.h"
#include "videocall/core/videocall_manager.h"

#include "core/util_tip.h"
#include "videocall/core/videocall_rtc_wrap.h"
#include "videocall/core/videocall_session.h"
#include "videocall/core/data_mgr.h"
#include "core/component/share_view_wnd.h"

#include <QDebug>

VideoCallShareWidget::VideoCallShareWidget(QWidget* parent)
        : QDialog(parent), ui(new Ui::VideoCallShareWidget) {
    ui->setupUi(this);
    initTranslations();
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    ui->screen_views->setMinimumWidth(width());
    ui->window_views->setMinimumWidth(width());
    updateData();
    connect(ui->btn_close, &QPushButton::clicked, this, [=] { this->reject(); });
    connect(ui->screen_views, &ShareViewContainer::sigItemPressed, this,
        [=](SnapshotAttr attr) {
            if (!canStartSharing()) {
                return;
            }
            vrd::VideoCallSession::instance().startScreenShare([=](int code) {
                if (code != 200) {
                    auto errorMsg = QString::fromUtf8("sharing error:") + QString::number(code);
                    qDebug() << errorMsg;
                    vrd::util::showToastInfo(QObject::tr("somebody_is_sharing_screen").toStdString());
                    return;
                }
                auto r = videocall::DataMgr::instance().room();
                r.screen_shared_uid = videocall::DataMgr::instance().user_id();
                videocall::DataMgr::instance().setRoom(std::move(r));
                std::vector<void*> excluded;
                VideoCallRtcEngineWrap::instance().startScreenCapture(
                    attr.source_id, excluded);
                VideoCallRtcEngineWrap::instance().startScreenAudioCapture();
                this->accept();
            });

        });

    connect(ui->window_views, &ShareViewContainer::sigItemPressed, this,
        [=](SnapshotAttr attr) {
            if (!canStartSharing()) {
                return;
            }
            vrd::VideoCallSession::instance().startScreenShare([=](int code) {
                if (code != 200) {
                    auto errorMsg = QString::fromUtf8("sharing error:") + QString::number(code);
                    qDebug() << errorMsg;
                    vrd::util::showToastInfo(QObject::tr("somebody_is_sharing_screen").toStdString());
                    return;
                }
                auto r = videocall::DataMgr::instance().room();
                r.screen_shared_uid = videocall::DataMgr::instance().user_id();
                videocall::DataMgr::instance().setRoom(std::move(r));
                VideoCallRtcEngineWrap::instance().startScreenCaptureByWindowId(
                    attr.source_id);
                VideoCallRtcEngineWrap::instance().startScreenAudioCapture();
                this->accept();
                });
        });
}

VideoCallShareWidget::~VideoCallShareWidget() { 
    delete ui; 
}

void VideoCallShareWidget::updateData() {
    std::vector<SnapshotAttr> vec;
    VideoCallRtcEngineWrap::getShareList(vec);
    ui->screen_views->clear();
    ui->window_views->clear();
    for (auto& attr : vec) {
        if (attr.type == SnapshotAttr::kScreen) {
            ui->screen_views->addItem(attr,
                std::move(VideoCallRtcEngineWrap::getThumbnail(
                    attr.type, attr.source_id, 160, 90)));
        }
        else {
            ui->window_views->addItem(attr,
                std::move(VideoCallRtcEngineWrap::getThumbnail(
                    attr.type, attr.source_id, 160, 90)));
        }
    }
}

bool VideoCallShareWidget::canStartSharing() {
    auto cur_share_uid =
        videocall::DataMgr::instance().room().screen_shared_uid;
    if (!cur_share_uid.empty() &&
        cur_share_uid != videocall::DataMgr::instance().user_id()) {
        vrd::util::showToastInfo(QObject::tr("grab_sharing").toStdString());
        return false;
    }
    return true;
}

void VideoCallShareWidget::initTranslations(){
    ui->lbl_info->setText(QObject::tr("choose_sharing_content"));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->screenScrollArea), QObject::tr("desktop"));
    ui->tabWidget->setTabText(ui->tabWidget->indexOf(ui->windowScrollArea), QObject::tr("windows"));
}
