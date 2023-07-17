// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

import com.volcengine.vertcdemo.videocall.bean.VideoCallUserInfo;

import java.util.List;

/**
 * 刷新房间内用户列表事件
 */
public class RefreshUserLayoutEvent {

    public final List<VideoCallUserInfo> userInfoList;

    public RefreshUserLayoutEvent(List<VideoCallUserInfo> userInfoList) {
        this.userInfoList = userInfoList;
    }
}
