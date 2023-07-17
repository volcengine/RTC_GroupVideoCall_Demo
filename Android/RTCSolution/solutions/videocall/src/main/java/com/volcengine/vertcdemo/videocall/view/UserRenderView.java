// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.view;

import android.content.Context;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.volcengine.vertcdemo.utils.Utils;
import com.volcengine.vertcdemo.core.SolutionDataManager;
import com.volcengine.vertcdemo.utils.Utils;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.bean.VideoCallUserInfo;
import com.volcengine.vertcdemo.videocall.core.VideoCallRTCManager;
import com.volcengine.vertcdemo.videocall.databinding.LayoutUserRenderViewBinding;

/**
 * 用户列表中单个用户控件
 *
 * 功能：
 * 1.绑定用户信息 {@link #bindInfo(VideoCallUserInfo)}
 * 2.更新用户视频状态 {@link #updateVideoStatus(String, boolean, boolean)}
 * 3.更新用户音频状态 {@link #updateAudioStatus(String, boolean)}
 * 4.更新用户说话音量状态 {@link #updateSpeakingStatus(String, boolean)}
 * 5.改变内部元素尺寸 {@link #shrinkPrefixUISize(boolean)}
 */
public class UserRenderView extends FrameLayout {

    private LayoutUserRenderViewBinding mViewBinding;

    // 当前用户信息
    private VideoCallUserInfo mUserInfo;

    public UserRenderView(@NonNull Context context) {
        super(context);
        initView();
    }

    public UserRenderView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public UserRenderView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    private void initView() {
        View view = View.inflate(getContext(), R.layout.layout_user_render_view, this);
        mViewBinding = LayoutUserRenderViewBinding.bind(view);
    }

    /**
     * 缩小用户名首字母显示和音量显示控件尺寸
     *
     * @param shrink 是否缩小
     */
    public void shrinkPrefixUISize(boolean shrink) {
        int prefixSize;
        int speakStatusSize;
        if (shrink) {
            prefixSize = (int) Utils.dp2Px(42);
            speakStatusSize = (int) Utils.dp2Px(58);
        } else {
            prefixSize = (int) Utils.dp2Px(80);
            speakStatusSize = (int) Utils.dp2Px(112);
        }
        ViewGroup.LayoutParams prefixParams = mViewBinding.userRenderNamePrefix.getLayoutParams();
        prefixParams.width = prefixSize;
        prefixParams.height = prefixSize;
        mViewBinding.userRenderNamePrefix.setLayoutParams(prefixParams);
        ViewGroup.LayoutParams speakingParams = mViewBinding.userRenderNameSpeaking.getLayoutParams();
        speakingParams.width = speakStatusSize;
        speakingParams.height = speakStatusSize;
        mViewBinding.userRenderNameSpeaking.setLayoutParams(speakingParams);
    }

    /**
     * 绑定用户信息
     *
     * @param userInfo 用户信息
     */
    public void bindInfo(VideoCallUserInfo userInfo) {
        boolean isSelf = userInfo != null && TextUtils.equals(userInfo.userId,
                SolutionDataManager.ins().getUserId());
        mUserInfo = userInfo;

        mViewBinding.userRenderNameSpeaking.setVisibility(INVISIBLE);
        if (userInfo == null || TextUtils.isEmpty(userInfo.userId)) {
            mViewBinding.userRenderName.setText("");
            mViewBinding.userRenderContainer.removeAllViews();
            mViewBinding.userRenderNamePrefix.setVisibility(INVISIBLE);
            mViewBinding.userExtraInfoLayout.setVisibility(INVISIBLE);
            mViewBinding.userRenderNameSpeaking.setVisibility(INVISIBLE);
        } else {
            mViewBinding.userExtraInfoLayout.setVisibility(VISIBLE);
            String userName = isSelf
                    ? getContext().getString(R.string.xxx_me_, userInfo.userName)
                    : userInfo.userName;

            if (userInfo.isScreenShare) {
                userName = getResources().getString(R.string.xxxs_screen_sharing, userName);
            }
            mViewBinding.userRenderName.setText(userName);

            updateAudioStatus(userInfo.userId, userInfo.isMicOn);

            updateVideoStatus(userInfo.userId, userInfo.isScreenShare, userInfo.isCameraOn);
        }
    }

    /**
     * 根据视频开关状态更新UI
     *
     * @param uid      用户id
     * @param isScreen 是不是屏幕流
     * @param isOn     是否打开
     */
    public void updateVideoStatus(String uid, boolean isScreen, boolean isOn) {
        if (mUserInfo == null || TextUtils.isEmpty(uid)) {
            return;
        }
        if (!TextUtils.equals(uid, mUserInfo.userId)) {
            return;
        }
        if (isScreen != mUserInfo.isScreenShare) {
            return;
        }
        mUserInfo.isCameraOn = isOn;

        if (isOn || mUserInfo.isScreenShare) {
            mViewBinding.userRenderNamePrefix.setVisibility(INVISIBLE);
            mViewBinding.userRenderNameSpeaking.setVisibility(INVISIBLE);

            TextureView view;
            if (mUserInfo.isScreenShare) {
                view = VideoCallRTCManager.ins().getScreenRenderView(uid);
            } else {
                view = VideoCallRTCManager.ins().getRenderView(uid);
            }
            ViewGroup.LayoutParams params = new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT);
            Utils.attachViewToViewGroup(mViewBinding.userRenderContainer, view, params);

            if (TextUtils.equals(mUserInfo.userId, SolutionDataManager.ins().getUserId())) {
                VideoCallRTCManager.ins().setLocalVideoCanvas(mUserInfo.isScreenShare, view);
            } else {
                VideoCallRTCManager.ins().setRemoteVideCanvas(mUserInfo.userId, mUserInfo.isScreenShare, view);
            }
        } else {
            mViewBinding.userRenderContainer.removeAllViews();
            mViewBinding.userRenderNamePrefix.setVisibility(VISIBLE);
            mViewBinding.userRenderNamePrefix.setText(mUserInfo.getNamePrefix());
        }
    }

    /**
     * 根据音频开关状态更新UI
     *
     * @param uid  用户id
     * @param isOn 是否打开
     */
    public void updateAudioStatus(String uid, boolean isOn) {
        if (mUserInfo == null || TextUtils.isEmpty(uid)) {
            return;
        }
        if (!TextUtils.equals(uid, mUserInfo.userId)) {
            return;
        }
        mUserInfo.isMicOn = isOn;

        int resId;
        if (isOn) {
            resId = R.drawable.microphone_enable_icon;
        } else {
            resId = R.drawable.microphone_disable_icon;
        }
        mViewBinding.userRenderMic.setImageResource(resId);
    }

    /**
     * 根据是否正在说话更新UI
     *
     * @param uid        用户id
     * @param isSpeaking 是否正在说话
     */
    public void updateSpeakingStatus(String uid, boolean isSpeaking) {
        if (mUserInfo == null || TextUtils.isEmpty(uid)) {
            return;
        }
        if (!TextUtils.equals(uid, mUserInfo.userId)) {
            return;
        }
        if (!mUserInfo.isCameraOn) {
            mViewBinding.userRenderNameSpeaking.setVisibility(isSpeaking ? VISIBLE : INVISIBLE);
        }

        int resId;
        if (mUserInfo.isMicOn) {
            if (isSpeaking) {
                resId = R.drawable.microphone_active_icon;
            } else {
                resId = R.drawable.microphone_enable_icon;
            }
        } else {
            resId = R.drawable.microphone_disable_icon;
        }
        mViewBinding.userRenderMic.setImageResource(resId);
    }
}
