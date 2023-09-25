import { fetchBaseQuery } from '@reduxjs/toolkit/query';
import { HOST, BOE_HOST, isProd } from '@/config';

const baseQuery = fetchBaseQuery({
  baseUrl: isProd ? HOST : BOE_HOST,
  mode: 'cors',
});

export interface BasicBody {
  room_id: string;
  user_id: string;
  login_token: string;
}

export default baseQuery;
