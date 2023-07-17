#include "videocall_rtc_wrap.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <algorithm>

#include "core/util_tip.h"
#include "videocall/core/data_mgr.h"

/** {zh}
 * 单例对象，便于其他类代码中访问对应的接口
 */

/** {en}
* Singleton object, which is convenient for accessing the corresponding interface in other class codes
*/
VideoCallRtcEngineWrap& VideoCallRtcEngineWrap::instance() {
  static VideoCallRtcEngineWrap engine;
  return engine;
}

int VideoCallRtcEngineWrap::init() {
	auto& engine_wrap = instance();
	int ret = RtcEngineWrap::instance().setEnv(bytertc::kEnvProduct);
	RtcEngineWrap::instance().initDevices();

	enableLocalAudio(true);
	enableLocalVideo(true);
	setAudioVolumeIndicate(1000);

	QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnRoomStateChanged,
		&instance(), &VideoCallRtcEngineWrap::sigOnRoomStateChanged);
    QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnUserJoined,
        &instance(), &VideoCallRtcEngineWrap::onUserJoinedVideoCall);
    QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnUserLeave,
        &instance(), &VideoCallRtcEngineWrap::onUserLeaveVideoCall);

    QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnUserStartVideoCapture,
        &instance(), [=](const std::string& roomId, const std::string& userId) {
            emit instance().onUserCameraStatusChange(userId, true);
        });
    QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnUserStopVideoCapture,
        &instance(), [=](const std::string& roomId, const std::string& userId) {
            emit instance().onUserCameraStatusChange(userId, false);
        });

    QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnUserPublishStream,
        &instance(), [](const std::string& user_id, bytertc::MediaStreamType type) {
            if (type & bytertc::kMediaStreamTypeAudio) {
                instance().onUserMicStatusChange(user_id, true);
            }
        });
    QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnUserUnPublishStream,
        &instance(), [](const std::string& uid, bytertc::MediaStreamType type, bytertc::StreamRemoveReason reason) {
            if (type & bytertc::MediaStreamType::kMediaStreamTypeAudio) {
                instance().onUserMicStatusChange(uid, false);
            }
        });
    QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnUserPublishScreen,
		&instance(), [=](std::string uid, bytertc::MediaStreamType type) {
			emit instance().sigOnShareScreenStatusChanged(uid, true);
		});
    QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnUserUnPublishScreen,
        &instance(), [=](std::string uid, bytertc::MediaStreamType type) {
			emit instance().sigOnShareScreenStatusChanged(uid, false);
        });

	QObject::connect(
		&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnLocalVideoStateChanged,
		&engine_wrap,
		[=](bytertc::StreamIndex idx, bytertc::LocalVideoStreamState state,
			bytertc::LocalVideoStreamError error) {
				if (state == bytertc::kLocalVideoStreamStateFailed &&
					(error == bytertc::kLocalVideoStreamErrorDeviceNoPermission ||
						error == bytertc::kLocalVideoStreamErrorCaptureFailure ||
						error == bytertc::kLocalVideoStreamErrorDeviceBusy)) {
					if (!videocall::DataMgr::instance().mute_video()) {
						videocall::DataMgr::instance().setMuteVideo(true);
						VideoCallRtcEngineWrap::muteLocalVideo(true);
                        vrd::util::showToastInfo(QObject::tr("no_camera_permission").toStdString());
						emit instance().sigUpdateVideo();
					}
				}
		});

    QObject::connect(
        &RtcEngineWrap::instance(), &RtcEngineWrap::sigOnVideoDeviceStateChanged,
        &engine_wrap,
        [=](std::string device_id, bytertc::RTCVideoDeviceType type,
            bytertc::MediaDeviceState state, bytertc::MediaDeviceError error) {
                if (type == bytertc::kRTCVideoDeviceTypeCaptureDevice) {
                    instance().onVideoStateChanged(device_id, state, error);
                }
        });

    QObject::connect(
        &RtcEngineWrap::instance(), &RtcEngineWrap::sigOnAudioDeviceStateChanged,
        &engine_wrap,
        [=](std::string device_id, bytertc::RTCAudioDeviceType type,
            bytertc::MediaDeviceState state, bytertc::MediaDeviceError error) {
                if (type == bytertc::kRTCAudioDeviceTypeCaptureDevice) {
                    instance().onAudioStateChanged(device_id, state, error);
                }
        });

    QObject::connect(
        &RtcEngineWrap::instance(), &RtcEngineWrap::sigOnRemoteAudioVolumeIndication,
        &engine_wrap,
        [=](std::vector<AudioVolumeInfoWrap> speakers, int totalVolume) {
            videocall::DataMgr::instance().setRemoteVolumes(std::move(speakers));
            emit instance().sigOnAudioVolumeUpdate();
        });

    QObject::connect(
        &RtcEngineWrap::instance(), &RtcEngineWrap::sigOnLocalAudioVolumeIndication,
        &engine_wrap,
        [=](std::vector<AudioVolumeInfoWrap> speakers) {
            videocall::DataMgr::instance().setLocalVolumes(std::move(speakers));
            emit instance().sigOnAudioVolumeUpdate();
        });

	

	QObject::connect(&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnLocalStreamStats,
		&engine_wrap, [=](bytertc::LocalStreamStats stats) {
			videocall::StreamInfo info =
				videocall::DataMgr::instance().local_stream_info();
			info.audio_kbitrate = stats.audio_stats.send_kbitrate;
			info.video_kbitrate = stats.video_stats.sent_kbitrate;
			info.video_fps = stats.video_stats.sent_frame_rate;
			info.width = stats.video_stats.encoded_frame_width;
			info.height = stats.video_stats.encoded_frame_height;
			info.audio_loss_rate = stats.audio_stats.audio_loss_rate;
			info.video_loss_rate = stats.video_stats.video_loss_rate;
			info.audio_delay = stats.audio_stats.rtt;
			info.video_delay = stats.video_stats.rtt;
			info.natwork_quality = stats.local_rx_quality;

			videocall::DataMgr::instance().setLocalStreamInfo(info);
			emit instance().sigUpdateInfo(videocall::DataMgr::instance().user_id());
		});

	QObject::connect(
		&RtcEngineWrap::instance(), &RtcEngineWrap::sigOnRemoteStreamStats,
		&engine_wrap, [=](RemoteStreamStatsWrap stats) {
			auto& infos = videocall::DataMgr::instance().ref_remote_stream_infos();
			auto iter = std::find_if(infos.begin(), infos.end(), [&stats](const videocall::StreamInfo& streamInfo) {
				return streamInfo.user_id == stats.uid;
				});
			if (iter != infos.end()) {
				iter->video_kbitrate = stats.video_stats.received_kbitrate;
				iter->audio_kbitrate = stats.audio_stats.received_kbitrate;
				iter->audio_loss_rate = stats.audio_stats.audio_loss_rate * 100;
				iter->video_loss_rate = stats.video_stats.video_loss_rate * 100;
				iter->video_delay = stats.video_stats.rtt;
				iter->audio_delay = stats.audio_stats.rtt;
				iter->video_fps = stats.video_stats.renderer_output_frame_rate;
				iter->natwork_quality = stats.remote_rx_quality;
                iter->width = stats.video_stats.width;
                iter->height = stats.video_stats.height;

				emit instance().sigUpdateInfo(stats.uid);
            }
		});
	return ret;
}

