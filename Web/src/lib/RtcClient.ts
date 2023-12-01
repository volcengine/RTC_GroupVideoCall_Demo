import VERTC, {
  VideoRenderMode,
  IRTCEngine,
  StreamIndex,
  RoomProfileType,
  onUserJoinedEvent,
  onUserLeaveEvent,
  MediaType,
  LocalStreamStats,
  RemoteStreamStats,
  StreamRemoveReason,
  LocalAudioPropertiesInfo,
  RemoteAudioPropertiesInfo,
  MirrorType,
  AudioProfileType,
  VideoEncoderConfig,
  DeviceInfo,
  UserMessageEvent,
  TrackCaptureConfig,
  AutoPlayFailedEvent,
  PlayerEvent,
  ScreenEncoderConfig,
} from '@volcengine/rtc';
import { v4 as uuid } from 'uuid';
import RTCBeautyExtension from '@volcengine/rtc/extension-beauty';
import Utils from '@/utils/utils';
import { BasicBody } from '@/app/baseQuery';
import TeaClient from '@/lib/TeaClient';

export interface IEventListener {
  handleTrackEnded: (e: { kind: string; isScreen: boolean }) => void;
  handleError: (e: { errorCode: any }) => void;
  handleUserJoin: (e: onUserJoinedEvent) => void;
  handleUserLeave: (e: onUserLeaveEvent) => void;
  handleUserPublishStream: (e: { userId: string; mediaType: MediaType }) => void;
  handleUserUnpublishStream: (e: {
    userId: string;
    mediaType: MediaType;
    reason: StreamRemoveReason;
  }) => void;
  handleUserStartVideoCapture: (e: { userId: string }) => void;
  handleUserStopVideoCapture: (e: { userId: string }) => void;
  handleUserPublishScreen: (e: { userId: string; mediaType: MediaType }) => void;
  handleUserUnpublishScreen: (e: {
    userId: string;
    mediaType: MediaType;
    reason: StreamRemoveReason;
  }) => void;
  handleRemoteStreamStats: (e: RemoteStreamStats) => void;
  handleLocalStreamStats: (e: LocalStreamStats) => void;
  handleLocalAudioPropertiesReport: (e: LocalAudioPropertiesInfo[]) => void;
  handleRemoteAudioPropertiesReport: (e: RemoteAudioPropertiesInfo[]) => void;
  handleVideoDeviceStateChanged: (e: DeviceInfo) => void;
  handleAudioDeviceStateChanged: (e: DeviceInfo) => void;
  handleUserMessageReceivedOutsideRoom: (e: { userId: string; message: any }) => void;
  handleUserMessageReceived: (e: { userId: string; message: any }) => void;
  handleRoomMessageReceived: (e: { userId: string; message: any }) => void;
  handleAutoPlayFail: (e: AutoPlayFailedEvent) => void;
  handlePlayerEvent: (e: PlayerEvent) => void;
}

interface EngineOptions {
  appId: string;
  uid: string;
  rtsUid: string;
  roomId: string;
  rtmToken: string;
  serverUrl: string;
  serverSignature: string;
  bid: string;
}

const DefaultCaptureConfig = {
  width: 1280,
  height: 720,
  frameRate: 15,
};

const DefaultEncoderConfig = {
  width: 1280,
  height: 720,
  frameRate: 15,
  maxKbps: 1200,
};

export const beautyExtension = new RTCBeautyExtension();

export const RtcSdkApiWrapper = async (sdkApiName: string, sdkApiCall: Function) => {
  try {
    sdkApiCall = sdkApiCall.bind(this);
    const result = sdkApiCall();

    // async function
    if (result && result instanceof Promise) {
      return result.catch((error: any) => {
        TeaClient.reportRTCSdkAPIFailure(sdkApiName, JSON.stringify(error));
        return Promise.reject(error);
      });
    }

    // sync function
    return result;
  } catch (error: any) {
    const failureReason = error?.toString() || 'unknown';
    TeaClient.reportRTCSdkException(
      JSON.stringify({
        error: failureReason,
        sdkApiName,
      })
    );

    // still throw to print on console
    throw error;
  }
};

export class RtcClient {
  engine!: IRTCEngine;

  config!: EngineOptions;

  rtsBody!: BasicBody;

  private _videoCaptureDevice?: string;

  private _audioCaptureDevice?: string;

  private _audioPlaybackDevice?: string;

  private _captureConfig: TrackCaptureConfig = DefaultCaptureConfig;

  private _encoderConfig: VideoEncoderConfig | VideoEncoderConfig[] = DefaultEncoderConfig;

  beautyEnabled: boolean = false;

