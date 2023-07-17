import VERTC, {
  LocalAudioPropertiesInfo,
  RemoteAudioPropertiesInfo,
  LocalStreamStats,
  MediaType,
  onUserJoinedEvent,
  onUserLeaveEvent,
  RemoteStreamStats,
  StreamRemoveReason,
  StreamIndex,
  DeviceInfo,
  AutoPlayFailedEvent,
  PlayerEvent,
} from '@volcengine/rtc';
import { useDispatch } from 'react-redux';
import { useNavigate } from 'react-router-dom';
import { message as Message } from 'antd';
import { useTranslation } from 'react-i18next';
import { useRef } from 'react';

import {
  IUser,
  localLeaveRoom,
  remoteUserJoin,
  remoteUserLeave,
  setBeauty,
  startShare,
  stopShare,
  updateLocalUser,
  updateRemoteUser,
  addAutoPlayFail,
  removeAutoPlayFail,
} from '@/store/slices/room';
import RtcClient, { beautyExtension, IEventListener } from './RtcClient';

import { setMicrophoneList, setCameraList, updateSelectedDevice } from '@/store/slices/device';
import { resetConfig } from '@/store/slices/stream';
import Utils from '@/utils/utils';

const useRtcListeners = (isDev: boolean): IEventListener => {
  const dispatch = useDispatch();
  const { t } = useTranslation();
  const navigate = useNavigate();
  const playStatus = useRef<{ [key: string]: { audio: boolean; video: boolean } }>({});

  const handleUserJoin = (e: onUserJoinedEvent) => {
    const extraInfo = JSON.parse(e.userInfo.extraInfo || '{}');

    const userId = extraInfo.user_id || e.userInfo.userId;
    const username = extraInfo.user_name || e.userInfo.userId;

    dispatch(
      remoteUserJoin({
        userId,
        username,
      })
    );
  };

  const handleError = (e: { errorCode: typeof VERTC.ErrorCode.DUPLICATE_LOGIN }) => {
    const { errorCode } = e;
    if (errorCode === VERTC.ErrorCode.DUPLICATE_LOGIN) {
      console.log('踢人');
    }
  };

  const handleUserLeave = (e: onUserLeaveEvent) => {
    dispatch(remoteUserLeave(e.userInfo));
    dispatch(removeAutoPlayFail(e.userInfo));
  };

  const handleUserStartVideoCapture = (e: { userId: string }) => {
    const { userId } = e;
    const payload: IUser = { userId };
    payload.publishVideo = true;
    dispatch(updateRemoteUser(payload));
  };

  const handleUserStopVideoCapture = (e: { userId: string }) => {
    const { userId } = e;
    const payload: IUser = { userId };
    payload.publishVideo = false;

    dispatch(updateRemoteUser(payload));
  };

  const handleUserPublishStream = (e: { userId: string; mediaType: MediaType }) => {
    const { userId, mediaType } = e;

    const payload: IUser = { userId };
    if (mediaType === MediaType.AUDIO) {
      payload.publishAudio = true;
    }

    if (mediaType === MediaType.AUDIO_AND_VIDEO) {
      payload.publishAudio = true;
    }

    dispatch(updateRemoteUser(payload));
  };

  const handleUserUnpublishStream = (e: {
    userId: string;
    mediaType: MediaType;
    reason: StreamRemoveReason;
  }) => {
    const { userId, mediaType } = e;

    const payload: IUser = { userId };
    if (mediaType === MediaType.AUDIO) {
      payload.publishAudio = false;
    }

    if (mediaType === MediaType.AUDIO_AND_VIDEO) {
      payload.publishAudio = false;
    }

    dispatch(updateRemoteUser(payload));
  };

  const handleUserPublishScreen = async (e: { userId: string; mediaType: MediaType }) => {
    const { userId } = e;
    await RtcClient.subscribeScreen(userId);
    dispatch(
      startShare({
        shareUser: userId,
      })
    );
  };

  const handleUserUnpublishScreen = async (e: {
    userId: string;
    mediaType: MediaType;
    reason: StreamRemoveReason;
  }) => {
    dispatch(stopShare());
  };

  const handleTrackEnded = async (event: { kind: string; isScreen: boolean }) => {
    const { kind, isScreen } = event;
    if (isScreen && kind === 'video') {
      RtcClient.sendServerMessage('videocallEndShareScreen');
      await RtcClient.stopScreenCapture();
      dispatch(stopShare());
    }
  };

  const handleRemoteStreamStats = (e: RemoteStreamStats) => {
    if (isDev) {
      return;
    }
    dispatch(
      updateRemoteUser({
        userId: e.userId,
        audioStats: e.audioStats,
        videoStats: e.videoStats,
      })
    );
  };

  const handleLocalStreamStats = (e: LocalStreamStats) => {
    if (isDev) {
      return;
    }
    dispatch(
      updateLocalUser({
        audioStats: e.audioStats,
        videoStats: e.videoStats,
      })
    );
  };

  const handleLocalAudioPropertiesReport = (e: LocalAudioPropertiesInfo[]) => {
    if (isDev) {
      return;
    }
    const localAudioInfo = e.find(
      (audioInfo) => audioInfo.streamIndex === StreamIndex.STREAM_INDEX_MAIN
    );
    if (localAudioInfo) {
      dispatch(
        updateLocalUser({
          audioPropertiesInfo: localAudioInfo.audioPropertiesInfo,
        })
      );
    }
  };

  const handleRemoteAudioPropertiesReport = (e: RemoteAudioPropertiesInfo[]) => {
    if (isDev) {
      return;
    }
    const remoteAudioInfo = e
      .filter((audioInfo) => audioInfo.streamKey.streamIndex === StreamIndex.STREAM_INDEX_MAIN)
      .map((audioInfo) => ({
        userId: audioInfo.streamKey.userId,
        audioPropertiesInfo: audioInfo.audioPropertiesInfo,
      }));

    if (remoteAudioInfo.length) {
      dispatch(updateRemoteUser(remoteAudioInfo));
    }
  };

  const handleVideoDeviceStateChanged = async (device: DeviceInfo) => {
    const devices = await RtcClient.getDevices();

    let deviceId = device.mediaDeviceInfo?.deviceId;
    if (device.deviceState === 'inactive') {
      deviceId = devices.videoInputs?.[0].deviceId || '';
    }

    RtcClient.switchDevice('camera', deviceId);
    dispatch(setCameraList(devices.videoInputs));
    dispatch(
      updateSelectedDevice({
        selectedCamera: deviceId,
      })
    );
  };

  const handleAudioDeviceStateChanged = async (device: DeviceInfo) => {
    const devices = await RtcClient.getDevices();

    if (device.mediaDeviceInfo.kind === 'audioinput') {
      let deviceId = device.mediaDeviceInfo.deviceId;
      if (device.deviceState === 'inactive') {
        deviceId = devices.audioInputs?.[0].deviceId || '';
      }
      RtcClient.switchDevice('microphone', deviceId);
      dispatch(setMicrophoneList(devices.audioInputs));

      dispatch(
        updateSelectedDevice({
          selectedMicrophone: deviceId,
        })
      );
    }

    // if (device.mediaDeviceInfo.kind === 'audiooutput') {
    //   let deviceId = device.mediaDeviceInfo.deviceId;
    //   if (device.deviceState === 'inactive') {
    //     deviceId = devices.audioOutputs?.[0].deviceId || '';
    //   }
    //   RtcClient.switchDevice('playback', deviceId);
    //   dispatch(setAudioPlayBackList(devices.audioOutputs));
    //   dispatch(setAudioPlayBack(deviceId));
    // }
  };

  const handleOnCloseRoom = async (e: { userId: string; message: any }) => {
    const { userId, message } = e;
    if (userId !== 'server') {
      return;
    }

    const msgObj = JSON.parse(message || '{}');

    if (msgObj.message_type === 'inform' && msgObj.event === 'videocallOnCloseRoom') {
      if (msgObj.data.room_id) {
        Message.error(t('timeout'));

        dispatch(setBeauty(false));
        if (RtcClient.beautyEnabled) {
          beautyExtension.disable();
        }
        await RtcClient.stopAudioCapture();
        await RtcClient.stopVideoCapture();
        await RtcClient.stopScreenCapture();
        RtcClient.leaveRoom();
        Utils.removeLoginInfo();
        dispatch(localLeaveRoom());
        dispatch(resetConfig());
        navigate('/login');
      }
    }
  };

  const handleUserMessageReceivedOutsideRoom = (e: { userId: string; message: any }) => {
    handleOnCloseRoom(e);
  };

  const handleUserMessageReceived = (e: { userId: string; message: any }) => {
    handleOnCloseRoom(e);
  };
  const handleRoomMessageReceived = (e: { userId: string; message: any }) => {
    handleOnCloseRoom(e);
  };

  const handleAutoPlayFail = (event: AutoPlayFailedEvent) => {
    const { userId, kind } = event;
    let playUser = playStatus.current?.[userId] || {};
    playUser = { ...playUser, [kind]: false };
    playStatus.current[userId] = playUser;

    dispatch(
      addAutoPlayFail({
        userId,
      })
    );
  };

  const addFailUser = (userId: string) => {
    dispatch(addAutoPlayFail({ userId }));
  };

  const playerFail = (params: { type: 'audio' | 'video'; userId: string }) => {
    const { type, userId } = params;
    let playUser = playStatus.current?.[userId] || {};

    playUser = { ...playUser, [type]: false };

    const { audio, video } = playUser;

    if (audio === false || video === false) {
      addFailUser(userId);
    }

    return playUser;
  };

  const handlePlayerEvent = (event: PlayerEvent) => {
    const { userId, rawEvent, type } = event;
    let playUser = playStatus.current?.[userId] || {};

    if (!playStatus.current) return;

    if (rawEvent.type === 'playing') {
      playUser = { ...playUser, [type]: true };
      const { audio, video } = playUser;
      if (audio !== false && video !== false) {
        dispatch(removeAutoPlayFail({ userId }));
      }
    } else if (rawEvent.type === 'pause') {
      playUser = playerFail({ type, userId });
    }

    playStatus.current[userId] = playUser;
  };

  return {
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
  };
};

export default useRtcListeners;
