// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallRoomViewController+Listener.h"
#import "VideoCallRTSManager.h"
#import "VideoCallRTCManager.h"

@implementation VideoCallRoomViewController (Listener)

- (void)addRTSListener {
    __weak __typeof(self) wself = self;
    //User Join
    [VideoCallRTSManager onCloseRoomWithBlock:^(NSString * _Nonnull roomId) {
        [wself hangUp];
        [[ToastComponent shareToastComponent] showWithMessage:LocalizedString(@"minutes_meeting") delay:0.8];
    }];
        
}

@end
