import { useEffect, useState } from 'react';
import { message as Message } from 'antd';
import { useSelector, useDispatch } from 'react-redux';
import { MediaType } from '@volcengine/rtc';
import { useTranslation } from 'react-i18next';
import { useNavigate } from 'react-router-dom';
import { v4 as uuid } from 'uuid';

import { useFreeLogin } from './loginHook';
import { useJoinRTMMutation } from '@/app/roomQuery';
import Utils from '@/utils/utils';
import RtcClient, { beautyExtension } from '@/lib/RtcClient';
import TeaClient, { TeaEventSource } from '@/lib/TeaClient';
import { BusinessId, RESOLUTIOIN_LIST, isDev } from '@/config';
import {
  localJoinRoom,
  setBeautyEnabled,
  updateRoomTime,
  setBeauty,
  localLeaveRoom,
} from '@/store/slices/room';

import useRtcListeners from '@/lib/listenerHooks';
import { RootState } from '@/store';

import {
  updateMediaInputs,
  updateSelectedDevice,
  setDevicePermissions,
} from '@/store/slices/device';
import { resetConfig } from '@/store/slices/stream';

export interface FormProps {
  username: string;
  roomId: string;
  publishAudio: boolean;
  publishVideo: boolean;
}

export const useGetDevicePermission = () => {
  const [permission, setPermission] = useState<{
    video: boolean;
    audio: boolean;
  }>();

  const dispatch = useDispatch();

  useEffect(() => {
    (async () => {
      const permission = await RtcClient.checkPermission();
      dispatch(setDevicePermissions(permission));
      setPermission(permission);
    })();
  }, [dispatch]);
  return permission;
};

