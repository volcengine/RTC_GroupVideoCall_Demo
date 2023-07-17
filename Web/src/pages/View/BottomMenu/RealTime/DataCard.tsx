import {
  LocalAudioStats,
  RemoteAudioStats,
  LocalVideoStats,
  RemoteVideoStats,
} from '@volcengine/rtc';
import { useTranslation } from 'react-i18next';
import styles from './index.module.less';
import { getDataList } from './utils';

export const enum Status {
  UNKNOWN = 'Unknown',
  EXCELLENT = 'Excellent',
  GOOD = 'Good',
  POOR = 'Poor',
}

export interface DataItemProps {
  name: string;
  text: string;
  status: Status;
}

function DataItem(props: DataItemProps) {
  const { name, text, status } = props;
  return (
    <div className={styles.dataItem}>
      <p className={`${styles.dataItemText} ${styles[`indicator${status}`]}`}>{text}</p>
      <p className={styles.dataItemName}>{name}</p>
    </div>
  );
}

interface BaseRealTime {
  userId?: string;
  isLocal?: boolean;
  username?: string;
  txQuality?: number;
  rxQuality?: number;
}

export type VideoRealTime = BaseRealTime & {
  videoStats?: LocalVideoStats | RemoteVideoStats;
};

export type AudioRealTime = BaseRealTime & {
  audioStats?: LocalAudioStats | RemoteAudioStats;
};

export interface DataCardProps {
  data: VideoRealTime | AudioRealTime;
  type: 'video' | 'audio';
}

function DataCard(props: DataCardProps) {
  const { data, type } = props;

  const { t } = useTranslation();
  const dataList = getDataList({ data, type }, t);

  return (
    <div className={styles.dataCard}>
      <p className={styles.username}>
        <span className={styles.usernameWrapper}>{data?.username}</span>
        {data.isLocal ? `(${t('Me')})` : ''}
      </p>
      <div className={styles.dataCardContent}>
        {dataList.map((item) => (
          <DataItem {...item} key={item.name} />
        ))}
      </div>
    </div>
  );
}

export default DataCard;
