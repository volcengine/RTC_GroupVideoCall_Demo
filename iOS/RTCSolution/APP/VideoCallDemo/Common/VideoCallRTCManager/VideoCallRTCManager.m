// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallRTCManager.h"
#import "AlertActionManager.h"
#import "VideoCallMockDataComponent.h"

@interface VideoCallRTCManager () <ByteRTCVideoDelegate>
// RTC / RTS 房间
@property (nonatomic, strong, nullable) ByteRTCRoom *rtcRoom;
@property (nonatomic, strong) NSMutableDictionary <NSString *, VideoCallRoomParamInfoModel *>*videoStatsInfoDic;
@property (nonatomic, strong) NSMutableDictionary <NSString *, VideoCallRoomParamInfoModel *>*audioStatsInfoDic;
@property (nonatomic, strong) NSMutableDictionary<NSString *, UIView *> *streamViewDic;
@property (nonatomic, assign) NSInteger currnetCameraID;
@property (nonatomic, assign) ByteRTCAudioRoute audioRoute;
@property (nonatomic, strong) UIView *localVideoView;

@end

@implementation VideoCallRTCManager

+ (VideoCallRTCManager *_Nullable)shareRtc {
    static VideoCallRTCManager *manager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        manager = [[VideoCallRTCManager alloc] init];
    });
    return manager;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        // 默认前置摄像头、扬声器模式
        self.currnetCameraID = ByteRTCCameraIDFront;
        self.audioRoute = ByteRTCAudioRouteSpeakerphone;
    }
    return self;
}

#pragma mark - Publish Action

- (void)configeRTCEngine {
    [super configeRTCEngine];
}

- (void)joinRTCRoomWithModel:(VideoCallRoomUserModel *)localUserModel
                    rtcToken:(NSString *)rtcToken {
    // 设置音频场景类型
    [self.rtcEngineKit setAudioScenario:ByteRTCAudioScenarioCommunication];
    
    // 开/关 本地相机采集
    [self switchVideoCapture:localUserModel.isEnableVideo];
    
    // 开启本地麦克风采集
    [self.rtcEngineKit startAudioCapture];
    
    // 设置音频路由模式，YES 扬声器/NO 听筒
    ByteRTCAudioRoute audioRoute = localUserModel.isSpeakers ? ByteRTCAudioRouteSpeakerphone : ByteRTCAudioRouteEarpiece;
    [self setDeviceAudioRoute:audioRoute];
    
    // 开启/关闭发言者音量键控
    ByteRTCAudioPropertiesConfig *audioPropertiesConfig = [[ByteRTCAudioPropertiesConfig alloc] init];
    audioPropertiesConfig.interval = 1000;
    [self.rtcEngineKit enableAudioPropertiesReport:audioPropertiesConfig];
    
    // 加入房间，开始连麦,需要申请AppId和Token
    ByteRTCUserInfo *userInfo = [[ByteRTCUserInfo alloc] init];
    userInfo.userId = localUserModel.uid;
    NSDictionary *extraInfo = @{
        @"user_name": localUserModel.name,
        @"user_id": localUserModel.uid
    };
    userInfo.extraInfo = [extraInfo yy_modelToJSONString];
    ByteRTCRoomConfig *config = [[ByteRTCRoomConfig alloc] init];
    config.profile = ByteRTCRoomProfileCommunication;
    config.isAutoPublish = YES;
    config.isAutoSubscribeAudio = YES;
    config.isAutoSubscribeVideo = YES;
    self.rtcRoom = [self.rtcEngineKit createRTCRoom:localUserModel.roomId];
    self.rtcRoom.delegate = self;
    [self.rtcRoom joinRoom:rtcToken userInfo:userInfo roomConfig:config];
    
    // 开启/关闭 本地音频推流
    [self publishAudioStream:localUserModel.isEnableAudio];
}

