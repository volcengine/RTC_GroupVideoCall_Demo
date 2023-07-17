import { useEffect, useMemo } from 'react';
import { useSelector } from 'react-redux';
import RtcClient from '@/lib/RtcClient';
import { RootState } from '@/store';
import styles from './localshare.module.less';

interface IProps {
  localShare: boolean;
}

function LocalShareLayout(props: IProps) {
  const { localShare } = props;
  const localUser = useSelector((state: RootState) => state.room.localUser);
  const domId = useMemo(() => `local-${localUser?.userId}-1`, [localUser]);

  useEffect(() => {
    RtcClient.setScreenPlayer(localUser.userId!, domId);
    return () => {
      RtcClient.setScreenPlayer(localUser.userId!, undefined);
    };
  }, [domId]);

  return (
    <div
      className={styles.localShareLayout}
      style={{
        display: localShare ? 'block' : 'none',
      }}
    >
      <div id={domId} className={styles.videoPlayer} />
    </div>
  );
}

export default LocalShareLayout;
