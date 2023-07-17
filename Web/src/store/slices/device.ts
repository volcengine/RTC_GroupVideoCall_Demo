import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { DeviceType } from '@/interface';

export const medias = [DeviceType.Camera, DeviceType.Microphone];

export const MediaName = {
  [DeviceType.Camera]: 'camera',
  [DeviceType.Microphone]: 'microphone',
};

export interface DeviceState {
  videoInputs: MediaDeviceInfo[];
  audioInputs: MediaDeviceInfo[];
  selectedCamera?: string;
  selectedMicrophone?: string;
  devicePermissions: {
    video: boolean;
    audio: boolean;
  };
}
const initialState: DeviceState = {
  videoInputs: [],
  audioInputs: [],
  devicePermissions: {
    video: true,
    audio: true,
  },
};

export const DeviceSlice = createSlice({
  name: 'deivce',
  initialState,
  reducers: {
    updateMediaInputs: (state, { payload }) => {
      state.videoInputs = payload.videoInputs;
      state.audioInputs = payload.audioInputs;
    },
    updateSelectedDevice: (state, { payload }) => {
      if (payload.selectedCamera) {
        state.selectedCamera = payload.selectedCamera;
      }
      if (payload.selectedMicrophone) {
        state.selectedMicrophone = payload.selectedMicrophone;
      }
    },

    setCameraList: (state, action: PayloadAction<MediaDeviceInfo[]>) => {
      state.videoInputs = action.payload;
    },

    setMicrophoneList: (state, action: PayloadAction<MediaDeviceInfo[]>) => {
      state.audioInputs = action.payload;
    },

    setDevicePermissions: (
      state,
      action: PayloadAction<{
        video: boolean;
        audio: boolean;
      }>
    ) => {
      state.devicePermissions = action.payload;
    },
  },
});
export const {
  updateMediaInputs,
  updateSelectedDevice,
  setCameraList,
  setMicrophoneList,
  setDevicePermissions,
} = DeviceSlice.actions;

export default DeviceSlice.reducer;
