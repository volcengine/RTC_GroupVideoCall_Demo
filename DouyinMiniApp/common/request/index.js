const app = getApp();

const bsaeurl = '/rtc_demo_special';

function get(path, options = {}) {
  return new Promise((resolve, reject) => {
    tt.request({
      url: `${app.host}${bsaeurl}${path}`,
      header: {
        'content-type': 'application/json',
        ...(options.header || {}),
      },

      method: 'GET',
      dataType: options.dataType || 'json',
      responseType: options.responseType || 'text',
      success(res) {
        resolve(res);
      },
      fail(err) {
        reject(err);
      },
    });
  });
}

function post(path, data, options = {}) {
  return new Promise((resolve, reject) => {
    tt.request({
      url: `${app.host}${bsaeurl}${path}`,
      data,
      header: {
        'content-type': 'application/json',
        ...(options.header || {}),
      },

      method: 'POST',
      dataType: options.dataType || 'json',
      responseType: options.responseType || 'text',
      success(res) {
        resolve(res);
      },
      fail(err) {
        resolve(err);
      },
    });
  });
}

module.exports.get = get;

module.exports.post = post;