int VideoCallRtcEngineWrap::unInit() {
	QObject::disconnect(&RtcEngineWrap::instance(), nullptr, &instance(), nullptr);
	RtcEngineWrap::instance().resetDevices();
	return 0;
}

int VideoCallRtcEngineWrap::setupLocalView(void* view, bytertc::RenderMode mode,
                                         const std::string& uid) {
    auto& engine_wrap = instance();
    return RtcEngineWrap::instance().setLocalVideoCanvas(
        uid, bytertc::StreamIndex::kStreamIndexMain, mode, view);
}

int VideoCallRtcEngineWrap::setupRemoteView(void* view, bytertc::RenderMode mode,
                                          const std::string& uid) {
    auto& engine_wrap = instance();
    return RtcEngineWrap::instance().setRemoteVideoCanvas(
        uid, bytertc::StreamIndex::kStreamIndexMain, mode, view);
}

int VideoCallRtcEngineWrap::startPreview() {
    auto& engine_wrap = instance();
    return RtcEngineWrap::instance().startPreview();
}

int VideoCallRtcEngineWrap::stopPreview() {
    auto& engine_wrap = instance();
    return RtcEngineWrap::instance().stopPreview();
}

int VideoCallRtcEngineWrap::enableLocalAudio(bool enable) {
	return RtcEngineWrap::instance().enableLocalAudio(enable);
}

