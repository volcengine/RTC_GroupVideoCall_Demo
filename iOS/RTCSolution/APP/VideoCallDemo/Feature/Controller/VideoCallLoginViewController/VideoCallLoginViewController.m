// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallLoginViewController.h"
#import "VideoCallRTCManager.h"
#import "VideoCallRoomViewController.h"
#import "VideoCallRoomUserModel.h"
#import "VideoCallRTSManager.h"
#import "VideoCallMockDataComponent.h"

#define TEXTFIELD_MAX_LENGTH 18

@interface VideoCallLoginViewController () <UITextFieldDelegate>
@property (nonatomic, strong) UIImageView *logoImageView;
@property (nonatomic, strong) BaseButton *enableAudioBtn;
@property (nonatomic, strong) BaseButton *enableVideoBtn;
@property (nonatomic, strong) UIButton *setingBtn;
@property (nonatomic, strong) UIButton *enterRoomBtn;
@property (nonatomic, strong) BaseButton *navLeftButton;
@property (nonatomic, strong) UITextField *roomIdTextField;
@property (nonatomic, strong) UILabel *verLabel;
@property (nonatomic, strong) UIImageView *emptImageView;
@property (nonatomic, strong) UIView *videoView;
@property (nonatomic, strong) UIView *maskView;
@property (nonatomic, strong) UITapGestureRecognizer *tap;
@property (nonatomic, assign) BOOL isSpeakers;
@end

@implementation VideoCallLoginViewController
- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor blackColor];
    [self initUIComponent];
    [self authorizationStatusMicAndCamera];
    
    NSString *sdkVer = [VideoCallRTCManager getSdkVersion];
    NSString *appVer = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    NSString *appStr = [NSString stringWithFormat:LocalizedString(@"app_version_v%@"), appVer];
    NSString *sdkStr = [NSString stringWithFormat:LocalizedString(@"sdk_version_v%@"), sdkVer];
    self.verLabel.text = [NSString stringWithFormat:@"%@ / %@", appStr, sdkStr];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyBoardDidShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyBoardDidHide:) name:UIKeyboardWillHideNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];
    
    [self applicationBecomeActive];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    [self.navigationController setNavigationBarHidden:YES animated:NO];
    [UIApplication sharedApplication].statusBarStyle = UIStatusBarStyleLightContent;
    
    ByteRTCVideoCanvas *canvas = [[ByteRTCVideoCanvas alloc] init];
    canvas.view = self.videoView;
    canvas.renderMode = ByteRTCRenderModeHidden;
    canvas.view.backgroundColor = [UIColor clearColor];
    [[VideoCallRTCManager shareRtc] updateAudioAndVideoSettings];
    [[VideoCallRTCManager shareRtc] setDeviceAudioRoute:ByteRTCAudioRouteSpeakerphone];
    [[VideoCallRTCManager shareRtc] switchVideoCapture:YES];
    [[VideoCallRTCManager shareRtc] setupLocalVideo:canvas];
    
    self.isSpeakers = YES;
    [self updateSetingBtnImage];
    if (self.enableAudioBtn.status != ButtonStatusIllegal) {
        self.enableAudioBtn.status = ButtonStatusNone;
    }
    if (self.enableVideoBtn.status != ButtonStatusIllegal) {
        self.enableVideoBtn.status = ButtonStatusNone;
        self.emptImageView.hidden = YES;
        self.videoView.hidden = NO;
    }
}

#pragma mark - Notify

- (void)applicationBecomeActive {
    UIViewController *topVC = [DeviceInforTool topViewController];
    if ([topVC isKindOfClass:[VideoCallLoginViewController class]]) {
        AlertActionModel *alertModel = [[AlertActionModel alloc] init];
        alertModel.title = LocalizedString(@"ok");
        [[AlertActionManager shareAlertActionManager] showWithMessage:LocalizedString(@"minutes_meeting_title")
                                                              actions:@[alertModel]];
    }
}

