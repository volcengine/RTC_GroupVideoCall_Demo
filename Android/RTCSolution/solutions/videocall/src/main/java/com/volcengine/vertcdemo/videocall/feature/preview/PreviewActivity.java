// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.feature.preview;

import static com.volcengine.vertcdemo.core.net.rts.RTSInfo.KEY_RTS;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextUtils;
import android.util.Log;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.annotation.Keep;

import com.ss.bytertc.engine.RTCVideo;
import com.vertcdemo.joinrtsparams.bean.JoinRTSRequest;
import com.vertcdemo.joinrtsparams.common.JoinRTSManager;
import com.volcengine.vertcdemo.common.IAction;
import com.volcengine.vertcdemo.common.LengthFilterWithCallback;
import com.volcengine.vertcdemo.common.SolutionBaseActivity;
import com.volcengine.vertcdemo.common.SolutionToast;
import com.volcengine.vertcdemo.common.TextWatcherAdapter;
import com.volcengine.vertcdemo.core.SolutionDataManager;
import com.volcengine.vertcdemo.core.eventbus.SolutionDemoEventManager;
import com.volcengine.vertcdemo.core.eventbus.AppTokenExpiredEvent;
import com.volcengine.vertcdemo.core.net.ErrorTool;
import com.volcengine.vertcdemo.core.net.IRequestCallback;
import com.volcengine.vertcdemo.core.net.ServerResponse;
import com.volcengine.vertcdemo.core.net.rts.RTSBaseClient;
import com.volcengine.vertcdemo.core.net.rts.RTSInfo;
import com.volcengine.vertcdemo.utils.AppUtil;
import com.volcengine.vertcdemo.utils.Utils;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.bean.JoinRoomResponse;
import com.volcengine.vertcdemo.videocall.core.Constants;
import com.volcengine.vertcdemo.videocall.core.VideoCallRTCManager;
import com.volcengine.vertcdemo.videocall.databinding.ActivityPreviewBinding;
import com.volcengine.vertcdemo.videocall.event.AudioRouterEvent;
import com.volcengine.vertcdemo.videocall.event.MediaStatusEvent;
import com.volcengine.vertcdemo.videocall.event.RefreshPreviewEvent;
import com.volcengine.vertcdemo.videocall.feature.main.RoomMainActivity;

import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.regex.Pattern;

/**
 * 预览页面
 *
 * 功能：
 * 1.控制摄像头、麦克风、扬声器开关状态
 * 2.预览自己视频
 * 3.进入房间
 */
public class PreviewActivity extends SolutionBaseActivity {

    private static final String TAG = "PreviewActivity";

    public static final String ROOM_INPUT_REGEX = "^[0-9]+$";

    // 该场景需要用到的 rts 相关数据
    private RTSInfo mRTSInfo;

    // 房间id
    private String mRoomId;

    private ActivityPreviewBinding mViewBinding;

    // 房间id长度是否过长
    private boolean mRoomIdOverflow = false;

    // 房间id长度提示自动消失延时任务
    private final Runnable mRoomIdDismissRunnable = () -> mViewBinding.previewInputError.setVisibility(View.GONE);

    private final TextWatcherAdapter mTextWatcher = new TextWatcherAdapter() {
        @Override
        public void afterTextChanged(Editable s) {
            updateJoinButtonStatus();
        }
    };