int VideoCallRtcEngineWrap::enableLocalVideo(bool enable) {
  return RtcEngineWrap::instance().enableLocalVideo(enable);
}

int VideoCallRtcEngineWrap::muteLocalAudio(bool bMute) {
  auto& engine_wrap = instance();
  return RtcEngineWrap::instance().muteLocalAudio(bMute);
}

int VideoCallRtcEngineWrap::muteLocalVideo(bool bMute) {
  auto& engine_wrap = instance();
  return RtcEngineWrap::instance().muteLocalVideo(bMute);
}

int VideoCallRtcEngineWrap::login(const std::string& roomid,
                                const std::string& uid,
                                const std::string& token) {
  auto& engine_wrap = instance();
  auto userName = videocall::DataMgr::instance().user_name();
  
  QJsonObject extra_info;
  extra_info["user_id"] = QString::fromStdString(uid);
  extra_info["user_name"] = QString::fromStdString(userName);
  
  auto infoStr = QString(QJsonDocument(extra_info).toJson());
  auto infoStdString = std::string(infoStr.toUtf8());

  bytertc::UserInfo user = {uid.c_str(), infoStdString.c_str()};
  return RtcEngineWrap::instance().joinRoom(
      token, roomid, user,
      bytertc::RoomProfileType::kRoomProfileTypeCommunication);
}

int VideoCallRtcEngineWrap::logout() {
  auto& engine_wrap = instance();
  return RtcEngineWrap::instance().leaveRoom();
}

int VideoCallRtcEngineWrap::setVideoProfiles(const videocall::VideoConfiger& vc) {
    auto& engine_wrap = instance();
    bytertc::VideoEncoderConfig config;
    config.width = vc.resolution.width;
    config.height = vc.resolution.height;
    config.frameRate = vc.fps;
    config.maxBitrate = vc.kbps;
    return RtcEngineWrap::instance().setVideoProfiles(config);
}

int VideoCallRtcEngineWrap::setAudioProfiles(const videocall::AudioQuality& aq) {
    auto& engine_wrap = instance();
    bytertc::AudioProfileType profileType = static_cast<bytertc::AudioProfileType>(static_cast<int>(aq));
    return RtcEngineWrap::instance().setAudioProfiles(profileType);
}

int VideoCallRtcEngineWrap::setScreenProfiles(const videocall::VideoConfiger& vc) {
    bytertc::ScreenVideoEncoderConfig config;
    config.frameRate = vc.fps;
    config.height = vc.resolution.height;
    config.width = vc.resolution.width;
    config.maxBitrate = vc.kbps;
    return RtcEngineWrap::instance().setScreenProfiles(config);
}

int VideoCallRtcEngineWrap::setLocalMirrorMode(bool isMirrored) {
	auto type = isMirrored ? bytertc::MirrorType::kMirrorTypeRenderAndEncoder
		: bytertc::MirrorType::kMirrorTypeNone;
	return RtcEngineWrap::instance().setLocalPreviewMirrorMode(type);
}

