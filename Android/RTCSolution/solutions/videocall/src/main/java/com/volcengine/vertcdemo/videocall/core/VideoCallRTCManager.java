// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.core;

import static com.ss.bytertc.engine.VideoCanvas.RENDER_MODE_FIT;
import static com.ss.bytertc.engine.VideoCanvas.RENDER_MODE_HIDDEN;
import static com.ss.bytertc.engine.data.EffectBeautyMode.EFFECT_SHARPEN_MODE;
import static com.ss.bytertc.engine.data.EffectBeautyMode.EFFECT_SMOOTH_MODE;
import static com.ss.bytertc.engine.data.EffectBeautyMode.EFFECT_WHITE_MODE;
import static com.ss.bytertc.engine.type.NetworkQuality.NETWORK_QUALITY_UNKNOWN;
import static com.volcengine.vertcdemo.videocall.core.Constants.VOLUME_SPEAKING_INTERVAL;
import static com.volcengine.vertcdemo.videocall.core.Constants.VOLUME_SPEAKING_THRESHOLD;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Process;
import android.text.TextUtils;
import android.util.Log;
import android.util.Pair;
import android.view.TextureView;

import androidx.annotation.NonNull;

import com.ss.bytertc.engine.RTCRoom;
import com.ss.bytertc.engine.RTCRoomConfig;
import com.ss.bytertc.engine.RTCVideo;
import com.ss.bytertc.engine.UserInfo;
import com.ss.bytertc.engine.VideoCanvas;
import com.ss.bytertc.engine.VideoEncoderConfig;
import com.ss.bytertc.engine.data.AudioPropertiesConfig;
import com.ss.bytertc.engine.data.AudioRoute;
import com.ss.bytertc.engine.data.CameraId;
import com.ss.bytertc.engine.data.LocalAudioPropertiesInfo;
import com.ss.bytertc.engine.data.MirrorType;
import com.ss.bytertc.engine.data.RemoteAudioPropertiesInfo;
import com.ss.bytertc.engine.data.RemoteStreamKey;
import com.ss.bytertc.engine.data.StreamIndex;
import com.ss.bytertc.engine.type.AudioProfileType;
import com.ss.bytertc.engine.type.AudioScenarioType;
import com.ss.bytertc.engine.type.ChannelProfile;
import com.ss.bytertc.engine.type.LocalStreamStats;
import com.ss.bytertc.engine.type.MediaStreamType;
import com.ss.bytertc.engine.type.NetworkQualityStats;
import com.ss.bytertc.engine.type.RemoteStreamStats;
import com.ss.bytertc.engine.type.StreamRemoveReason;
import com.volcengine.vertcdemo.common.GsonUtils;
import com.volcengine.vertcdemo.common.SolutionToast;
import com.volcengine.vertcdemo.utils.AppUtil;
import com.volcengine.vertcdemo.core.SolutionDataManager;
import com.volcengine.vertcdemo.core.eventbus.SolutionDemoEventManager;
import com.volcengine.vertcdemo.core.net.rts.RTCRoomEventHandlerWithRTS;
import com.volcengine.vertcdemo.core.net.rts.RTCVideoEventHandlerWithRTS;
import com.volcengine.vertcdemo.core.net.rts.RTSInfo;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.bean.VideoCallUserInfo;
import com.volcengine.vertcdemo.videocall.event.AudioPropertiesReportEvent;
import com.volcengine.vertcdemo.videocall.event.AudioRouterEvent;
import com.volcengine.vertcdemo.videocall.event.LocalStreamStatsEvent;
import com.volcengine.vertcdemo.videocall.event.MediaStatusEvent;
import com.volcengine.vertcdemo.core.eventbus.SDKReconnectToRoomEvent;
import com.volcengine.vertcdemo.videocall.event.RemoteStreamStatsEvent;

import java.util.HashMap;
import java.util.Map;

/**
 * RTC对象管理类
 *
 * 使用单例形式，调用RTC接口，并在调用中更新 VideoCallDataManager 数据
 * 内部记录开关状态
 *
 * 功能：
 * 1.开关和媒体状态
 * 2.获取当前媒体状态
 * 3.接收RTC各种回调，例如：用户进退房、媒体状态改变、媒体状态数据回调、网络状态回调、音量大小回调
 * 4.管理用户视频渲染view
 * 5.加入离开房间
 * 6.创建和销毁引擎
 */
public class VideoCallRTCManager {

    private static final String TAG = "VideoCallRTCManager";