- (void)keyBoardDidShow:(NSNotification *)notifiction {
    CGRect keyboardRect = [[notifiction.userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    [UIView animateWithDuration:0.25 animations:^{
        [self.enterRoomBtn mas_updateConstraints:^(MASConstraintMaker *make) {
            make.bottom.equalTo(self.view).offset(-keyboardRect.size.height - 80/2);
        }];
    }];
    self.emptImageView.hidden = YES;
    [self.view layoutIfNeeded];
}

- (void)keyBoardDidHide:(NSNotification *)notifiction {
    [UIView animateWithDuration:0.25 animations:^{
        [self.enterRoomBtn mas_updateConstraints:^(MASConstraintMaker *make) {
            make.bottom.equalTo(self.view).offset(-288/2 - [DeviceInforTool getVirtualHomeHeight]);
        }];
    }];
    self.emptImageView.hidden = (self.enableVideoBtn.status == ButtonStatusNone);
    [self.view layoutIfNeeded];
}

#pragma mark - Action Method

- (void)tapGestureAction:(id)sender {
    [self.roomIdTextField resignFirstResponder];
}

- (void)onClickEnterRoom:(UIButton *)sender {
    if (self.roomIdTextField.text.length <= 0) {
        return;
    }
    BOOL checkRoomId = ![LocalUserComponent isMatchRoomID:self.roomIdTextField.text];
    if (checkRoomId) {
        return;
    }
    
    VideoCallRoomUserModel *userModel = [[VideoCallRoomUserModel alloc] init];
    userModel.uid = [LocalUserComponent userModel].uid;
    userModel.name = [LocalUserComponent userModel].name;
    userModel.roomId = [NSString stringWithFormat:@"call_%@", self.roomIdTextField.text];
    userModel.isEnableAudio = self.enableAudioBtn.status == ButtonStatusNone;
    userModel.isEnableVideo = self.enableVideoBtn.status == ButtonStatusNone;
    userModel.isScreen = NO;
    userModel.isSpeakers = self.isSpeakers;
    
    [PublicParameterComponent share].roomId = userModel.roomId;
    
    [[ToastComponent shareToastComponent] showLoading];
    __weak __typeof(self) wself = self;
    [VideoCallRTSManager joinRoom:userModel
                            block:^(NSString * _Nonnull token,
                                    NSInteger duration,
                                    RTSACKModel * _Nonnull model) {
        if (model.result) {
            [wself jumpToRoomVC:userModel
                       rtcToken:token
                       duration:duration];
        } else {
            AlertActionModel *alertModel = [[AlertActionModel alloc] init];
            alertModel.title = LocalizedString(@"ok");
            [[AlertActionManager shareAlertActionManager] showWithMessage:model.message actions:@[alertModel]];
        }
        [[ToastComponent shareToastComponent] dismiss];
    }];
}

- (void)jumpToRoomVC:(VideoCallRoomUserModel *)localSession
            rtcToken:(NSString *)rtcToken
            duration:(NSInteger)duration {
    VideoCallRoomViewController *roomVC = [[VideoCallRoomViewController alloc] initWithVideoSession:localSession rtcToken:rtcToken duration:duration];
    UINavigationController *navController = [[UINavigationController alloc] initWithRootViewController:roomVC];
    navController.modalPresentationStyle = UIModalPresentationFullScreen;
    [self presentViewController:navController animated:YES completion:nil];
}

- (void)onClickEnableAudio:(BaseButton *)sender {
    if (sender.status != ButtonStatusIllegal) {
        sender.status = (sender.status == ButtonStatusActive) ? ButtonStatusNone : ButtonStatusActive;
    } else {
        AlertActionModel *alertCancelModel = [[AlertActionModel alloc] init];
        alertCancelModel.title = LocalizedString(@"cancel");
        AlertActionModel *alertModel = [[AlertActionModel alloc] init];
        alertModel.title = LocalizedString(@"ok");
        alertModel.alertModelClickBlock = ^(UIAlertAction * _Nonnull action) {
            if ([action.title isEqualToString:LocalizedString(@"ok")]) {
                [SystemAuthority autoJumpWithAuthorizationStatusWithType:AuthorizationTypeAudio];
            }
        };
        [[AlertActionManager shareAlertActionManager] showWithMessage:LocalizedString(@"microphone_permission_disabled") actions:@[alertCancelModel, alertModel]];
    }
}

- (void)onClickEnableVideo:(BaseButton *)sender {
    if (sender.status != ButtonStatusIllegal) {
        sender.status = (sender.status == ButtonStatusActive) ? ButtonStatusNone : ButtonStatusActive;
        BOOL isEnableVideo = (sender.status == ButtonStatusActive) ? NO : YES;
        self.videoView.hidden = !isEnableVideo;
        self.emptImageView.hidden = isEnableVideo;
        [[VideoCallRTCManager shareRtc] switchVideoCapture:isEnableVideo];
    } else {
        AlertActionModel *alertCancelModel = [[AlertActionModel alloc] init];
        alertCancelModel.title = LocalizedString(@"cancel");
        AlertActionModel *alertModel = [[AlertActionModel alloc] init];
        alertModel.title = LocalizedString(@"ok");
        alertModel.alertModelClickBlock = ^(UIAlertAction * _Nonnull action) {
            if ([action.title isEqualToString:LocalizedString(@"ok")]) {
                [SystemAuthority autoJumpWithAuthorizationStatusWithType:AuthorizationTypeCamera];
            }
        };
        [[AlertActionManager shareAlertActionManager] showWithMessage:LocalizedString(@"camera_permission_disabled") actions:@[alertCancelModel, alertModel]];
    }
}

- (void)onClickSet:(id)sender {
    self.isSpeakers = !self.isSpeakers;
    [self updateSetingBtnImage];
}

- (void)navBackAction:(BaseButton *)sender {
    [self.navigationController popViewControllerAnimated:YES];
}

#pragma mark - UITextField delegate

- (void)roomNumTextFieldChange:(UITextField *)textField {
    [self updateTextFieldChange:textField];
}

- (void)updateTextFieldChange:(UITextField *)textField {
    NSInteger tagNum = (self.roomIdTextField == textField) ? 3001 : 3002;
    UILabel *label = [self.view viewWithTag:tagNum];
    
    NSString *message = @"";
    BOOL isExceedMaximLength = NO;
    if (textField.text.length > TEXTFIELD_MAX_LENGTH) {
        textField.text = [textField.text substringToIndex:TEXTFIELD_MAX_LENGTH];
        isExceedMaximLength = YES;
    }
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(dismissErrorLabel:) object:textField];
    BOOL isIllegal = NO;
    if (self.roomIdTextField == textField) {
        isIllegal = ![LocalUserComponent isMatchNumber:textField.text];
    }
    if (isIllegal || isExceedMaximLength) {
        if (isIllegal) {
            message = LocalizedString(@"room_number_error_content_limit");
            [self updateEnterRoomButtonColor:NO];
        } else if (isExceedMaximLength) {
            [self performSelector:@selector(dismissErrorLabel:) withObject:textField afterDelay:2];
            message = LocalizedString(@"room_number_error_content_limit");
        } else {
            message = @"";
        }
    } else {
        BOOL isEnterEnable = self.roomIdTextField.text.length > 0;
        [self updateEnterRoomButtonColor:isEnterEnable];
        message = @"";
    }
    label.text = message;
}

- (void)dismissErrorLabel:(UITextField *)textField {
    NSInteger tagNum = (self.roomIdTextField == textField) ? 3001 : 3002;
    UILabel *label = [self.view viewWithTag:tagNum];
    label.text = @"";
}

#pragma mark - Private Action

- (void)updateButtonColor:(UIButton *)button {
    [button setImageEdgeInsets:UIEdgeInsetsMake(11, 11, 11, 11)];
    button.imageView.contentMode = UIViewContentModeScaleAspectFit;
    button.backgroundColor = [UIColor colorWithWhite:1 alpha:0.1];
    button.layer.masksToBounds = YES;
    button.layer.cornerRadius = 44/2;
}

- (void)updateEnterRoomButtonColor:(BOOL)isEnable {
    [self.enterRoomBtn setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    self.enterRoomBtn.backgroundColor = [UIColor colorFromHexString:@"#165DFF"];
    if (isEnable) {
        self.enterRoomBtn.alpha = 1.0;
    } else {
        self.enterRoomBtn.alpha = 0.4;
    }
}

- (void)addLineView:(UIView *)view {
    UIView *lineView = [[UIView alloc] init];
    lineView.backgroundColor = [UIColor colorFromHexString:@"#FFFFFF"];
    [self.view addSubview:lineView];
    [lineView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(630/2, 1));
        make.centerX.equalTo(self.view);
        make.top.mas_equalTo(view.mas_bottom).offset(0);
    }];
}

