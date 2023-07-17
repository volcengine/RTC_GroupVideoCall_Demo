import { useFreeLoginMutation } from '@/app/roomQuery';
import Utils from '@/utils/utils';

export const useFreeLogin = (): {
  freeLoginApi: (user_name: string) => Promise<Record<string, string>>;
} => {
  const [freeLogin] = useFreeLoginMutation();

  const freeLoginApi = async (username: string) => {
    try {
      const res = await freeLogin({
        device_id: Utils.getDeviceId(),
        user_name: username,
      });

      if ('data' in res) {
        if (res?.data?.code === 200) {
          Utils.setLoginToken(res?.data.response.login_token);
          return res?.data.response;
        }
      }
      return {};
    } catch (err) {
      return {};
    }
  };

  return {
    freeLoginApi,
  };
};
