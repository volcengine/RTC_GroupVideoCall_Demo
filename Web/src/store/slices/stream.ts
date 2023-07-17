import { createSlice } from '@reduxjs/toolkit';
import { AudioProfileType, MirrorType } from '@volcengine/rtc';
import { AudioProfile, RESOLUTIOIN_LIST } from '@/config';

export interface StreamState {
  videoEncodeConfig: typeof RESOLUTIOIN_LIST[number]['text'];
  audioProfile: AudioProfileType;
  mirror: MirrorType;
  shareScreenConfig: typeof RESOLUTIOIN_LIST[number]['text'];
}
const initialState: StreamState = {
  videoEncodeConfig: RESOLUTIOIN_LIST[0].text,
  audioProfile: AudioProfile[0].type,
  mirror: MirrorType.MIRROR_TYPE_RENDER,
  shareScreenConfig: '1280 * 720',
};

export const streamSlice = createSlice({
  name: 'stream',
  initialState,
  reducers: {
    updateVideoEncodeConfig: (state, { payload }) => {
      state.videoEncodeConfig = payload.videoEncodeConfig;
    },
    updateAudioProfile: (state, { payload }) => {
      state.audioProfile = payload.audioProfile;
    },
    updateMirror: (state, { payload }) => {
      state.mirror = payload.mirror;
    },
    updateAllStreamConfig: (state, { payload }) => {
      state.videoEncodeConfig = payload.videoEncodeConfig;
      state.audioProfile = payload.audioProfile;
      state.mirror = payload.mirror;
    },

    updateShareConfig: (state, { payload }) => {
      state.shareScreenConfig = payload.shareScreenConfig;
    },

    resetConfig: (state) => {
      state.videoEncodeConfig = initialState.videoEncodeConfig;
      state.audioProfile = initialState.audioProfile;
      state.mirror = initialState.mirror;
      state.shareScreenConfig = initialState.shareScreenConfig;
    },
  },
});

export const {
  updateVideoEncodeConfig,
  updateAudioProfile,
  updateAllStreamConfig,
  updateShareConfig,
  resetConfig,
} = streamSlice.actions;

export default streamSlice.reducer;