  createEngine = async (props: EngineOptions) => {
    this.config = props;
    this.rtsBody = {
      room_id: props.roomId,
      user_id: props.rtsUid,
      login_token: Utils.getLoginToken(),
    };

    this.engine = await RtcSdkApiWrapper('createEngine', () => {
      return VERTC.createEngine(this.config.appId);
    });

    try {
      await this.engine.registerExtension(beautyExtension).catch((e) => {
        TeaClient.reportRTCSdkAPIFailure('registerExtension', JSON.stringify(e));
        throw e;
      });
      beautyExtension.disableBeauty();
      this.beautyEnabled = true;
    } catch (e) {
      console.error((e as any).message);
      this.beautyEnabled = false;
      TeaClient.reportRTCSdkException((e as any).message);
    }
  };

  joinWithRTS = async () => {
    await RtcSdkApiWrapper('login', () => {
      return this.engine.login(this.config.rtmToken, this.config.rtsUid);
    });

    await RtcSdkApiWrapper('setServerParams', () => {
      return this.engine.setServerParams(this.config.serverSignature, this.config.serverUrl);
    });
  };

  sendServerMessage = async (eventname: string) => {
    return new Promise((resolve, reject) => {
      const requestId = uuid();

      const content = {
        app_id: this.config.appId,
        device_id: Utils.getDeviceId(),
        room_id: this.config.roomId,
        user_id: this.config.rtsUid,
        request_id: requestId,
        event_name: eventname,
        content: JSON.stringify(this.rtsBody),
      };

      const callback = (e: UserMessageEvent) => {
        const { userId, message } = e;

        if (userId === 'server') {
          try {
            const res = JSON.parse(message as string);
            if (res.request_id === requestId) {
              this.engine.removeListener(VERTC.events.onUserMessageReceivedOutsideRoom, callback);

              resolve(res);
            }
          } catch (e) {
            reject(e);
          }
        }
      };

      this.engine.on(VERTC.events.onUserMessageReceivedOutsideRoom, callback);

      RtcSdkApiWrapper('sendServerMessage', () => {
        return this.engine.sendServerMessage(JSON.stringify(content));
      });
    });
  };

  addEventListeners = ({
    handleError,
    handleTrackEnded,
    handleUserJoin,
    handleUserLeave,
    handleUserPublishStream,
    handleUserUnpublishStream,
    handleUserStartVideoCapture,
    handleUserStopVideoCapture,
    handleUserPublishScreen,
    handleUserUnpublishScreen,
    handleRemoteStreamStats,
    handleLocalStreamStats,
    handleLocalAudioPropertiesReport,
    handleRemoteAudioPropertiesReport,
    handleVideoDeviceStateChanged,
    handleAudioDeviceStateChanged,
    handleUserMessageReceivedOutsideRoom,
    handleUserMessageReceived,
    handleRoomMessageReceived,
    handleAutoPlayFail,
    handlePlayerEvent,
  }: IEventListener) => {
    this.engine.on(VERTC.events.onError, handleError);
    this.engine.on(VERTC.events.onTrackEnded, handleTrackEnded);
    this.engine.on(VERTC.events.onUserJoined, handleUserJoin);
    this.engine.on(VERTC.events.onUserLeave, handleUserLeave);
    this.engine.on(VERTC.events.onUserPublishStream, handleUserPublishStream);
    this.engine.on(VERTC.events.onUserUnpublishStream, handleUserUnpublishStream);
    this.engine.on(VERTC.events.onUserStartVideoCapture, handleUserStartVideoCapture);
    this.engine.on(VERTC.events.onUserStopVideoCapture, handleUserStopVideoCapture);
    this.engine.on(VERTC.events.onUserPublishScreen, handleUserPublishScreen);
    this.engine.on(VERTC.events.onUserUnpublishScreen, handleUserUnpublishScreen);
    this.engine.on(VERTC.events.onRemoteStreamStats, handleRemoteStreamStats);
    this.engine.on(VERTC.events.onLocalStreamStats, handleLocalStreamStats);
    this.engine.on(VERTC.events.onVideoDeviceStateChanged, handleVideoDeviceStateChanged);
    this.engine.on(VERTC.events.onAudioDeviceStateChanged, handleAudioDeviceStateChanged);
    this.engine.on(VERTC.events.onLocalAudioPropertiesReport, handleLocalAudioPropertiesReport);
    this.engine.on(VERTC.events.onRemoteAudioPropertiesReport, handleRemoteAudioPropertiesReport);

    this.engine.on(
      VERTC.events.onUserMessageReceivedOutsideRoom,
      handleUserMessageReceivedOutsideRoom
    );
    this.engine.on(VERTC.events.onUserMessageReceived, handleUserMessageReceived);
    this.engine.on(VERTC.events.onRoomMessageReceived, handleRoomMessageReceived);
    this.engine.on(VERTC.events.onAutoplayFailed, handleAutoPlayFail);
    this.engine.on(VERTC.events.onPlayerEvent, handlePlayerEvent);
  };