    private static VideoCallRTCManager sInstance;
    // rts 实例
    private VideoCallRTSClient mRTSClient;
    // rtc 引擎实例
    private RTCVideo mRTCVideo;
    // rtc room 实例
    private RTCRoom mRTCRoom;
    // 摄像头视频流渲染
    public Map<String, TextureView> mUserIdViewMap = new HashMap<>();
    // 屏幕视频流渲染
    public Map<String, TextureView> mScreenUserIdViewMap = new HashMap<>();

    // 摄像头状态
    private boolean mIsCameraOn = true;
    // 麦克风状态
    private boolean mIsMicOn = true;
    // 是否发布音频流
    private boolean mIsAudioMute = false;
    // 是否使用扬声器
    private boolean mIsSpeakerphone = true;
    // 是否开启视频镜像
    private boolean mVideoMirror = true;
    // 是否使用前置摄像头
    private boolean mIsFrontCamera = true;
    // 当前房间的roomId，需要在渲染远端用户视频的时候传入
    private String mRoomId;
    // 当前的音频路由
    private AudioRoute mCurrentAudioRoute = null;

    // 视频分辨率
    private String mResolution = Constants.DEFAULT_RESOLUTION;
    // 音频质量
    private String mAudioQuality = Constants.DEFAULT_QUALITY;

    private final HashMap<String, Integer> mQualityCache = new HashMap<>();

    // 缓存远端用户的麦克风状态，减少 Event 发送数量
    private final HashMap<String, Boolean> mRemoteSpeakingCache = new HashMap<>();

    // RTC 引擎回调对象
    private final RTCVideoEventHandlerWithRTS mIRTCEngineEventHandler = new RTCVideoEventHandlerWithRTS() {

        private boolean mLocalSpeaking = false;

        /**
         * 本地音频包括使用 RTC SDK 内部机制采集的麦克风音频和屏幕音频。
         * @param audioPropertiesInfos 本地音频信息，详见 LocalAudioPropertiesInfo 。
         */
        @Override
        public void onLocalAudioPropertiesReport(LocalAudioPropertiesInfo[] audioPropertiesInfos) {
            super.onLocalAudioPropertiesReport(audioPropertiesInfos);
            Log.d(TAG, String.format("onLocalAudioPropertiesReport: %s", GsonUtils.gson().toJson(audioPropertiesInfos)));

            if (audioPropertiesInfos == null || audioPropertiesInfos.length == 0) {
                return;
            }

            LocalAudioPropertiesInfo info = audioPropertiesInfos[0];
            boolean isLocalSpeaking = info.audioPropertiesInfo.linearVolume > VOLUME_SPEAKING_THRESHOLD;

            if (isLocalSpeaking != mLocalSpeaking) {
                SolutionDemoEventManager.post(new AudioPropertiesReportEvent(
                        info.streamIndex,
                        SolutionDataManager.ins().getUserId(),
                        isLocalSpeaking));
                mLocalSpeaking = isLocalSpeaking;
            }
        }

        /**
         * 远端用户的音频包括使用 RTC SDK 内部机制/自定义机制采集的麦克风音频和屏幕音频。
         *
         * @param audioPropertiesInfos 远端音频信息，其中包含音频流属性、房间 ID、用户 ID ，详见 RemoteAudioPropertiesInfo。
         * @param totalRemoteVolume    订阅的所有远端流的总音量。
         */
        @Override
        public void onRemoteAudioPropertiesReport(RemoteAudioPropertiesInfo[] audioPropertiesInfos,
                                                  int totalRemoteVolume) {
            super.onRemoteAudioPropertiesReport(audioPropertiesInfos, totalRemoteVolume);
            Log.d(TAG, String.format("onRemoteAudioPropertiesReport: %s", GsonUtils.gson().toJson(audioPropertiesInfos)));
            for (RemoteAudioPropertiesInfo audioPropertiesInfo : audioPropertiesInfos) {
                final boolean isSpeaking = audioPropertiesInfo.audioPropertiesInfo.linearVolume > VOLUME_SPEAKING_THRESHOLD;
                final String userId = audioPropertiesInfo.streamKey.getUserId();
                final boolean oldValue = mRemoteSpeakingCache.get(userId) == Boolean.TRUE;
                if (oldValue != isSpeaking) { // 变更时，再进行通知
                    SolutionDemoEventManager.post(new AudioPropertiesReportEvent(
                            audioPropertiesInfo.streamKey.getStreamIndex(),
                            userId,
                            isSpeaking));
                    if (isSpeaking) {
                        mRemoteSpeakingCache.put(userId, Boolean.TRUE);
                    } else {
                        mRemoteSpeakingCache.remove(userId);
                    }
                }
            }
        }

        /**
         * 房间内的可见用户调用 startVideoCapture 开启内部视频采集时，房间内其他用户会收到此回调。
         * @param roomId 开启视频采集的远端用户所在的房间 ID
         * @param uid 开启视频采集的远端用户 ID
         */
        @Override
        public void onUserStartVideoCapture(String roomId, String uid) {
            super.onUserStartVideoCapture(roomId, uid);
            VideoCallDataManager.ins().updateVideoStatus(uid, true);
        }

        /**
         * 房间内的可见用户调用 stopVideoCapture 关闭内部视频采集时，房间内其他用户会收到此回调。
         * @param roomId 关闭视频采集的远端用户所在的房间 ID
         * @param uid 关闭视频采集的远端用户 ID
         */
        @Override
        public void onUserStopVideoCapture(String roomId, String uid) {
            super.onUserStopVideoCapture(roomId, uid);
            VideoCallDataManager.ins().updateVideoStatus(uid, false);
        }

        /**
         * 音频播放路由变化时，收到该回调。
         * @param route 新的音频播放路由，详见 AudioRoute
         */
        @Override
        public void onAudioRouteChanged(AudioRoute route) {
            super.onAudioRouteChanged(route);
            Log.d(TAG, String.format("onAudioRouteChanged: %s", route));
            AudioRoute lastRouter = mCurrentAudioRoute;
            mCurrentAudioRoute = route;

            if (isHeadSet(lastRouter) && !isHeadSet(route)) {
                // 对应的拔掉耳机的情况
                useSpeakerphone(true);
            }
        }
    };

