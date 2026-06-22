/**
 * OneNET 平台 API 配置
 *
 * 使用说明:
 *   TOKEN 生成方式（当前Token过期或失效时重新生成）：
 *   https://open.iot.10086.cn/doc/v5/fuse/detail/867
 *   access_key 在 OneNET 控制台 → 产品 → 设备详情 → 产品密钥 中获取
 */

var CONFIG = {
  // ===== OneNET 产品信息 =====
  PRODUCT_ID: 'BTpvEtwX1a',    /* 产品ID */
  DEVICE_NAME: 'home',         /* 设备名称 */

  // ===== OneNET HTTP API =====
  // 注意：域名是 iot-api.heclouds.com（有横杠），
  API_BASE: 'https://iot-api.heclouds.com',
  API_QUERY: '/thingmodel/query-device-property',  /* 查询设备属性 */
  API_SET: '/thingmodel/set-device-property',      /* 下发属性设置 */

  // ===== 鉴权 Token =====
  // 与 esp8266.h 中的 password 完全一致
  TOKEN: 'version=2018-10-31&res=products%2FBTpvEtwX1a%2Fdevices%2Fhome&et=1832067109&method=md5&sign=CxUhJ6FqWg2M2t9orRz1zA%3D%3D',

  // ===== 数据刷新间隔（毫秒） =====
  REFRESH_INTERVAL: 3000,

  // ===== ???????????? =====
  // ??? 1~2 ???????????? 15 ????????????
  ONLINE_TIMEOUT: 15000,

  // ===== STM32 上报的属性名（与 esp8266.h 中的物模型一致） =====
  PROPS: {
    TEMPERATURE: 'Tmp',
    HUMIDITY: 'Hum',
    LIGHT: 'ldr',
    ALARM_TEM: 'Tem_beep',
    ALARM_HUM: 'Hum_beep',
    ALARM_LIGHT: 'CO2_beep',
    CURTAIN: 'curtain',
    MODE: 'mode',
    TEM_SET: 'tem_set',
    LDR_SET: 'ldr_set'
  }
}

module.exports = CONFIG