- (void)addErrorLabel:(UIView *)view tag:(NSInteger)tag {
    UILabel *label = [[UILabel alloc] init];
    label.tag = tag;
    label.text = @"";
    label.textColor = [UIColor colorFromHexString:@"#F53F3F"];
    label.adjustsFontSizeToFitWidth = YES;
    [self.view addSubview:label];
    [label mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(view);
        make.right.mas_lessThanOrEqualTo(-20);
        make.top.mas_equalTo(view.mas_bottom).offset(4);
    }];
}

- (void)initUIComponent {
    [self.view addSubview:self.videoView];
    [self.videoView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.edges.equalTo(self.view);
    }];
    
    [self.view addSubview:self.maskView];
    [self.maskView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.edges.equalTo(self.view);
    }];
    
    [self.view addSubview:self.logoImageView];
    [self.logoImageView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(106, 20));
        make.centerX.equalTo(self.view);
        make.top.mas_equalTo(128/2 + [DeviceInforTool getStatusBarHight]);
    }];
    
    [self.view addSubview:self.emptImageView];
    [self.emptImageView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.width.height.mas_equalTo(120);
        make.centerX.equalTo(self.view);
        make.top.mas_equalTo(self.logoImageView.mas_bottom).offset(50);
    }];
    
    [self.maskView addGestureRecognizer:self.tap];
    
    [self.view addSubview:self.enableAudioBtn];
    [self.enableAudioBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(44, 44));
        make.left.mas_equalTo(123/2);
        make.bottom.mas_equalTo(-120/2 - [DeviceInforTool getVirtualHomeHeight]);
    }];
    
    [self.view addSubview:self.enableVideoBtn];
    [self.enableVideoBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(44, 44));
        make.centerX.equalTo(self.view);
        make.bottom.mas_equalTo(-120/2 - [DeviceInforTool getVirtualHomeHeight]);
    }];
    
    [self.view addSubview:self.setingBtn];
    [self.setingBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(44, 44));
        make.right.mas_equalTo(-123/2);
        make.bottom.mas_equalTo(-120/2 - [DeviceInforTool getVirtualHomeHeight]);
    }];
    
    [self.view addSubview:self.enterRoomBtn];
    [self.enterRoomBtn mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(630/2, 100/2));
        make.centerX.equalTo(self.view);
        make.bottom.equalTo(self.view).offset(-288/2 - [DeviceInforTool getVirtualHomeHeight]);
    }];
    
    [self.view addSubview:self.roomIdTextField];
    [self.roomIdTextField mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(630/2, (40 + 46)/2));
        make.centerX.equalTo(self.view);
        make.bottom.equalTo(self.enterRoomBtn.mas_top).offset(-40);
    }];
    
    [self.view addSubview:self.verLabel];
    [self.verLabel mas_makeConstraints:^(MASConstraintMaker *make) {
        make.centerX.equalTo(self.view);
        make.bottom.equalTo(self.view).offset(-([DeviceInforTool getVirtualHomeHeight] + 20));
    }];
    
    [self.view addSubview:self.navLeftButton];
    [self.navLeftButton mas_makeConstraints:^(MASConstraintMaker *make) {
        make.height.width.mas_equalTo(16);
        make.left.mas_equalTo(16);
        make.top.equalTo(self.view).offset([DeviceInforTool getStatusBarHight] + 16);
    }];
    
    [self addLineView:self.roomIdTextField];
    
    [self addErrorLabel:self.roomIdTextField tag:3001];
}