- (void)updateAudioAndVideoSettings {
    // 修改分辨率
    VideoCallMockDataComponent *mockData = [VideoCallMockDataComponent shared];
    NSNumber *res = mockData.currentResolutionDic[@"value"];
    ByteRTCVideoEncoderConfig *config = [[ByteRTCVideoEncoderConfig alloc] init];
    config.width = res.CGSizeValue.width;
    config.height = res.CGSizeValue.height;
    config.frameRate = 15;
    config.maxBitrate = -1;
    config.encoderPreference = ByteRTCVideoEncoderPreferenceDisabled;
    [self.rtcEngineKit setMaxVideoEncoderConfig:config];
    
    // 设置镜像，后置摄像头取消镜像
    if (mockData.isOpenMirror && self.currnetCameraID == ByteRTCCameraIDFront) {
        [self.rtcEngineKit setLocalVideoMirrorType:ByteRTCMirrorTypeRenderAndEncoder];
    } else {
        [self.rtcEngineKit setLocalVideoMirrorType:ByteRTCMirrorTypeNone];
    }
    
    // 设置音频质量
    NSNumber *audioProfile = mockData.currentaudioProfileDic[@"value"];
    [self.rtcEngineKit setAudioProfile:audioProfile.integerValue];
}

- (void)switchCamera {
    // 切换 前置/后置 摄像头
    VideoCallMockDataComponent *mockData = [VideoCallMockDataComponent shared];
        
    ByteRTCCameraID cameraID = self.currnetCameraID;
    if (cameraID == ByteRTCCameraIDFront) {
        cameraID = ByteRTCCameraIDBack;
    } else {
        cameraID = ByteRTCCameraIDFront;
    }
    
    if (cameraID == ByteRTCCameraIDFront && mockData.isOpenMirror) {
        [self.rtcEngineKit setLocalVideoMirrorType:ByteRTCMirrorTypeRenderAndEncoder];
    } else {
        [self.rtcEngineKit setLocalVideoMirrorType:ByteRTCMirrorTypeNone];
    }
    
    [self.rtcEngineKit switchCamera:cameraID];
    self.currnetCameraID = cameraID;
}

- (void)setDeviceAudioRoute:(ByteRTCAudioRoute)route {
    // 打开/关闭 外放
    if (self.audioRoute == ByteRTCAudioRouteEarpiece ||
        self.audioRoute == ByteRTCAudioRouteSpeakerphone) {
        if (self.audioRoute == route) {
            return;
        }
        self.audioRoute = route;
        
        [self.rtcEngineKit setDefaultAudioRoute:route];
    }
}

- (void)publishAudioStream:(BOOL)isPublish {
    // 开启/关闭 本地音频
    if (isPublish) {
        [self.rtcRoom publishStream:ByteRTCMediaStreamTypeAudio];
    } else {
        [self.rtcRoom unpublishStream:ByteRTCMediaStreamTypeAudio];
    }
}

- (void)switchVideoCapture:(BOOL)isStart {
    // 开启/关闭 本地视频采集
    if (isStart) {
        [self.rtcEngineKit startVideoCapture];
        [self startPreview:self.localVideoView];
    } else {
        [self.rtcEngineKit stopVideoCapture];
        [self startPreview:nil];
    }
}

- (void)leaveRTCRoom {
    // 离开 RTC 房间
    if (self.currnetCameraID != ByteRTCCameraIDFront) {
        self.currnetCameraID = ByteRTCCameraIDFront;
        [self.rtcEngineKit switchCamera:ByteRTCCameraIDFront];
    }
    [self.streamViewDic removeAllObjects];
    [self.rtcEngineKit stopAudioCapture];
    [self.rtcRoom leaveRoom];
    [self.rtcRoom destroy];
    self.rtcRoom = nil;
}

- (void)setupRemoteVideoStreamKey:(ByteRTCRemoteStreamKey *)streamKey
                           canvas:(ByteRTCVideoCanvas *)canvas {
    // 设置远端相机视频流
    if (!canvas || !streamKey) {
        return;
    }
    [self.rtcEngineKit setRemoteVideoCanvas:streamKey withCanvas:canvas];
}

- (void)setupLocalVideo:(ByteRTCVideoCanvas *)canvas {
    // 设置本地相机视频流
    self.localVideoView = canvas.view;
    [self.rtcEngineKit setLocalVideoCanvas:ByteRTCStreamIndexMain withCanvas:canvas];
}

