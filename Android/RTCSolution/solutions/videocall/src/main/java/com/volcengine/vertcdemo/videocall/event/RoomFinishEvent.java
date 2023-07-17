// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

import com.google.gson.annotations.SerializedName;
import com.volcengine.vertcdemo.core.net.rts.RTSBizInform;

/**
 * 房间结束事件，用于关闭房间页面
 */
public class RoomFinishEvent implements RTSBizInform {

    @SerializedName("room_id")
    public String roomId;

    @Override
    public String toString() {
        return "RoomFinishEvent{" +
                "roomId='" + roomId + '\'' +
                '}';
    }
}