- (void)authorizationStatusMicAndCamera {
    [SystemAuthority authorizationStatusWithType:AuthorizationTypeAudio block:^(BOOL isAuthorize) {
        if (!isAuthorize) {
            self.enableAudioBtn.status = ButtonStatusIllegal;
        }
    }];
    
    [SystemAuthority authorizationStatusWithType:AuthorizationTypeCamera block:^(BOOL isAuthorize) {
        if (!isAuthorize) {
            self.emptImageView.hidden = NO;
            self.enableVideoBtn.status = ButtonStatusIllegal;
        }
    }];
}

- (void)updateSetingBtnImage {
    if (self.isSpeakers) {
        [self.setingBtn setImage:[UIImage imageNamed:@"login_speakers" bundleName:HomeBundleName] forState:UIControlStateNormal];
    } else {
        [self.setingBtn setImage:[UIImage imageNamed:@"login_earpieces" bundleName:HomeBundleName] forState:UIControlStateNormal];
    }
}

#pragma mark - Getter

- (UITextField *)roomIdTextField {
    if (!_roomIdTextField) {
        _roomIdTextField = [[UITextField alloc] init];
        _roomIdTextField.delegate = self;
        [_roomIdTextField setBackgroundColor:[UIColor clearColor]];
        [_roomIdTextField setTextColor:[UIColor whiteColor]];
        _roomIdTextField.font = [UIFont systemFontOfSize:16 weight:UIFontWeightRegular];
        _roomIdTextField.keyboardType = UIKeyboardTypeNumberPad;
        [_roomIdTextField addTarget:self action:@selector(roomNumTextFieldChange:) forControlEvents:UIControlEventEditingChanged];
        NSString *message = LocalizedString(@"please_enter_room_number");
        NSMutableAttributedString *attrString = [[NSMutableAttributedString alloc] initWithString:message];
        
        [attrString addAttributes:@{NSForegroundColorAttributeName : [UIColor colorFromHexString:@"#FFFFFF"]}
                            range:NSMakeRange(0, message.length)];
        [attrString addAttributes:@{NSFontAttributeName : [UIFont systemFontOfSize:14]}
                            range:NSMakeRange(0, message.length)];
        
        _roomIdTextField.attributedPlaceholder = attrString;
        _roomIdTextField.text = @"";
    }
    return _roomIdTextField;
}

