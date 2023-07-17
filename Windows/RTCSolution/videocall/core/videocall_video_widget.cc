#include <QHBoxLayout>
#include <QResizeEvent>
#include <QIcon>
#include <unordered_map>
#include <iostream>

#include "videocall_video_widget.h"

struct VideoWidgetInfo {
  int user_logo_font_size;
  int font_size;
  int user_logo_width;
  int user_logo_height;
};

VideoCallVideoWidget::HasVideoWidget::HasVideoWidget(QWidget* parent /*= nullptr*/) 
    : QWidget(parent) {
    this->setObjectName("HasVideoWidget");
    video_ = new QWidget(this);
    
    info_content_ = new QWidget(this);
    lbl_share_logo_ = new QLabel(info_content_);
    lbl_user_name_ = new QLabel(info_content_);
    lbl_mic_state_ = new QLabel(info_content_);
    info_content_->setFixedHeight(32);
    auto hbox_layout = new QHBoxLayout(info_content_);
    hbox_layout->setSpacing(8);
    hbox_layout->setContentsMargins(8, 8, 8, 8);
    hbox_layout->addWidget(lbl_mic_state_);
    hbox_layout->addWidget(lbl_share_logo_);
    hbox_layout->addWidget(lbl_user_name_);
    lbl_mic_state_->setVisible(false);
    lbl_user_name_->setVisible(false);
    lbl_share_logo_->setVisible(false);
    info_content_->setLayout(hbox_layout);

    info_content_->setStyleSheet(
        "font-family: \"Microsoft YaHei\";\n"
        "font-size: 12px;\n");
    video_->setStyleSheet("background:#272e3B;");
    setShare(false);
}

void* VideoCallVideoWidget::HasVideoWidget::getVideoWinID() { 
    return reinterpret_cast<void*>(video_->winId()); 
}

void VideoCallVideoWidget::HasVideoWidget::setVideoUpdateEnabled(bool enabled) {
    video_->setUpdatesEnabled(enabled);
}

void VideoCallVideoWidget::HasVideoWidget::setUserName(const QString& str) {
    lbl_user_name_->setText(str);
    lbl_user_name_->setVisible(true);
    info_content_->resize(info_content_->layout()->sizeHint());
}

void VideoCallVideoWidget::HasVideoWidget::hideVideo() { 
    video_->hide(); 
    info_content_->hide();
}
void VideoCallVideoWidget::HasVideoWidget::showVideo() { 
    video_->show(); 
}

void VideoCallVideoWidget::HasVideoWidget::setHighLight(bool enabled) {
    if (enabled) {
        setStyleSheet("#HasVideoWidget{border:2px solid #23C343;}");
    }
    else {
        setStyleSheet("#HasVideoWidget{border:none}");
    }
}

void VideoCallVideoWidget::HasVideoWidget::setShare(bool enabled) {
    lbl_share_logo_->setVisible(enabled);

    QIcon shareIcon = QIcon(":img/videocall_share_checked");
    QPixmap m_pic = shareIcon.pixmap(shareIcon.actualSize(QSize(16, 16)));
    lbl_share_logo_->setPixmap(m_pic);
    info_content_->resize(info_content_->layout()->sizeHint());
    info_content_->move(2,
            height() - info_content_->height() - 2);
}

void VideoCallVideoWidget::HasVideoWidget::setMic(bool enabled) {
    lbl_mic_state_->setVisible(true);

    QIcon micIcon = QIcon(enabled ? ":img/videocall_mic_on" : ":img/videocall_mic_off");
    QPixmap m_pic = micIcon.pixmap(micIcon.actualSize(QSize(16, 16)));
    lbl_mic_state_->setPixmap(m_pic);
    info_content_->resize(info_content_->layout()->sizeHint());
    info_content_->move(2,
        height() - info_content_->height() - 2);
}

void VideoCallVideoWidget::HasVideoWidget::resizeEvent(QResizeEvent* e) {
    info_content_->move(2,
        e->size().height() - info_content_->height() - 2);
    video_->setGeometry(2, 2, e->size().width() - 4, e->size().height() - 4);
}

