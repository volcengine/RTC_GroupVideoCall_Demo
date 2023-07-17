import { useEffect, useMemo } from 'react';
import { useSelector } from 'react-redux';
import { useTranslation } from 'react-i18next';
import Icon from '@/components/Icon';
import { getIcon } from '@/components/MediaButton/utils';
import RtcClient from '@/lib/RtcClient';
import { RootState } from '@/store';
import { IUser } from '@/store/slices/room';
import styles from './playerLayout.module.less';

interface IProps {
  remoteShare: boolean;
}

function RemoteShareLayout(props: IProps) {
  const { remoteShare } = props;
  const room = useSelector((state: RootState) => state.room);
  const domId = useMemo(() => `remote-${room.shareUser!}-1`, [room.shareUser]);
  const { t } = useTranslation();

  const shareUser: IUser | undefined = useMemo(() => {
    const user = room.remoteUsers.find((u) => u.userId === room.shareUser);
    return user;
  }, [room.remoteUsers, room.shareUser]);

  useEffect(() => {
    if (shareUser) {
      RtcClient.setScreenPlayer(shareUser.userId!, domId);
    }

    return () => {
      if (shareUser) {
        RtcClient.setScreenPlayer(shareUser.userId!, undefined);
      }
    };
  }, [domId, shareUser?.userId]);

  return (
    <div
      className={styles.remoteScreenPlayerWrapper}
      style={{
        display: remoteShare ? 'block' : 'none',
      }}
    >
      <div id={domId} className={styles.remoteScreenPlayer} />
      <div className={styles.userInfo}>
        <Icon
          src={getIcon('microphone', shareUser?.publishAudio ? 'On' : 'Off')}
          className={styles.userMicrophone}
        />

        <span>
          {shareUser?.username}
          {t('whosScreen')}
        </span>
      </div>
    </div>
  );
}

export default RemoteShareLayout;
