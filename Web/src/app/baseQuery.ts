import { fetchBaseQuery } from '@reduxjs/toolkit/query';
import { HOST } from '@/appConfig';

const baseQuery = fetchBaseQuery({
  baseUrl: HOST,
  mode: 'cors',
});

export interface BasicBody {
  room_id: string;
  user_id: string;
  login_token: string;
}

export default baseQuery;
