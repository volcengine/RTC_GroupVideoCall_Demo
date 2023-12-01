import Tea from '@datarangers/sdk-javascript';
import { EffectBeautyMode } from '@volcengine/rtc/extension-beauty';
import { isDev, isProd, isBoe, ENABLE_TEA, TEA_APP_ID_VOLC, TEA_CHANNEL_VOLC } from '@/config';
import Utils from '@/utils/utils';

const TEA_ENV = isProd ? 'prod' : isBoe ? 'boe' : isDev ? 'dev-opensource' : 'unknown';

export interface LoginInfo {
  user_id: string;
  user_name: string;
  room_id: string;
  device_id: string;
}

export const enum TeaEvents {
  TOGGLE_CAMERA = 'toggle_camera',
  TOGGLE_MICROPHONE = 'toggle_microphone',
  TOGGLE_SETTING_MENU = 'toggle_setting_menu',
  UPDATE_SETTING_OPTIONS = 'update_setting_options',
  USER_LOGIN = 'user_login',
  SWITCH_MICROPHONE = 'switch_microphone',
  SWITCH_CAMERA = 'switch_camera',
  TOGGLE_STATISTICS_MENU = 'toggle_statistics_menu',
  TOGGLE_STATISTICS_MEDIA_TYPE = 'toggle_statistics_media_type',
  TOGGLE_SHARE_SCREEN = 'toggle_share_screen',
  START_SHARING = 'start_sharing',
  STOP_SHARING = 'stop_sharing',
  SWITCH_SHARE_SCREEN_PREFERENCE = 'switch_share_screen_preference',
  TOGGLE_BEAUTY = 'toggle_beauty',
  UPDATE_BEAUTY_OPTIONS = 'update_beauty_options',
  OPEN_LEAVE_ROOM_MENU = 'open_leave_room_menu',
  CONFIRM_LEAVE_ROOM = 'confirm_leave_room',
  RTC_SDK_API_CALL_FAILURE = 'rtc_sdk_api_call_failure',
  RTC_SDK_EXCEPTION = 'rtc_sdk_exception',
  DEMO_EXCEPTION = 'demo_exception',
}

export const enum TeaEventSource {
  LOGIN_PAGE = 'login_page',
  VIEW_PAGE = 'view_page',
  LEAVE_ROOM_MENU = 'leave_room_menu',
  STATISTIC_MENU = 'statistic_menu',
  PAGE_REFRESH = 'page_refresh',
  BROWSER = 'browser',
}

export class TeaClient {
  private _reportId = 0;

  private _userId?: string;

  private _userName?: string;

  private _deivceId?: string;

  private _roomId?: string;

  constructor() {
    Tea.init({
      app_id: TEA_APP_ID_VOLC,
      channel: TEA_CHANNEL_VOLC,
      log: true,
    });

    this._deivceId = Utils.getDeviceId();
    Tea.start();
  }

  setLoginInfo = (userName: string, roomId: string) => {
    this._userName = userName;
    this._roomId = roomId;
  };

  updateLoginInfo = (loginParams: LoginInfo) => {
    this._userId = loginParams.user_id;
    this._userName = loginParams.user_name;
    this._deivceId = loginParams.device_id;
    this._roomId = loginParams.room_id;
  };

  // 每次事件上报共同的信息
  _getCommonTeaBeaconInfo = () => {
    return {
      timestamp: Date.now(),
      report_id: this._reportId++,
      user_id: this._userId,
      user_name: this._userName,
      room_id: this._roomId,
      device_id: this._deivceId,
      env: TEA_ENV,
    };
  };

  // 适用于正常事件, 进入队列，按照一定间隔转发
  sendTeaBeacon = (eventName: TeaEvents, paramsJson: object) => {
    if (ENABLE_TEA && typeof Tea.event === 'function') {
      const commonReports = this._getCommonTeaBeaconInfo();
      Tea.event(eventName, {
        ...paramsJson,
        ...commonReports,
      });
    }
  };

  // 适用于用户离开页面, 及时转发
  sendTeaBeaconImmediately = (eventName: TeaEvents, paramsJson: object) => {
    if (ENABLE_TEA && typeof Tea.beconEvent === 'function') {
      const commonReports = this._getCommonTeaBeaconInfo();
      Tea.beconEvent(eventName, {
        ...paramsJson,
        ...commonReports,
      });
    }
  };

  // SDK API 调用失败数据上报
  reportRTCSdkAPIFailure = (apiName: string, message: string) => {
    this.sendTeaBeacon(TeaEvents.RTC_SDK_API_CALL_FAILURE, {
      sdk_api_name: apiName,
      message,
    });
  };

  // SDK API 调用异常数据上报
  reportRTCSdkException = (message: string) => {
    this.sendTeaBeacon(TeaEvents.RTC_SDK_EXCEPTION, {
      message,
    });
  };

  // Demo 内部异常数据上报
  reportDemoInternalException = (message: string) => {
    this.sendTeaBeacon(TeaEvents.DEMO_EXCEPTION, {
      message,
    });
  };

