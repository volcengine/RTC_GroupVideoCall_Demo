import { Button, Form, Input, message as Message } from 'antd';
import { useSelector } from 'react-redux';
import { useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import Header from '@/components/Header';
import styles from './index.module.less';
import MediaButton from '@/components/MediaButton';
import { DeviceType } from '@/interface';
import { MediaName, medias } from '@/store/slices/device';
import { RootState } from '@/store';
import { getIcon } from '@/components/MediaButton/utils';
import Utils from '@/utils/utils';
import { useJoin, useGetDevicePermission } from '../../lib/useCommon';
import Setting from '../View/BottomMenu/Setting';
import TeaClient, { TeaEventSource } from '@/lib/TeaClient';

export interface FormProps {
  username: string;
  roomId: string;
  publishAudio: boolean;
  publishVideo: boolean;
}

export default function () {
  const localUser = useSelector((state: RootState) => state.room.localUser);

  const localRoomId = useSelector(
    (state: RootState) => state.room.roomId?.replace('call_', '') || Utils.getUrlArgs()?.roomId
  );

  const devicePermissions = useSelector((state: RootState) => state.device.devicePermissions);
  const permission = useGetDevicePermission();

  const { t } = useTranslation();

  const [form] = Form.useForm();
  const publishVideo = Form.useWatch('publishVideo', form) ?? true;
  const publishAudio = Form.useWatch('publishAudio', form) ?? true;

  const [joining, dispatchJoin] = useJoin();

  const handleStart = async (formValues: FormProps) => {
    if (joining) {
      return;
    }
    dispatchJoin(
      {
        ...formValues,
      },
      false
    );
  };

  const getMediaStatus = (media: DeviceType) => {
    switch (media) {
      case DeviceType.Camera:
        return publishVideo;
      case DeviceType.Microphone:
        return publishAudio;
      default:
        return publishAudio;
    }
  };

  const getTransKey = (media: DeviceType, enable: boolean) => {
    return `${media}${enable ? 'Enabled' : 'Disabled'}`;
  };

  const handleCameraPermChange = (toOn: boolean) => {
    TeaClient.reportToggleCameraButton(toOn, TeaEventSource.LOGIN_PAGE);
  };

  const handleMicrophonePermChange = (toOn: boolean) => {
    TeaClient.reportToggleMicrophoneButton(toOn, TeaEventSource.LOGIN_PAGE);
  };

  useEffect(() => {
    if (!permission) return;
    if (!permission.video) {
      Message.error(t('noCameraPerm'));
      form.setFieldValue('publishVideo', false);
    }
    if (!permission.audio) {
      Message.error(t('noMicPerm'));
      form.setFieldValue('publishAudio', false);
    }
  }, [permission]);

  return (
    <div className={styles.container}>
      <Header />
      <div className={styles['form-wrapper']}>
        <div className={styles.main}>
          <h1>{t('videocalls')}</h1>
          <Form
            form={form}
            onFinish={handleStart}
            initialValues={{ ...localUser, roomId: localRoomId }}
          >
            <Form.Item
              name="roomId"
              validateTrigger="onChange"
              rules={[
                {
                  required: true,
                  validator: (_, value) => {
                    const res = /^[0-9]{1,18}$/.test(value);
                    if (!res) {
                      return Promise.reject(new Error(t('roomIdError')));
                    }
                    return Promise.resolve();
                  },
                },
              ]}
            >
              <Input placeholder={t('roomIdPlaceholder')} className={styles['login-input']} />
            </Form.Item>
            <Form.Item
              name="username"
              validateTrigger="onChange"
              rules={[
                {
                  required: true,
                  validator: (_, value) => {
                    const res = /^[a-zA-Z0-9_@\u4e00-\u9fa5]*$/.test(value);
                    if (!res) {
                      return Promise.reject(new Error(t('usernameError')));
                    }

                    if (value.length === 0) {
                      return Promise.resolve();
                    }

                    const lengthRes = /^[a-zA-Z0-9_@\u4e00-\u9fa5]{1,128}$/.test(value);
                    if (!lengthRes) {
                      return Promise.reject(new Error(t('usernameLengthError')));
                    }

                    return Promise.resolve();
                  },
                },
              ]}
            >
              <Input placeholder={t('usernamePlaceHolder')} className={styles['login-input']} />
            </Form.Item>
            <div className={styles.mediaButtons}>
              {medias.map((media) => {
                const mediaStatus = getMediaStatus(media);

                return (
                  <Form.Item
                    key={media}
                    noStyle
                    name={MediaName[media] === 'camera' ? 'publishVideo' : 'publishAudio'}
                  >
                    <MediaButton
                      text={t(getTransKey(media, mediaStatus))}
                      iconClassName={styles.mediaIcon}
                      icon={getIcon(MediaName[media], mediaStatus ? 'On' : 'Off')}
                      disabled={
                        MediaName[media] === 'camera'
                          ? !devicePermissions.video
                          : !devicePermissions.audio
                      }
                      disableMsg={
                        MediaName[media] === 'camera' ? t('noCameraPerm') : t('noMicPerm')
                      }
                      onChange={
                        MediaName[media] === 'camera'
                          ? handleCameraPermChange
                          : handleMicrophonePermChange
                      }
                      className={styles.loginButton}
                    />
                  </Form.Item>
                );
              })}
              <Form.Item noStyle>
                <Setting btnClassName={styles.loginButton} iconClassName={styles.mediaIcon} />
              </Form.Item>
            </div>

            <Form.Item
              noStyle
              shouldUpdate={(prevValues, curValues) =>
                ['roomId', 'username'].some((key) => prevValues[key] !== curValues[key])
              }
            >
              {({ getFieldValue }) => (
                <Button
                  type="primary"
                  htmlType="submit"
                  disabled={!getFieldValue('roomId') || !getFieldValue('username') || joining}
                  loading={joining}
                  className={styles.startButton}
                >
                  {t('startVideoCall')}
                </Button>
              )}
            </Form.Item>
          </Form>
        </div>
      </div>
    </div>
  );
}
