// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.core;

import android.text.TextUtils;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.volcengine.vertcdemo.core.eventbus.SolutionDemoEventManager;
import com.volcengine.vertcdemo.videocall.bean.VideoCallUserInfo;
import com.volcengine.vertcdemo.videocall.event.MediaStatusEvent;
import com.volcengine.vertcdemo.videocall.event.RefreshUserLayoutEvent;
import com.volcengine.vertcdemo.videocall.event.RoomUserEvent;
import com.volcengine.vertcdemo.videocall.event.ScreenShareEvent;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * 房间内用户数据管理类
 *
 * 使用单例形式，通过RTC回调修改数据，再通过eventbus通知UI更新 {@link #ins()}
 *
 * 功能：
 * 1.增加用户 {@link #addUser(VideoCallUserInfo)}
 * 2.移除用户 {@link #removeUser(String)}
 * 3.移除所有用户 {@link #removeAllUser()}
 * 4.获取用户列表 {@link #getUserList()}
 * 5.更新用户的音频状态 {@link #updateAudioStatus(String, boolean)}
 * 6.更新用户的视频状态 {@link #updateVideoStatus(String, boolean)}
 * 7.设置屏幕分享用户信息 {@link #setScreenShareUser(VideoCallUserInfo)}
 * 8.移除屏幕分享用户信息 {@link #removeScreenShareUser(String)}
 * 9.获取屏幕分享用户信息 {@link #getScreenShareUser()}
 * 10.通过用户id获取用户名 {@link #getUserNameByUserId(String)}
 */
public class VideoCallDataManager {

    private final List<VideoCallUserInfo> mUserList = new ArrayList<>();
    private VideoCallUserInfo mScreenShareUser;

    private static VideoCallDataManager sInstance;

    private VideoCallDataManager() {

    }

    public static @NonNull
    VideoCallDataManager ins() {
        if (sInstance == null) {
            sInstance = new VideoCallDataManager();
        }
        return sInstance;
    }

    /**
     * 增加用户信息
     *
     * @param userInfo 用户信息
     */
    public void addUser(@Nullable VideoCallUserInfo userInfo) {
        if (userInfo == null || TextUtils.isEmpty(userInfo.userId)) {
            return;
        }
        if (mUserList.contains(userInfo)) {
            SolutionDemoEventManager.post(new RefreshUserLayoutEvent(getUserList()));
            return;
        }
        mUserList.add(userInfo);

        SolutionDemoEventManager.post(new RoomUserEvent(userInfo, true));
        SolutionDemoEventManager.post(new RefreshUserLayoutEvent(getUserList()));
    }

    /**
     * 移除用户信息
     *
     * @param userId 用户id
     */
    public void removeUser(@Nullable String userId) {
        if (TextUtils.isEmpty(userId)) {
            return;
        }
        Iterator<VideoCallUserInfo> iterator = mUserList.listIterator();
        while (iterator.hasNext()) {
            VideoCallUserInfo ui = iterator.next();
            if (TextUtils.equals(ui.userId, userId)) {
                iterator.remove();

                SolutionDemoEventManager.post(new RoomUserEvent(ui, false));
                SolutionDemoEventManager.post(new RefreshUserLayoutEvent(getUserList()));
            }
        }
    }

    /**
     * 移除所有用户信息
     */
    public void removeAllUser() {
        mUserList.clear();
        mScreenShareUser = null;
    }

    /**
     * 更新用户的音频状态
     *
     * @param userId 用户id
     * @param on     音频状态
     */
    public void updateAudioStatus(@Nullable String userId, boolean on) {
        if (TextUtils.isEmpty(userId)) {
            return;
        }
        for (VideoCallUserInfo ui : mUserList) {
            if (TextUtils.equals(ui.userId, userId)) {
                ui.isMicOn = on;
            }
        }

        SolutionDemoEventManager.post(new MediaStatusEvent(
                userId,
                Constants.MEDIA_TYPE_AUDIO,
                on ? Constants.MEDIA_STATUS_ON : Constants.MEDIA_STATUS_OFF));
    }

    /**
     * 更新用户的视频状态
     *
     * @param userId 用户id
     * @param on     视频状态
     */
    public void updateVideoStatus(@Nullable String userId, boolean on) {
        for (VideoCallUserInfo ui : mUserList) {
            if (TextUtils.equals(ui.userId, userId)) {
                ui.isCameraOn = on;
            }
        }

        SolutionDemoEventManager.post(new MediaStatusEvent(
                userId,
                Constants.MEDIA_TYPE_VIDEO,
                on ? Constants.MEDIA_STATUS_ON : Constants.MEDIA_STATUS_OFF));
    }

    /**
     * 获取当前房间内用户列表，如果有屏幕分享用户，则该用户信息位于用户列表第一个
     *
     * @return 用户列表，始终不为null
     */
    public @NonNull
    List<VideoCallUserInfo> getUserList() {
        List<VideoCallUserInfo> userInfoList = new ArrayList<>(mUserList);
        if (mScreenShareUser != null) {
            userInfoList.add(0, mScreenShareUser);
        }
        return userInfoList;
    }

    /**
     * 在当前用户列表中，通过用户id找到用户昵称
     *
     * @param userId 用户id
     * @return 用户昵称，找不到返回空字符串
     */
    public @NonNull
    String getUserNameByUserId(@Nullable String userId) {
        if (TextUtils.isEmpty(userId)) {
            return "";
        }
        for (VideoCallUserInfo userInfo : mUserList) {
            if (TextUtils.equals(userId, userInfo.userId)) {
                return userInfo.userName;
            }
        }
        return "";
    }

    /**
     * 在当前用户列表中，通过用户id找到用户信息
     *
     * @param userId 用户id
     * @return 用户信息，找不到返回空对象
     */
    public @Nullable
    VideoCallUserInfo getUserByUserId(@Nullable String userId) {
        if (TextUtils.isEmpty(userId)) {
            return null;
        }
        for (VideoCallUserInfo userInfo : mUserList) {
            if (TextUtils.equals(userId, userInfo.userId)) {
                return userInfo;
            }
        }
        return null;
    }

    /**
     * 设置屏幕分享用户信息
     *
     * @param userInfo 用户信息
     */
    public void setScreenShareUser(@Nullable VideoCallUserInfo userInfo) {
        VideoCallUserInfo videoCallUserInfo = getUserByUserId(userInfo == null ? null : userInfo.userId);
        // 同步以前的用户的摄像头、麦克风状态
        if (videoCallUserInfo != null && userInfo != null) {
            userInfo.isMicOn = videoCallUserInfo.isMicOn;
            userInfo.isCameraOn = videoCallUserInfo.isCameraOn;
        }

        mScreenShareUser = userInfo;
        if (userInfo != null) {
            SolutionDemoEventManager.post(new RefreshUserLayoutEvent(getUserList()));
            SolutionDemoEventManager.post(new ScreenShareEvent(true));
        }
    }

    /**
     * 移除屏幕分享用户信息
     *
     * @param uid 用户id
     */
    public void removeScreenShareUser(@Nullable String uid) {
        if (TextUtils.isEmpty(uid) || mScreenShareUser == null) {
            return;
        }
        if (TextUtils.equals(uid, mScreenShareUser.userId)) {
            mScreenShareUser = null;
            SolutionDemoEventManager.post(new RefreshUserLayoutEvent(getUserList()));
            SolutionDemoEventManager.post(new ScreenShareEvent(false));
        }
    }

    /**
     * 获取屏幕分享用户信息
     *
     * @return 用户信息
     */
    public @Nullable
    VideoCallUserInfo getScreenShareUser() {
        return mScreenShareUser;
    }
}
