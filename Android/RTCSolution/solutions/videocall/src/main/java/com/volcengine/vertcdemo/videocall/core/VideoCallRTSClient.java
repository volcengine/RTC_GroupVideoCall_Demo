// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT

package com.volcengine.vertcdemo.videocall.core;

import android.text.TextUtils;

import androidx.annotation.NonNull;

import com.google.gson.JsonObject;
import com.ss.bytertc.engine.RTCVideo;
import com.volcengine.vertcdemo.common.AppExecutors;
import com.volcengine.vertcdemo.common.AbsBroadcast;
import com.volcengine.vertcdemo.common.AppExecutors;
import com.volcengine.vertcdemo.core.SolutionDataManager;
import com.volcengine.vertcdemo.core.eventbus.SolutionDemoEventManager;
import com.volcengine.vertcdemo.core.net.IRequestCallback;
import com.volcengine.vertcdemo.core.net.rts.RTSBaseClient;
import com.volcengine.vertcdemo.core.net.rts.RTSBizInform;
import com.volcengine.vertcdemo.core.net.rts.RTSBizResponse;
import com.volcengine.vertcdemo.core.net.rts.RTSInfo;
import com.volcengine.vertcdemo.videocall.bean.JoinRoomResponse;
import com.volcengine.vertcdemo.videocall.bean.LeaveRoomResponse;
import com.volcengine.vertcdemo.videocall.bean.ReconnectResponse;
import com.volcengine.vertcdemo.videocall.event.RoomFinishEvent;

import java.util.UUID;

/**
 * 客户端向业务服务器主动请求和接收广播的类
 *
 * 使用RTS功能发送和接收消息
 *
 * 功能：
 * 1.向业务服务器请求加入房间
 * 2.向业务服务器请求离开房间
 * 3.向业务服务器请求重连到当前房间
 * 4.接收房间到最大时间广播
 */
public class VideoCallRTSClient extends RTSBaseClient {

    // 前后端约定的主动请求和广播命令
    private static final String CMD_VIDEO_CALL_JOIN = "videocallJoinRoom";
    private static final String CMD_VIDEO_CALL_LEAVE = "videocallLeaveRoom";
    private static final String CMD_VIDEO_CALL_RECONNECT = "videocallReconnect";

    private static final String ON_ROOM_FINISH = "videocallOnCloseRoom";

    public VideoCallRTSClient(@NonNull RTCVideo rtcVideo, @NonNull RTSInfo rtmInfo) {
        super(rtcVideo, rtmInfo);
        initEventListener();
    }

    // 生成默认请求参数
    private JsonObject getCommonParams(String cmd) {
        JsonObject params = new JsonObject();
        params.addProperty("app_id", mRTSInfo.appId);
        params.addProperty("room_id", "");
        params.addProperty("user_id", SolutionDataManager.ins().getUserId());
        params.addProperty("event_name", cmd);
        params.addProperty("request_id", UUID.randomUUID().toString());
        params.addProperty("device_id", SolutionDataManager.ins().getDeviceId());
        params.addProperty("login_token", SolutionDataManager.ins().getToken());
        return params;
    }

    private void initEventListener() {
        // 房间到最大时间关闭事件
        putEventListener(new AbsBroadcast<>(ON_ROOM_FINISH, RoomFinishEvent.class,
                SolutionDemoEventManager::post));
    }

    private void putEventListener(AbsBroadcast<? extends RTSBizInform> absBroadcast) {
        mEventListeners.put(absBroadcast.getEvent(), absBroadcast);
    }

    public void removeAllEventListener() {
        mEventListeners.remove(ON_ROOM_FINISH);
    }

    private <T extends RTSBizResponse> void sendServerMessageOnNetwork(String roomId,
                                                                       JsonObject content,
                                                                       Class<T> resultClass,
                                                                       IRequestCallback<T> callback) {
        String cmd = content.get("event_name").getAsString();
        if (TextUtils.isEmpty(cmd)) {
            return;
        }
        AppExecutors.networkIO().execute(() ->
                sendServerMessage(cmd, roomId, content, resultClass, callback));
    }

    /**
     * 请求加入房间
     * @param roomId 房间id
     * @param callback 请求发送后回调
     */
    public void requestJoinRoom(String roomId, IRequestCallback<JoinRoomResponse> callback) {
        JsonObject content = getCommonParams(CMD_VIDEO_CALL_JOIN);
        content.addProperty("room_id", roomId);
        sendServerMessageOnNetwork(roomId, content, JoinRoomResponse.class, callback);
    }

    /**
     * 请求重连到房间
     * @param roomId 房间id
     * @param callback 请求发送后回调
     */
    public void requestReconnect(String roomId, IRequestCallback<ReconnectResponse> callback) {
        JsonObject content = getCommonParams(CMD_VIDEO_CALL_RECONNECT);
        content.addProperty("room_id", roomId);
        sendServerMessageOnNetwork(roomId, content, ReconnectResponse.class, callback);
    }

    /**
     * 请求离开房间
     * @param roomId 房间id
     * @param callback 请求发送后回调
     */
    public void requestLeaveRoom(String roomId, IRequestCallback<LeaveRoomResponse> callback) {
        JsonObject content = getCommonParams(CMD_VIDEO_CALL_LEAVE);
        content.addProperty("room_id", roomId);
        sendServerMessageOnNetwork(roomId, content, LeaveRoomResponse.class, callback);
    }
}
