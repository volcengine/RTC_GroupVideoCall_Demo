// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallFullScreenView.h"
#import "VideoCallRTCManager.h"
#import "VideoCallNameView.h"

@interface VideoCallFullScreenView ()

@property (nonatomic, strong) UIView *renderScreenView;
@property (nonatomic, strong) BaseButton *orientationButton;
@property (nonatomic, strong) UIView *userTagView;
@property (nonatomic, strong) UIView *userTagLandscapeView;
@property (nonatomic, strong) VideoCallNameView *nameView;

@property (nonatomic, copy) void (^dismissBlock)(BOOL isDismiss);
@property (nonatomic, assign) BOOL isLandscape;

@end

@implementation VideoCallFullScreenView

- (instancetype)init {
    self = [super init];
    if (self) {
        self.backgroundColor = [UIColor colorFromHexString:@"#101319"];
        self.hidden = YES;
        self.isLandscape = NO;
        self.userInteractionEnabled = YES;
        UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(fullClickActione)];
        [self addGestureRecognizer:tap];
        
        [self addConstraints];
    }
    return self;
}

#pragma mark - Publish Action

- (void)show:(NSString *)uid
    userName:(NSString *)userName
      roomId:(NSString *)roomId
       block:(void (^)(BOOL isRemove))block {
    if (self.hidden) {
        self.hidden = NO;
        self.dismissBlock = block;
        
        UIView *screenRenderView = [[VideoCallRTCManager shareRtc] getScreenStreamViewWithUid:uid];
        screenRenderView.hidden = NO;
        [self.renderScreenView addSubview:screenRenderView];
        [screenRenderView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.edges.equalTo(self.renderScreenView);
        }];
        
        [self.nameView setName:userName];
        VideoCallNameView *nameView = [self.userTagLandscapeView viewWithTag:3001];
        [nameView setName:userName];
    }
}

- (void)dismiss:(BOOL)isRemove {
    if (!self.hidden) {
        self.isLandscape = NO;
        self.hidden = YES;
        if (self.clickOrientationBlock) {
            self.clickOrientationBlock(YES);
        }
        if (self.dismissBlock) {
            self.dismissBlock(isRemove);
        }
    }
}

#pragma mark - Action Method

- (void)orientationButtonAction {
    if (self.clickOrientationBlock) {
        self.clickOrientationBlock(self.isLandscape);
    }
    self.isLandscape = !self.isLandscape;
}

- (void)fullClickActione {
    [self dismiss:NO];
}

#pragma mark - Private Action

- (void)setIsLandscape:(BOOL)isLandscape {
    _isLandscape = isLandscape;
    
    self.orientationButton.status = isLandscape ? ButtonStatusActive : ButtonStatusNone;
    self.userTagLandscapeView.hidden = isLandscape ? NO : YES;
    self.nameView.hidden = isLandscape ? YES : NO;
}

- (void)addConstraints {
    [self addSubview:self.userTagLandscapeView];
    [self addSubview:self.renderScreenView];
    [self addSubview:self.orientationButton];
    [self addSubview:self.nameView];
    
    [self.userTagLandscapeView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.top.right.equalTo(self);
        make.height.mas_equalTo(30);
    }];
    
    [self.renderScreenView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(self.userTagLandscapeView.mas_bottom);
        make.left.right.bottom.equalTo(self);
    }];

    [self.orientationButton mas_makeConstraints:^(MASConstraintMaker *make) {
        make.size.mas_equalTo(CGSizeMake(44, 44));
        make.right.bottom.mas_equalTo(-32/2);
    }];
    
    [self.nameView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.left.equalTo(self).offset(4.f);
        make.bottom.equalTo(self).offset(-4.f);
    }];
}


#pragma mark - Getter

- (VideoCallNameView *)nameView {
    if (!_nameView) {
        _nameView = [[VideoCallNameView alloc] init];
    }
    return _nameView;
}

- (UIView *)userTagLandscapeView {
    if (!_userTagLandscapeView) {
        _userTagLandscapeView = [[UIView alloc] init];
        _userTagLandscapeView.backgroundColor = [UIColor clearColor];
        _userTagLandscapeView.hidden = YES;
        
        VideoCallNameView *nameView = [[VideoCallNameView alloc] init];
        nameView.tag = 3001;
        [_userTagLandscapeView addSubview:nameView];
        [nameView mas_makeConstraints:^(MASConstraintMaker *make) {
            make.center.equalTo(_userTagLandscapeView);
        }];
    }
    return _userTagLandscapeView;
}

- (BaseButton *)orientationButton {
    if (!_orientationButton) {
        _orientationButton = [[BaseButton alloc] init];
        [_orientationButton addTarget:self action:@selector(orientationButtonAction) forControlEvents:UIControlEventTouchUpInside];
        [_orientationButton bingImage:[UIImage imageNamed:@"room_orientation" bundleName:HomeBundleName] status:ButtonStatusNone];
        [_orientationButton bingImage:[UIImage imageNamed:@"room_orientation_v" bundleName:HomeBundleName] status:ButtonStatusActive];
    }
    return _orientationButton;
}

- (UIView *)renderScreenView {
    if (!_renderScreenView) {
        _renderScreenView = [[UIView alloc] init];
        _renderScreenView.backgroundColor = [UIColor clearColor];
    }
    return _renderScreenView;
}

@end
