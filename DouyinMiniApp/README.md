## 操作步骤

### **步骤 1：获取 AppID 和 AppKey**

在火山引擎控制台->[应用管理](https://console.volcengine.com/rtc/listRTC)页面创建应用或使用已创建应用获取 **AppID** 和 **AppAppKey**

### **步骤 2：获取 AccessKeyID 和 SecretAccessKey**

在火山引擎控制台-> [密钥管理](https://console.volcengine.com/iam/keymanage/)页面获取 **AccessKeyID** 和 **SecretAccessKey**

## 配置 Demo 工程文件

### 配置 AppID、AppKey、AccessKeyID、SecretAccessKey

1. 进入工程目录，修改 appId 和 appKey。使用控制台获取的 AppID 替换当前文件夹下 app.js 里的 `Your_AppID`, AppKey 替换 `Your_AppKey`
2. 进入工程目录，修改 accessKeyID 和 secretAccesskey 使用控制台获取的 AccessKeyID 替换当前文件夹下 appConfig.ts 里的 `Your_AccessKey`,SecretAccessKey 替换 `Your_SecretAccesskey`



### 配置 HOST

进入工程目录，修改 HOST。使用自己的服务 HOST 替换 src 文件夹下 appConfig.ts 里的 `HOST`。

当前你可以使用 https://common.rtc.volcvideo.com/rtc_demo_special 作为测试服务器域名，仅提供跑通测试服务，无法保障正式需求。

## 运行小程序
你需要创建自己的抖音小程序,参考[创建小程序](https://developer.open-douyin.com/docs/resource/zh-CN/mini-app/develop/guide/preparation/mini-app-setup)

