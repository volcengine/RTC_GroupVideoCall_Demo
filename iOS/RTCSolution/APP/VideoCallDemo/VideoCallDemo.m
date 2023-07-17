// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallDemo.h"
#import "JoinRTSParams.h"
#import "VideoCallLoginViewController.h"
#import "VideoCallRTCManager.h"
#import "VideoCallDemoConstants.h"

@implementation VideoCallDemo

- (void)pushDemoViewControllerBlock:(void (^)(BOOL result))block {
    [super pushDemoViewControllerBlock:block];

    JoinRTSInputModel *inputModel = [[JoinRTSInputModel alloc] init];
    inputModel.scenesName = self.scenesName;
    inputModel.loginToken = [LocalUserComponent userModel].loginToken;
    __weak __typeof(self) wself = self;
    [JoinRTSParams getJoinRTSParams:inputModel
                              block:^(JoinRTSParamsModel * _Nonnull model) {
        [wself joinRTS:model block:block];
    }];
}

- (void)joinRTS:(JoinRTSParamsModel * _Nonnull)model
          block:(void (^)(BOOL result))block {
    if (!model) {
        [[ToastComponent shareToastComponent] showWithMessage:LocalizedString(@"connection_failed")];
        if (block) {
            block(NO);
        }
        return;
    }
    // Connect RTS
    [[VideoCallRTCManager shareRtc] connect:model.appId
                                   RTSToken:model.RTSToken
                                  serverUrl:model.serverUrl
                                  serverSig:model.serverSignature
                                        bid:model.bid
                                      block:^(BOOL result) {
        if (result) {
            VideoCallLoginViewController *next = [[VideoCallLoginViewController alloc] init];
            UIViewController *topVC = [DeviceInforTool topViewController];
            [topVC.navigationController pushViewController:next animated:YES];
        } else {
            [[ToastComponent shareToastComponent] showWithMessage:LocalizedString(@"connection_failed")];
        }
        if (block) {
            block(result);
        }
    }];
}

@end