    private final RTCRoomEventHandlerWithRTS mRTCRoomEventHandler = new RTCRoomEventHandlerWithRTS() {

        /**
         * 房间状态改变回调，加入房间、离开房间、发生房间相关的警告或错误时会收到此回调。
         * @param roomId 房间id
         * @param uid 用户id
         * @param state 房间状态码
         * @param extraInfo 额外信息
         */
        @Override
        public void onRoomStateChanged(String roomId, String uid, int state, String extraInfo) {
            super.onRoomStateChanged(roomId, uid, state, extraInfo);
            Log.d(TAG, String.format("onRoomStateChanged: %s %s %d %s", roomId, uid, state, extraInfo));
            if (isFirstJoinRoomSuccess(state, extraInfo)) {
                VideoCallUserInfo userInfo = new VideoCallUserInfo();
                userInfo.userId = uid;
                userInfo.userName = SolutionDataManager.ins().getUserName();
                userInfo.isCameraOn = mIsCameraOn;
                userInfo.isMicOn = mIsMicOn && !mIsAudioMute;
                userInfo.isScreenShare = false;

                VideoCallDataManager.ins().addUser(userInfo);
            } else if (isReconnectSuccess(state, extraInfo)) {
                SolutionDemoEventManager.post(new SDKReconnectToRoomEvent(roomId));
            }
            mRoomId = roomId;
        }

        /**
         * 房间内新增远端摄像头/麦克风采集音视频流的回调。
         * @param uid 远端流发布用户的用户 ID。
         * @param type 远端媒体流的类型，参看 MediaStreamType。
         */
        @Override
        public void onUserPublishStream(String uid, MediaStreamType type) {
            super.onUserPublishStream(uid, type);
            Log.d(TAG, String.format("onUserPublishStream: %s %s", uid, type.toString()));
            if (type == MediaStreamType.RTC_MEDIA_STREAM_TYPE_BOTH) {
                VideoCallDataManager.ins().updateAudioStatus(uid, true);
                VideoCallDataManager.ins().updateVideoStatus(uid, true);
            } else if (type == MediaStreamType.RTC_MEDIA_STREAM_TYPE_AUDIO) {
                VideoCallDataManager.ins().updateAudioStatus(uid, true);
            } else if (type == MediaStreamType.RTC_MEDIA_STREAM_TYPE_VIDEO) {
                VideoCallDataManager.ins().updateVideoStatus(uid, true);
            }
        }

        /**
         * 房间内远端摄像头/麦克风采集的媒体流移除的回调。
         * @param uid 移除的远端流发布用户的用户 ID。
         * @param type 移除的远端流类型，参看 MediaStreamType。
         * @param reason 远端流移除的原因，参看 StreamRemoveReason。
         */
        @Override
        public void onUserUnpublishStream(String uid, MediaStreamType type, StreamRemoveReason reason) {
            super.onUserUnpublishStream(uid, type, reason);
            Log.d(TAG, String.format("onUserUnPublishStream: %s, %s, %s", uid, type.toString(), reason.toString()));
            if (type == MediaStreamType.RTC_MEDIA_STREAM_TYPE_BOTH) {
                VideoCallDataManager.ins().updateAudioStatus(uid, false);
                VideoCallDataManager.ins().updateVideoStatus(uid, false);
            } else if (type == MediaStreamType.RTC_MEDIA_STREAM_TYPE_AUDIO) {
                VideoCallDataManager.ins().updateAudioStatus(uid, false);
            } else if (type == MediaStreamType.RTC_MEDIA_STREAM_TYPE_VIDEO) {
                VideoCallDataManager.ins().updateVideoStatus(uid, false);
            }
        }

        /**
         * 房间内新增远端屏幕共享音视频流的回调。
         * @param uid 远端流发布用户的用户 ID。
         * @param type 远端媒体流的类型，参看 MediaStreamType。
         */
        @Override
        public void onUserPublishScreen(String uid, MediaStreamType type) {
            super.onUserPublishScreen(uid, type);
            Log.d(TAG, String.format("onUserPublishScreen: %s %s", uid, type.toString()));

            VideoCallUserInfo vcUserInfo = new VideoCallUserInfo();
            vcUserInfo.userId = uid;
            vcUserInfo.userName = VideoCallDataManager.ins().getUserNameByUserId(uid);
            vcUserInfo.isScreenShare = true;
            vcUserInfo.isMicOn = true;
            vcUserInfo.isCameraOn = true;
            VideoCallDataManager.ins().setScreenShareUser(vcUserInfo);
        }

        /**
         * 房间内远端屏幕共享音视频流移除的回调。
         * @param uid 移除的远端流发布用户的用户 ID。
         * @param type 移除的远端流类型，参看 MediaStreamType。
         * @param reason 远端流移除的原因，参看 StreamRemoveReason。
         */
        @Override
        public void onUserUnpublishScreen(String uid, MediaStreamType type, StreamRemoveReason reason) {
            super.onUserUnpublishScreen(uid, type, reason);
            Log.d(TAG, String.format("onUserUnPublishScreen: %s, %s, %s", uid, type.toString(), reason.toString()));

            VideoCallDataManager.ins().removeScreenShareUser(uid);
        }

        /**
         * 加入房间后， 以 2 秒 1 次的频率，报告用户的网络质量信息
         *
         * @param localQuality    本地网络质量，详见 NetworkQualityStats。
         * @param remoteQualities 已订阅用户的网络质量，详见 NetworkQualityStats。
         * @see NetworkQualityStats
         */
        @Override
        public void onNetworkQuality(NetworkQualityStats localQuality, NetworkQualityStats[] remoteQualities) {
            super.onNetworkQuality(localQuality, remoteQualities);
            for (NetworkQualityStats remoteQuality : remoteQualities) {
                mQualityCache.put(remoteQuality.uid, remoteQuality.rxQuality);
            }
        }

        /**
         * 反映通话中本地设备发送音/视频流的统计信息以及网络状况的回调。
         * @param stats 音视频流以及网络状况统计信息。参见 LocalStreamStats。
         */
        @Override
        public void onLocalStreamStats(LocalStreamStats stats) {
            super.onLocalStreamStats(stats);
            Log.d(TAG, String.format("onLocalStreamStats: %s", stats));
            SolutionDemoEventManager.post(new LocalStreamStatsEvent(stats));
        }

        /**
         * 反映通话中本地设备接收订阅的远端音/视频流的统计信息以及网络状况的回调。每隔 2s 收到此回调。
         * @param stats 音视频流以及网络状况统计信息。参见 RemoteStreamStats。
         */
        @Override
        public void onRemoteStreamStats(RemoteStreamStats stats) {
            super.onRemoteStreamStats(stats);
            Log.d(TAG, String.format("onRemoteStreamStats: %s", stats));

            final Integer value = mQualityCache.get(stats.uid);
            int quality = value == null ? NETWORK_QUALITY_UNKNOWN : value;

            stats.rxQuality = quality;
            if (stats.audioStats != null) {
                stats.audioStats.quality = quality;
            }
            SolutionDemoEventManager.post(new RemoteStreamStatsEvent(stats));
        }

        /**
         * 可见用户加入房间，或房内隐身用户切换为可见的回调。
         * @param userInfo 用户信息
         * @param elapsed 主播角色用户调用 joinRoom 加入房间到房间内其他用户收到该事件经历的时间，单位为 ms。
         */
        @Override
        public void onUserJoined(UserInfo userInfo, int elapsed) {
            super.onUserJoined(userInfo, elapsed);
            Log.d(TAG, String.format("onUserJoined: %s %d", userInfo.toString(), elapsed));

            VideoCallUserInfo videoCallUserInfo = rtcUserInfoToVCUserInfo(userInfo);
            VideoCallDataManager.ins().addUser(videoCallUserInfo);
        }

        /**
         * 远端用户离开房间，或切至不可见时，本地用户会收到此事件
         * @param uid 离开房间，或切至不可见的的远端用户 ID。
         * @param reason 用户离开房间的原因：
         * • 0: 远端用户调用 leaveRoom 主动退出房间。
         * • 1: 远端用户因 Token 过期或网络原因等掉线。
         * • 2: 远端用户调用 setUserVisibility 切换至不可见状态。
         * • 3: 服务端调用 OpenAPI 将该远端用户踢出房间。
         */
        @Override
        public void onUserLeave(String uid, int reason) {
            super.onUserLeave(uid, reason);
            Log.d(TAG, String.format("onUserLeave: %s %d", uid, reason));

            VideoCallDataManager.ins().removeUser(uid);
            mQualityCache.remove(uid);
            mRemoteSpeakingCache.remove(uid);
        }
    };

