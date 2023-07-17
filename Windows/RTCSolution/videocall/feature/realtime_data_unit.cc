#include "realtime_data_unit.h"
#include "ui_realtime_data_unit.h"

static std::vector<QString> networkQualityStr{
    QObject::tr("unknown"), QObject::tr("excellent"), QObject::tr("good"),
    QObject::tr("poor"), QObject::tr("extremely_bad"), QObject::tr("stuck_stopped")
};

realTimeDataUnit::realTimeDataUnit(QWidget* parent)
    : QWidget(parent), ui(new Ui::realTimeDataUnit) {
    ui->setupUi(this);
}

void realTimeDataUnit::updateInfo(const videocall::StreamInfo& info, bool isVideoInfo) {
    ui->userName->setText(QString::fromStdString(info.user_name));
    if (isVideoInfo) {
        ui->labelText_1->setText(QObject::tr("resolution"));
        ui->labelValue_1->setText(QString("%1*%2").arg(info.width).arg(info.height));

        ui->labelText_2->setText(QObject::tr("bitrate"));
        ui->labelValue_2->setText(QString("%1").arg(info.video_kbitrate));

        ui->labelText_3->setText(QObject::tr("frame_rate"));
        ui->labelValue_3->setText(QString("%1").arg(info.video_fps));
        
        ui->labelText_4->setText(QObject::tr("delay"));
        ui->labelValue_4->setText(QString("%1").arg(info.video_delay));

        ui->labelText_5->setText(QObject::tr("packet_loss_rate"));
        ui->labelValue_5->setText(QString("%1").arg(info.video_loss_rate));

        ui->labelText_6->setText(QObject::tr("network_status"));
        ui->labelValue_6->setText(networkQualityStr[info.natwork_quality]);
    }
    else {
        ui->labelText_1->setText(QObject::tr("bitrate"));
        ui->labelValue_1->setText(QString("%1").arg(info.audio_kbitrate));

        ui->labelText_2->setText(QObject::tr("delay"));
        ui->labelValue_2->setText(QString("%1").arg(info.audio_delay));

        ui->labelText_3->setText(QObject::tr("packet_loss_rate"));
        ui->labelValue_3->setText(QString("%1").arg(info.audio_loss_rate));

        ui->labelText_4->setText(QObject::tr("network_status"));
        ui->labelValue_4->setText(networkQualityStr[info.natwork_quality]);

        ui->labelText_5->setText("");
        ui->labelValue_5->setText("");

        ui->labelText_6->setText("");
        ui->labelValue_6->setText("");
    }
}

realTimeDataUnit::~realTimeDataUnit() { 
    delete ui; 
}

