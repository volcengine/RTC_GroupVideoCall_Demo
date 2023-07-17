## 操作步骤

### **步骤 1：获取 AppID 和 AppKey**

在火山引擎控制台->[应用管理](https://console.volcengine.com/rtc/listRTC)页面创建应用或使用已创建应用获取 **AppID** 和 **AppAppKey**

### **步骤 2：获取 AccessKeyID 和 SecretAccessKey**

在火山引擎控制台-> [密钥管理](https://console.volcengine.com/iam/keymanage/)页面获取 **AccessKeyID** 和 **SecretAccessKey**

## 配置 Demo 工程文件

### 配置 AppID、AppKey、AccessKeyID、SecretAccessKey

1. 进入工程目录，修改 appId 和 appKey。使用控制台获取的 AppID 替换 src 文件夹下 appConfig.ts 里的 `Your_AppID`, AppKey 替换 `Your_AppKey`
2. 进入工程目录，修改 volcAk 和 volcSk。 使用控制台获取的 AccessKeyID 替换 src 文件夹下 appConfig.ts 里的 `Your_AccessKeyID`,SecretAccessKey 替换 `Your_SecretAccessKey`

```
export const appId = 'Your_AppID';
export const appKey = 'Your_AppKey';
export const volcAk = 'Your_AccessKeyID';
export const volcSk = 'Your_SecretAccessKey';

```

### 配置 HOST

进入工程目录，修改 HOST。使用自己的服务 HOST 替换 src 文件夹下 appConfig.ts 里的 `HOST`。

当前你可以使用 https://common.rtc.volcvideo.com/rtc_demo_special 作为测试服务器域名，仅提供跑通测试服务，无法保障正式需求。

## 运行 demo

1. 全局安装 NodeJS

2. 安装 yarn

```
 npm install -g yarn
```

3. 安装依赖

```
yarn
```

4. 启动项目

```
yarn dev
```