int VideoCallRtcEngineWrap::getAudioInputDevices(
    std::vector<RtcDevice>& devices) {
	return RtcEngineWrap::instance().getAudioInputDevices(devices);
}

int VideoCallRtcEngineWrap::setAudioInputDevice(int index) {
	return RtcEngineWrap::instance().setAudioInputDevice(index);
}

int VideoCallRtcEngineWrap::getAudioInputDevice(std::string& guid) {
	return RtcEngineWrap::instance().getAudioInputDevice(guid);
}

int VideoCallRtcEngineWrap::setAudioVolumeIndicate(int indicate) {
	return RtcEngineWrap::instance().setAudioVolumeIndicate(indicate);
}

int VideoCallRtcEngineWrap::getAudioOutputDevices(
    std::vector<RtcDevice>& devices) {
	return RtcEngineWrap::instance().getAudioOutputDevices(devices);
}

int VideoCallRtcEngineWrap::setAudioOutputDevice(int index) {
	return RtcEngineWrap::instance().setAudioOutputDevice(index);
}

int VideoCallRtcEngineWrap::getAudioOutputDevice(std::string& guid) {
	return RtcEngineWrap::instance().getAudioOutputDevice(guid);
}

int VideoCallRtcEngineWrap::getVideoCaptureDevices(
    std::vector<RtcDevice>& devices) {
	return RtcEngineWrap::instance().getVideoCaptureDevices(devices);
}

int VideoCallRtcEngineWrap::setVideoCaptureDevice(int index) {
	return RtcEngineWrap::instance().setVideoCaptureDevice(index);
}

int VideoCallRtcEngineWrap::getVideoCaptureDevice(std::string& guid) {
	return RtcEngineWrap::instance().getVideoCaptureDevice(guid);
}

int VideoCallRtcEngineWrap::setRemoteScreenView(const std::string& uid,
                                              void* view) {
    return RtcEngineWrap::instance().setRemoteVideoCanvas(
        uid, bytertc::StreamIndex::kStreamIndexScreen,
        bytertc::RenderMode::kRenderModeFit, view);
}

int VideoCallRtcEngineWrap::startScreenCapture(
    void* source_id, const std::vector<void*>& excluded) {
	return RtcEngineWrap::instance().startScreenCapture(source_id, excluded);
}

int VideoCallRtcEngineWrap::startScreenCaptureByWindowId(void* window_id) {
	return RtcEngineWrap::instance().startScreenCaptureByWindowId(window_id);
}

int VideoCallRtcEngineWrap::stopScreenCapture() {
	return RtcEngineWrap::instance().stopScreenCapture();
}

int VideoCallRtcEngineWrap::startScreenAudioCapture() {
	return RtcEngineWrap::instance().startScreenAudioCapture();
}

int VideoCallRtcEngineWrap::stopScreenAudioCapture() {
	return RtcEngineWrap::instance().stopScreenAudioCapture();
}

int VideoCallRtcEngineWrap::getShareList(std::vector<SnapshotAttr>& list) {
	return RtcEngineWrap::instance().getShareList(list);
}

QPixmap VideoCallRtcEngineWrap::getThumbnail(SnapshotAttr::SnapshotType type,
                                           void* source_id, int max_width,
                                           int max_height) {
	return RtcEngineWrap::instance().getThumbnail(type, source_id, max_width,
                                         max_height);
}

bool VideoCallRtcEngineWrap::audioRecordDevicesTest() {
    return RtcEngineWrap::instance().audioReocrdDeviceTest();
}