- (void)bingScreenCanvasViewToUid:(NSString *)uid {
    dispatch_queue_async_safe(dispatch_get_main_queue(), (^{
        UIView *remoteRoomView = [self getScreenStreamViewWithUid:uid];
        if (!remoteRoomView) {
            remoteRoomView = [[UIView alloc] init];
            remoteRoomView.hidden = YES;
            ByteRTCVideoCanvas *canvas = [[ByteRTCVideoCanvas alloc] init];
            canvas.renderMode = ByteRTCRenderModeFit;
            canvas.view.backgroundColor = [UIColor clearColor];
            canvas.view = remoteRoomView;
            
            ByteRTCRemoteStreamKey *streamKey = [[ByteRTCRemoteStreamKey alloc] init];
            streamKey.userId = uid;
            streamKey.roomId = self.rtcRoom.getRoomId;
            streamKey.streamIndex = ByteRTCStreamIndexScreen;
            
            [self.rtcEngineKit setRemoteVideoCanvas:streamKey
                                         withCanvas:canvas];
            
            NSString *groupKey = [NSString stringWithFormat:@"screen_%@", uid];
            [self.streamViewDic setValue:remoteRoomView forKey:groupKey];
        }
    }));
}

- (UIView *)getScreenStreamViewWithUid:(NSString *)uid {
    if (IsEmptyStr(uid)) {
        return nil;
    }
    NSString *typeStr = @"screen";
    NSString *key = [NSString stringWithFormat:@"%@_%@", typeStr, uid];
    UIView *view = self.streamViewDic[key];
    return view;
}

#pragma mark - ByteRTCRoomDelegate

- (void)rtcRoom:(ByteRTCRoom *)rtcRoom onRoomStateChanged:(NSString *)roomId
        withUid:(NSString *)uid
          state:(NSInteger)state
      extraInfo:(NSString *)extraInfo {
    [super rtcRoom:rtcRoom onRoomStateChanged:roomId withUid:uid state:state extraInfo:extraInfo];
    
    dispatch_queue_async_safe(dispatch_get_main_queue(), ^{
        RTCJoinModel *joinModel = [RTCJoinModel modelArrayWithClass:extraInfo state:state roomId:roomId];
        if ([self.delegate respondsToSelector:@selector(videoCallRTCManager:onRoomStateChanged:)]) {
            [self.delegate videoCallRTCManager:self onRoomStateChanged:joinModel];
        }
    });
}

- (void)rtcRoom:(ByteRTCRoom *)rtcRoom onUserJoined:(ByteRTCUserInfo *)userInfo elapsed:(NSInteger)elapsed {
    NSString *name = userInfo.userId;
    if(userInfo.extraInfo) {
        NSDictionary *extraInfo =
        [NSJSONSerialization JSONObjectWithData:[userInfo.extraInfo dataUsingEncoding:NSUTF8StringEncoding]
                                        options:NSJSONReadingMutableContainers
                                          error:nil];
        if (extraInfo) {
            id value = extraInfo[@"user_name"];
            if ([value isKindOfClass:[NSString class]]) {
                name = value;
            }
        }
    }
    
    if ([self.delegate respondsToSelector:@selector(rtcManager:onUserJoined:userName:)]) {
        [self.delegate rtcManager:self onUserJoined:userInfo.userId userName:name];
    }

}
- (void)rtcRoom:(ByteRTCRoom *)rtcRoom onUserLeave:(NSString *)uid reason:(ByteRTCUserOfflineReason)reason {
    if ([self.delegate respondsToSelector:@selector(rtcManager:onUserLeaved:)]) {
        [self.delegate rtcManager:self onUserLeaved:uid];
    }
}

- (void)rtcRoom:(ByteRTCRoom *)rtcRoom onUserPublishScreen:(NSString *)userId type:(ByteRTCMediaStreamType)type {
    [self bingScreenCanvasViewToUid:userId];
    if ([self.delegate respondsToSelector:@selector(rtcManager:didScreenStreamAdded:)]) {
        [self.delegate rtcManager:self didScreenStreamAdded:userId];
    }
}

