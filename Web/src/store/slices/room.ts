import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import {
  AudioPropertiesInfo,
  LocalAudioStats,
  LocalVideoStats,
  RemoteAudioStats,
  RemoteVideoStats,
} from '@volcengine/rtc';

export interface IUser {
  username?: string;
  userId?: string;
  publishAudio?: boolean;
  publishVideo?: boolean;
  publishScreen?: boolean;
  audioStats?: RemoteAudioStats;
  videoStats?: RemoteVideoStats;
  audioPropertiesInfo?: AudioPropertiesInfo;
}

export type LocalUser = Omit<IUser, 'audioStats' | 'videoStats'> & {
  loginToken?: string;
  audioStats?: LocalAudioStats;
  videoStats?: LocalVideoStats;
};

export interface RoomState {
  time: number;
  beautyOn: boolean;
  beautyEnabled: boolean;
  roomId?: string;
  localUser: LocalUser;
  remoteUsers: IUser[];
  shareUser?: string;
  autoPlayFailUser: string[];
}
const initialState: RoomState = {
  beautyOn: false,
  beautyEnabled: false,
  time: -1,
  remoteUsers: [],
  localUser: {
    publishAudio: true,
    publishVideo: true,
  },
  autoPlayFailUser: [],
};

export const roomSlice = createSlice({
  name: 'room',
  initialState,
  reducers: {
    localJoinRoom: (
      state,
      {
        payload,
      }: {
        payload: {
          roomId: string;
          user: LocalUser;
        };
      }
    ) => {
      state.roomId = payload.roomId;
      state.localUser = payload.user;
    },
    localLeaveRoom: (state) => {
      state.roomId = undefined;
      state.time = -1;
      state.localUser = {
        publishAudio: true,
        publishVideo: true,
      };
      state.remoteUsers = [];
      state.shareUser = undefined;
    },
    remoteUserJoin: (state, { payload }) => {
      state.remoteUsers.push(payload);
    },
    remoteUserLeave: (state, { payload }) => {
      const findIndex = state.remoteUsers.findIndex((user) => user.userId === payload.userId);
      state.remoteUsers.splice(findIndex, 1);
    },

    updateLocalUser: (state, { payload }: { payload: LocalUser }) => {
      state.localUser = {
        ...state.localUser,
        ...payload,
      };
    },

    updateRemoteUser: (state, { payload }: { payload: IUser | IUser[] }) => {
      if (!Array.isArray(payload)) {
        payload = [payload];
      }

      payload.forEach((user) => {
        const findIndex = state.remoteUsers.findIndex((u) => u.userId === user.userId);
        state.remoteUsers[findIndex] = {
          ...state.remoteUsers[findIndex],
          ...user,
        };
      });
    },

    startShare: (state, { payload }) => {
      state.shareUser = payload.shareUser;
      if (payload.shareUser !== state.localUser.userId) {
        state.remoteUsers = state.remoteUsers.sort((first) => {
          if (first.userId === payload.shareUser) {
            return 0;
          }
          return 1;
        });
      }
    },
    stopShare: (state) => {
      state.shareUser = undefined;
    },

    updateRoomTime: (state, { payload }) => {
      state.time = payload.time;
    },

    setBeauty: (state, action: PayloadAction<boolean>) => {
      state.beautyOn = action.payload;
    },

    setBeautyEnabled: (state, action: PayloadAction<boolean>) => {
      state.beautyEnabled = action.payload;
    },

    addAutoPlayFail: (state, { payload }) => {
      const autoPlayFailUser = state.autoPlayFailUser;
      const index = autoPlayFailUser.findIndex((item) => item === payload.userId);
      if (index === -1) {
        state.autoPlayFailUser.push(payload.userId);
      }
    },
    removeAutoPlayFail: (state, { payload }) => {
      const autoPlayFailUser = state.autoPlayFailUser;
      const _autoPlayFailUser = autoPlayFailUser.filter((item) => item !== payload.userId);
      state.autoPlayFailUser = _autoPlayFailUser;
    },
    clearAutoPlayFail: (state) => {
      state.autoPlayFailUser = [];
    },
  },
});

export const {
  localJoinRoom,
  localLeaveRoom,
  remoteUserJoin,
  remoteUserLeave,
  updateRemoteUser,
  updateLocalUser,
  startShare,
  stopShare,
  updateRoomTime,
  setBeauty,
  setBeautyEnabled,
  addAutoPlayFail,
  removeAutoPlayFail,
  clearAutoPlayFail,
} = roomSlice.actions;

export default roomSlice.reducer;
