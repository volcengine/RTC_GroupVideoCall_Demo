// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.feature.main;

import static android.view.View.GONE;
import static com.volcengine.vertcdemo.videocall.core.Constants.MEDIA_STATUS_ON;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.volcengine.vertcdemo.common.SolutionBaseActivity;
import com.volcengine.vertcdemo.common.SolutionToast;
import com.volcengine.vertcdemo.common.IAction;
import com.volcengine.vertcdemo.common.SolutionCommonDialog;
import com.volcengine.vertcdemo.core.SolutionDataManager;
import com.volcengine.vertcdemo.core.eventbus.SolutionDemoEventManager;
import com.volcengine.vertcdemo.core.eventbus.AppTokenExpiredEvent;
import com.volcengine.vertcdemo.core.net.ErrorTool;
import com.volcengine.vertcdemo.core.net.IRequestCallback;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.bean.LeaveRoomResponse;
import com.volcengine.vertcdemo.videocall.bean.ReconnectResponse;
import com.volcengine.vertcdemo.videocall.bean.VideoCallUserInfo;
import com.volcengine.vertcdemo.videocall.core.Constants;
import com.volcengine.vertcdemo.videocall.core.VideoCallDataManager;
import com.volcengine.vertcdemo.videocall.core.VideoCallRTCManager;
import com.volcengine.vertcdemo.videocall.databinding.ActivityRoomMainBinding;
import com.volcengine.vertcdemo.videocall.event.AudioPropertiesReportEvent;
import com.volcengine.vertcdemo.videocall.event.FullScreenFinishEvent;
import com.volcengine.vertcdemo.videocall.event.MediaStatusEvent;
import com.volcengine.vertcdemo.videocall.event.RefreshPreviewEvent;
import com.volcengine.vertcdemo.videocall.event.RefreshUserLayoutEvent;
import com.volcengine.vertcdemo.videocall.event.RoomFinishEvent;
import com.volcengine.vertcdemo.core.eventbus.SDKReconnectToRoomEvent;
import com.volcengine.vertcdemo.videocall.event.ScreenShareEvent;
import com.volcengine.vertcdemo.videocall.view.TitleView;

import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

/**
 * 房间主页页面
 *
 * 功能：
 * 1.展示房间信息
 * 2.展示房间内用户视频信息
 * 3.媒体控制功能
 */
public class RoomMainActivity extends SolutionBaseActivity {

    private static final String TAG = "RoomMainActivity";

    // 会议持续时间
    private long mLastTs;
    // 房间id
    private String mRoomId;
    // 加入rtc房间token
    private String mToken;

    private ActivityRoomMainBinding mViewBinding;

    /**
     * 向业务服务器请求重连进当前房间接口的回调，用于服务端控制房间人数
     */
    private final IRequestCallback<ReconnectResponse> mReconnectCallback
            = new IRequestCallback<ReconnectResponse>() {
        @Override
        public void onSuccess(ReconnectResponse data) {

        }

        @Override
        public void onError(int errorCode, String message) {
            SolutionToast.show(ErrorTool.getErrorMessageByErrorCode(errorCode, message));
            finish();
        }
    };

    /**
     * 向业务服务器请求离开当前房间接口的回调，用于服务端控制房间人数
     */
    private final IRequestCallback<LeaveRoomResponse> mLeaveCallback
            = new IRequestCallback<LeaveRoomResponse>() {
        @Override
        public void onSuccess(LeaveRoomResponse data) {

        }

        @Override
        public void onError(int errorCode, String message) {

        }
    };

    /**
     * 房间上部功能区域点击事件
     */
    private final TitleView.ITitleCallback mITitleCallback = new TitleView.ITitleCallback() {
        // 缩小事件，暂时没用
        @Override
        public void onZoomClick() {

        }

        // 点击挂断事件，展示确认退出房间对话框
        @Override
        public void onHangUpClick() {
            attemptLeaveRoom();
        }
    };

