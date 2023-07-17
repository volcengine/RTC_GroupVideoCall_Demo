// import { StreamIndex } from '@volcengine/rtc';
import { Form, Modal, Select, Switch } from 'antd';
import { useState } from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { useTranslation } from 'react-i18next';
import MediaButton from '@/components/MediaButton';
import { getIcon } from '@/components/MediaButton/utils';
import { AudioProfile, RESOLUTIOIN_LIST } from '@/config';
// import RtcClient from '@/lib/RtcClient';
import { RootState } from '@/store';
import { updateAllStreamConfig } from '@/store/slices/stream';
import styles from './index.module.less';

interface SettingProps {
  shared?: boolean;
  btnClassName?: string;
  iconClassName?: string;
}
function Setting(props: SettingProps) {
  const { shared, btnClassName, iconClassName } = props;
  const [modalVisible, setModalVisible] = useState(false);
  const stream = useSelector((state: RootState) => state.stream);
  const dispatch = useDispatch();
  const [form] = Form.useForm();
  const { t } = useTranslation();
  const handleOk = () => {
    const formValues = form.getFieldsValue();

    const newStreamConfig = {
      ...formValues,
      mirror: +formValues.mirror,
    };

    // if (stream.mirror !== newStreamConfig.mirror) {
    //   RtcClient.setMirrorType(newStreamConfig.mirror);
    // }

    // if (stream.audioProfile !== newStreamConfig.audioProfile) {
    //   RtcClient.setAudioProfile(newStreamConfig.audioProfile);
    // }

    // if (stream.videoEncodeConfig !== newStreamConfig.videoEncodeConfig) {
    //   const encodeConfig = RESOLUTIOIN_LIST.find(
    //     (resolution) => resolution.text === newStreamConfig.videoEncodeConfig
    //   );
    //   RtcClient.setVideoCaptureConfig(encodeConfig!.val);
    //   RtcClient.setVideoEncoderConfig(StreamIndex.STREAM_INDEX_MAIN, encodeConfig!.val);
    // }

    dispatch(updateAllStreamConfig(newStreamConfig));

    setModalVisible(false);
  };

  const handleCancel = () => {
    form.resetFields();
    setModalVisible(false);
  };

  return (
    <>
      <MediaButton
        className={btnClassName}
        iconClassName={iconClassName}
        text={shared ? undefined : t('Settings')}
        icon={getIcon('setting')}
        onClick={() => {
          setModalVisible(true);
        }}
      />

      <Modal
        title={t('Settings')}
        visible={modalVisible}
        cancelText={t('Cancel')}
        okText={t('OK')}
        closeIcon={<img src={getIcon('close')} alt="close" />}
        transitionName=""
        maskTransitionName=""
        width={400}
        wrapClassName={styles.settingModal}
        onOk={handleOk}
        onCancel={handleCancel}
      >
        <Form
          colon={false}
          form={form}
          labelCol={{ span: 7 }}
          wrapperCol={{ span: 17 }}
          initialValues={{
            ...stream,
            mirror: !!stream.mirror,
          }}
        >
          <Form.Item label={t('resolution')} name="videoEncodeConfig">
            <Select>
              {RESOLUTIOIN_LIST.map((resolution) => (
                <Select.Option key={resolution.text} value={resolution.text}>
                  {resolution.text}
                </Select.Option>
              ))}
            </Select>
          </Form.Item>
          <Form.Item label={t('SoundQuality')} name="audioProfile">
            <Select>
              {AudioProfile.map((profile) => (
                <Select.Option key={profile.type} value={profile.type}>
                  {profile.text}
                </Select.Option>
              ))}
            </Select>
          </Form.Item>
          <Form.Item label={t('Mirroring')} name="mirror" valuePropName="checked">
            <Switch checkedChildren="ON" unCheckedChildren="OFF" />
          </Form.Item>
        </Form>
      </Modal>
    </>
  );
}

export default Setting;