- (BaseButton *)enableAudioBtn {
    if (!_enableAudioBtn) {
        _enableAudioBtn = [[BaseButton alloc] init];
        [_enableAudioBtn bingImage:[UIImage imageNamed:@"room_mic" bundleName:HomeBundleName] status:ButtonStatusNone];
        [_enableAudioBtn bingImage:[UIImage imageNamed:@"room_mic_s" bundleName:HomeBundleName] status:ButtonStatusActive];
        [_enableAudioBtn bingImage:[UIImage imageNamed:@"room_mic_s" bundleName:HomeBundleName] status:ButtonStatusIllegal];
        [_enableAudioBtn addTarget:self action:@selector(onClickEnableAudio:) forControlEvents:UIControlEventTouchUpInside];
        [self updateButtonColor:_enableAudioBtn];
    }
    return _enableAudioBtn;
}

- (BaseButton *)enableVideoBtn {
    if (!_enableVideoBtn) {
        _enableVideoBtn = [[BaseButton alloc] init];
        [_enableVideoBtn bingImage:[UIImage imageNamed:@"login_video" bundleName:HomeBundleName] status:ButtonStatusNone];
        [_enableVideoBtn bingImage:[UIImage imageNamed:@"room_video_s" bundleName:HomeBundleName] status:ButtonStatusActive];
        [_enableVideoBtn bingImage:[UIImage imageNamed:@"room_video_s" bundleName:HomeBundleName] status:ButtonStatusIllegal];
        [_enableVideoBtn addTarget:self action:@selector(onClickEnableVideo:) forControlEvents:UIControlEventTouchUpInside];
        [self updateButtonColor:_enableVideoBtn];
    }
    return _enableVideoBtn;
}

