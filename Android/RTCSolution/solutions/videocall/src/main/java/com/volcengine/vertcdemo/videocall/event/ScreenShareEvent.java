// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

/**
 * 房间内用户开始结束屏幕分享事件
 */
public class ScreenShareEvent {

    public boolean isStart;

    public ScreenShareEvent(boolean isStart) {
        this.isStart = isStart;
    }
}
