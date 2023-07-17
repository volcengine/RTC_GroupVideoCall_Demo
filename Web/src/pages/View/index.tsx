import { useEffect, useMemo, useCallback, useRef } from 'react';

import { Spin } from 'antd';
import { useSelector, useDispatch } from 'react-redux';
import { useNavigate } from 'react-router-dom';
import { RootState } from '@/store';
import Utils from '@/utils/utils';
import RtcClient from '@/lib/RtcClient';

import Header from '@/components/Header';
import BottomMenu from './BottomMenu';
import PlayerWrapper from './PlayerWrapper';
import RoomInfo from './RoomInfo';
import AutoPlayModal from './AutoPlayModal';
import { useJoin, useLeave } from '@/lib/useCommon';
import { setDevicePermissions } from '@/store/slices/device';
import './index.module.less';

function View() {
  const navigate = useNavigate();

  const room = useSelector((state: RootState) => state.room);

  const [joining, disPatchJoin] = useJoin();
  const leave = useLeave();

  const ref = useRef({
    publishAudio: room.localUser.publishAudio,
    publishVideo: room.localUser.publishVideo,
  });

  const isLocalUserShared = useMemo(
    () => (!room.localUser?.userId ? false : room.localUser?.userId === room.shareUser),
    [room.localUser, room.shareUser]
  );

  const hasLogin = useMemo(() => Utils.checkLoginInfo(), []);

  const dispatch = useDispatch();

  useEffect(() => {
    if (!room.roomId && !hasLogin) {
      const roomArgs = Utils.getUrlArgs()?.roomId;
      navigate(`/login${roomArgs ? `?roomId=${roomArgs}` : ''}`);
    } else if (!room.roomId) {
      (async () => {
        const permission = await RtcClient.checkPermission();
        dispatch(setDevicePermissions(permission));

        if (!permission) return;

        const formValues = Utils.getLoginInfo();

        if (false || permission) {
          disPatchJoin(
            {
              ...formValues,
              publishAudio: permission.audio && JSON.parse(formValues?.publishAudio || 'false'),
              publishVideo: permission.video && JSON.parse(formValues?.publishVideo || 'false'),
            },
            true
          );
        }
      })();
    }
  }, [room.roomId, navigate, hasLogin]);

  useEffect(() => {
    const { publishAudio: prevA, publishVideo: prevV } = ref.current;
    const { publishAudio, publishVideo } = room.localUser;

    if (prevA !== publishAudio || prevV !== publishVideo) {
      ref.current = { publishAudio, publishVideo };
      Utils.setSessionInfo({
        publishVideo: !!publishVideo,
        publishAudio: !!publishAudio,
      });
    }
  }, [room.localUser.publishAudio, room.localUser.publishVideo]);

  // useEffect(() => {
  //   if (sessionStorage.getItem('store')) {
  //     const a = sessionStorage.getItem('store');
  //     a && alert(a);
  //   }
  // }, []);
  const leaveRoom = useCallback(() => {
    if (!RtcClient.engine) return;
    leave();
    // sessionStorage.setItem('store', JSON.stringify({ test: new Date().toString() }));
  }, []);

  useEffect(() => {
    window.addEventListener('pagehide', leaveRoom);
    return () => {
      leaveRoom();
      window.removeEventListener('pagehide', leaveRoom);
    };
  }, [leaveRoom]);

  useEffect(() => {
    window.addEventListener('popstate', leaveRoom);

    return () => {
      window.removeEventListener('popstate', leaveRoom);
    };
  }, [leaveRoom]);

  return (
    <Spin spinning={joining}>
      <Header hide={isLocalUserShared}>
        <RoomInfo />
      </Header>
      <PlayerWrapper />
      <BottomMenu />
      <AutoPlayModal />
    </Spin>
  );
}

export default View;