    private final IRequestCallback<JoinRoomResponse> mJoinCallback = new IRequestCallback<JoinRoomResponse>() {
        @Override
        public void onSuccess(JoinRoomResponse data) {
            Intent intent = new Intent(PreviewActivity.this, RoomMainActivity.class);
            intent.putExtra(Constants.EXTRA_ROOM_ID, mRoomId);
            intent.putExtra(Constants.EXTRA_TOKEN, data.rtcToken);
            intent.putExtra(Constants.EXTRA_LAST_TS, data.durationS * 1000);
            startActivity(intent);
        }

        @Override
        public void onError(int errorCode, String message) {
            SolutionToast.show(ErrorTool.getErrorMessageByErrorCode(errorCode, message));
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        mViewBinding = ActivityPreviewBinding.inflate(getLayoutInflater());
        setContentView(mViewBinding.getRoot());
        
        initRTSInfo();

        SolutionToast.show(R.string.minutes_meeting_title);

        mViewBinding.previewBackBtn.setOnClickListener((v) -> finish());

        mViewBinding.previewInputEt.removeTextChangedListener(mTextWatcher);
        mViewBinding.previewInputEt.addTextChangedListener(mTextWatcher);

        InputFilter roomIDFilter = new LengthFilterWithCallback(18, (overflow) -> {
            if (overflow) {
                mViewBinding.previewInputError.setVisibility(View.VISIBLE);
                mViewBinding.previewInputError.setText(R.string.room_number_error_content_limit);
                mViewBinding.previewInputError.removeCallbacks(mRoomIdDismissRunnable);
                mViewBinding.previewInputError.postDelayed(mRoomIdDismissRunnable, 2500);
            } else {
                mViewBinding.previewInputError.setVisibility(View.GONE);
                mViewBinding.previewInputError.removeCallbacks(mRoomIdDismissRunnable);
            }
            mRoomIdOverflow = overflow;
        });
        InputFilter[] meetingIDFilters = new InputFilter[]{roomIDFilter};
        mViewBinding.previewInputEt.setFilters(meetingIDFilters);
        
        mViewBinding.previewEnterBtn.setOnClickListener((v) -> enterRoom());
        mViewBinding.previewEnterBtn.setEnabled(false);

        String appVersion = getString(R.string.app_version_vxxx, AppUtil.getAppVersionName());
        String SDKVersion = getString(R.string.sdk_version_vxxx, RTCVideo.getSDKVersion());
        String version = String.format("%s / %s", appVersion, SDKVersion);
        mViewBinding.previewVersion.setText(version);

        initRTC();

        SolutionDemoEventManager.register(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        setLocalVideoCanvas();
        
        if (VideoCallRTCManager.ins().isCameraOn()) {
            VideoCallRTCManager.ins().startVideoCapture(true);
        }
        if (VideoCallRTCManager.ins().isMicOn()) {
            VideoCallRTCManager.ins().startPublishAudio(true);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        SolutionDemoEventManager.unregister(this);
        VideoCallRTCManager.ins().clearUserView();
        VideoCallRTCManager.ins().destroyEngine();
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

    /**
     * 从Intent中获取RTS信息
     */
    private void initRTSInfo() {
        Intent intent = getIntent();
        if (intent == null) {
            return;
        }
        mRTSInfo = intent.getParcelableExtra(RTSInfo.KEY_RTS);
        if (mRTSInfo == null || !mRTSInfo.isValid()) {
            finish();
        }
    }

    /**
     * 根据从服务端获取的RTS信息初始化RTC
     */
    private void initRTC() {
        VideoCallRTCManager.ins().initEngine(mRTSInfo);
        RTSBaseClient rtsClient = VideoCallRTCManager.ins().getRTSClient();
        if (rtsClient == null) {
            finish();
            return;
        }
        rtsClient.login(mRTSInfo.rtsToken, (resultCode, message) -> {
            if (resultCode == RTSBaseClient.LoginCallBack.SUCCESS) {
                initView();
            } else {
                SolutionToast.show("Login Rtm Fail Error:" + resultCode + ",Message:" + message);
            }
        });
    }

    private void initView() {
        mViewBinding.previewCameraBtn.setImageResource(VideoCallRTCManager.ins().isCameraOn()
                ? R.drawable.camera_enable_icon : R.drawable.camera_disable_icon);
        mViewBinding.previewMicBtn.setImageResource(VideoCallRTCManager.ins().isMicOn()
                ? R.drawable.microphone_enable_icon : R.drawable.microphone_disable_icon);
        mViewBinding.previewSpeakerPhoneBtn.setImageResource(VideoCallRTCManager.ins().isSpeakerphone()
                ? R.drawable.speakerphone_icon : R.drawable.earpiece_icon);

        mViewBinding.previewCameraBtn.setOnClickListener((v) -> VideoCallRTCManager.ins()
                .startVideoCapture(!VideoCallRTCManager.ins().isCameraOn()));
        mViewBinding.previewMicBtn.setOnClickListener((v) -> VideoCallRTCManager.ins()
                .startPublishAudio(!VideoCallRTCManager.ins().isMicOn()));
        mViewBinding.previewSpeakerPhoneBtn.setOnClickListener((v) -> VideoCallRTCManager.ins()
                .useSpeakerphone(!VideoCallRTCManager.ins().isSpeakerphone()));

        setLocalVideoCanvas();
    }

    /**
     * 根据用户输入的房间名，控制加入房间按钮的状态
     */
    private void updateJoinButtonStatus() {
        int roomIDLength = mViewBinding.previewInputEt.getText().length();
        boolean meetingIdRegexMatch = false;
        if (Pattern.matches(ROOM_INPUT_REGEX, mViewBinding.previewInputEt.getText().toString())) {
            if (mRoomIdOverflow) {
                mViewBinding.previewInputError.setVisibility(View.VISIBLE);
                mViewBinding.previewInputError.setText(R.string.room_number_error_content_limit);
                mViewBinding.previewInputError.removeCallbacks(mRoomIdDismissRunnable);
                mViewBinding.previewInputError.postDelayed(mRoomIdDismissRunnable, 2500);
            } else {
                mViewBinding.previewInputError.setVisibility(View.INVISIBLE);
                meetingIdRegexMatch = true;
            }
        } else {
            if (roomIDLength > 0) {
                mViewBinding.previewInputError.setVisibility(View.VISIBLE);
                mViewBinding.previewInputError.setText(R.string.room_number_error_content_limit);
            } else {
                mViewBinding.previewInputError.setVisibility(View.INVISIBLE);
            }
        }
        boolean joinBtnEnable = roomIDLength > 0 && roomIDLength <= 18
                && meetingIdRegexMatch;
        mViewBinding.previewEnterBtn.setEnabled(joinBtnEnable);
    }

    /**
     * 加入房间页面
     */
    private void enterRoom() {
        String roomId = mViewBinding.previewInputEt.getText().toString().trim();
        if (TextUtils.isEmpty(roomId)) {
            return;
        }
        // todo 隔离临时方案
        roomId = "call_" + roomId;
        mRoomId = roomId;
        VideoCallRTCManager.ins().getRTSClient().requestJoinRoom(roomId, mJoinCallback);
    }

    /**
     * 设置本端视频渲染
     */
    private void setLocalVideoCanvas() {
        if (VideoCallRTCManager.ins().isCameraOn()) {
            TextureView textureView = VideoCallRTCManager.ins().getRenderView(SolutionDataManager.ins().getUserId());
            FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
            Utils.attachViewToViewGroup(mViewBinding.previewRenderContainer, textureView, params);
            VideoCallRTCManager.ins().setLocalVideoCanvas(false, textureView);
            mViewBinding.previewRenderDisable.setVisibility(View.GONE);
        } else {
            mViewBinding.previewRenderContainer.removeAllViews();
            mViewBinding.previewRenderDisable.setVisibility(View.VISIBLE);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onMediaStatusEvent(MediaStatusEvent event) {
        if (!TextUtils.equals(SolutionDataManager.ins().getUserId(), event.uid)) {
            return;
        }
        boolean on = event.status == Constants.MEDIA_STATUS_ON;
        if (event.mediaType == Constants.MEDIA_TYPE_AUDIO) {
            mViewBinding.previewMicBtn.setImageResource(on ? R.drawable.microphone_enable_icon
                    : R.drawable.microphone_disable_icon);
        } else if (event.mediaType == Constants.MEDIA_TYPE_VIDEO) {
            mViewBinding.previewCameraBtn.setImageResource(on ? R.drawable.camera_enable_icon :
                    R.drawable.camera_disable_icon);
            setLocalVideoCanvas();
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onAudioRouterEvent(AudioRouterEvent event) {
        mViewBinding.previewSpeakerPhoneBtn.setImageResource(event.isSpeakerPhone ? R.drawable.speakerphone_icon
                : R.drawable.earpiece_icon);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onRefreshPreviewEvent(RefreshPreviewEvent event) {
        // 所有媒体恢复到默认状态
        VideoCallRTCManager.ins().startVideoCapture(true);
        VideoCallRTCManager.ins().startPublishAudio(true);
        VideoCallRTCManager.ins().switchCamera(true);
        VideoCallRTCManager.ins().useSpeakerphone(true);
        VideoCallRTCManager.ins().setMirrorType(true);

        setLocalVideoCanvas();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onTokenExpiredEvent(AppTokenExpiredEvent event) {
        finish();
    }

    /**
     * 场景的入口方法
     *
     * 壳工程通过反射调用该方法，成功后则打开本页面，否则弹出提示
     *
     * @param activity   调用者页面
     * @param doneAction 完成joinRTM后回调
     */
    @Keep
    @SuppressWarnings("unused")
    public static void prepareSolutionParams(Activity activity, IAction<Object> doneAction) {
        Log.d(TAG, "prepareSolutionParams() invoked");
        IRequestCallback<ServerResponse<RTSInfo>> callback = new IRequestCallback<ServerResponse<RTSInfo>>() {
            @Override
            public void onSuccess(ServerResponse<RTSInfo> response) {
                RTSInfo data = response == null ? null : response.getData();
                if (data == null || !data.isValid()) {
                    onError(-1, "");
                    return;
                }
                Intent intent = new Intent(Intent.ACTION_MAIN);
                intent.setClass(AppUtil.getApplicationContext(), PreviewActivity.class);
                intent.putExtra(KEY_RTS, data);
                activity.startActivity(intent);
                if (doneAction != null) {
                    doneAction.act(null);
                }
            }

            @Override
            public void onError(int errorCode, String message) {
                if (doneAction != null) {
                    doneAction.act(null);
                }
            }
        };
        JoinRTSRequest request = new JoinRTSRequest(Constants.SOLUTION_NAME_ABBR, SolutionDataManager.ins().getToken());
        JoinRTSManager.setAppInfoAndJoinRTM(request, callback);
    }
}