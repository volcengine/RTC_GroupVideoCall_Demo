import { useDispatch } from 'react-redux';
import { useTranslation } from 'react-i18next';
import RtcClient from '@/lib/RtcClient';
import { stopShare } from '@/store/slices/room';
import styles from './index.module.less';
import TeaClient from '@/lib/TeaClient';

function StopShareBtn() {
  const dispatch = useDispatch();
  const { t } = useTranslation();

  const handleStopShare = async () => {
    RtcClient.sendServerMessage('videocallEndShareScreen');
    await RtcClient.stopScreenCapture();
    TeaClient.reportStopScreenSharing();
    dispatch(stopShare());
  };

  return (
    <div className={styles.StopShareWrapper}>
      <div className={styles.stopSperature} />
      <div className={styles.stop} onClick={handleStopShare}>
        {t('stopShare')}
      </div>
    </div>
  );
}

export default StopShareBtn;