    // 用户列表中用户控件点击事件
    private final IAction<VideoCallUserInfo> mUserViewClick = userInfo -> {
        if (userInfo.isScreenShare) {
            mViewBinding.portraitScreenLayout.setVisibility(View.VISIBLE);
            mViewBinding.portraitScreenLayout.bind(userInfo);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mViewBinding = ActivityRoomMainBinding.inflate(getLayoutInflater());
        setContentView(mViewBinding.getRoot());

        Intent intent = getIntent();
        mLastTs = intent.getLongExtra(Constants.EXTRA_LAST_TS, 0);
        mRoomId = intent.getStringExtra(Constants.EXTRA_ROOM_ID);
        mToken = intent.getStringExtra(Constants.EXTRA_TOKEN);
        
        mViewBinding.userListPagerLayout.setOnUserViewClick(mUserViewClick);
        
        mViewBinding.portraitScreenLayout.setExpandAction((userInfo) ->
                startActivity(new Intent(this, FullScreenActivity.class)));
        mViewBinding.portraitScreenLayout.setOnClickListener((v) -> {
            mViewBinding.portraitScreenLayout.bind(null);
            mViewBinding.portraitScreenLayout.setVisibility(GONE);

            VideoCallUserInfo userInfo = VideoCallDataManager.ins().getScreenShareUser();
            if (userInfo != null) {
                mViewBinding.userListPagerLayout.updateUserVideoStatus(userInfo.userId, true, true);
            }
        });
        mViewBinding.portraitScreenLayout.setVisibility(GONE);

        mViewBinding.titleView.setITitleCallback(mITitleCallback);
        mViewBinding.titleView.setRoomId(mRoomId);
        mViewBinding.titleView.startCountDown(mLastTs);

        SolutionDemoEventManager.register(this);

        // 加入RTC房间
        VideoCallRTCManager.ins().joinRoom(mToken, mRoomId, SolutionDataManager.ins().getUserId(),
                SolutionDataManager.ins().getUserName());
    }

    /**
     * 弹出确认离开房间对话框
     */
    private void attemptLeaveRoom() {
        SolutionCommonDialog dialog = new SolutionCommonDialog(this);
        dialog.setMessage(getString(R.string.exit_room));
        dialog.setNegativeListener((v) -> dialog.dismiss());
        dialog.setPositiveListener((v) -> {
            dialog.dismiss();
            finish();
        });
        dialog.show();
    }

    @Override
    protected boolean onMicrophonePermissionClose() {
        Log.d(TAG, "onMicrophonePermissionClose");
        finish();
        return true;
    }

    @Override
    protected boolean onCameraPermissionClose() {
        Log.d(TAG, "onCameraPermissionClose");
        finish();
        return true;
    }

    @Override
    public void onBackPressed() {
        attemptLeaveRoom();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        SolutionDemoEventManager.unregister(this);

        VideoCallDataManager.ins().removeAllUser();
        VideoCallRTCManager.ins().getRTSClient().requestLeaveRoom(mRoomId, mLeaveCallback);
        VideoCallRTCManager.ins().leaveRoom();
        SolutionDemoEventManager.post(new RefreshPreviewEvent());
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onRefreshUserLayoutEvent(RefreshUserLayoutEvent event) {
        // 更新用户列表控件
        mViewBinding.userListPagerLayout.setUserList(event.userInfoList);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onAudioPropertiesReportEvent(AudioPropertiesReportEvent event) {
        // 更新用户说话状态UI
        mViewBinding.userListPagerLayout.updateUserSpeakingStatus(event.uid, event.isSpeaking);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onMediaStatusEvent(MediaStatusEvent event) {
        // 更新用户媒体状态UI
        if (event.mediaType == Constants.MEDIA_TYPE_VIDEO) {
            mViewBinding.userListPagerLayout.updateUserVideoStatus(event.uid, false, event.status == MEDIA_STATUS_ON);
        } else if (event.mediaType == Constants.MEDIA_TYPE_AUDIO) {
            mViewBinding.userListPagerLayout.updateUserAudioStatus(event.uid, event.status == MEDIA_STATUS_ON);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onSDKReconnectToRoomEvent(SDKReconnectToRoomEvent event) {
        VideoCallRTCManager.ins().getRTSClient().requestReconnect(mRoomId, mReconnectCallback);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onRoomFinishEvent(RoomFinishEvent event) {
        // 房间时长限制，到最大时间后，服务端下发通知，客户端退房
        SolutionToast.show(R.string.minutes_meeting);
        finish();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onScreenShareEvent(ScreenShareEvent event) {
        if (!event.isStart) {
            // 屏幕分享结束时，尝试绑定新的用户数据
            VideoCallUserInfo userInfo = VideoCallDataManager.ins().getScreenShareUser();
            mViewBinding.portraitScreenLayout.bind(userInfo);
            mViewBinding.portraitScreenLayout.setVisibility(userInfo == null ? GONE : View.VISIBLE);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onFullScreenFinishEvent(FullScreenFinishEvent event) {
        // 横屏页面切回竖屏页面后，需要重新绑定一次竖屏的屏幕分享页面
        // After the horizontal screen page is switched back to the vertical screen page, it is necessary to re-bind the screen sharing page of the vertical screen
        VideoCallUserInfo userInfo = VideoCallDataManager.ins().getScreenShareUser();
        mViewBinding.portraitScreenLayout.bind(userInfo);
        mViewBinding.portraitScreenLayout.setVisibility(userInfo == null ? GONE : View.VISIBLE);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onTokenExpiredEvent(AppTokenExpiredEvent event) {
        finish();
    }
}