    public static @NonNull
    VideoCallRTCManager ins() {
        if (sInstance == null) {
            sInstance = new VideoCallRTCManager();
        }
        return sInstance;
    }

    public VideoCallRTSClient getRTSClient() {
        return mRTSClient;
    }

    public void initEngine(RTSInfo info) {
        Log.d(TAG, String.format("initEngine: %s", info));
        destroyEngine();
        // 创建引擎实例
        mRTCVideo = RTCVideo.createRTCVideo(AppUtil.getApplicationContext(), info.appId,
                mIRTCEngineEventHandler, null, null);
        /*
         设置业务标识参数
         可通过 businessId 区分不同的业务场景。businessId 由客户自定义，相当于一个“标签”，
         可以分担和细化现在 AppId 的逻辑划分的功能，但不需要鉴权。
         */
        mRTCVideo.setBusinessId(info.bid);
        // 设置音频场景类型
        mRTCVideo.setAudioScenario(AudioScenarioType.AUDIO_SCENARIO_COMMUNICATION);
        // 立即开启内部视频采集。默认为关闭状态。
        mRTCVideo.startVideoCapture();
        // 开启内部音频采集。默认为关闭状态。
        mRTCVideo.startAudioCapture();
        initDefaultVideoEffect();
        // 启用音频信息提示。
        mRTCVideo.enableAudioPropertiesReport(new AudioPropertiesConfig(VOLUME_SPEAKING_INTERVAL, true, false));
        mRTSClient = new VideoCallRTSClient(mRTCVideo, info);
        mIRTCEngineEventHandler.setBaseClient(mRTSClient);
        mRTCRoomEventHandler.setBaseClient(mRTSClient);

        setVideoResolution(mResolution);
        setAudioProfile(mAudioQuality);
        setMirrorType(true);
    }

