// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.view;

import android.content.Context;
import android.view.ViewGroup;
import android.view.WindowManager;

import androidx.appcompat.app.AppCompatDialog;

import com.volcengine.vertcdemo.common.WindowUtils;
import com.volcengine.vertcdemo.videocall.R;
import com.volcengine.vertcdemo.videocall.core.Constants;
import com.volcengine.vertcdemo.videocall.core.VideoCallRTCManager;
import com.volcengine.vertcdemo.videocall.databinding.DialogSettingBinding;

import java.util.ArrayList;

/**
 * 房间内设置对话框
 *
 * 功能：
 * 1.设置视频分辨率
 * 2.设置通话质量
 * 3.设置本地镜像
 */
public class SettingDialog extends AppCompatDialog {

    private final DialogSettingBinding mViewBinding;

    public SettingDialog(Context context) {
        super(context, R.style.SolutionCommonDialog);
        setCancelable(true);

        mViewBinding = DialogSettingBinding.inflate(getLayoutInflater());

        mViewBinding.getRoot().setOnClickListener((v) -> {
        });
        mViewBinding.settingBack.setOnClickListener((v) -> dismiss());

        mViewBinding.settingResolution.setData(getContext().getString(R.string.resolution),
                VideoCallRTCManager.ins().getResolution());
        mViewBinding.settingQuality.setData(getContext().getString(R.string.call_quality),
                VideoCallRTCManager.ins().getAudioQuality());
        mViewBinding.settingMirror.setData(getContext().getString(R.string.local_mirror),
                VideoCallRTCManager.ins().isVideoMirror());

        mViewBinding.settingResolution.setOnClickListener((v) -> {
            SubSettingDialog subSettingDialog = new SubSettingDialog(getContext());
            subSettingDialog.setData(getContext().getString(R.string.resolution),
                    new ArrayList<>(Constants.RESOLUTION_MAP.keySet()),
                    VideoCallRTCManager.ins().getResolution(),
                    (position, str) -> {
                        VideoCallRTCManager.ins().setVideoResolution(str);
                        mViewBinding.settingResolution.setData(getContext().getString(R.string.resolution),
                                VideoCallRTCManager.ins().getResolution());
                    });
            subSettingDialog.show();
        });
        mViewBinding.settingQuality.setOnClickListener((v) -> {
            SubSettingDialog subSettingDialog = new SubSettingDialog(getContext());
            subSettingDialog.setData(getContext().getString(R.string.call_quality),
                    new ArrayList<>(Constants.QUALITY_MAP.keySet()),
                    VideoCallRTCManager.ins().getAudioQuality(),
                    (position, str) -> {
                        VideoCallRTCManager.ins().setAudioProfile(str);
                        mViewBinding.settingQuality.setData(getContext().getString(R.string.call_quality),
                                VideoCallRTCManager.ins().getAudioQuality());
                    });
            subSettingDialog.show();
        });
        mViewBinding.settingMirror.setOnCheckListener((v, isChecked) -> {
            VideoCallRTCManager.ins().setMirrorType(isChecked);
            mViewBinding.settingMirror.setData(getContext().getString(R.string.local_mirror),
                    VideoCallRTCManager.ins().isVideoMirror());
        });
    }

    @Override
    public void show() {
        super.show();
        WindowManager.LayoutParams params = getWindow().getAttributes();
        params.width = WindowUtils.getScreenWidth(getContext());
        params.height = ViewGroup.LayoutParams.MATCH_PARENT;
        getWindow().setAttributes(params);
        getWindow().setContentView(mViewBinding.getRoot(), new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
    }
}
