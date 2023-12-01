import { useDispatch, useSelector } from 'react-redux';
import { useEffect, useMemo, useRef, useState } from 'react';
import { Radio, RadioChangeEvent } from 'antd';
import { useTranslation } from 'react-i18next';
import MediaButton from '@/components/MediaButton';
import { getIcon } from '@/components/MediaButton/utils';
import styles from './index.module.less';
import ArrowIcon from '@/assets/img/Arrow.svg';
import { RootState } from '@/store';
import { updateSelectedDevice } from '@/store/slices/device';
import RtcClient from '@/lib/RtcClient';
import TeaClient from '@/lib/TeaClient';

interface DeviceButtonProps {
  deviceType: 'camera' | 'microphone';
  onClick?: (deviceType: 'camera' | 'microphone') => void;
  shared?: boolean;
  text?: string;
}

function DeviceButton(props: DeviceButtonProps) {
  const { onClick, shared, deviceType, text } = props;
  const [showOptions, setShowOptions] = useState(false);
  const devicePermissions = useSelector((state: RootState) => state.device.devicePermissions);
  const { t } = useTranslation();

  const devices = useSelector((state: RootState) => state.device);
  const localUser = useSelector((state: RootState) => state.room.localUser);

  const dispatch = useDispatch();
  const btnRef = useRef<HTMLDivElement>(null);

  const handleClick = () => {
    onClick && onClick(deviceType);
  };

  useEffect(() => {
    const hidePop = (e: Event) => {
      if (!btnRef.current?.contains(e.target as unknown as HTMLElement)) {
        setShowOptions(false);
      }
    };

    window.addEventListener('click', hidePop);
    return () => {
      window.removeEventListener('click', hidePop);
    };
  }, [btnRef]);

  const handleSetDevices = () => {
    setShowOptions(!showOptions);
  };

  const handleDeviceChange = (e: RadioChangeEvent) => {
    if (deviceType === 'microphone') {
      TeaClient.reportSwitchMicrophone(
        e.target.value,
        devices.selectedMicrophone,
        devices.audioInputs
      );
    } else {
      TeaClient.reportSwitchCamera(e.target.value, devices.selectedCamera, devices.videoInputs);
    }
    RtcClient.switchDevice(deviceType, e.target.value);

    dispatch(
      updateSelectedDevice({
        [deviceType === 'microphone' ? 'selectedMicrophone' : 'selectedCamera']: e.target.value,
      })
    );

    setShowOptions(false);
  };

  const deviceList = useMemo(
    () => (deviceType === 'microphone' ? devices.audioInputs : devices.videoInputs),
    [devices, deviceType]
  );

  return (
    <div className={styles.menuButton} ref={btnRef}>
      <MediaButton
        iconClassName={styles.cameraButtonIcon}
        onClick={handleClick}
        text={shared ? undefined : text || t(deviceType)}
        icon={getIcon(
          deviceType,
          localUser?.[deviceType === 'microphone' ? 'publishAudio' : 'publishVideo'] ? 'On' : 'Off'
        )}
        disabled={deviceType === 'camera' ? !devicePermissions.video : !devicePermissions.audio}
        disableMsg={deviceType === 'camera' ? t('noCameraPerm') : t('noMicPerm')}
      />
      <div
        className={`${styles.ArrowIcon} ${showOptions ? styles.ArrowIconRotate : ''}`}
        onClick={handleSetDevices}
      >
        <img src={ArrowIcon} alt="arrow" />
      </div>

      <div
        className={styles.devicesList}
        onMouseLeave={() => {
          setShowOptions(false);
        }}
        style={{
          display: showOptions ? 'block' : 'none',
        }}
      >
        <div className={styles.mediaDevicesContent}>
          <Radio.Group
            onChange={handleDeviceChange}
            value={
              deviceType === 'microphone' ? devices.selectedMicrophone : devices.selectedCamera
            }
          >
            {deviceList.map((device) => (
              <Radio value={device.deviceId} key={device.deviceId}>
                {device.label}
              </Radio>
            ))}
          </Radio.Group>
        </div>
      </div>
    </div>
  );
}

export default DeviceButton;
