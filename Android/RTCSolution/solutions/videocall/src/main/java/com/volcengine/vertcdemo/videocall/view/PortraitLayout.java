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
import com.volcengine.vertcdemo.videocall.databinding.LayoutPortraitScreenBinding;

/**
 * 房间页面展示竖屏状态下屏幕共享控件
 *
 * 功能：
 * 1.绑定用户数据 {@link #bind(VideoCallUserInfo)}
 * 2.设置放大事件 {@link #setExpandAction(IAction)}
 */
public class PortraitLayout extends FrameLayout {

    private LayoutPortraitScreenBinding mViewBinding;

    // 当前展示的用户信息
    private VideoCallUserInfo mUserInfo;
    // 放大按钮的点击事件
    private IAction<VideoCallUserInfo> mExpandAction;

    public PortraitLayout(@NonNull Context context) {
        super(context);
        initView();
    }

    public PortraitLayout(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public PortraitLayout(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    private void initView() {
        View view = View.inflate(getContext(), R.layout.layout_portrait_screen, this);
        mViewBinding = LayoutPortraitScreenBinding.bind(view);

        mViewBinding.fullScreenUserRenderZoom.setOnClickListener((v) -> {
            if (mExpandAction != null) {
                mExpandAction.act(mUserInfo);
            }
        });
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
     * 设置放大事件回调
     *
     * @param expandAction 回调
     */
    public void setExpandAction(IAction<VideoCallUserInfo> expandAction) {
        mExpandAction = expandAction;
    }
}