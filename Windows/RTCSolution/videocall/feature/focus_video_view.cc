#include "focus_video_view.h"
#include "ui_focus_video_view.h"

#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>
#include <QTimer>
#include <QWheelEvent>

#include "videocall/core/videocall_manager.h"

FocusVideoView::FocusVideoView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FocusVideoView) {

    ui->setupUi(this);
    ui->video_list->setLayout(new QVBoxLayout);
    ui->video_list->layout()->setContentsMargins(0, 0, 0, 0);
    ui->video_list->layout()->setSpacing(8);
    ui->big_view->setLayout(new QHBoxLayout);
    ui->big_view->layout()->setContentsMargins(0, 0, 0, 0);
    ui->big_view->layout()->setSpacing(0);
}

FocusVideoView::~FocusVideoView() { 
    delete ui;
}

void FocusVideoView::init() {
    auto list = videocall::VideoCallManager::getVideoList();
    auto lay = ui->video_list->layout();
    for (int i = 0; i < list.size(); i++) {
        lay->removeWidget(list[i].get());
        list[i]->hide();
    }
    cnt_ = 0;
}

void FocusVideoView::showWidget(int cnt) {
    auto list = videocall::VideoCallManager::getVideoList();
    auto lay = ui->video_list->layout();
    for (int i = cnt; i < cnt_; i++) {
        list[i]->hide();
    }
    auto width = ui->scrollArea->width();
    ui->big_view->layout()->addWidget(
        videocall::VideoCallManager::getScreenVideo().get());
    for (int i = 0; i < cnt; i++) {
        lay->addWidget(list[i].get());
        list[i]->setFixedSize(width, width / 16 * 9);
        list[i]->show();
    }
    cnt_ = cnt;
    ui->video_list->layout()->setAlignment(Qt::AlignCenter);
}

void FocusVideoView::wheelEvent(QWheelEvent *e) {
    int numberDegrees = e->angleDelta().y() / 8;
    int numberSteps = numberDegrees;
    ui->scrollArea->verticalScrollBar()->setValue(
        ui->scrollArea->verticalScrollBar()->value() - numberSteps);
    e->accept();
}

void FocusVideoView::paintEvent(QPaintEvent *e) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}