  joinRoom = (token: string | null, username: string): Promise<void> => {
    this.engine.enableAudioPropertiesReport({ interval: 2000 });

    return RtcSdkApiWrapper('joinRoom', () => {
      return this.engine.joinRoom(
        token,
        `${this.config.roomId!}`,
        {
          userId: this.config.uid!,
          extraInfo: JSON.stringify({
            user_name: username,
            user_id: this.config.uid,
          }),
        },
        {
          isAutoPublish: true,
          isAutoSubscribeAudio: true,
          isAutoSubscribeVideo: true,
          roomProfileType: RoomProfileType.meeting,
        }
      );
    });
  };

  leaveRoom = async () => {
    await RtcSdkApiWrapper('leaveRoom', () => {
      this.engine.leaveRoom();
    });

    await RtcSdkApiWrapper('destroyEngine', () => {
      VERTC.destroyEngine(this.engine);
    });

    this._videoCaptureDevice = undefined;
    this._audioCaptureDevice = undefined;
    this._audioPlaybackDevice = undefined;

    this._captureConfig = DefaultCaptureConfig;
    this._encoderConfig = DefaultEncoderConfig;
  };

  checkPermission(): Promise<{
    video: boolean;
    audio: boolean;
  }> {
    return VERTC.enableDevices({
      video: true,
      audio: true,
    });
  }

  /**
   * get the devices
   * @returns
   */
  async getDevices(): Promise<{
    audioInputs: MediaDeviceInfo[];
    videoInputs: MediaDeviceInfo[];
    audioOutputs: MediaDeviceInfo[];
  }> {
    // const permissions = await this.checkPermission();
    const devices = await VERTC.enumerateDevices();

    const audioInputs: MediaDeviceInfo[] = devices.filter(
      (i) => i.deviceId && i.kind === 'audioinput'
    );

    const videoInputs: MediaDeviceInfo[] = devices.filter(
      (i) => i.deviceId && i.kind === 'videoinput'
    );

    const audioOutputs: MediaDeviceInfo[] = devices.filter(
      (i) => i.deviceId && i.kind === 'audiooutput'
    );

    this._audioCaptureDevice = audioInputs.filter((i) => i.deviceId)?.[0]?.deviceId;
    this._videoCaptureDevice = videoInputs.filter((i) => i.deviceId)?.[0]?.deviceId;
    this._audioPlaybackDevice = audioOutputs.filter((i) => i.deviceId)?.[0]?.deviceId;

    return {
      audioInputs,
      videoInputs,
      audioOutputs,
    };
  }

  startAudioCapture = async (mic?: string) => {
    await RtcSdkApiWrapper('startAudioCapture', () => {
      return this.engine.startAudioCapture(mic || this._audioCaptureDevice);
    });
  };

  stopAudioCapture = async () => {
    await RtcSdkApiWrapper('stopAudioCapture', () => {
      return this.engine.stopAudioCapture();
    });
  };

  startVideoCapture = async (camera?: string) => {
    await RtcSdkApiWrapper('setVideoEncoderConfig', () => {
      return this.engine.setVideoEncoderConfig(this._encoderConfig);
    });

    this._videoCaptureDevice = camera || this._videoCaptureDevice;

    await RtcSdkApiWrapper('startVideoCapture', () => {
      return this.engine.startVideoCapture(this._videoCaptureDevice);
    });
  };

  stopVideoCapture = async () => {
    await RtcSdkApiWrapper('stopVideoCapture', () => {
      return this.engine.stopVideoCapture();
    });
  };

  publishStream = async (mediaType: MediaType) => {
    await RtcSdkApiWrapper('publishStream', () => {
      return this.engine.publishStream(mediaType);
    });
  };

  unpublishStream = async (mediaType: MediaType) => {
    await RtcSdkApiWrapper('unpublishStream', () => {
      return this.engine.unpublishStream(mediaType);
    });
  };

  /**
   * 设置视频流播放器
   * @param userId
   * @param renderDom
   */
  setVideoPlayer = (userId: string, renderDom?: string | HTMLElement) => {
    // 本端用户
    if (this.config && userId === this.config.uid) {
      RtcSdkApiWrapper('setLocalVideoPlayer', () => {
        return this.engine.setLocalVideoPlayer(StreamIndex.STREAM_INDEX_MAIN, {
          renderDom,
          userId,
          renderMode: VideoRenderMode.RENDER_MODE_FIT,
        });
      });
    }
    // 远端用户
    else {
      RtcSdkApiWrapper('setRemoteVideoPlayer', () => {
        return this.engine.setRemoteVideoPlayer(StreamIndex.STREAM_INDEX_MAIN, {
          renderDom,
          userId,
          renderMode: VideoRenderMode.RENDER_MODE_FIT,
        });
      });
    }
  };