- (void)rtcRoom:(ByteRTCRoom *)rtcRoom onUserUnpublishScreen:(NSString *)userId type:(ByteRTCMediaStreamType)type reason:(ByteRTCStreamRemoveReason)reason {
    if ([self.delegate respondsToSelector:@selector(rtcManager:didScreenStreamRemoved:)]) {
        [self.delegate rtcManager:self didScreenStreamRemoved:userId];
    }
}

- (void)rtcRoom:(ByteRTCRoom *)rtcRoom onLocalStreamStats:(ByteRTCLocalStreamStats *)stats {

    VideoCallRoomParamInfoModel *videoStatsInfo = [VideoCallRoomParamInfoModel new];
    videoStatsInfo.uid = [LocalUserComponent userModel].uid;
    videoStatsInfo.width = stats.video_stats.encodedFrameWidth;
    videoStatsInfo.height = stats.video_stats.encodedFrameHeight;
    videoStatsInfo.bitRate = stats.video_stats.sentKBitrate;
    videoStatsInfo.fps = stats.video_stats.sentFrameRate;
    videoStatsInfo.delay = stats.video_stats.rtt;
    videoStatsInfo.lost = stats.video_stats.videoLossRate;
    
    VideoCallRoomParamInfoModel *audioStatsInfo = [VideoCallRoomParamInfoModel new];
    audioStatsInfo.uid = [LocalUserComponent userModel].uid;
    audioStatsInfo.bitRate = stats.audio_stats.sentKBitrate;
    audioStatsInfo.delay = stats.audio_stats.rtt;
    audioStatsInfo.lost = stats.audio_stats.audioLossRate;
    
    ByteRTCNetworkQuality quality = MAX(stats.tx_quality, stats.rx_quality);
    VideoCallRoomParamNetQuality netQuality;
    switch (quality) {
        case ByteRTCNetworkQualityUnknown:
            netQuality = VideoCallRoomParamNetQualityNormal;
            break;
        case ByteRTCNetworkQualityExcellent:
        case ByteRTCNetworkQualityGood:
            netQuality = VideoCallRoomParamNetQualityGood;
            break;
        case ByteRTCNetworkQualityPoor:
            netQuality = VideoCallRoomParamNetQualityNormal;
            break;
        case ByteRTCNetworkQualityBad:
        case ByteRTCNetworkQualityVeryBad:
            netQuality = VideoCallRoomParamNetQualityBad;
            break;
        default:
            netQuality = VideoCallRoomParamNetQualityNormal;
            break;
    }
    videoStatsInfo.netQuality = netQuality;
    audioStatsInfo.netQuality = netQuality;

    [self updateVideoCallRoomVideoStatsInfo:videoStatsInfo audioStatsInfo:audioStatsInfo];
}

- (void)rtcRoom:(ByteRTCRoom *)rtcRoom onRemoteStreamStats:(ByteRTCRemoteStreamStats *)stats {
    VideoCallRoomParamInfoModel *videoStatsInfo = [VideoCallRoomParamInfoModel new];
    videoStatsInfo.uid = stats.uid;
    videoStatsInfo.width = stats.video_stats.width;
    videoStatsInfo.height = stats.video_stats.height;
    videoStatsInfo.bitRate = stats.video_stats.receivedKBitrate;
    videoStatsInfo.fps = stats.video_stats.receivedFrameRate;
    videoStatsInfo.delay = stats.video_stats.rtt;
    videoStatsInfo.lost = stats.video_stats.videoLossRate;
    
    VideoCallRoomParamInfoModel *audioStatsInfo = [VideoCallRoomParamInfoModel new];
    audioStatsInfo.uid = stats.uid;
    audioStatsInfo.bitRate = stats.audio_stats.receivedKBitrate;
    audioStatsInfo.delay = stats.audio_stats.rtt;
    audioStatsInfo.lost = stats.audio_stats.audioLossRate;
    
    ByteRTCNetworkQuality quality = MAX(stats.tx_quality, stats.rx_quality);
    VideoCallRoomParamNetQuality netQuality;
    switch (quality) {
        case ByteRTCNetworkQualityUnknown:
            netQuality = VideoCallRoomParamNetQualityNormal;
            break;
        case ByteRTCNetworkQualityExcellent:
        case ByteRTCNetworkQualityGood:
            netQuality = VideoCallRoomParamNetQualityGood;
            break;
        case ByteRTCNetworkQualityPoor:
            netQuality = VideoCallRoomParamNetQualityNormal;
            break;
        case ByteRTCNetworkQualityBad:
        case ByteRTCNetworkQualityVeryBad:
            netQuality = VideoCallRoomParamNetQualityBad;
            break;
        default:
            netQuality = VideoCallRoomParamNetQualityNormal;
            break;
    }
    videoStatsInfo.netQuality = netQuality;
    audioStatsInfo.netQuality = netQuality;

    [self updateVideoCallRoomVideoStatsInfo:videoStatsInfo audioStatsInfo:audioStatsInfo];
}

