#pragma once
#include <QObject>
#include <QThread>

#include "core/rtc_engine_wrap.h"
#include "videocall/core/videocall_model.h"

/** {zh}
 * 本场景内的需要用到的RTC接口和回调的封装类
 */

/** {en}
* The RTC interface and callback encapsulation class that need to be used in this scene
*/
class VideoCallRtcEngineWrap : public QObject {
	Q_OBJECT

public:
	static VideoCallRtcEngineWrap& instance();
	static int init();
	static int unInit();
	static int setupLocalView(void* view, bytertc::RenderMode mode,
		const std::string& uid);
	static int setupRemoteView(void* view, bytertc::RenderMode mode,
		const std::string& uid);
	static int startPreview();
	static int stopPreview();
	static int enableLocalAudio(bool enable);
	static int enableLocalVideo(bool enable);

	static int muteLocalAudio(bool bMute);
	static int muteLocalVideo(bool bMute);

	static int login(const std::string& roomid, const std::string& uid,
		const std::string& token);
	static int logout();

	static int setVideoProfiles(const videocall::VideoConfiger& vc);
	static int setAudioProfiles(const videocall::AudioQuality& aq);
	static int setScreenProfiles(const videocall::VideoConfiger& vc);
	static int setLocalMirrorMode(bool isMirrored);

	static int getAudioInputDevices(std::vector<RtcDevice>&);
	static int setAudioInputDevice(int index);
	static int getAudioInputDevice(std::string& guid);

	static int setAudioVolumeIndicate(int indicate);

	static int getAudioOutputDevices(std::vector<RtcDevice>&);
	static int setAudioOutputDevice(int index);
	static int getAudioOutputDevice(std::string& guid);

	static int getVideoCaptureDevices(std::vector<RtcDevice>&);
	static int setVideoCaptureDevice(int index);
	static int getVideoCaptureDevice(std::string& guid);

	static int setRemoteScreenView(const std::string& uid, void* view);
	static int startScreenCapture(void* source_id,
		const std::vector<void*>& excluded);
	static int startScreenCaptureByWindowId(void* window_id);
	static int stopScreenCapture();
	static int startScreenAudioCapture();
	static int stopScreenAudioCapture();

	static int getShareList(std::vector<SnapshotAttr>& list);
	static QPixmap getThumbnail(SnapshotAttr::SnapshotType type, void* source_id,
		int max_width, int max_height);
	static bool audioRecordDevicesTest();
	static void setBasicBeauty(bool enabled);
	static int feedBack(const std::string& str);

public:
    void onVideoStateChanged(std::string device_id,
        bytertc::MediaDeviceState device_state, bytertc::MediaDeviceError error);
    void onAudioStateChanged(std::string device_id,
        bytertc::MediaDeviceState device_state, bytertc::MediaDeviceError error);
    void onUserJoinedVideoCall(UserInfoWrap user_info, int elapsed);
    void onUserLeaveVideoCall(std::string uid, bytertc::UserOfflineReason reason);
    void onUserCameraStatusChange(std::string uid, bool enabled);
    void onUserMicStatusChange(std::string uid, bool enabled);

signals:
	void sigOnRoomStateChanged(std::string room_id, std::string uid, int state, std::string extra_info);
	void sigOnShareScreenStatusChanged(std::string uid, bool isSharing);
	void sigUpdateAudio();
	void sigUpdateVideo();
	void sigUpdateVideoDevices();
	void sigUpdateAudioDevices();
	void sigOnAudioVolumeUpdate();
	void sigUpdateInfo(std::string uid);
	void sigUpdateMainPageData();

 protected:
	 VideoCallRtcEngineWrap();
	 ~VideoCallRtcEngineWrap();
};