  // 用户点击 结束通话 button
  reportOpenUserLeaveRoomMenu = () => {
    this.sendTeaBeacon(TeaEvents.OPEN_LEAVE_ROOM_MENU, {
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户是否确定离房
  reportUserLeaveRoom = (isLeave: boolean) => {
    this.sendTeaBeacon(TeaEvents.CONFIRM_LEAVE_ROOM, {
      confirm_status: isLeave ? 'leave' : 'stay',
      source: TeaEventSource.LEAVE_ROOM_MENU,
    });
  };

  // 用户 toggle 美颜 button
  reportToggleBeauty = (toOn: boolean) => {
    this.sendTeaBeacon(TeaEvents.TOGGLE_BEAUTY, {
      toggle_status: toOn ? 'on' : 'off',
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户更新美颜选项
  reportUpdateBeautyOptions = (
    updatingOption: EffectBeautyMode,
    whitePercent: number,
    smoothPercent: number,
    sharpenPercent: number
  ) => {
    this.sendTeaBeacon(TeaEvents.UPDATE_BEAUTY_OPTIONS, {
      white_percent: whitePercent,
      smooth_percent: smoothPercent,
      sharpen_percent: sharpenPercent,
      beauty_option: updatingOption,
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户切换屏幕共享偏好
  reportSwitchShareScreenPreferenceToClarity = (isClarityPreferred: boolean) => {
    this.sendTeaBeacon(TeaEvents.SWITCH_SHARE_SCREEN_PREFERENCE, {
      target_preference: isClarityPreferred ? 'clarity' : 'fluency',
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户 toggle 屏幕共享 button
  reportToggleShareScreen = () => {
    this.sendTeaBeacon(TeaEvents.TOGGLE_SHARE_SCREEN, {
      toggle_status: 'on', // always on
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户开始共享屏幕
  reportStartScreenSharing = () => {
    this.sendTeaBeacon(TeaEvents.START_SHARING, {
      source: TeaEventSource.BROWSER,
    });
  };

  // 用户点击结束共享屏幕
  reportStopScreenSharing = () => {
    this.sendTeaBeacon(TeaEvents.STOP_SHARING, {
      source: TeaEventSource.BROWSER,
    });
  };

  // 用户 toggle 实时数据 button
  reportToggleStatisticMenu = (toOn: boolean) => {
    this.sendTeaBeacon(TeaEvents.TOGGLE_STATISTICS_MENU, {
      toggle_status: toOn ? 'on' : 'off',
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户切换 video / audio 数据显示
  reportToggleStatisticMediaType = (dataType: string) => {
    this.sendTeaBeacon(TeaEvents.TOGGLE_STATISTICS_MEDIA_TYPE, {
      toggle_status: dataType,
      source: TeaEventSource.STATISTIC_MENU,
    });
  };

  // 用户 toggle View page 的 摄像头 button
  reportToggleCameraButton = (toOn: boolean, source: TeaEventSource) => {
    this.sendTeaBeacon(TeaEvents.TOGGLE_CAMERA, {
      toggle_status: toOn ? 'on' : 'off',
      source,
    });
  };

  // 用户切换摄像头
  reportSwitchCamera = (
    targetDevice: string,
    currentDevice: string | undefined,
    deviceList: MediaDeviceInfo[]
  ) => {
    this.sendTeaBeacon(TeaEvents.SWITCH_CAMERA, {
      target_device_id: targetDevice,
      prev_device_id: currentDevice,
      device_list: deviceList,
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户 toggle 麦克风 button
  reportToggleMicrophoneButton = (toOn: boolean, source: TeaEventSource) => {
    this.sendTeaBeacon(TeaEvents.TOGGLE_MICROPHONE, {
      toggle_status: toOn ? 'on' : 'off',
      source,
    });
  };

  // 用户切换麦克风
  reportSwitchMicrophone = (
    targetDevice: string,
    currentDevice: string | undefined,
    deviceList: MediaDeviceInfo[]
  ) => {
    this.sendTeaBeacon(TeaEvents.SWITCH_MICROPHONE, {
      target_device_id: targetDevice,
      prev_device_id: currentDevice || '',
      device_list: deviceList,
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户点击 login button 登录
  reportUserLogin = (res: boolean, reason: string, source: string) => {
    if (res) {
      this.sendTeaBeacon(TeaEvents.USER_LOGIN, {
        result: 'succ',
        source,
      });
    } else {
      this.sendTeaBeacon(TeaEvents.USER_LOGIN, {
        result: 'fail',
        reason,
        source,
      });
    }
  };

  // 用户 toggle Setting Menu button
  reportToggleSettingMenu = (toOn: boolean) => {
    this.sendTeaBeacon(TeaEvents.TOGGLE_SETTING_MENU, {
      toggle_status: toOn ? 'on' : 'off',
      source: TeaEventSource.VIEW_PAGE,
    });
  };

  // 用户更新 Setting Menu 的内容
  reportUpdateSettingOptions = (resolution: string, soundQuality: string, mirrowing: string) => {
    this.sendTeaBeacon(TeaEvents.UPDATE_SETTING_OPTIONS, {
      resolution,
      sound_quality: soundQuality,
      mirrowing,
      source: TeaEventSource.VIEW_PAGE,
    });
  };
}

export default new TeaClient();
