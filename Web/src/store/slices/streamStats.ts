import { createSlice } from '@reduxjs/toolkit';
import { LocalStreamStats, RemoteStreamStats } from '@volcengine/rtc';

export interface StatsState {
  [key: string]: RemoteStreamStats | LocalStreamStats;
}
const initialState: StatsState = {};

export const statsSlice = createSlice({
  name: 'stats',
  initialState,
  reducers: {
    updateLocal: (state, { payload }) => {
      state.local = payload;
    },
    updateRemote: (state, { payload }) => {
      state[payload.userId] = payload;
    },
  },
});

export const { updateLocal, updateRemote } = statsSlice.actions;

export default statsSlice.reducer;
