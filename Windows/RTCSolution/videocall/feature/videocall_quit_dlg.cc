#include "videocall_quit_dlg.h"
#include "ui_videocall_quit_dlg.h"

#include "videocall/core/data_mgr.h"
#include "videocall/core/videocall_session.h"


VideoCallQuitDlg::VideoCallQuitDlg(QWidget *parent)
    : QDialog(parent), ui(new Ui::VideoCallQuitDlg) {
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    connect(ui->btn_cancel, &QPushButton::clicked, this, [=] { reject(); });
    connect(ui->btn_end, &QPushButton::clicked, this, [=] {
        ui->btn_end->setEnabled(false);
        vrd::VideoCallSession::instance().leaveCall(
            [=](int code) {
                accept(); 
                ui->btn_end->setEnabled(true);
            });
        });
    initView();
}

VideoCallQuitDlg::~VideoCallQuitDlg() { delete ui; }

void VideoCallQuitDlg::initView() {
    ui->btn_end->setText(QObject::tr("ok"));
    ui->btn_cancel->setText(QObject::tr("cancel"));
    ui->lbl_info->setText(QObject::tr("leave_room"));
}
