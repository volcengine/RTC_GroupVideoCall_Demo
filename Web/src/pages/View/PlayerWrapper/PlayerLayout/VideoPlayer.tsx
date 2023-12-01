import { useEffect, useMemo } from 'react';
import { useSelector } from 'react-redux';
import { useTranslation } from 'react-i18next';
import { IUser, LocalUser } from '@/store/slices/room';
import styles from './playerLayout.module.less';
import MicHasVolumeImg from '@/assets/img/micHasVolume.png';
import MicrophoneOn from '@/assets/img/MicrophoneOn.svg';
import MicrophoneOff from '@/assets/img/MicrophoneOff.svg';
import RtcClient from '@/lib/RtcClient';
import { getIcon } from '@/components/MediaButton/utils';
import { RootState } from '@/store';
import Icon from '@/components/Icon';

interface IProps {
  user?: IUser | LocalUser;
}

function VideoPlayer(props: IProps) {
  const { user } = props;
  const { t } = useTranslation();
  const room = useSelector((state: RootState) => state.room);
  const isLocalUser = room.localUser?.userId === user?.userId;

  const isUserShare = user?.userId === room.shareUser;

  const domId = `${isLocalUser ? 'local' : 'remote'}-${user?.userId}-0`;

  const startPublish = useMemo(() => {
    return !!(user?.publishVideo || user?.publishAudio);
  }, [user?.publishVideo, user?.publishAudio]);

  const startPublishVideo = useMemo(() => {
    return !!user?.publishVideo;
  }, [user?.publishVideo]);

  useEffect(() => {
    RtcClient.setVideoPlayer((user as IUser).userId!, domId);
    return () => {
      RtcClient.setVideoPlayer((user as IUser).userId!, undefined);
    };
  }, [startPublish]);

  let micSrcImg = MicrophoneOff;
  const isMicrophoneOn = user?.publishAudio && !user?.audioPropertiesInfo?.linearVolume;
  const isMicrophoneOff = !user?.publishAudio;
  const hasIcon = isMicrophoneOn || isMicrophoneOff;
  if (isMicrophoneOn) {
    micSrcImg = MicrophoneOn;
  }
  if (isMicrophoneOff) {
    micSrcImg = MicrophoneOff;
  }

  return (
    <div
      className={`${styles.Player} ${
        !!user?.audioPropertiesInfo?.linearVolume && user.publishAudio ? styles.speaking : ''
      }`}
      style={{
        visibility: user ? 'visible' : 'hidden',
      }}
    >
      {!startPublishVideo && (
        <div className={styles.userAvatar}>
          <span>{user?.username?.[0]}</span>
        </div>
      )}
      <div
        id={domId}
        className={styles.videoPlayer}
        style={{
          display: user?.publishVideo ? 'block' : 'none',
        }}
      />
      <div className={styles.userInfo}>
        {hasIcon && <Icon src={micSrcImg} className={styles.userMicrophone} />}
        {user?.publishAudio && !!user?.audioPropertiesInfo?.linearVolume && (
          <img
            src={MicHasVolumeImg}
            alt=""
            style={{
              width: 12,
              height: 15,
              marginRight: 7,
              marginLeft: 7,
            }}
          />
        )}

        {isUserShare && <Icon src={getIcon('shareScreen')} className={styles.userScreen} />}
        <span className={styles.usernameWrapper}>
          <span className={styles.username}>{user?.username}</span>
          {isLocalUser ? `(${t('Me')})` : ''}
        </span>
      </div>
    </div>
  );
}

export default VideoPlayer;
