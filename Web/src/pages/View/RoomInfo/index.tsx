import { useSelector } from 'react-redux';
import { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import styles from './index.module.less';

import { RootState } from '@/store';
import Utils from '@/utils/utils';

function RoomInfo() {
  const room = useSelector((state: RootState) => state.room);
  const { t } = useTranslation();

  const [time, setTime] = useState(room.time);

  useEffect(() => {
    let timer: NodeJS.Timeout | null = null;
    if (time > -1) {
      timer = setInterval(() => {
        setTime((v) => v + 1);
      }, 1000);
    }
    return () => {
      if (timer) {
        clearInterval(timer);
        timer = null;
      }
    };
  }, [time, t]);

  useEffect(() => {
    setTime(room.time);
  }, [room.time]);

  return (
    <div className={styles.roomInfo}>
      <div className={styles.roomInfoContent}>
        <span>{t('roomID')}</span>:<span>{room.roomId?.replace('call_', '')}</span>
        <span className={styles.sperature} />
        <span>{Utils.formatTime(time)}</span>
      </div>
    </div>
  );
}

export default RoomInfo;
