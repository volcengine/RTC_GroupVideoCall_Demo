#include "videocall_realtime_data.h"
#include "ui_videocall_realtime_data.h"

#include "videocall/feature/realtime_data_unit.h"
#include "videocall/core/videocall_rtc_wrap.h"
#include "videocall/core/data_mgr.h"


VideoCallData::VideoCallData(QWidget* parent)
    : QDialog(parent), ui(new Ui::VideoCallData) {
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    ui->btn_confirm->setText(QObject::tr("ok"));
    ui->btn_cancel->setText(QObject::tr("cancel"));
    ui->content_widget->layout()->setAlignment(Qt::AlignTop);
    QObject::connect(ui->audioButton, &QRadioButton::clicked, this, [this]() {
        mIsVideoInfo = false;
        updateData();
    });
    QObject::connect(ui->videoButton, &QRadioButton::clicked, this, [this]() {
        mIsVideoInfo = true;
        updateData();
    });
}

void VideoCallData::onConfirm() {
    close();
}

void VideoCallData::onClose() {
    close();
}

void VideoCallData::onCancel() {
    close();
}

void VideoCallData::initView() {
    m_infos.clear(); 
    QLayoutItem* child;
    while ((child = ui->content_widget->layout()->takeAt(0)) != nullptr) {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    auto& localInfo = videocall::DataMgr::instance().ref_local_stream_info();
    localInfo.user_id = videocall::DataMgr::instance().user_id();
    localInfo.user_name = videocall::DataMgr::instance().user_name();
    auto localDataWidget = new realTimeDataUnit(this);
    localDataWidget->updateInfo(localInfo, mIsVideoInfo);
    m_infos[videocall::DataMgr::instance().user_id()] = localDataWidget;
    ui->content_widget->layout()->addWidget(localDataWidget);

    auto remoteStreamInfos = videocall::DataMgr::instance().remote_stream_infos();
    for (auto& info : remoteStreamInfos) {
        auto remoteInfoWidget = new realTimeDataUnit(this);
        remoteInfoWidget->updateInfo(info, mIsVideoInfo);
        m_infos[info.user_id] = remoteInfoWidget;
        ui->content_widget->layout()->addWidget(remoteInfoWidget);
    }
}

void VideoCallData::updateData(const std::string& uid) {
    if (this->isVisible() && m_infos.contains(uid)) {
        auto remoteStreamInfos = videocall::DataMgr::instance().remote_stream_infos();
        auto iter = std::find_if(remoteStreamInfos.begin(), 
            remoteStreamInfos.end(), [uid](const videocall::StreamInfo& streamInfo) {
            return streamInfo.user_id == uid;
            });
        if (iter != remoteStreamInfos.end()) {
            m_infos[uid]->updateInfo(*iter, mIsVideoInfo);
        }

        if (uid == videocall::DataMgr::instance().user_id()) {
            auto localInfo = videocall::DataMgr::instance().local_stream_info();
            m_infos[localInfo.user_id]->updateInfo(localInfo, mIsVideoInfo);
        }
    }
}

void VideoCallData::updateData() {
    if (this->isVisible()) {
        auto localInfo = videocall::DataMgr::instance().local_stream_info();
        m_infos[localInfo.user_id]->updateInfo(localInfo, mIsVideoInfo);

        auto remoteStreamInfos = videocall::DataMgr::instance().remote_stream_infos();
        for (auto& info : remoteStreamInfos) {
            if(m_infos.contains(info.user_id))
            m_infos[info.user_id]->updateInfo(info, mIsVideoInfo);
        }
    }
}

VideoCallData::~VideoCallData() { 
    delete ui; 
}

