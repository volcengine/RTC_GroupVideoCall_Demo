// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "BaseRTCManager.h"
#import "VideoCallRTCManager.h"
#import "VideoCallRoomParamInfoModel.h"
#import "VideoCallRoomUserModel.h"
#import "VideoCallRoomViewController.h"

NS_ASSUME_NONNULL_BEGIN

@class VideoCallRTCManager;
@protocol VideoCallRTCManagerDelegate <NSObject>

/**
 * @brief 房间状态改变时的回调。 通过此回调，您会收到与房间相关的警告、错误和事件的通知。 例如，用户加入房间，用户被移出房间等。
 * @param manager GameRTCManager 模型
 * @param joinModel RTCJoinModel模型房间信息、加入成功失败等信息。
 */
- (void)videoCallRTCManager:(VideoCallRTCManager *)manager
         onRoomStateChanged:(RTCJoinModel *)joinModel;

/**
 * @brief 视频信息状态变化
 * @param rtcManager VideoCallRTCManager 对象
 * @param statsInfo 状态信息，key 为 user id， value 为状态信息内容。
 */
- (void)rtcManager:(VideoCallRTCManager *_Nullable)rtcManager didUpdateVideoStatsInfo:(NSDictionary<NSString *, VideoCallRoomParamInfoModel *> *_Nullable)statsInfo;

/**
 * @brief 音频信息状态变化
 * @param rtcManager VideoCallRTCManager 对象
 * @param statsInfo 状态信息，key 为 user id， value 为状态信息内容。
 */
- (void)rtcManager:(VideoCallRTCManager *_Nullable)rtcManager didUpdateAudioStatsInfo:(NSDictionary<NSString *, VideoCallRoomParamInfoModel *> *_Nullable)statsInfo;

/**
 * @brief 远端用户加入房间回调
 * @param rtcManager VideoCallRTCManager 对象
 * @param uid 用户 user id
 * @param name 用户昵称
 */
- (void)rtcManager:(VideoCallRTCManager *_Nonnull)rtcManager onUserJoined:(NSString *_Nullable)uid userName:(NSString *_Nullable)name;

/**
 * @brief 远端用户离开房间回调
 * @param rtcManager VideoCallRTCManager 对象
 * @param uid 用户 user id
 */
- (void)rtcManager:(VideoCallRTCManager *_Nonnull)rtcManager onUserLeaved:(NSString *_Nullable)uid;

/**
 * @brief 新增远端用户屏幕流回调
 * @param rtcManager VideoCallRTCManager 对象
 * @param screenStreamsUid 用户 user id
 */
- (void)rtcManager:(VideoCallRTCManager *_Nonnull)rtcManager didScreenStreamAdded:(NSString *_Nullable)screenStreamsUid;

/**
 * @brief 移除远端用户屏幕流回调
 * @param rtcManager VideoCallRTCManager 对象
 * @param screenStreamsUid 用户 user id
 */
- (void)rtcManager:(VideoCallRTCManager *_Nonnull)rtcManager didScreenStreamRemoved:(NSString *_Nullable)screenStreamsUid;

/**
 * @brief 用户麦克风音量变化回调
 * @param rtcManager VideoCallRTCManager 对象
 * @param volumeInfo 音量信息
 */
- (void)rtcManager:(VideoCallRTCManager *_Nonnull)rtcManager reportAllAudioVolume:(NSDictionary<NSString *, NSNumber *> *_Nonnull)volumeInfo;

/**
 * @brief 远端用户音频流变化回调
 * @param rtcManager VideoCallRTCManager 对象
 * @param uid 用户 user id
 * @param isMute 为 true 时暂停音频流，为 false 时恢复音频流。
 */
- (void)rtcManager:(VideoCallRTCManager *_Nonnull)rtcManager onUserMuteAudio:(NSString *_Nonnull)uid isMute:(BOOL)isMute;

/**
 * @brief 远端用户视频流变化回调
 * @param rtcManager VideoCallRTCManager 对象
 * @param uid 用户 user id
 * @param isMute 为 true 时暂停视频流，为 false 时恢复视频流。
 */
- (void)rtcManager:(VideoCallRTCManager *_Nonnull)rtcManager onUserMuteVideo:(NSString *_Nonnull)uid isMute:(BOOL)isMute;

/**
 * @brief 音频路由变化回调
 * @param isHeadset 为 true 时当前路由为耳机，为 false 时当前路由为其他。
 */
- (void)rtcManager:(VideoCallRTCManager *_Nonnull)rtcManager onAudioRouteChanged:(BOOL)isHeadset;

@end

@interface VideoCallRTCManager : BaseRTCManager

@property (nonatomic, weak) id<VideoCallRTCManagerDelegate> _Nullable delegate;

+ (VideoCallRTCManager *_Nullable)shareRtc;

#pragma mark - Base Method

/**
 * @brief 加入RTC房间
 * @param localUserModel 当前登录用户模型
 * @param rtcToken RTC token
 */
- (void)joinRTCRoomWithModel:(VideoCallRoomUserModel *)localUserModel
                    rtcToken:(NSString *)rtcToken;

/**
 * @brief 离开 RTC 房间
 */
- (void)leaveRTCRoom;

/**
 * @brief 更新音视频设置
 */
- (void)updateAudioAndVideoSettings;

/**
 * @brief 切换前置/后置相机
 */
- (void)switchCamera;

/**
 * @brief 设置音频路由
 * @param route ByteRTCAudioRoute 对象
 */
- (void)setDeviceAudioRoute:(ByteRTCAudioRoute)route;

/**
 * @brief 控制本地音频流的发送状态：发送/不发送
 * @param isPublish true：发送, false：不发送
 */
- (void)publishAudioStream:(BOOL)isPublish;

/**
 * @brief 开关本地视频采集
 * @param isStart ture：开启视频采集 false：关闭视频采集
 */
- (void)switchVideoCapture:(BOOL)isStart;

/**
 * @brief 渲染远端相机流
 * @param streamKey ByteRTCRemoteStreamKey 对象
 * @param canvas ByteRTCVideoCanvas 对象
 */
- (void)setupRemoteVideoStreamKey:(ByteRTCRemoteStreamKey *)streamKey
                           canvas:(ByteRTCVideoCanvas *)canvas;

/**
 * @brief 渲染本地相机流
 * @param canvas ByteRTCVideoCanvas 对象
 */
- (void)setupLocalVideo:(ByteRTCVideoCanvas *_Nullable)canvas;

/**
 * @brief 获取屏幕流渲染视图
 * @param uid 用户 user id
 */
- (UIView *)getScreenStreamViewWithUid:(NSString *)uid;

@end

NS_ASSUME_NONNULL_END