  /**
   * 设置分享流播放器
   * @param userId
   * @param renderDom
   */
  setScreenPlayer = (userId: string, renderDom?: string | HTMLElement) => {
    // 本端用户
    if (this.config && userId === this.config.uid) {
      RtcSdkApiWrapper('setLocalVideoPlayer', () => {
        return this.engine.setLocalVideoPlayer(StreamIndex.STREAM_INDEX_SCREEN, {
          renderDom,
          userId,
          renderMode: VideoRenderMode.RENDER_MODE_FIT,
        });
      });
    }
    // 远端用户
    else {
      RtcSdkApiWrapper('setRemoteVideoPlayer', () => {
        return this.engine.setRemoteVideoPlayer(StreamIndex.STREAM_INDEX_SCREEN, {
          renderDom,
          userId,
          renderMode: VideoRenderMode.RENDER_MODE_FIT,
        });
      });
    }
  };

  /**
   * 订阅远端用户屏幕流
   * @param userId
   */
  subscribeScreen = async (userId: string): Promise<void> => {
    await RtcSdkApiWrapper('subscribeScreen', () => {
      return this.engine.subscribeScreen(userId, MediaType.AUDIO_AND_VIDEO);
    });
  };

  /**
   * 设置业务标识参数
   * @param businessId
   */
  setBusinessId = (businessId: string) => {
    RtcSdkApiWrapper('subscribeScreen', () => {
      return this.engine.setBusinessId(businessId);
    });
  };

  /**
   * 开始屏幕共享
   */
  startScreenCapture = async () => {
    try {
      await this.engine
        .startScreenCapture({
          enableAudio: true,
        })
        .catch((e) => {
          TeaClient.reportRTCSdkAPIFailure('startScreenCapture', JSON.stringify(e));
          throw e;
        });

      await this.engine.publishScreen(MediaType.AUDIO_AND_VIDEO).catch((e) => {
        TeaClient.reportRTCSdkAPIFailure('publishScreen', JSON.stringify(e));
      });
      return 'success';
    } catch (e: any) {
      const failureReason = e?.error?.message || e?.code || 'Screen Capture Failed';
      TeaClient.reportRTCSdkException(failureReason);

      return failureReason;
    }
  };

  /**
   * 停止屏幕共享
   */
  stopScreenCapture = async () => {
    await RtcSdkApiWrapper('stopScreenCapture', () => {
      return this.engine.stopScreenCapture();
    });

    await RtcSdkApiWrapper('unpublishScreen', () => {
      return this.engine.unpublishScreen(MediaType.AUDIO_AND_VIDEO);
    });
  };

  /**
   * 镜像模式
   * @param mirrorType
   */
  setMirrorType = (mirrorType: MirrorType) => {
    RtcSdkApiWrapper('setLocalVideoMirrorType', () => {
      return this.engine.setLocalVideoMirrorType(mirrorType);
    });
  };

  /**
   * 设置音质档位
   */
  setAudioProfile = (profile: AudioProfileType) => {
    RtcSdkApiWrapper('setAudioProfile', () => {
      return this.engine.setAudioProfile(profile);
    });
  };

  /**
   * 设置画质
   * @param streamIndex
   * @param descriptions
   */
  setVideoEncoderConfig = async (descriptions: VideoEncoderConfig) => {
    RtcSdkApiWrapper('setVideoEncoderConfig', () => {
      return this.engine.setVideoEncoderConfig(descriptions);
    });
    this._encoderConfig = descriptions;
  };

  /**
   * 设置屏幕共享画质
   * @param streamIndex
   * @param descriptions
   */
  setScreenEncoderConfig = async (descriptions: ScreenEncoderConfig) => {
    RtcSdkApiWrapper('setScreenEncoderConfig', () => {
      return this.engine.setScreenEncoderConfig(descriptions);
    });
    this._encoderConfig = {
      width: descriptions.width,
      height: descriptions.height,
      frameRate: descriptions.frameRate,
      maxKbps: descriptions.maxKbps,
    };
  };

  /**
   * 切换设备
   */
  switchDevice = async (deviceType: 'camera' | 'microphone', deviceId: string) => {
    if (deviceType === 'microphone') {
      this._audioCaptureDevice = deviceId;
      RtcSdkApiWrapper('setAudioCaptureDevice', () => {
        return this.engine.setAudioCaptureDevice(deviceId);
      });
    } else {
      this._videoCaptureDevice = deviceId;
      RtcSdkApiWrapper('setVideoCaptureDevice', () => {
        return this.engine.setVideoCaptureDevice(deviceId);
      });
    }
  };
}

export default new RtcClient();