void VideoCallVideoWidget::HasVideoWidget::showEvent(QShowEvent*) {
    info_content_->move(2, height() - info_content_->height() - 2);
}
void VideoCallVideoWidget::HasVideoWidget::moveEvent(QMoveEvent*) {}

VideoCallVideoWidget::NoVideoWidget::NoVideoWidget(QWidget* parent /*= nullptr*/) 
    : QWidget(parent) {
    this->setObjectName("NoVideoWidget");
    setStyleSheet(
        "#NoVideoWidget{background:#272E3B;"
        "font-family: \"Microsoft YaHei\";\n"
        "font-size: 12px;}\n");

    auto p = new QVBoxLayout();
    p->setContentsMargins(0, 0, 0, 0);
    p->setSpacing(0);
    setLayout(p);
    p->addItem(
        new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
    auto logo_layout = new QHBoxLayout();
    logo_layout->setContentsMargins(0, 0, 0, 0);
    logo_layout->setSpacing(0);
    lbl_user_logo_ = new QLabel(this);
    lbl_user_logo_->setFixedSize(40, 40);
    logo_layout->addWidget(lbl_user_logo_);
    lbl_user_logo_->setAlignment(Qt::AlignCenter);
    lbl_user_logo_->setStyleSheet(
        "border-radius:20px; background:#4E5969;border: 2px solid #4080FF;");
    p->addItem(logo_layout);
    p->addItem(
        new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

    info_content_ = new QWidget(this);
    lbl_share_logo_ = new QLabel(info_content_);
    lbl_user_name_ = new QLabel(info_content_);
    lbl_mic_state_ = new QLabel(info_content_);
    info_content_->setFixedHeight(32);
    auto hbox_layout = new QHBoxLayout(info_content_);
    hbox_layout->setSpacing(8);
    hbox_layout->setContentsMargins(8, 8, 8, 8);
    hbox_layout->addWidget(lbl_mic_state_);
    hbox_layout->addWidget(lbl_share_logo_);
    hbox_layout->addWidget(lbl_user_name_);
    lbl_mic_state_->setVisible(false);
    lbl_user_name_->setVisible(false);
    lbl_share_logo_->setVisible(false);
    info_content_->setLayout(hbox_layout);

    info_content_->setStyleSheet(
        "font-family: \"Microsoft YaHei\";\n"
        "font-size: 12px;\n"
        "border:none;");
}

void VideoCallVideoWidget::NoVideoWidget::setUserLogoSize() {
    if (this->width() > 600 && this->height() > 600) {
        lbl_user_logo_->setFixedSize(160,160);
        lbl_user_logo_->setStyleSheet(
            "border-radius:80px; background:#4E5969;border:none;"
            "font-family: 'Inter';font-weight: 500;font-size: 64px; ");
    }
    else if (this->width() > 350 && this->height() > 200) {
        lbl_user_logo_->setFixedSize(80, 80);
        lbl_user_logo_->setStyleSheet(
            "border-radius:40px; background:#4E5969;border:none;"
            "font-family: 'Inter';font-weight: 500;font-size: 32px; ");
    }
    else {
        lbl_user_logo_->setFixedSize(40, 40);
        lbl_user_logo_->setStyleSheet(
            "border-radius:20px; background:#4E5969;border:none;"
            "font-family: 'Inter';font-weight: 500;font-size: 16px; ");
    }
}

void VideoCallVideoWidget::NoVideoWidget::setUserName(const QString& str) {
    lbl_user_name_->setText(str);
    lbl_user_logo_->setText(str.left(1).toUpper());
    lbl_user_name_->setVisible(true);
}

void VideoCallVideoWidget::NoVideoWidget::setShare(bool enabled) {
    lbl_share_logo_->setVisible(enabled);

    QIcon shareIcon = QIcon(":img/videocall_share_checked");
    QPixmap m_pic = shareIcon.pixmap(shareIcon.actualSize(QSize(16, 16)));
    lbl_share_logo_->setPixmap(m_pic);
    info_content_->resize(info_content_->layout()->sizeHint());
    info_content_->move(2,
        height() - info_content_->height() - 2);
}

void VideoCallVideoWidget::NoVideoWidget::setMic(bool enabled) {
    lbl_mic_state_->setVisible(true);
    QIcon micIcon = QIcon(enabled ? ":img/videocall_mic_on" : ":img/videocall_mic_off");
    QPixmap m_pic = micIcon.pixmap(micIcon.actualSize(QSize(16, 16)));
    lbl_mic_state_->setPixmap(m_pic);
    info_content_->resize(info_content_->layout()->sizeHint());
    info_content_->move(2,
        height() - info_content_->height() - 2);
}

void VideoCallVideoWidget::NoVideoWidget::setHighLight(bool enabled) {
    if (enabled) {
        setStyleSheet("#NoVideoWidget{background:#272E3B;"
            "font-family: \"Microsoft YaHei\";font-size: 12px;\n"
            "border:2px solid #23C343;}");
    }
    else {
        setStyleSheet("#NoVideoWidget{background:#272E3B;"
            "font-family: \"Microsoft YaHei\";font-size: 12px;\n"
            "border:none;}");
    }
}

void VideoCallVideoWidget::NoVideoWidget::resizeEvent(QResizeEvent* e) {
    info_content_->move(2,
        e->size().height() - info_content_->height() - 2);
}

void VideoCallVideoWidget::NoVideoWidget::showEvent(QShowEvent*) {
    info_content_->move(2, height() - info_content_->height() - 2);
}

VideoCallVideoWidget::VideoCallVideoWidget(QWidget* parent)
    : QWidget(parent){
    setLayout(new QHBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);

    stacked_widget_ = new QStackedWidget(this);
    stacked_widget_->addWidget(new HasVideoWidget(this));
    stacked_widget_->addWidget(new NoVideoWidget(this));
    stacked_widget_->setCurrentIndex(0);
    stacked_widget_->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(stacked_widget_);
}

void VideoCallVideoWidget::setUserName(const QString& str) {
    static_cast<HasVideoWidget*>(stacked_widget_->widget(0))->setUserName(str);
    static_cast<NoVideoWidget*>(stacked_widget_->widget(1))->setUserName(str);
}

void VideoCallVideoWidget::setShare(bool enabled) {
    static_cast<HasVideoWidget*>(stacked_widget_->widget(0))->setShare(enabled);
    static_cast<NoVideoWidget*>(stacked_widget_->widget(1))->setShare(enabled);
}

void VideoCallVideoWidget::setMic(bool isOn) {
    static_cast<HasVideoWidget*>(stacked_widget_->widget(0))->setMic(isOn);
    static_cast<NoVideoWidget*>(stacked_widget_->widget(1))->setMic(isOn);
}

void VideoCallVideoWidget::setHasVideo(bool has_video) {
    stacked_widget_->setCurrentIndex(!has_video);
}

void* VideoCallVideoWidget::getWinID() {
    return static_cast<HasVideoWidget*>(stacked_widget_->widget(0))
        ->getVideoWinID();
}

void VideoCallVideoWidget::setVideoUpdateEnabled(bool enabled) {
    static_cast<HasVideoWidget*>(stacked_widget_->widget(0))
        ->setVideoUpdateEnabled(enabled);
}

void VideoCallVideoWidget::hideVideo() {
    static_cast<HasVideoWidget*>(stacked_widget_->widget(0))->hideVideo();
}

void VideoCallVideoWidget::showVideo() {
    static_cast<HasVideoWidget*>(stacked_widget_->widget(0))->showVideo();
}

void VideoCallVideoWidget::setHighLight(bool enabled) {
    static_cast<HasVideoWidget*>(stacked_widget_->widget(0))
        ->setHighLight(enabled);
    static_cast<NoVideoWidget*>(stacked_widget_->widget(1))
        ->setHighLight(enabled);
}

void VideoCallVideoWidget::setUserLogoSize() {
    static_cast<NoVideoWidget*>(stacked_widget_->widget(1))
        ->setUserLogoSize();
}
