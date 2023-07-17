import React, { useEffect, useState } from 'react';
import { Modal, Button } from 'antd';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from '@/store';
import RtcClient from '@/lib/RtcClient';
import { clearAutoPlayFail } from '@/store/slices/room';

import styles from './index.module.less';

function AutoPlayModal() {
  const [visible, setVisible] = useState<boolean>(false);
  const room = useSelector((state: RootState) => state.room);
  const dispatch = useDispatch();

  useEffect(() => {
    setVisible(!!room.autoPlayFailUser.length);
  }, [room.autoPlayFailUser]);

  const handleAutoPlay = () => {
    const users: string[] = room.autoPlayFailUser;
    if (users && users.length) {
      users.forEach((user) => {
        RtcClient?.engine.play(user);
      });
    }
    dispatch(clearAutoPlayFail());
  };

  return (
    <Modal
      className={styles.container}
      footer={null}
      closable={false}
      width={450}
      visible={visible}
      style={{ top: '20%', minHeight: 150 }}
    >
      <Button className={styles.button} block size="large" onClick={handleAutoPlay}>
        自动播放
      </Button>
    </Modal>
  );
}

export default AutoPlayModal;
