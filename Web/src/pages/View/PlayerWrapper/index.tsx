import { useEffect, useMemo, useState } from 'react';
import { useSelector } from 'react-redux';
import { MAX_PLAYERS } from '@/config';
import { RootState } from '@/store';
import styles from './index.module.less';
import LocalShare from './PlayerLayout/LocalShare';
import RemoteShare from './PlayerLayout/RemoteShare';
import PlayerLayout from './PlayerLayout/PlayerLayout';
import { IUser } from '@/store/slices/room';

function PlayerWrapper() {
  const room = useSelector((state: RootState) => state.room);
  const playerPages = useMemo(
    () => Math.ceil((room.remoteUsers.length + 1) / MAX_PLAYERS),
    [room.remoteUsers]
  );

  const [curPlayerPage, setCurPlayerPage] = useState(0);

  useEffect(() => {
    if (curPlayerPage + 1 > playerPages) {
      setCurPlayerPage(playerPages - 1);
    }
  }, [playerPages]);

  const curPlayingUsers = useMemo(() => {
    if (room.shareUser) {
      return [room.localUser, ...room.remoteUsers];
    }

    let curPlayingRemoteUsers: IUser[] = [];
    switch (curPlayerPage) {
      case 1:
        curPlayingRemoteUsers = room.remoteUsers.filter(
          (_, index) => index >= MAX_PLAYERS - 1 && index < 2 * MAX_PLAYERS - 1
        );
        break;
      case 2:
        curPlayingRemoteUsers = room.remoteUsers.filter(
          (_, index) => index >= 2 * MAX_PLAYERS - 1 && index < 3 * MAX_PLAYERS - 1
        );
        break;

      case 0:
      default:
        curPlayingRemoteUsers = room.remoteUsers.filter(
          (_, index) => index >= 0 && index < MAX_PLAYERS - 1
        );
        break;
    }

    if (curPlayerPage === 0) {
      return [room.localUser, ...curPlayingRemoteUsers];
    }
    return curPlayingRemoteUsers;
  }, [curPlayerPage, room.remoteUsers, room.localUser, room.shareUser]);

  const isLocalUserShared: boolean = useMemo(() => {
    return !!(room.localUser.userId && room.localUser?.userId === room.shareUser);
  }, [room.localUser, room.shareUser]);

  const isRemoteUserShared: boolean = useMemo(() => {
    return !!(room.shareUser && room.localUser?.userId !== room.shareUser);
  }, [room.localUser, room.shareUser]);

  if (!room.roomId) {
    return (
      <div
        className={styles.playerWrapper}
        style={{
          height: isLocalUserShared ? '100%' : 'calc(100% - 50px - 64px)',
        }}
      />
    );
  }

  return (
    <div
      className={styles.playerWrapper}
      style={{
        height: isLocalUserShared ? '100%' : 'calc(100% - 50px - 64px)',
      }}
    >
      <div
        style={{
          height: playerPages > 1 && !isLocalUserShared ? 'calc(100% - 8px)' : '100%',
          display: 'flex',
          width: '100%',
        }}
      >
        <LocalShare localShare={isLocalUserShared} />
        <RemoteShare remoteShare={isRemoteUserShared} />
        <PlayerLayout
          renderUsers={curPlayingUsers}
          curPlayerPage={curPlayerPage}
          isLocalUserShared={isLocalUserShared}
          isRemoteUserShared={isRemoteUserShared}
        />
      </div>
      <div
        className={styles.steps}
        style={{
          display: playerPages > 1 && !room.shareUser ? 'flex' : 'none',
        }}
      >
        {new Array(playerPages).fill(null).map((_, page) => (
          <div
            key={page}
            className={`${styles.step} ${curPlayerPage === page && styles.activeStep}`}
            onClick={() => setCurPlayerPage(page)}
          />
        ))}
      </div>
    </div>
  );
}

export default PlayerWrapper;
