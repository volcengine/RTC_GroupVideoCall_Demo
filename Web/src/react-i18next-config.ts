/**
 * Copyright 2022 Beijing Volcanoengine Technology Ltd. All Rights Reserved.
 * SPDX-license-identifier: BSD-3-Clause
 */

import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import cn from './locales/zh-cn.json';
import en from './locales/en-us.json';

const resources = {
  cn: {
    translation: cn,
  },
  en: {
    translation: en,
  },
};
i18n.use(initReactI18next).init({
  resources,
  fallbackLng: process.env.REACT_APP_LOCAL,
});

export default i18n;