    public void destroyEngine() {
        Log.d(TAG, "destroyEngine");
        if (mRTCRoom != null) {
            mRTCRoom.leaveRoom();
            mRTCRoom.destroy();
            mRTCRoom = null;
        }
        // 销毁引擎实例
        RTCVideo.destroyRTCVideo();
        mRTCVideo = null;
        if (mRTSClient != null) {
            mRTSClient.removeAllEventListener();
        }
        mRTSClient = null;
    }

    /**
     * 设置基础美颜效果
     */
    public void initDefaultVideoEffect() {
        mRTCVideo.enableEffectBeauty(true);
        // 设置美白效果
        mRTCVideo.setBeautyIntensity(EFFECT_WHITE_MODE, 0.2F);
        // 设置磨皮效果
        mRTCVideo.setBeautyIntensity(EFFECT_SMOOTH_MODE, 0.3F);
        // 设置锐化效果
        mRTCVideo.setBeautyIntensity(EFFECT_SHARPEN_MODE, 0.4F);
    }

    /**
     * 加入RTC房间
     *
     * @param token    rtc加房token
     * @param roomId   房间id
     * @param userId   用户id
     * @param userName 用户昵称
     */
    public void joinRoom(String token, String roomId, String userId, String userName) {
        if (token == null) {
            token = "";
        }
        if (roomId == null) {
            roomId = "";
        }
        if (userId == null) {
            userId = "";
        }
        if (userName == null) {
            userName = "";
        }
        Log.d(TAG, String.format("joinRoom: %s %s %s %s", token, roomId, userId, userName));
        leaveRoom();
        if (mRTCVideo == null) {
            return;
        }
        // 加入RTC房间，开启自动发布和订阅
        mRTCRoom = mRTCVideo.createRTCRoom(roomId);
        mRTCRoom.setRTCRoomEventHandler(mRTCRoomEventHandler);
        RTCRoomConfig roomConfig = new RTCRoomConfig(ChannelProfile.CHANNEL_PROFILE_COMMUNICATION,
                true, true, true);
        mRTCRoom.joinRoom(token, userIdNameToUserInfo(userId, userName), roomConfig);

        // 根据设置，手动设置是否发布视频
        startVideoCapture(mIsCameraOn);
        // 根据设置，手动设置是否发布音频
        startPublishAudio(!mIsAudioMute && mIsMicOn);
    }

