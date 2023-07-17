#include "videocall_manager.h"

#include <QMessageBox>
#include <QTimer>
#include <chrono>
#include <type_traits>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QScreen>
#include <QTranslator>
#include <QApplication>

#include "core/util_tip.h"
#include "videocall/core/videocall_session.h"
#include "videocall/core/videocall_notify.h"
#include "videocall/core/data_mgr.h"
#include "videocall/feature/share_button_bar.h"
#include "videocall/feature/videocall_share_widget.h"
#include "videocall/feature/videocall_quit_dlg.h"
#include "videocall/feature/videocall_setting.h"
#include "videocall/feature/videocall_realtime_data.h"
#include "videocall/feature/videocall_login.h"
#include "videocall/feature/videocall_main_page.h"

namespace videocall {
VideoCallManager& VideoCallManager::instance() {
  static VideoCallManager mgr;
  return mgr;
}

void VideoCallManager::init() {
    initTranslations();
	instance().login_widget_ =
		std::unique_ptr<VideoCallLoginWidget>(new VideoCallLoginWidget);

	QObject::connect(instance().login_widget_.get(),
		&VideoCallLoginWidget::sigClose,
		[=] { emit instance().sigReturnMainPage(); });

    QObject::connect(&VideoCallRtcEngineWrap::instance(),
                    &VideoCallRtcEngineWrap::sigUpdateAudio, []() {
                        auto& users = videocall::DataMgr::instance().ref_users();
                        auto iter = std::find_if(users.begin(), users.end(),
                            [](const User& user) {
                                return user.user_id ==
                                    videocall::DataMgr::instance().user_id();
                            });
                        if (iter != users.end()) {
                            iter->is_mic_on = !videocall::DataMgr::instance().mute_audio();
                        }

                        instance().main_page_->setMicState(
                            !videocall::DataMgr::instance().mute_audio());

                        instance().login_widget_->setMicState(
                            !videocall::DataMgr::instance().mute_audio());
                        if (instance().share_button_bar_ 
                            && instance().share_button_bar_->isVisible()) {
                            instance().share_button_bar_->setMicState(
                                !videocall::DataMgr::instance().mute_audio());
                        }
                    });

    QObject::connect(&VideoCallRtcEngineWrap::instance(),
                    &VideoCallRtcEngineWrap::sigUpdateVideo, []() {
                        auto& users = videocall::DataMgr::instance().ref_users();
                        auto iter = std::find_if(users.begin(), users.end(),
                            [](const User& user) {
                                return user.user_id ==
                                    videocall::DataMgr::instance().user_id();
                            });
                        if (iter != users.end()) {
                            iter->is_camera_on = !videocall::DataMgr::instance().mute_video();
                        }

                        instance().main_page_->setCameraState(
                            !videocall::DataMgr::instance().mute_video());
                        instance().login_widget_->setCameraState(
                            !videocall::DataMgr::instance().mute_video());

                        if (instance().share_button_bar_
                            && instance().share_button_bar_->isVisible()) {
                            instance().share_button_bar_->setCameraState(
                                !videocall::DataMgr::instance().mute_video());
                        }
                    });

    QObject::connect(
        &VideoCallRtcEngineWrap::instance(),
        &VideoCallRtcEngineWrap::sigOnAudioVolumeUpdate, [=]() {
            auto remote_speackers = videocall::DataMgr::instance().remote_volumes();
            auto local_speackers = videocall::DataMgr::instance().local_volumes();
            auto total_speakers = remote_speackers;
            for (auto& speaker : local_speackers) {
                if (speaker.stream_index == bytertc::kStreamIndexMain) {
                    speaker.uid = videocall::DataMgr::instance().user_id();
                    speaker.roomId = videocall::DataMgr::instance().room_id();
                    total_speakers.push_back(speaker);
                }
            }
            std::sort(total_speakers.begin(), total_speakers.end(),
                [](const AudioVolumeInfoWrap& l, const AudioVolumeInfoWrap& r) {
                    return l.volume > r.volume;
                });
            auto users = videocall::DataMgr::instance().ref_users();
            for (auto& speacker : total_speakers) {
                auto iter = std::find_if(users.begin(), users.end(),
                    [speacker](const User& user) {
                        return user.user_id == speacker.uid;
                    });
                if (iter != users.end()) {
                    iter->audio_volume = speacker.volume;
                }
            }
            if (total_speakers.size() > 0 && total_speakers[0].volume > 5) {
                DataMgr::instance().setHighLight(total_speakers[0].uid);
            }
            else {
                DataMgr::instance().setHighLight("");
            }

            ForwardEvent::PostEvent(&VideoCallManager::instance(), [] {
                updateData();
            });
        });

	QObject::connect(&VideoCallRtcEngineWrap::instance(),
		&VideoCallRtcEngineWrap::sigOnRoomStateChanged,
        [=](std::string room_id, std::string uid, int state, std::string extra_info) {
			if (room_id == videocall::DataMgr::instance().room().room_id
				&& uid == videocall::DataMgr::instance().user_id()) {
				auto infoArray = QByteArray(extra_info.data(), static_cast<int>(extra_info.size()));
				auto infoJsonObj = QJsonDocument::fromJson(infoArray).object();
				auto joinType = infoJsonObj["join_type"].toInt();
				if (state == 0 && joinType == 1) {
					vrd::VideoCallSession::instance().userReconnect([=](int code) {
						if (code == 422 || code == 419 || code == 404) {
							instance().main_page_->froceClose();
						}
					});
				}
			}
        });

    QObject::connect(&VideoCallRtcEngineWrap::instance(),
        &VideoCallRtcEngineWrap::sigOnShareScreenStatusChanged,
        [=](std::string uid, bool isSharing) {
            if (uid == videocall::DataMgr::instance().user_id()) return;
            auto& users = videocall::DataMgr::instance().ref_users();
            for (size_t i = 0; i < users.size(); i++) {
                if (users[i].user_id == uid) {
                    VideoCallManager::instance().main_page_->changeViewMode(
                        isSharing ? VideoCallMainPage::kFocusPage : VideoCallMainPage::kNormalPage);
                    users[i].is_sharing = isSharing;
                    auto r = videocall::DataMgr::instance().room();
                    r.screen_shared_uid = isSharing ? uid : "";
                    videocall::DataMgr::instance().setRoom(std::move(r));
                    VideoCallManager::setRemoteScreenVideoWidget(users[i]);
                    break;
                }
            }
        });

    instance().main_page_ = std::unique_ptr<VideoCallMainPage>(new VideoCallMainPage);
    QObject::connect(instance().main_page_.get(), &VideoCallMainPage::sigClose, [=] {
        VideoCallNotify::instance().offAll();
        if (videocall::DataMgr::instance().room().screen_shared_uid ==
            videocall::DataMgr::instance().user_id()) {
            videocall::DataMgr::instance().setShareScreen(false);
            instance().share_button_bar_->hide();
            VideoCallRtcEngineWrap::instance().stopScreenAudioCapture();
            VideoCallRtcEngineWrap::instance().stopScreenCapture();
        }
        videocall::DataMgr::instance().ref_room().screen_shared_uid = "";
        for (auto video : instance().getVideoList()) {
            video->setParent(nullptr);
        }
        instance().getScreenVideo()->setParent(nullptr);

        videocall::DataMgr::instance().setUsers(std::vector<User>());
        VideoCallRtcEngineWrap::instance().logout();
        showLogin();
    });

    QObject::connect(&VideoCallRtcEngineWrap::instance(),
        &VideoCallRtcEngineWrap::sigUpdateInfo,
        [=](const std::string& uid ) { 
            if (instance().data_page_) {
                instance().data_page_->updateData(uid);
            }
        });

    QObject::connect(&VideoCallRtcEngineWrap::instance(),
        &VideoCallRtcEngineWrap::sigUpdateMainPageData, &instance(),
        []{
            instance().updateData();
        },Qt::QueuedConnection);

	instance().screen_widget_ = std::make_shared<VideoCallVideoWidget>();
	instance().videos_.resize(kMaxShowWidgetNum);
	for (int i = 0; i < kMaxShowWidgetNum; i++) {
		instance().videos_[i] = std::make_shared<VideoCallVideoWidget>();
	}

    QObject::connect(instance().main_page_.get(),
        &VideoCallMainPage::sigShareButtonClicked, 
        [=]{
            showShareWidget();
        });


    QObject::connect(instance().main_page_.get(),
        &VideoCallMainPage::sigCameraEnabled,
        [=](bool is_enabled) { getCurrentVideo()->setHasVideo(is_enabled); });

    QObject::connect(instance().main_page_.get(),
		&VideoCallMainPage::sigVideoCallSetting,
		[=] { showSetting(); });

    QObject::connect(instance().main_page_.get(),
        &VideoCallMainPage::sigRealTimeDataClicked,
        [=] { showRealTimeData(instance().main_page_.get()); });
}

/** {zh}
 * 加载翻译文案
 */

/** {en}
* Load translation text
*/
void VideoCallManager::initTranslations() {
    QTranslator* translator = new QTranslator(qApp);
    if (translator->load(QLocale(), "videocall", "_", ":/videocall_translations") && QApplication::instance()) {
        QApplication::instance()->installTranslator(translator);
    }
    else if (translator->load(QString("videocall_zh_CN"), QString(":/videocall_translations"))) {
        QApplication::instance()->installTranslator(translator);
    }
}

void VideoCallManager::showLogin(QWidget* parent) {
    instance().login_widget_->show();
    instance().current_widget_ = instance().login_widget_.get();
}

void VideoCallManager::showTips(QWidget* parent) {
    QTimer::singleShot(100, []() {
        QMessageBox::information(nullptr, QObject::tr("audio_video_calls"),
            QObject::tr("minutes_meeting_title"),
            QMessageBox::Ok);
        });
}

void VideoCallManager::showSetting(QWidget* parent) {
    auto dlg = std::unique_ptr<VideoCallSetting>(new VideoCallSetting(parent));
    dlg->initView();
    if (dlg->exec() == QDialog::Accepted) {
        auto setting = videocall::DataMgr::instance().setting();
        VideoCallRtcEngineWrap::setVideoProfiles(setting.camera);
        VideoCallRtcEngineWrap::setAudioProfiles(setting.audio_quality);
        VideoCallRtcEngineWrap::setLocalMirrorMode(setting.enable_camera_mirror ? 
            bytertc::MirrorType::kMirrorTypeRenderAndEncoder : bytertc::MirrorType::kMirrorTypeNone);
    }
}

void VideoCallManager::showRealTimeData(QWidget* parent /*= nullptr*/) {
    if (!instance().data_page_) {
        instance().data_page_ = new VideoCallData(parent);
    }
    instance().data_page_->initView();
    instance().data_page_->setVisible(true);
}

void VideoCallManager::showShareWidget(QWidget* parent) {
    auto dlg = std::unique_ptr<VideoCallShareWidget>(new VideoCallShareWidget(parent));
    if (dlg->exec() == QDialog::Accepted) {
        videocall::DataMgr::instance().setShareScreen(true);
        hideRoom();
        showShareControlBar();
    }
}

void VideoCallManager::showShareControlBar() {
    if (!instance().share_button_bar_) {
        instance().share_button_bar_ =
            std::unique_ptr<ShareButtonBar>(new ShareButtonBar);

        instance().share_button_bar_->connect(instance().share_button_bar_.get(),
            &ShareButtonBar::sigShareStateChanged,
            [=](bool is_share) {
                if (is_share) {
                    showShareWidget(instance().currentWidget());
                }
                else {
                    stopScreen();
                }
            });

        instance().share_button_bar_->connect(instance().share_button_bar_.get(),
            &ShareButtonBar::sigOpenSetting,
            [=] { showSetting(); });
    }

    instance().share_button_bar_->setMicState(!videocall::DataMgr::instance().mute_audio());
    instance().share_button_bar_->setCameraState(!videocall::DataMgr::instance().mute_video());
    instance().share_button_bar_->show();
    QScreen* screen = QGuiApplication::primaryScreen();
    instance().share_button_bar_->move(
        (screen->size().width() - instance().share_button_bar_->width()) / 2, screen->size().height() - 100);
}

int VideoCallManager::showCallExpDlg(QWidget* parent) {
    auto quit_dlg = std::unique_ptr<VideoCallQuitDlg>(new VideoCallQuitDlg());
    if (parent) {
        quit_dlg->move(QPoint(
            parent->pos().x() + (parent->width() - quit_dlg->width()) / 2,
            parent->pos().y() + ((parent->height() - quit_dlg->height()) / 2)));
    }

    return quit_dlg->exec();
}

void VideoCallManager::setLocalVideoWidget(const User& user, int idx) {
    VideoCallRtcEngineWrap::setupLocalView(
        videocall::VideoCallManager::getVideoList()[idx]->getWinID(),
        bytertc::RenderMode::kRenderModeHidden, "local");

    auto isLocalCameraOn = !videocall::DataMgr::instance().mute_video();
    videocall::VideoCallManager::getVideoList()[idx]->setUserName(QObject::tr("xxx(me)").arg(QString::fromStdString(user.user_name)));
    videocall::VideoCallManager::getVideoList()[idx]->setHighLight(videocall::DataMgr::instance().high_light() == user.user_id);
    videocall::VideoCallManager::getVideoList()[idx]->setShare(videocall::DataMgr::instance().share_screen());
    videocall::VideoCallManager::getVideoList()[idx]->setMic(!videocall::DataMgr::instance().mute_audio());
    videocall::VideoCallManager::getVideoList()[idx]->setHasVideo(isLocalCameraOn);
    videocall::VideoCallManager::getVideoList()[idx]->setUserLogoSize();
}

void VideoCallManager::setRemoteVideoWidget(const User& user, int idx) {
    VideoCallRtcEngineWrap::setupRemoteView(
        videocall::VideoCallManager::getVideoList()[idx]->getWinID(),
        bytertc::RenderMode::kRenderModeHidden, user.user_id);
    videocall::VideoCallManager::getVideoList()[idx]->setUserName(
        user.user_name.c_str());
    videocall::VideoCallManager::getVideoList()[idx]->setShare(user.is_sharing);
    videocall::VideoCallManager::getVideoList()[idx]->setMic(user.is_mic_on);
    videocall::VideoCallManager::getVideoList()[idx]->setHasVideo(user.is_camera_on);
    videocall::VideoCallManager::getVideoList()[idx]->setUserLogoSize();
    videocall::VideoCallManager::getVideoList()[idx]->setHighLight(videocall::DataMgr::instance().high_light() == user.user_id);
}

void VideoCallManager::setRemoteScreenVideoWidget(const User& user) {
    auto& ins = instance();
    auto video = getScreenVideo();
    SubscribeConfig config;
    config.is_screen = true;
    config.sub_video = true;
    VideoCallRtcEngineWrap::setRemoteScreenView(user.user_id, video->getWinID());
    video->setUserName(QObject::tr("xxx's_screen_sharing").arg(QString::fromStdString(user.user_name)));
    video->setShare(user.is_sharing);
    video->setHasVideo(user.is_sharing);
    video->setUserLogoSize();
    if (user.is_sharing) {
        video->setVideoUpdateEnabled(false);
    }
    else {
        video->setVideoUpdateEnabled(true);
    }
}

void VideoCallManager::initRoom() {
    videoCallNotify();
    instance().main_page_->init();
    showRoom();
    updateData();
}

void VideoCallManager::showRoom() {
    instance().main_page_->show();
    instance().main_page_->setCameraState(
        !videocall::DataMgr::instance().mute_video());
    instance().main_page_->setMicState(
        !videocall::DataMgr::instance().mute_audio());
    instance().current_widget_ = instance().main_page_.get();
    
    auto cur_share_uid = videocall::DataMgr::instance().room().screen_shared_uid;
    if (!cur_share_uid.empty() 
        && cur_share_uid != videocall::DataMgr::instance().user_id()) {
        auto users = videocall::DataMgr::instance().users();
        auto iter = std::find_if(users.begin(), users.end(), [](const User& user) {
            return user.user_id ==
                videocall::DataMgr::instance().room().screen_shared_uid;
            });
        if (iter != users.end()) {
            setRemoteScreenVideoWidget(*iter);
        }
    }
}

QWidget* VideoCallManager::currentWidget() { 
    return instance().current_widget_; 
}

void VideoCallManager::hideRoom() { 
    instance().main_page_->hide();
}

std::vector<std::shared_ptr<VideoCallVideoWidget>> VideoCallManager::getVideoList() {
    return instance().videos_;
}

std::shared_ptr<VideoCallVideoWidget> VideoCallManager::getCurrentVideo() {
    int i = 0;
    for (auto& user : videocall::DataMgr::instance().users()) {
        if (user.user_id == videocall::DataMgr::instance().user_id()) {
            return instance().videos_[i];
        }
        i++;
    }
    return std::shared_ptr<VideoCallVideoWidget>();
}

std::shared_ptr<VideoCallVideoWidget> VideoCallManager::getScreenVideo() {
    return instance().screen_widget_;
}

void VideoCallManager::updateData() {
    if (instance().updating) {
        return;
    }
    instance().updating = true;
    instance().main_page_->updateVideoWidget();
    instance().updating = false;
}

void VideoCallManager::videoCallNotify() {
    VideoCallNotify::instance().onCallEnd([](int) {
        instance().main_page_->froceClose();
        vrd::util::showToastInfo(QObject::tr("minutes_meeting").toStdString());
    });
}

void VideoCallManager::stopScreen() {
    videocall::DataMgr::instance().setShareScreen(false);
    videocall::DataMgr::instance().ref_room().screen_shared_uid = "";
    showRoom();
    instance().share_button_bar_->hide();
    vrd::VideoCallSession::instance().stopScreenShare([](int code) {
        if (code != 200) {
            qDebug() << "failed to stop screen sharing, error = " << code;
            return;
        }
    });
    VideoCallRtcEngineWrap::instance().stopScreenAudioCapture();
    VideoCallRtcEngineWrap::instance().stopScreenCapture();
}

void VideoCallManager::customEvent(QEvent* e) {
    if (e->type() == QEvent::User) {
        auto user_event = static_cast<ForwardEvent*>(e);
        user_event->execTask();
    }
}

}  // namespace videocall
