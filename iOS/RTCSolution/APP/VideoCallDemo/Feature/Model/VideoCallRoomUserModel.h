// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "BaseUserModel.h"

@interface VideoCallRoomUserModel : BaseUserModel

@property (nonatomic, copy) NSString *roomId;
@property (nonatomic, assign) BOOL isScreen;
@property (nonatomic, assign) BOOL isEnableVideo;
@property (nonatomic, assign) BOOL isEnableAudio;

// speaker or earpiece
@property (nonatomic, assign) BOOL isSpeakers;

@end

