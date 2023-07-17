// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

import com.volcengine.vertcdemo.videocall.bean.VideoCallUserInfo;

/**
 * 房间内用户加入离开事件
 */
public class RoomUserEvent {

    public final boolean isJoin;
    public final VideoCallUserInfo userInfo;

    public RoomUserEvent(VideoCallUserInfo userInfo, boolean isJoin) {
        this.userInfo = userInfo;
        this.isJoin = isJoin;
    }
}