    /**
     * 组装RTC的userInfo
     *
     * @param userId   用户id
     * @param userName 用户昵称
     * @return userinfo
     */
    public UserInfo userIdNameToUserInfo(String userId, String userName) {
        VideoCallUserInfo userInfo = new VideoCallUserInfo();
        userInfo.userId = userId;
        userInfo.userName = userName;
        String extra = GsonUtils.gson().toJson(userInfo);
        return new UserInfo(userId, extra);
    }

    /**
     * 将RTC的UserInfo转成业务UserInfo
     *
     * @param userInfo RTC UserInfo对象
     * @return 业务UserInfo
     */
    public VideoCallUserInfo rtcUserInfoToVCUserInfo(UserInfo userInfo) {
        String extraInfo = userInfo.getExtraInfo();
        if (TextUtils.isEmpty(extraInfo)) {
            return new VideoCallUserInfo(userInfo.getUid());
        }
        VideoCallUserInfo videoCallUserInfo = GsonUtils.gson().fromJson(extraInfo, VideoCallUserInfo.class);
        videoCallUserInfo.userId = userInfo.getUid();
        return videoCallUserInfo;
    }

    /**
     * 离开房间
     */
    public void leaveRoom() {
        Log.d(TAG, "leaveRoom");
        mRoomId = null;
        if (mRTCRoom != null) {
            mRTCRoom.leaveRoom();
            mRTCRoom.destroy();
        }
        mRTCRoom = null;
    }

    /**
     * 是否有摄像头权限
     *
     * @return 是否有权限
     */
    public boolean hasCameraPermission() {
        return hasPermission(Manifest.permission.CAMERA);
    }

    /**
     * 是否有麦克风权限
     *
     * @return 是否有权限
     */
    public boolean hasAudioPermission() {
        return hasPermission(Manifest.permission.RECORD_AUDIO);
    }

    /**
     * 是否有对应权限
     *
     * @param permission 权限名
     * @return 是否有权限
     */
    public boolean hasPermission(String permission) {
        int res = AppUtil.getApplicationContext().checkPermission(
                permission, android.os.Process.myPid(), Process.myUid());
        return res == PackageManager.PERMISSION_GRANTED;
    }

    /**
     * 开启摄像头
     *
     * @param on 是否打开
     */
    public void startVideoCapture(boolean on) {
        Log.d(TAG, String.format("startVideoCapture: %b", on));
        if (!hasCameraPermission()) {
            mIsCameraOn = false;
            SolutionToast.show(R.string.camera_permission_disabled);
            mRTCVideo.stopVideoCapture();
            return;
        }
        mIsCameraOn = on;
        if (mRTCVideo != null) {
            if (on) {
                // 立即开启内部视频采集。默认为关闭状态。
                mRTCVideo.startVideoCapture();
            } else {
                // 立即关闭内部视频采集。默认为关闭状态。
                mRTCVideo.stopVideoCapture();
            }
        }
        if (mRTCRoom != null) {
            if (on) {
                // 在当前所在房间内发布本地通过摄像头/麦克风采集的媒体流
                mRTCRoom.publishStream(MediaStreamType.RTC_MEDIA_STREAM_TYPE_VIDEO);
            } else {
                // 停止将本地摄像头/麦克风采集的媒体流发布到当前所在房间中
                mRTCRoom.unpublishStream(MediaStreamType.RTC_MEDIA_STREAM_TYPE_VIDEO);
            }
        }

        SolutionDemoEventManager.post(new MediaStatusEvent(
                SolutionDataManager.ins().getUserId(),
                Constants.MEDIA_TYPE_VIDEO,
                mIsCameraOn ? Constants.MEDIA_STATUS_ON : Constants.MEDIA_STATUS_OFF));
    }