void VideoCallRtcEngineWrap::setBasicBeauty(bool enabled) {
    int res = RtcEngineWrap::instance().enableEffectBeauty(enabled);
    if (res == 0 && enabled) {
        RtcEngineWrap::instance().setBeautyIntensity(bytertc::kEffectBeautyWhite, 0.2);
        RtcEngineWrap::instance().setBeautyIntensity(bytertc::kEffectBeautySmooth, 0.3);
        RtcEngineWrap::instance().setBeautyIntensity(bytertc::kEffectBeautySharpen, 0.4);
    }
}

int VideoCallRtcEngineWrap::feedBack(const std::string& str) { 
	return 0; 
}

void VideoCallRtcEngineWrap::onVideoStateChanged(std::string device_id,
    bytertc::MediaDeviceState device_state, bytertc::MediaDeviceError error) {
	std::vector<RtcDevice> devices;
	VideoCallRtcEngineWrap::getVideoCaptureDevices(devices);
	if ((devices.empty() || error == bytertc::kMediaDeviceErrorDeviceNoPermission)
		&& !videocall::DataMgr::instance().mute_video()) {
		if (error == bytertc::kMediaDeviceErrorDeviceNoPermission) {
			vrd::util::showToastInfo(QObject::tr("no_camera_permission").toStdString());
		}
		videocall::DataMgr::instance().setMuteVideo(true);
		VideoCallRtcEngineWrap::muteLocalVideo(true);
		emit instance().sigUpdateVideo();

	} else {
		VideoCallRtcEngineWrap::muteLocalVideo(
			videocall::DataMgr::instance().mute_video());
	}

	switch (device_state) {
		// {zh} 插入一个新的设备
		// {en} Insert a new device
		case bytertc::kMediaDeviceStateAdded:
			vrd::util::showToastInfo(QObject::tr("new_camera_plugin").toStdString());
			emit instance().sigUpdateVideoDevices();
			break;
		// {zh} 拔出设备
		// {en} Pull out the device
		case bytertc::kMediaDeviceStateRemoved:
			vrd::util::showToastInfo(QObject::tr("camera_unplugged").toStdString());
			emit instance().sigUpdateVideoDevices();
			VideoCallRtcEngineWrap::setVideoCaptureDevice(RtcEngineWrap::instance().getCurrentVideoCaptureDeviceIndex());
			break;
		default:
			break;
	}
}

void VideoCallRtcEngineWrap::onAudioStateChanged(std::string device_id,
    bytertc::MediaDeviceState device_state, bytertc::MediaDeviceError error) {
    std::vector<RtcDevice> devices;
    VideoCallRtcEngineWrap::getAudioInputDevices(devices);
    if ((devices.empty()) && !videocall::DataMgr::instance().mute_audio()) {
        videocall::DataMgr::instance().setMuteAudio(true);
        VideoCallRtcEngineWrap::muteLocalAudio(true);
        emit instance().sigUpdateAudio();
    }
    else {
        VideoCallRtcEngineWrap::muteLocalAudio(
            videocall::DataMgr::instance().mute_audio());
    }

	switch (device_state) {
	// {zh} 插入一个新的设备
	// {en} Insert a new device
	case bytertc::kMediaDeviceStateAdded:  
		vrd::util::showToastInfo(QObject::tr("new_mic_plugin").toStdString());
		emit instance().sigUpdateAudioDevices();
		break;
	// {zh} 拔出设备
	// {en} Pull out the device
	case bytertc::kMediaDeviceStateRemoved:
	{
		std::string curDeviceStr;
		VideoCallRtcEngineWrap::getAudioInputDevice(curDeviceStr);
		if (strcmp(curDeviceStr.c_str(), device_id.c_str()) == 0)
		{
			RtcEngineWrap::instance().followSystemCaptureDevice(true);
		}
        vrd::util::showToastInfo(QObject::tr("mic_unplugged").toStdString());
        emit instance().sigUpdateAudioDevices();
		break;
	}
    case bytertc::kMediaDeviceStateRuntimeError:
    {
        std::string curDeviceStr;
        VideoCallRtcEngineWrap::getAudioInputDevice(curDeviceStr);
        if (strcmp(curDeviceStr.c_str(), device_id.c_str()) == 0 && !videocall::DataMgr::instance().mute_audio()){
            if (error == bytertc::kMediaDeviceErrorDeviceNoPermission 
                || error == bytertc::kMediaDeviceErrorDeviceFailure
                || error == bytertc::kMediaDeviceErrorDeviceBusy) {
                vrd::util::showToastInfo(QObject::tr("no_microphone_permission").toStdString());
                videocall::DataMgr::instance().setMuteAudio(true);
                VideoCallRtcEngineWrap::muteLocalAudio(true);
                emit instance().sigUpdateAudio();
            }
        }
        break;
    }
	default:
		break;
	}
}

