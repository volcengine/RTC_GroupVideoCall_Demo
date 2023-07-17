// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.bean;

import com.google.gson.annotations.SerializedName;
import com.volcengine.vertcdemo.core.net.rts.RTSBizResponse;

/**
 * 加入业务服务器房间接口返回模型
 */
public class JoinRoomResponse implements RTSBizResponse {

    // 房间持续时间，单位: second
    @SerializedName("duration")
    public long durationS;
    // 加入rtc房间的token
    @SerializedName("rtc_token")
    public String rtcToken;

    @Override
    public String toString() {
        return "JoinRoomResponse{" +
                "durationS=" + durationS +
                ", rtcToken='" + rtcToken + '\'' +
                '}';
    }
}
