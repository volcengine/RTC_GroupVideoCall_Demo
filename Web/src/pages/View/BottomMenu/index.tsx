import { useDispatch, useSelector } from 'react-redux';
import { MediaType } from '@volcengine/rtc';
import { useMemo } from 'react';
import styles from './index.module.less';
import { RootState } from '@/store';
import Stop from './Stop';
import RealTime from './RealTime';
import RtcClient from '@/lib/RtcClient';
import DeviceButton from './DeviceButton';
import ShareButton from './ShareButton';
import StopShareBtn from './StopShareBtn';
import { updateLocalUser } from '@/store/slices/room';
import BeautifyButton from './BeautifyButton';
import TeaClient, { TeaEventSource } from '@/lib/TeaClient';

function BottomMenu() {
  const dispatch = useDispatch();
  const room = useSelector((state: RootState) => state.room);

  const isLocalUserShared = useMemo(() => {
    const user = room.localUser;
    return !user?.userId ? false : user?.userId === room.shareUser;
  }, [room.localUser, room.shareUser]);

  const handleSreamPublish = (deviceType: 'camera' | 'microphone') => {
    const publishType = deviceType === 'camera' ? 'publishVideo' : 'publishAudio';

    dispatch(
      updateLocalUser({
        [publishType]: !room.localUser![publishType],
      })
    );

    if (deviceType === 'camera') {
      if (!room.localUser![publishType]) {
        TeaClient.reportToggleCameraButton(true, TeaEventSource.VIEW_PAGE);
        RtcClient.startVideoCapture();
      } else {
        TeaClient.reportToggleCameraButton(false, TeaEventSource.VIEW_PAGE);
        RtcClient.stopVideoCapture();
      }
    }

    if (deviceType === 'microphone') {
      if (!room.localUser![publishType]) {
        TeaClient.reportToggleMicrophoneButton(true, TeaEventSource.VIEW_PAGE);
        RtcClient.publishStream(MediaType.AUDIO);
      } else {
        TeaClient.reportToggleMicrophoneButton(false, TeaEventSource.VIEW_PAGE);
        RtcClient.unpublishStream(MediaType.AUDIO);
      }
    }
  };

  if (isLocalUserShared) {
    return (
      <div className={styles.localShareMenuWrapper}>
        <DeviceButton deviceType="camera" onClick={handleSreamPublish} shared />

        <DeviceButton deviceType="microphone" onClick={handleSreamPublish} shared />
        <RealTime shared />
        <ShareButton shared />

        <BeautifyButton shared />

        <StopShareBtn />
      </div>
    );
  }
  return (
    <div className={styles.menuWrapper}>
      <div className={styles.menuLeft}>
        <DeviceButton deviceType="camera" onClick={handleSreamPublish} />

        <DeviceButton deviceType="microphone" onClick={handleSreamPublish} />

        <RealTime />

        <ShareButton />

        <BeautifyButton />
      </div>
      <Stop />
    </div>
  );
}

export default BottomMenu;
