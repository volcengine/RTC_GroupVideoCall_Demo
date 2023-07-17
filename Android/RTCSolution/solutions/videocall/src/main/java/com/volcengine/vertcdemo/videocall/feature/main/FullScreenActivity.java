// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.feature.main;

import android.os.Bundle;
import android.text.TextUtils;

import com.volcengine.vertcdemo.common.SolutionBaseActivity;
import com.volcengine.vertcdemo.core.eventbus.SolutionDemoEventManager;
import com.volcengine.vertcdemo.core.eventbus.AppTokenExpiredEvent;
import com.volcengine.vertcdemo.videocall.bean.VideoCallUserInfo;
import com.volcengine.vertcdemo.videocall.core.VideoCallDataManager;
import com.volcengine.vertcdemo.videocall.databinding.ActivityFullScreenBinding;
import com.volcengine.vertcdemo.videocall.event.FullScreenFinishEvent;
import com.volcengine.vertcdemo.videocall.event.RoomFinishEvent;
import com.volcengine.vertcdemo.videocall.event.ScreenShareEvent;

import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

/**
 * 屏幕共享全屏横屏展示页面
 *
 * 该页面独立是为了简化UI变化的逻辑
 */
public class FullScreenActivity extends SolutionBaseActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ActivityFullScreenBinding viewBinding = ActivityFullScreenBinding.inflate(getLayoutInflater());
        setContentView(viewBinding.getRoot());

        VideoCallUserInfo userInfo = VideoCallDataManager.ins().getScreenShareUser();
        if (userInfo == null || TextUtils.isEmpty(userInfo.userId)) {
            finish();
        }
        viewBinding.fullScreenLayout.bind(userInfo);
        viewBinding.fullScreenLayout.setZoomAction((ui) -> finish());

        SolutionDemoEventManager.register(this);
    }

    @Override
    protected boolean onMicrophonePermissionClose() {
        finish();
        return true;
    }

    @Override
    protected boolean onCameraPermissionClose() {
        finish();
        return true;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // 本页面关闭时需要通知横屏屏幕共享已经结束
        SolutionDemoEventManager.post(new FullScreenFinishEvent());
        SolutionDemoEventManager.unregister(this);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onScreenShareEvent(ScreenShareEvent event) {
        // 屏幕共享结束时需要关闭本页面
        if (!event.isStart) {
            finish();
        }
    }


    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onRoomFinishEvent(RoomFinishEvent event) {
        // 房间关闭时需要关闭本页面
        finish();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onTokenExpiredEvent(AppTokenExpiredEvent event) {
        finish();
    }
}