    /**
     * 开始发布音频流
     *
     * @param on 是否发布
     */
    public void startPublishAudio(boolean on) {
        Log.d(TAG, String.format("startPublishAudio: %b", on));
        mIsAudioMute = !on;
        if (!hasAudioPermission()) {
            mIsMicOn = false;
            SolutionToast.show(R.string.microphone_permission_disabled);
            mRTCVideo.stopAudioCapture();
            return;
        }
        if (mRTCVideo != null) {
            if (on) {
                // 开启内部音频采集。默认为关闭状态。
                mRTCVideo.startAudioCapture();
            }
        }
        if (mRTCRoom != null) {
            if (on) {
                // 在当前所在房间内发布本地通过摄像头/麦克风采集的媒体流
                mRTCRoom.publishStream(MediaStreamType.RTC_MEDIA_STREAM_TYPE_AUDIO);
            } else {
                // 停止将本地摄像头/麦克风采集的媒体流发布到当前所在房间中
                mRTCRoom.unpublishStream(MediaStreamType.RTC_MEDIA_STREAM_TYPE_AUDIO);
            }
        }

        SolutionDemoEventManager.post(new MediaStatusEvent(
                SolutionDataManager.ins().getUserId(),
                Constants.MEDIA_TYPE_AUDIO,
                mIsMicOn && !mIsAudioMute ? Constants.MEDIA_STATUS_ON : Constants.MEDIA_STATUS_OFF));
    }

    /**
     * 当前的音频路由是否是耳机
     * @param audioRoute 音频路由
     * @return true表示任意类型的耳机
     */
    public boolean isHeadSet(AudioRoute audioRoute) {
        return audioRoute == AudioRoute.AUDIO_ROUTE_HEADSET
                || audioRoute == AudioRoute.AUDIO_ROUTE_HEADSET_USB
                || audioRoute == AudioRoute.AUDIO_ROUTE_HEADSET_BLUETOOTH;
    }

    /**
     * 是否使用扬声器
     *
     * @param use 使用扬声器
     */
    public void useSpeakerphone(boolean use) {
        Log.d(TAG, String.format("useSpeakerphone: %b", use));
        if (isHeadSet(mCurrentAudioRoute)) {
            return;
        }
        mIsSpeakerphone = use;
        if (mRTCVideo != null) {
            // 设置当前音频播放路由。默认使用 setDefaultAudioRoute 中设置的音频路由。
            mRTCVideo.setAudioRoute(use ? AudioRoute.AUDIO_ROUTE_SPEAKERPHONE
                    : AudioRoute.AUDIO_ROUTE_EARPIECE);
        }

        SolutionDemoEventManager.post(new AudioRouterEvent(use));
    }

    /**
     * 设置视频分辨率
     * @param str 视频分辨率字符串。该字符串为枚举值
     */
    public void setVideoResolution(String str) {
        if (!Constants.RESOLUTION_MAP.containsKey(str)) {
            str = Constants.DEFAULT_RESOLUTION;
        }
        mResolution = str;
        Pair<Integer, Integer> resolution = Constants.RESOLUTION_MAP.get(str);
        if (resolution != null) {
            setVideoResolution(resolution.first, resolution.second);
        }
    }

    /**
     * 设置音频质量
     * @param audioProfile 音频配置
     */
    public void setAudioProfile(String audioProfile) {
        if (!Constants.QUALITY_MAP.containsKey(audioProfile)) {
            audioProfile = Constants.DEFAULT_QUALITY;
        }
        mAudioQuality = audioProfile;
        AudioProfileType audioProfileType = Constants.QUALITY_MAP.get(audioProfile);
        if (audioProfileType != null) {
            setAudioProfile(audioProfileType);
        }
    }

    /**
     * 设置视频分辨率
     * @param width 宽度
     * @param height 高度
     */
    public void setVideoResolution(int width, int height) {
        Log.d(TAG, String.format("setResolution: %d  %d", width, height));
        if (mRTCVideo != null) {
            VideoEncoderConfig videoEncoderConfig = new VideoEncoderConfig();
            videoEncoderConfig.width = width;
            videoEncoderConfig.height = height;
            // 码流大小自适应
            videoEncoderConfig.maxBitrate = -1;
            videoEncoderConfig.frameRate = 15;
            // 视频发布端设置期望发布的最大分辨率视频流参数，包括分辨率、帧率、码率、缩放模式、网络不佳时的回退策略等。
            mRTCVideo.setVideoEncoderConfig(videoEncoderConfig);
        }
    }

    /**
     * 设置音频质量
     * @param audioProfileType 音频质量
     */
    public void setAudioProfile(AudioProfileType audioProfileType) {
        Log.d(TAG, String.format("setAudioProfile: %s", audioProfileType));
        if (mRTCVideo != null) {
            // 设置音质档位。当所选的 ChannelProfile 中的音频参数无法满足你的场景需求时，调用本接口切换的音质档位。
            mRTCVideo.setAudioProfile(audioProfileType);
        }
    }

