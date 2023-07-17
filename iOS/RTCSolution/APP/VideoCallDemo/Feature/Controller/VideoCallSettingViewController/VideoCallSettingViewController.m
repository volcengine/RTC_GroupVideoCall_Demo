// 
// Copyright (c) 2023 Beijing Volcano Engine Technology Ltd.
// SPDX-License-Identifier: MIT
// 

#import "VideoCallMockDataComponent.h"
#import "VideoCallSettingsRightLabelCell.h"
#import "VideoCallSettingsSwitchCell.h"
#import "VideoCallSettingViewController.h"
#import "VideoCallPickerComponent.h"
#import "VideoCallRTCManager.h"

static NSString *const kVideoCallSettingsRightLabelCellIdentifier = @"kVideoCallSettingsRightLabelCellIdentifier";
static NSString *const kVideoCallSettingsSwitchCellIdentifier = @"kVideoCallSettingsSwitchCellIdentifier";

typedef NS_ENUM(NSInteger, SettingsGroupType) {
    SettingsGroupTypeResolution,
    SettingsGroupTypeAudioProfile,
    SettingsGroupTypeMirror,
};

@interface VideoCallSettingViewController () <UITableViewDelegate, UITableViewDataSource>
@property (nonatomic, strong) UITableView *settingsTableView;
@property (nonatomic, copy) NSArray *groupTypes;
@property (nonatomic, strong) VideoCallPickerComponent *resolutionPicker;
@property (nonatomic, strong) VideoCallPickerComponent *audioProfilePicker;
@property (nonatomic, assign) BOOL isOpenMirror;

@end

@implementation VideoCallSettingViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor clearColor];
    self.navView.backgroundColor = [UIColor colorFromHexString:@"#1D2129"];
    self.groupTypes = @[LocalizedString(@"resolution"),
                        LocalizedString(@"call_quality"),
                        LocalizedString(@"local_mirror")];

    [self createUIComponent];
    [self.settingsTableView reloadData];
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];

    self.navTitle = LocalizedString(@"set_up");
}

- (void)createUIComponent {
    [self.view addSubview:self.settingsTableView];
    [self.settingsTableView mas_makeConstraints:^(MASConstraintMaker *make) {
        make.top.equalTo(self.navView.mas_bottom);
        make.left.right.bottom.equalTo(self.view);
    }];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [UITableViewCell new];
    VideoCallMockDataComponent *mockData = [VideoCallMockDataComponent shared];
    switch (indexPath.row) {
        case SettingsGroupTypeResolution: {
            cell = [tableView dequeueReusableCellWithIdentifier:kVideoCallSettingsRightLabelCellIdentifier forIndexPath:indexPath];
            VideoCallSettingsRightLabelCell *labelCell = (VideoCallSettingsRightLabelCell *)cell;
            [labelCell settingsLabel].text = self.groupTypes[indexPath.row];
            [labelCell settingsRightLabel].text = mockData.currentResolutionDic[@"title"];
        } break;
        case SettingsGroupTypeAudioProfile: {
            cell = (VideoCallSettingsRightLabelCell *)[tableView dequeueReusableCellWithIdentifier:kVideoCallSettingsRightLabelCellIdentifier forIndexPath:indexPath];
            VideoCallSettingsRightLabelCell *labelCell = (VideoCallSettingsRightLabelCell *)cell;
            [labelCell settingsLabel].text = self.groupTypes[indexPath.row];
            [labelCell settingsRightLabel].text = mockData.currentaudioProfileDic[@"title"];
        } break;
            
        case SettingsGroupTypeMirror: {
            cell = (VideoCallSettingsSwitchCell *)[tableView dequeueReusableCellWithIdentifier:kVideoCallSettingsSwitchCellIdentifier forIndexPath:indexPath];
            VideoCallSettingsSwitchCell *mirrorCell = (VideoCallSettingsSwitchCell *)cell;
            [mirrorCell settingsLabel].text = self.groupTypes[indexPath.row];
            [mirrorCell setSwitchOn:mockData.isOpenMirror];
            __weak __typeof(self) wself = self;
            [mirrorCell switchValueChangeCallback:^(BOOL on) {
                mockData.isOpenMirror = on;
                [wself updateAudioAndVideoSettings];
            }];
        } break;
        default:
            break;
    }
    if (cell) {
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    VideoCallMockDataComponent *mockData = [VideoCallMockDataComponent shared];
    [tableView deselectRowAtIndexPath:indexPath animated:NO];
    switch (indexPath.row) {
        case SettingsGroupTypeResolution: {
            [self.resolutionPicker show:mockData.resLists
                             selectItem:mockData.currentResolutionDic];
            __weak __typeof(self) wself = self;
            self.resolutionPicker.clickDismissBlock = ^(BOOL isCancel, id selectItem, NSInteger row) {
                if (!isCancel) {
                    [VideoCallMockDataComponent shared].currentResolutionDic = [VideoCallMockDataComponent shared].resLists[row];
                    [wself.settingsTableView reloadData];
                    [wself updateAudioAndVideoSettings];
                }
                wself.resolutionPicker = nil;
            };
        } break;
        case SettingsGroupTypeAudioProfile: {
            [self.audioProfilePicker show:mockData.audioProfileLists
                               selectItem:mockData.currentaudioProfileDic];
            __weak __typeof(self) wself = self;
            self.audioProfilePicker.clickDismissBlock = ^(BOOL isCancel, id selectItem, NSInteger row) {
                if (!isCancel) {
                    [VideoCallMockDataComponent shared].currentaudioProfileDic = [VideoCallMockDataComponent shared].audioProfileLists[row];
                    [wself.settingsTableView reloadData];
                    [wself updateAudioAndVideoSettings];
                }
                wself.audioProfilePicker = nil;
            };
        } break;
        
        default:
            break;
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    return 119/2;
}

#pragma mark - Private Action

- (void)updateAudioAndVideoSettings {
    [[VideoCallRTCManager shareRtc] updateAudioAndVideoSettings];
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.groupTypes.count;
}

#pragma mark - Getter

- (UITableView *)settingsTableView {
    if (!_settingsTableView) {
        _settingsTableView = [[UITableView alloc] init];
        _settingsTableView.separatorStyle = UITableViewCellSeparatorStyleNone;
        _settingsTableView.delegate = self;
        _settingsTableView.dataSource = self;
        [_settingsTableView registerClass:VideoCallSettingsRightLabelCell.class forCellReuseIdentifier:kVideoCallSettingsRightLabelCellIdentifier];
        [_settingsTableView registerClass:VideoCallSettingsSwitchCell.class forCellReuseIdentifier:kVideoCallSettingsSwitchCellIdentifier];
        _settingsTableView.backgroundColor = [UIColor colorFromHexString:@"#1D2129"];
    }
    return _settingsTableView;
}

- (VideoCallPickerComponent *)resolutionPicker {
    if (!_resolutionPicker) {
        _resolutionPicker = [[VideoCallPickerComponent alloc] initWithHeight:566/2 superView:self.view];
        _resolutionPicker.titleStr = LocalizedString(@"resolution");
    }
    return _resolutionPicker;
}

- (VideoCallPickerComponent *)audioProfilePicker {
    if (!_audioProfilePicker) {
        _audioProfilePicker = [[VideoCallPickerComponent alloc] initWithHeight:566/2 superView:self.view];
        _audioProfilePicker.titleStr = LocalizedString(@"call_quality");
    }
    return _audioProfilePicker;
}

#pragma mark - Tool

@end
