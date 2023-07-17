// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallRoomUserModel.h"
#import "VideoCallRTCManager.h"

@implementation VideoCallRoomUserModel

- (instancetype)initWithUid:(NSString *)uid {
    self = [super init];
    if (self) {
        self.uid = uid;
    }
    return self;
}

@end