void VideoCallRtcEngineWrap::onUserJoinedVideoCall(UserInfoWrap user_info, int elapsed) {
	videocall::User newUser;
	newUser.user_id = user_info.uid;

    auto infoArray = QByteArray(user_info.extra_info.data(), 
		static_cast<int>(user_info.extra_info.size()));
    auto infoJsonObj = QJsonDocument::fromJson(infoArray).object();
	newUser.user_name = std::string(infoJsonObj["user_name"].toString().toUtf8());
	if (newUser.user_name == "") newUser.user_name = user_info.uid;

    auto& users = videocall::DataMgr::instance().ref_users();
    auto iter = std::find_if(users.begin(), users.end(),
        [newUser](const videocall::User& user) {
            return user.user_id == newUser.user_id;
        });
    if (iter == users.end()) {
        users.push_back(newUser);
    }

	auto& remoteStreamInfos = videocall::DataMgr::instance().ref_remote_stream_infos();
	videocall::StreamInfo info;
	info.user_id = user_info.uid;
	info.user_name = std::string(infoJsonObj["user_name"].toString().toUtf8());
	remoteStreamInfos.push_back(info);
	emit sigUpdateMainPageData();
}

void VideoCallRtcEngineWrap::onUserLeaveVideoCall(std::string uid, 
	bytertc::UserOfflineReason reason) {

    auto& users = videocall::DataMgr::instance().ref_users();
    auto iter = std::find_if(users.begin(), users.end(),
        [uid](const videocall::User& user) {
			return user.user_id == uid;
        });
    if (iter != users.end()) {
		users.erase(iter);
    }

    auto& remoteStreamInfos = videocall::DataMgr::instance().ref_remote_stream_infos();
    auto infoIter = std::find_if(
		remoteStreamInfos.begin(), remoteStreamInfos.end(),
        [uid](const videocall::StreamInfo& info) { return info.user_id == uid; });
    if (infoIter != remoteStreamInfos.end()) {
		remoteStreamInfos.erase(infoIter);
    }
	emit sigUpdateMainPageData();
}

void VideoCallRtcEngineWrap::onUserCameraStatusChange(std::string uid, bool enabled) {
    auto& users = videocall::DataMgr::instance().ref_users();
    auto iter = std::find_if(users.begin(), users.end(),
        [uid](const videocall::User& user) {
            return user.user_id == uid;
        });
    if (iter != users.end()) {
		iter->is_camera_on = enabled;
    }
	emit sigUpdateMainPageData();
}

void VideoCallRtcEngineWrap::onUserMicStatusChange(std::string uid, bool enabled) {
    auto& users = videocall::DataMgr::instance().ref_users();
    auto iter = std::find_if(users.begin(), users.end(),
        [uid](const videocall::User& user) {
            return user.user_id == uid;
        });
    if (iter != users.end()) {
		iter->is_mic_on = enabled;
    }
	emit sigUpdateMainPageData();
}

VideoCallRtcEngineWrap::VideoCallRtcEngineWrap() : QObject(nullptr) {}

VideoCallRtcEngineWrap::~VideoCallRtcEngineWrap() {}