- (UIButton *)setingBtn {
    if (!_setingBtn) {
        _setingBtn = [[UIButton alloc] init];
        [_setingBtn setImage:[UIImage imageNamed:@"login_speakers" bundleName:HomeBundleName] forState:UIControlStateNormal];
        [_setingBtn addTarget:self action:@selector(onClickSet:) forControlEvents:UIControlEventTouchUpInside];
    }
    return _setingBtn;
}

- (UIButton *)enterRoomBtn {
    if (!_enterRoomBtn) {
        _enterRoomBtn = [[UIButton alloc] init];
        _enterRoomBtn.backgroundColor = [UIColor clearColor];
        _enterRoomBtn.layer.masksToBounds = YES;
        _enterRoomBtn.layer.cornerRadius = 50/2;
        [_enterRoomBtn setTitle:LocalizedString(@"enter_the_room") forState:UIControlStateNormal];
        _enterRoomBtn.titleLabel.font = [UIFont systemFontOfSize:16 weight:UIFontWeightRegular];
        [_enterRoomBtn addTarget:self action:@selector(onClickEnterRoom:) forControlEvents:UIControlEventTouchUpInside];
        [self updateEnterRoomButtonColor:NO];
    }
    return _enterRoomBtn;
}

- (UIView *)videoView {
    if (!_videoView) {
        _videoView = [[UIView alloc] init];
    }
    return _videoView;
}

- (UIImageView *)logoImageView {
    if (!_logoImageView) {
        _logoImageView = [[UIImageView alloc] init];
        _logoImageView.image = [UIImage imageNamed:@"icon" bundleName:HomeBundleName];
        _logoImageView.contentMode = UIViewContentModeScaleAspectFit;
    }
    return _logoImageView;
}

- (UITapGestureRecognizer *)tap {
    if (!_tap) {
        _tap = [[UITapGestureRecognizer alloc] initWithTarget:self
                                                       action:@selector(tapGestureAction:)];
    }
    return _tap;
}

- (UIView *)maskView {
    if (!_maskView) {
        _maskView = [[UIView alloc] init];
        _maskView.backgroundColor = [UIColor colorFromRGBHexString:@"#101319" andAlpha:0.2 * 255];
        _maskView.userInteractionEnabled = YES;
    }
    return _maskView;
}

- (UIImageView *)emptImageView {
    if (!_emptImageView) {
        _emptImageView = [[UIImageView alloc] init];
        _emptImageView.image = [UIImage imageNamed:@"login_empt" bundleName:HomeBundleName];
        _emptImageView.hidden = YES;
    }
    return _emptImageView;
}

- (UILabel *)verLabel {
    if (!_verLabel) {
        _verLabel = [[UILabel alloc] init];
        _verLabel.textColor = [UIColor colorFromHexString:@"#FFFFFF"];
        _verLabel.font = [UIFont systemFontOfSize:12 weight:UIFontWeightRegular];
    }
    return _verLabel;
}

- (BaseButton *)navLeftButton {
    if (!_navLeftButton) {
        _navLeftButton = [[BaseButton alloc] init];
        [_navLeftButton setImage:[UIImage imageNamed:@"nav_left" bundleName:HomeBundleName] forState:UIControlStateNormal];
        _navLeftButton.contentHorizontalAlignment = UIControlContentHorizontalAlignmentLeft;
        [_navLeftButton addTarget:self action:@selector(navBackAction:) forControlEvents:UIControlEventTouchUpInside];
    }
    return _navLeftButton;
}

- (void)dealloc {
    [[VideoCallRTCManager shareRtc] disconnect];
    [PublicParameterComponent clear];
}

@end