export const useJoin = (): [
  boolean,
  (formValues: FormProps, fromRefresh: boolean) => Promise<void | boolean>
] => {
  const devicePermissions = useSelector((state: RootState) => state.device.devicePermissions);

  const dispatch = useDispatch();

  const streamConfig = useSelector((state: RootState) => state.stream);
  const { t } = useTranslation();

  const [joining, setJoining] = useState(false);
  const { freeLoginApi } = useFreeLogin();
  const [joinRTM] = useJoinRTMMutation();
  const listeners = useRtcListeners(isDev);
  const navigate = useNavigate();

  async function disPatchJoin(
    formValues: FormProps,
    fromRefresh: boolean = false
  ): Promise<void | boolean> {
    if (joining) {
      return;
    }

    setJoining(true);
    let freeLoginRes = null;

    try {
      freeLoginRes = await freeLoginApi(formValues.username);

      TeaClient.updateLoginInfo({
        user_id: freeLoginRes.user_id,
        user_name: freeLoginRes.user_name,
        room_id: formValues.roomId,
        device_id: Utils.getDeviceId(),
      });

      if (!freeLoginRes.login_token) {
        const failureReason = `No login token ${JSON.stringify(freeLoginRes)}`;
        TeaClient.reportUserLogin(
          false,
          failureReason,
          fromRefresh ? TeaEventSource.PAGE_REFRESH : TeaEventSource.LOGIN_PAGE
        );
        return;
      }
      TeaClient.reportUserLogin(true, '', fromRefresh ? 'page_refresh' : 'login_button');
    } catch (e: any) {
      const failureReason = e?.toString() || 'unknown';
      TeaClient.reportUserLogin(
        false,
        failureReason,
        fromRefresh ? TeaEventSource.PAGE_REFRESH : TeaEventSource.LOGIN_PAGE
      );
      console.error(e);
      setJoining(false);
      return;
    }

    try {
      const joinRtsRes = await joinRTM({
        login_token: freeLoginRes.login_token,
        device_id: Utils.getDeviceId(),
      });

      if (!('data' in joinRtsRes)) {
        return;
      }

      const { response } = joinRtsRes.data;

      await RtcClient.createEngine({
        appId: response.app_id,
        roomId: `call_${formValues.roomId}`,
        rtsUid: freeLoginRes.user_id,
        uid: freeLoginRes.user_id,
        rtmToken: response.rtm_token,
        serverUrl: response.server_url,
        serverSignature: response.server_signature,
        bid: response.bid,
      });

      RtcClient.setBusinessId(BusinessId);

      dispatch(setBeautyEnabled(RtcClient.beautyEnabled));

      await RtcClient.joinWithRTS();
      RtcClient.addEventListeners(listeners);

      const joinRes: any = await RtcClient.sendServerMessage('videocallJoinRoom');

      if (joinRes.message_type !== 'return') {
        return;
      }

      if (joinRes.code !== 200) {
        if (joinRes.code === 406) {
          Message.error(t('limitUserInRoom'));
        }
        console.log('rts join error: ', joinRes);
        setJoining(false);
        navigate('/login');
        return;
      }

      if (fromRefresh) {
        const rtsUid = sessionStorage.getItem('rtsUid');
        const loginToken = sessionStorage.getItem('login_token');

        if (!rtsUid) return;

        const requestId = uuid();

        const content = {
          app_id: response.app_id,
          device_id: Utils.getDeviceId(),
          room_id: `call_${formValues.roomId}`,
          user_id: rtsUid,
          request_id: requestId,
          event_name: 'videocallLeaveRoom',
          content: JSON.stringify({
            room_id: `call_${formValues.roomId}`,
            user_id: rtsUid,
            login_token: loginToken,
          }),
        };

        RtcClient.engine
          .sendServerMessage(JSON.stringify(content))
          .then((res) =>
            console.log(
              'sendServerMessage fromRefresh',
              loginToken,
              content.user_id,
              response.app_id
            )
          )
          .catch((err: any) => {
            console.error('err', err);
            const failureReason = err?.toString() || 'unknown';
            TeaClient.reportRTCSdkAPIFailure('sendServerMessage', failureReason);
          });
      }

      RtcClient.setAudioProfile(streamConfig.audioProfile);
      const encodeConfig = RESOLUTIOIN_LIST.find(
        (resolution) => resolution.text === streamConfig.videoEncodeConfig
      );
      await RtcClient.setVideoEncoderConfig(encodeConfig!.val);
      await RtcClient.joinRoom(joinRes.response.rtc_token, formValues.username);

      const mediaDevices = await RtcClient.getDevices();

      console.log('mediaDevices', mediaDevices);
      if (devicePermissions.video && formValues.publishVideo) {
        try {
          await RtcClient.startVideoCapture();
          RtcClient.setMirrorType(streamConfig.mirror);
        } catch (e) {
          console.log('something goes wrong', e);
        }
      }

      if (devicePermissions.audio) {
        try {
          await RtcClient.startAudioCapture();
        } catch (e) {
          console.log('something goes wrong', e);
        }
      }

      if (!formValues.publishAudio) {
        RtcClient.unpublishStream(MediaType.AUDIO);
      }

      dispatch(updateRoomTime({ time: joinRes.response.duration }));

      dispatch(
        localJoinRoom({
          roomId: `call_${formValues.roomId}`,
          user: {
            username: formValues.username,
            userId: freeLoginRes.user_id,
            publishAudio: !!formValues.publishAudio,
            publishVideo: !!formValues.publishVideo,
          },
        })
      );

      dispatch(
        updateSelectedDevice({
          selectedCamera: mediaDevices.videoInputs[0]?.deviceId,
          selectedMicrophone: mediaDevices.audioInputs[0]?.deviceId,
        })
      );

      dispatch(updateMediaInputs(mediaDevices));

      Utils.setSessionInfo({
        ...formValues,
        publishAudio: !!formValues.publishAudio,
        publishVideo: !!formValues.publishVideo,
        rtsUid: freeLoginRes.user_id,
        login_token: freeLoginRes.login_token,
      });

      setJoining(false);

      navigate(`/?roomId=${formValues.roomId}`);
    } catch (e: any) {
      console.error(e);
      const failureReason = e?.toString() || 'unknown';
      TeaClient.reportDemoInternalException(failureReason);
      setJoining(false);
    }
  }

  return [joining, disPatchJoin];
};

export const useLeave = () => {
  const dispatch = useDispatch();

  return async function () {
    RtcClient.sendServerMessage('videocallLeaveRoom');
    dispatch(setBeauty(false));
    dispatch(localLeaveRoom());
    dispatch(resetConfig());
    if (RtcClient.beautyEnabled) {
      beautyExtension.disableBeauty();
    }
    await Promise.all([
      RtcClient.stopAudioCapture(),
      RtcClient.stopVideoCapture(),
      RtcClient.stopScreenCapture(),
    ]);
    await RtcClient.leaveRoom();
  };
};