#pragma mark - ByteRTCVideoDelegate

- (void)rtcEngine:(ByteRTCVideo *)engine onUserMuteAudio:(NSString *)roomId uid:(NSString *)uid muteState:(ByteRTCMuteState)muteState {
    // 远程用户开关麦克风
    if ([self.delegate respondsToSelector:@selector(rtcManager:onUserMuteAudio:isMute:)]) {
        [self.delegate rtcManager:self onUserMuteAudio:uid isMute:(muteState == ByteRTCMuteStateOn) ? YES : NO];
    }
}

- (void)rtcEngine:(ByteRTCVideo *)engine onUserStartVideoCapture:(NSString *)roomId uid:(NSString *)uid {
    // 判断远程用户打开相机
    if ([self.delegate respondsToSelector:@selector(rtcManager:onUserMuteVideo:isMute:)]) {
        [self.delegate rtcManager:self onUserMuteVideo:uid isMute:NO];
    }
}

- (void)rtcEngine:(ByteRTCVideo *)engine onUserStopVideoCapture:(NSString *)roomId uid:(NSString *)uid {
    // 判断远程用户关闭相机
    if ([self.delegate respondsToSelector:@selector(rtcManager:onUserMuteVideo:isMute:)]) {
        [self.delegate rtcManager:self onUserMuteVideo:uid isMute:YES];
    }
}

- (void)rtcEngine:(ByteRTCVideo *)engine onAudioRouteChanged:(ByteRTCAudioRoute)device {
    NSInteger isHeadset = -1;
    if (device == ByteRTCAudioRouteDefault ||
        device == ByteRTCAudioRouteHeadset ||
        device == ByteRTCAudioRouteHeadsetBluetooth ||
        device == ByteRTCAudioRouteHeadsetUSB) {
        // 插入耳机
        isHeadset = 1;
        self.audioRoute = device;
    } else {
        if (self.audioRoute == ByteRTCAudioRouteDefault ||
            self.audioRoute == ByteRTCAudioRouteHeadset ||
            self.audioRoute == ByteRTCAudioRouteHeadsetBluetooth ||
            self.audioRoute == ByteRTCAudioRouteHeadsetUSB) {
            // 拔出耳机
            isHeadset = 2;
            // 当耳机拔出时，SDK 会自动切换到外放模式
            self.audioRoute = ByteRTCAudioRouteSpeakerphone;
        } else {
            // 正常设置听筒/外放
        }
    }
    if (isHeadset > 0) {
        dispatch_queue_async_safe(dispatch_get_main_queue(), ^{
            if ([self.delegate respondsToSelector:@selector(rtcManager:onAudioRouteChanged:)]) {
                BOOL isHeadsetBool = (isHeadset == 1) ? YES : NO;
                [self.delegate rtcManager:self onAudioRouteChanged:isHeadsetBool];
            }
        });
    }
}

- (void)rtcEngine:(ByteRTCVideo *)engine reportSysStats:(const ByteRTCSysStats *)stats {
    
}

