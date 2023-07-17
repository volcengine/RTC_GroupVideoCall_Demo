// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.event;

import com.volcengine.vertcdemo.videocall.core.Constants;

/**
 * 媒体状态切换事件，用于UI展示
 */
public class MediaStatusEvent {

    public String uid;
    public final @Constants.MediaType int mediaType;
    public final @Constants.MediaStatus int status;

    public MediaStatusEvent(String uid,
            @Constants.MediaType int mediaType, @Constants.MediaStatus int status) {
        this.uid = uid;
        this.status = status;
        this.mediaType = mediaType;
    }
}
