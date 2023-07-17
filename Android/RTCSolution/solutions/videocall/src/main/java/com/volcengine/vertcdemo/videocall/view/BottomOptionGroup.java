// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.view;

import android.content.Context;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.widget.LinearLayout;

import androidx.annotation.Nullable;

import com.volcengine.vertcdemo.core.SolutionDataManager;
import com.volcengine.vertcdemo.core.eventbus.SolutionDemoEventManager;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.core.Constants;
import com.volcengine.vertcdemo.videocall.core.VideoCallRTCManager;
import com.volcengine.vertcdemo.videocall.databinding.LayoutBottomOptionGroupBinding;
import com.volcengine.vertcdemo.videocall.event.AudioRouterEvent;
import com.volcengine.vertcdemo.videocall.event.MediaStatusEvent;

import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

/**
 * 房间页面底部功能区域
 *
 * 包含有五个独立的功能按钮
 *
 * 包含功能：
 * 1.打开关闭麦克风（内部调用RTC manager接口，并根据事件改变UI显示）
 * 2.打开关闭摄像头（内部调用RTC manager接口，并根据事件改变UI显示）
 * 3.切换扬声器（内部调用RTC manager接口，并根据事件改变UI显示）
 * 4.打开实时数据统计对话框
 * 5.打开设置对话框
 */
public class BottomOptionGroup extends LinearLayout {

    private LayoutBottomOptionGroupBinding mViewBinding;

    public BottomOptionGroup(Context context) {
        super(context);
        initView();
    }

    public BottomOptionGroup(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public BottomOptionGroup(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    private void initView() {
        View view = View.inflate(getContext(), R.layout.layout_bottom_option_group, this);
        mViewBinding = LayoutBottomOptionGroupBinding.bind(view);

        mViewBinding.groupMicrophone.setText(R.string.microphone);
        mViewBinding.groupCamera.setText(R.string.camera);
        mViewBinding.groupMediaStats.setText(R.string.real_time_data);
        mViewBinding.groupSetting.setText(R.string.set_up);

        updateMicrophone(VideoCallRTCManager.ins().isMicOn());
        updateCamera(VideoCallRTCManager.ins().isCameraOn());
        updateSpeakerPhone(VideoCallRTCManager.ins().isSpeakerphone());
        mViewBinding.groupMediaStats.setIcon(R.drawable.media_stat_icon);
        mViewBinding.groupSetting.setIcon(R.drawable.setting_icon);

        mViewBinding.groupMicrophone.setOnClickListener((v) ->
                VideoCallRTCManager.ins().startPublishAudio(!VideoCallRTCManager.ins().isMicOn()));
        mViewBinding.groupCamera.setOnClickListener((v) ->
                VideoCallRTCManager.ins().startVideoCapture(!VideoCallRTCManager.ins().isCameraOn()));
        mViewBinding.groupSpeakerPhone.setOnClickListener((v) ->
                VideoCallRTCManager.ins().useSpeakerphone(!VideoCallRTCManager.ins().isSpeakerphone()));
        mViewBinding.groupMediaStats.setOnClickListener((v) -> {
            MediaStatsDialog mediaStatsDialog = new MediaStatsDialog(getContext());
            mediaStatsDialog.show();
        });
        mViewBinding.groupSetting.setOnClickListener((v) -> {
            SettingDialog settingDialog = new SettingDialog(getContext());
            settingDialog.show();
        });
    }

    /**
     * 根据麦克风状态更新UI
     *
     * @param isOn 是否打开
     */
    public void updateMicrophone(boolean isOn) {
        mViewBinding.groupMicrophone.setIcon(isOn ? R.drawable.microphone_enable_icon : R.drawable.microphone_disable_icon);
    }

    /**
     * 根据摄像头状态更新UI
     *
     * @param isOn 是否打开
     */
    public void updateCamera(boolean isOn) {
        mViewBinding.groupCamera.setIcon(isOn ? R.drawable.camera_enable_icon : R.drawable.camera_disable_icon);
    }

    /**
     * 根据音频路由状态更新UI
     *
     * @param isOn 是否使用扬声器
     */
    public void updateSpeakerPhone(boolean isOn) {
        mViewBinding.groupSpeakerPhone.setIcon(isOn ? R.drawable.speakerphone_icon
                : R.drawable.earpiece_icon);
        mViewBinding.groupSpeakerPhone.setText(isOn ? R.string.speaker : R.string.earpiece);
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        SolutionDemoEventManager.register(this);
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        SolutionDemoEventManager.unregister(this);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onMediaStatusEvent(MediaStatusEvent event) {
        if (!TextUtils.equals(SolutionDataManager.ins().getUserId(), event.uid)) {
            return;
        }
        boolean on = event.status == Constants.MEDIA_STATUS_ON;
        if (event.mediaType == Constants.MEDIA_TYPE_AUDIO) {
            mViewBinding.groupMicrophone.setIcon(on ? R.drawable.microphone_enable_icon
                    : R.drawable.microphone_disable_icon);
        } else if (event.mediaType == Constants.MEDIA_TYPE_VIDEO) {
            mViewBinding.groupCamera.setIcon(on ? R.drawable.camera_enable_icon :
                    R.drawable.camera_disable_icon);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onAudioRouterEvent(AudioRouterEvent event) {
        updateSpeakerPhone(event.isSpeakerPhone);
    }
}
