import { configureStore } from '@reduxjs/toolkit';
import roomSlice, { RoomState } from './slices/room';
import deviceSlice, { DeviceState } from './slices/device';
import streamSlice, { StreamState } from './slices/stream';
import statsSlice, { StatsState } from './slices/streamStats';

export interface RootState {
  room: RoomState;
  device: DeviceState;
  stream: StreamState;
  streamStats: StatsState;
}

const store = configureStore({
  reducer: {
    room: roomSlice,
    device: deviceSlice,
    stream: streamSlice,
    streamStats: statsSlice,
  },
  middleware: (getDefaultMiddleware) =>
    getDefaultMiddleware({
      serializableCheck: false,
    }),
});

export default store;
