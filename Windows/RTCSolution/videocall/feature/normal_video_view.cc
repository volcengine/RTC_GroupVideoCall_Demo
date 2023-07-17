#include "normal_video_view.h"
#include "ui_normal_video_view.h"

#include <QPainter>
#include <QStyleOption>
#include <QTimer>
#include <QGridLayout>
#include <QButtonGroup>

#include "videocall/core/videocall_manager.h"

NormalVideoView::NormalVideoView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NormalVideoView) {
    ui->setupUi(this);
    auto group = new QButtonGroup(this);
    group->addButton(ui->page1);
    group->addButton(ui->page2);
    group->addButton(ui->page3);

    ui->pageControlWidget->hide();
    QObject::connect(ui->page1, &QPushButton::clicked, this, [this] {showWidgetWithIndex(0); });
    QObject::connect(ui->page2, &QPushButton::clicked, this, [this] {showWidgetWithIndex(4); });
    QObject::connect(ui->page3, &QPushButton::clicked, this, [this] {showWidgetWithIndex(8); });

}

NormalVideoView::~NormalVideoView() {
    delete ui;
}

void NormalVideoView::showWidget(int cnt, bool forceUpdated) {
    // {zh} 人数不变，不做更新
    // {en} If the number of people does not change, do not update
    if (!forceUpdated && cnt == cnt_) { 
        return;
    }

    cnt_ = cnt;
    auto lay = ui->gridLayout;
    auto list = videocall::VideoCallManager::getVideoList();
    for (auto w : list) {
        if (w) {
            w->setMaximumSize(16777215, 16777215);
        }
    }

    if (cnt <= 4) {
        ui->pageControlWidget->hide();
        first_video_index_ = 0;

        QLayoutItem* childItem;
        while ((childItem = lay->takeAt(0)) != 0) {
            if (auto widget = dynamic_cast<VideoCallVideoWidget*>(childItem->widget())) {
                lay->removeWidget(widget);
                widget->setVideoUpdateEnabled(true);
                widget->hide();
            }
            else {
                childItem->widget()->hide();
                delete childItem;
            }
        }
    }

    switch (cnt) {
    case 1:
        lay->setContentsMargins(0, 0, 0, 0);
        lay->addWidget(list[0].get(), 0, 0);
        list[0]->show();
        list[0]->setVideoUpdateEnabled(false);
        break;
    case 2:
        lay->setContentsMargins(8, 8, 8, 8);
        lay->addWidget(list[0].get(), 0, 0);
        lay->addWidget(list[1].get(), 0, 1);
        list[0]->show();
        list[1]->show();
        list[0]->setVideoUpdateEnabled(false);
        list[1]->setVideoUpdateEnabled(false);
        break;
    case 3:
    case 4:
        for (int i = 0; i < cnt; i++) {
            lay->addWidget(list[i].get(), i / 2, i % 2);
            list[i]->show();
            list[i]->setVideoUpdateEnabled(false);
        }
        break;
    default:
        ui->pageControlWidget->show();
        if (cnt < 9) {
            ui->page1->show();
            ui->page2->show();
            ui->page3->hide();
        }
        else if (cnt <= 12) {
            ui->page1->show();
            ui->page2->show();
            ui->page3->show();
        }
        showWidgetWithIndex(first_video_index_);
        break;
    }
}

void NormalVideoView::showWidgetWithIndex(int firstIndex) {
    auto lay = ui->gridLayout;
    auto list = videocall::VideoCallManager::getVideoList();

    QLayoutItem* childItem;
    while ((childItem = lay->takeAt(0)) != 0) {
        if (auto widget = dynamic_cast<VideoCallVideoWidget*>(childItem->widget())) {
            lay->removeWidget(widget);
            widget->setVideoUpdateEnabled(true);
            widget->hide();
        }
        else {
            childItem->widget()->hide();
            delete childItem;
        }
    }

    auto index = firstIndex;
    for (int i = 0; i < 4; i++) {
        if (index < cnt_) {
            lay->addWidget(list[index].get(), i / 2, i % 2);
            list[index]->show();
            list[index]->setVideoUpdateEnabled(false);
        }
        else {
            // {zh} 空的widget，仅占位置
            // {en} Empty widget, only takes up space
            auto placeHolder = new QWidget(this);
            placeHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            lay->addWidget(placeHolder, i / 2, i % 2);
            placeHolder->show();
        }
        index++;
    }
    first_video_index_ = firstIndex;
}

void NormalVideoView::init() {
    auto list = videocall::VideoCallManager::getVideoList();
    auto lay = ui->gridLayout;
    for (int i = first_video_index_; i < first_video_index_ + 4; i++) {
        lay->removeWidget(list[i].get());
        list[i]->hide();
    }
    cnt_ = 0;
    first_video_index_ = 0;
    ui->page1->setChecked(true);
}

void NormalVideoView::paintEvent(QPaintEvent *e) {
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(e);
}
