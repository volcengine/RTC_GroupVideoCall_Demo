import { createApi } from '@reduxjs/toolkit/query/react';
import baseQuery from './baseQuery';

export const roomQuery = createApi({
  baseQuery,
  reducerPath: 'room',
  endpoints: (build) => ({
    freeLogin: build.mutation({
      query: (body: { device_id: string; user_name: string }) => ({
        url: '/login',
        method: 'POST',
        body: {
          event_name: 'passwordFreeLogin',
          device_id: body.device_id,
          content: JSON.stringify({
            user_name: body.user_name,
          }),
        },
      }),
    }),
    joinRTM: build.mutation({
      query: (body: { login_token: string; device_id: string }) => ({
        url: '/login',
        method: 'POST',
        body: {
          event_name: 'joinRTS',
          device_id: body.device_id,
          content: JSON.stringify({
            login_token: body.login_token,
            scenes_name: 'videocall',
          }),
        },
      }),
    }),
  }),
});

export const { useJoinRTMMutation, useFreeLoginMutation } = roomQuery;
