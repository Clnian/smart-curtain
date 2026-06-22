# 智能窗帘微信小程序

## 项目简介
配合 STM32 智能窗帘控制系统，通过 OneNET 云平台实时查看设备数据、控制窗帘关闭度。

## 功能
- **首页仪表盘**：实时显示温度、湿度、光照、告警状态
- **窗帘控制**：滑动条/快捷按钮控制窗帘关闭度（0全开，100全关）
- **模式切换**：自动/手动模式一键切换
- **操作日志**：记录操作历史

## 使用前

### 1. 获取 OneNET Token
1. 登录 OneNET 控制台：https://open.iot.10086.cn/
2. 点击「概览」→ 复制「用户 ID」和「access_key」
3. 打开 `utils/config.js`，填入：
   - `USER_ID` → 你的用户ID
   - `ACCESS_KEY` → 你的access_key

## 文件结构
```
智能窗帘小程序/
├── app.js              小程序入口
├── app.json            全局配置
├── app.wxss            全局样式
├── pages/
│   ├── index/          首页
│   └── curtain/        窗帘控制页
└── utils/
    ├── config.js       OneNET 配置
    └── api.js          API 工具函数
```

## 对应的设备标识符
| 标识符 | 类型 | 说明 |
|--------|------|------|
| Tmp | float | 温度 (°C) |
| Hum | float | 湿度 (%) |
| ldr | int | 光照 (lx) |
| Tem_beep | bool | 温度告警 |
| Hum_beep | bool | 湿度告警 |
| ldr_beep | bool | 光照告警 |
| curtain | int | 窗帘关闭度 (0全开 ~ 100全关) |
| mode | int | 运行模式 (0手动/1自动) |
