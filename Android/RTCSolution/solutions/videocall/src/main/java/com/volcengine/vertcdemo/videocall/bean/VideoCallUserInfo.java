// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.bean;

import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.google.gson.annotations.SerializedName;

import java.util.Objects;

/**
 * 用户数据模型
 */
public class VideoCallUserInfo {

    // 用户昵称
    @SerializedName("user_name")
    public String userName;
    // 用户id
    @SerializedName("user_id")
    public String userId;

    // 是否有音频数据
    public transient boolean isMicOn;
    // 是否有视频数据
    public transient boolean isCameraOn;
    // 是否是屏幕共享
    public transient boolean isScreenShare;

    public VideoCallUserInfo() {

    }

    public VideoCallUserInfo(String uid) {
        this.userId = uid;
        this.userName = uid;
    }

    // 获取用户名首位字符
    public @NonNull
    String getNamePrefix() {
        if (userName == null) {
            return "";
        }
        String str = userName.trim();
        if (TextUtils.isEmpty(str)) {
            return "";
        }
        return str.substring(0, 1);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }
        VideoCallUserInfo userInfo = (VideoCallUserInfo) o;
        return Objects.equals(userId, userInfo.userId) && (isScreenShare == userInfo.isScreenShare);
    }

    @Override
    public int hashCode() {
        return Objects.hash(userId, isScreenShare);
    }

    @Override
    public String toString() {
        return "VideoCallUserInfo{" +
                "userName='" + userName + '\'' +
                ", userId='" + userId + '\'' +
                ", isMicOn=" + isMicOn +
                ", isCameraOn=" + isCameraOn +
                ", isScreenShare=" + isScreenShare +
                '}';
    }
}
