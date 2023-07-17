// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.volcengine.vertcdemo.common.IAction;
import com.volcengine.vertcdemo.utils.Utils;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.bean.VideoCallUserInfo;
import com.volcengine.vertcdemo.videocall.core.VideoCallRTCManager;
import com.volcengine.vertcdemo.videocall.databinding.LayoutFullScreenBinding;

/**
 * 横屏屏幕共享页面控件
 *
 * 用于横屏页面数据展示
 *
 * 功能：
 * 1.绑定数据 {@link #bind(VideoCallUserInfo)}
 * 2.设置缩小回调 {@link #setZoomAction(IAction)}
 */
public class LandScapeScreenLayout extends FrameLayout {

    private LayoutFullScreenBinding mViewBinding;

    public VideoCallUserInfo mUserInfo;
    private IAction<VideoCallUserInfo> mZoomAction;

    public LandScapeScreenLayout(@NonNull Context context) {
        super(context);
        initView();
    }

    public LandScapeScreenLayout(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public LandScapeScreenLayout(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    private void initView() {
        View view = View.inflate(getContext(), R.layout.layout_full_screen, this);
        mViewBinding = LayoutFullScreenBinding.bind(view);
        mViewBinding.fullScreenUserRenderZoom.setOnClickListener((v -> {
            if (mZoomAction != null) {
                mZoomAction.act(mUserInfo);
            }
        }));
    }

    /**
     * 绑定用户数据
     *
     * @param userInfo 用户信息
     */
    public void bind(@Nullable VideoCallUserInfo userInfo) {
        mViewBinding.fullScreenUserRenderContainer.removeAllViews();
        mUserInfo = userInfo;
        if (userInfo == null) {
            mViewBinding.fullScreenUserName.setText("");
            mViewBinding.fullScreenUserMic.setImageResource(R.drawable.microphone_enable_icon);
            mViewBinding.fullScreenUserRenderContainer.removeAllViews();
            return;
        }
        mViewBinding.fullScreenUserName.setText(getResources().getString(R.string.xxxs_screen_sharing, userInfo.userName));
        ViewGroup.LayoutParams layoutParams = new LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        TextureView view = VideoCallRTCManager.ins().getScreenRenderView(userInfo.userId);
        Utils.attachViewToViewGroup(mViewBinding.fullScreenUserRenderContainer, view, layoutParams);
    }

    /**
     * 设置缩小事件回调
     *
     * @param zoomAction 回调
     */
    public void setZoomAction(@Nullable IAction<VideoCallUserInfo> zoomAction) {
        mZoomAction = zoomAction;
    }
}