- (void)rtcEngine:(ByteRTCVideo * _Nonnull)engine onLocalAudioPropertiesReport:(NSArray<ByteRTCLocalAudioPropertiesInfo *> * _Nonnull)audioPropertiesInfos {
    NSInteger minVolume = 10;
    NSMutableDictionary *parDic = [[NSMutableDictionary alloc] init];
    for (int i = 0; i < audioPropertiesInfos.count; i++) {
        ByteRTCLocalAudioPropertiesInfo *model = audioPropertiesInfos[i];
        if (model.audioPropertiesInfo.linearVolume > minVolume) {
            [parDic setValue:@(model.audioPropertiesInfo.linearVolume) forKey:[LocalUserComponent userModel].uid];
        }
    }
    
    dispatch_queue_async_safe(dispatch_get_main_queue(), ^{
        if ([self.delegate respondsToSelector:@selector(rtcManager:reportAllAudioVolume:)]) {
            [self.delegate rtcManager:self reportAllAudioVolume:[parDic copy]];
        }
    });
}

- (void)rtcEngine:(ByteRTCVideo *)engine onRemoteAudioPropertiesReport:(NSArray<ByteRTCRemoteAudioPropertiesInfo *> *)audioPropertiesInfos totalRemoteVolume:(NSInteger)totalRemoteVolume {
    
    NSInteger minVolume = 10;
    NSMutableDictionary *parDic = [[NSMutableDictionary alloc] init];
    for (int i = 0; i < audioPropertiesInfos.count; i++) {
        ByteRTCRemoteAudioPropertiesInfo *model = audioPropertiesInfos[i];
        if (model.audioPropertiesInfo.linearVolume > minVolume) {
            [parDic setValue:@(model.audioPropertiesInfo.linearVolume) forKey:model.streamKey.userId];
        }
    }
    
    dispatch_queue_async_safe(dispatch_get_main_queue(), ^{
        if ([self.delegate respondsToSelector:@selector(rtcManager:reportAllAudioVolume:)]) {
            [self.delegate rtcManager:self reportAllAudioVolume:[parDic copy]];
        }
    });
}

#pragma mark - Private Action

- (void)updateVideoCallRoomVideoStatsInfo:(VideoCallRoomParamInfoModel *)videoStatsInfo audioStatsInfo:(VideoCallRoomParamInfoModel *)audioStatsInfo {
    [self.videoStatsInfoDic setObject:videoStatsInfo forKey:videoStatsInfo.uid];
    if ([self.delegate respondsToSelector:@selector(rtcManager:didUpdateVideoStatsInfo:)]) {
        [self.delegate rtcManager:self didUpdateVideoStatsInfo:self.videoStatsInfoDic];
    }
    
    [self.audioStatsInfoDic setObject:audioStatsInfo forKey:audioStatsInfo.uid];
    if ([self.delegate respondsToSelector:@selector(rtcManager:didUpdateAudioStatsInfo:)]) {
        [self.delegate rtcManager:self didUpdateAudioStatsInfo:self.audioStatsInfoDic];
    }
}

- (void)startPreview:(UIView *)view {
    if (view) {
        self.localVideoView = view;
    }
    ByteRTCVideoCanvas *canvas = [[ByteRTCVideoCanvas alloc] init];
    canvas.view = view;
    canvas.renderMode = ByteRTCRenderModeHidden;
    canvas.view.backgroundColor = [UIColor clearColor];
    // 设置本地视频显示信息
    [self.rtcEngineKit setLocalVideoCanvas:ByteRTCStreamIndexMain withCanvas:canvas];
}

#pragma mark - Getter

- (NSMutableDictionary<NSString *, UIView *> *)streamViewDic {
    if (!_streamViewDic) {
        _streamViewDic = [[NSMutableDictionary alloc] init];
    }
    return _streamViewDic;
}

- (NSMutableDictionary<NSString *,VideoCallRoomParamInfoModel *> *)videoStatsInfoDic {
    if (!_videoStatsInfoDic) {
        _videoStatsInfoDic = [NSMutableDictionary dictionary];
    }
    return _videoStatsInfoDic;
}

- (NSMutableDictionary<NSString *,VideoCallRoomParamInfoModel *> *)audioStatsInfoDic {
    if (!_audioStatsInfoDic) {
        _audioStatsInfoDic = [NSMutableDictionary dictionary];
    }
    return _audioStatsInfoDic;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