    /**
     * 设置镜像效果
     * @param mirror 是否开启镜像
     */
    public void setMirrorType(boolean mirror) {
        Log.d(TAG, String.format("setMirrorType: %b", mirror));
        mVideoMirror = mirror;
        if (mRTCVideo != null) {
            // 为采集到的视频流开启镜像，只有前置且开启镜像才设置镜像，否则不镜像
            mRTCVideo.setLocalVideoMirrorType(mirror && mIsFrontCamera
                    ? MirrorType.MIRROR_TYPE_RENDER_AND_ENCODER
                    : MirrorType.MIRROR_TYPE_NONE);
        }
    }

    /**
     * 翻转摄像头
     * @param isFrontCamera 是否是前置摄像头
     */
    public void switchCamera(boolean isFrontCamera) {
        Log.d(TAG, String.format("switchCamera: %b", isFrontCamera));
        if (!mIsCameraOn) {
            return;
        }
        mIsFrontCamera = isFrontCamera;
        if (mRTCVideo != null) {
            // 切换视频内部采集时使用的前置/后置摄像头
            mRTCVideo.switchCamera(isFrontCamera
                    ? CameraId.CAMERA_ID_FRONT
                    : CameraId.CAMERA_ID_BACK);
        }
        // 摄像头变化后需要重新设置一下镜像效果
        setMirrorType(mVideoMirror);
    }

    /**
     * 设置本地用户视频渲染控件
     * @param isScreen 是不是屏幕流
     * @param textureView 渲染控件
     */
    public void setLocalVideoCanvas(boolean isScreen, TextureView textureView) {
        Log.d(TAG, String.format("setLocalVideoCanvas: %b", isScreen));
        if (mRTCVideo != null) {
            StreamIndex index = isScreen ? StreamIndex.STREAM_INDEX_SCREEN : StreamIndex.STREAM_INDEX_MAIN;
            VideoCanvas canvas = new VideoCanvas();
            canvas.renderView = textureView;
            canvas.renderMode = RENDER_MODE_HIDDEN;
            // 设置本地视频渲染时使用的视图，并设置渲染模式。
            mRTCVideo.setLocalVideoCanvas(index, canvas);
        }
    }

    /**
     * 设置远端用户视频渲染控件
     * @param uid 用户id
     * @param isScreen 是不是屏幕流
     * @param textureView 渲染控件
     */
    public void setRemoteVideCanvas(String uid, boolean isScreen, TextureView textureView) {
        Log.d(TAG, String.format("setRemoteVideCanvas: %s  %b", uid, isScreen));
        if (mRTCVideo != null) {
            StreamIndex index = isScreen ? StreamIndex.STREAM_INDEX_SCREEN : StreamIndex.STREAM_INDEX_MAIN;
            VideoCanvas canvas = new VideoCanvas(textureView, isScreen ? RENDER_MODE_FIT : RENDER_MODE_HIDDEN);
            RemoteStreamKey remoteStreamKey = new RemoteStreamKey(mRoomId, uid, index);
            canvas.renderView = textureView;
            // 渲染来自指定远端用户的视频流时，设置使用的视图和渲染模式。
            mRTCVideo.setRemoteVideoCanvas(remoteStreamKey, canvas);
        }
    }

    public boolean isCameraOn() {
        return mIsCameraOn;
    }

    public boolean isMicOn() {
        return mIsMicOn && !mIsAudioMute;
    }

    public boolean isSpeakerphone() {
        return mIsSpeakerphone;
    }

    public boolean isVideoMirror() {
        return mVideoMirror;
    }

    public boolean isFrontCamera() {
        return mIsFrontCamera;
    }

    public String getResolution() {
        return mResolution;
    }

    public String getAudioQuality() {
        return mAudioQuality;
    }

    /**
     * 获取用户摄像头流渲染的控件
     *
     * @param uid 用户id
     * @return 渲染控件
     */
    public TextureView getRenderView(String uid) {
        if (TextUtils.isEmpty(uid)) {
            return null;
        }
        TextureView textureView = mUserIdViewMap.get(uid);
        if (textureView != null) {
            return textureView;
        }
        textureView = new TextureView(AppUtil.getApplicationContext());
        mUserIdViewMap.put(uid, textureView);
        return textureView;
    }

    /**
     * 获取用户屏幕视频流渲染的控件
     *
     * @param uid 用户id
     * @return 渲染控件
     */
    public TextureView getScreenRenderView(String uid) {
        if (TextUtils.isEmpty(uid)) {
            return null;
        }
        TextureView textureView = mScreenUserIdViewMap.get(uid);
        if (textureView != null) {
            return textureView;
        }
        textureView = new TextureView(AppUtil.getApplicationContext());
        mScreenUserIdViewMap.put(uid, textureView);
        return textureView;
    }

    /**
     * 清除所有的用户渲染控件
     */
    public void clearUserView() {
        mUserIdViewMap.clear();
        mScreenUserIdViewMap.clear();
    }
}
