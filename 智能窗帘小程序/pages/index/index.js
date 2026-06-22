// pages/index/index.js - 首页：实时环境数据仪表盘
// 功能：展示温度/湿度/光照/窗帘状态，自动/手动模式切换，告警显示
// 数据来源：OneNET 物模型 API，对应微改工程 7 个 MQTT Publish 函数

const api = require('../../utils/api.js')
const CONFIG = require('../../utils/config.js')
const P = CONFIG.PROPS  // 属性名快捷引用

/**
 * 首页页面对象：展示设备实时状态，并提供自动/手动模式切换。
 * 作者：程磊
 * 时间：2026-04-27 16:35:27 +08:00
 */
Page({
  data: {
    online: false,        // 设备是否在线
    lastUpdate: '--',     // 最后更新时间 HH:MM:SS
    refreshCount: 0,      // 累计刷新次数（调试用）

    // 环境数据（与微改工程 esp8266.c 上报字段一一对应）
    temperature: '--',    // Tmp  温度 ℃
    humidity: '--',       // Hum  湿度 %
    light: '--',          // ldr  光照 lx
    curtainPos: '--',     // curtain 窗帘关闭度：0=全开，100=全关

    // 告警状态（微改工程 alarm_int() 中的联动逻辑）
    alarmTem: false,      // Tem_beep 温度超限
    alarmHum: false,      // Hum_beep 湿度超限
    alarmLight: false,    // CO2_beep 光照超限（字段名保留）

    // 模式
    mode: 1               // mode: 1=自动联动, 0=手动控制
  },

  _timer: null,

  onLoad() {
    this.refreshData()
    this._timer = setInterval(() => {
      this.refreshData()
    }, CONFIG.REFRESH_INTERVAL)
  },

  onUnload() {
    if (this._timer) {
      clearInterval(this._timer)
      this._timer = null
    }
  },

  onShow() {
    // 页面切回前台时立即刷新
    this.refreshData()
  },

  /**
   * 刷新设备数据
   * 解析 OneNET 返回的物模型属性，映射到页面 data
   * 作者：程磊
   * 时间：2026-04-27 16:35:27 +08:00
   */
  refreshData() {
    api.queryProperty().then(data => {
      const P = CONFIG.PROPS

      // 告警字段是 bool string 类型（"true"/"false"）
      const alarmTem = data[P.ALARM_TEM] === true || data[P.ALARM_TEM] === 'true'
      const alarmHum = data[P.ALARM_HUM] === true || data[P.ALARM_HUM] === 'true'
      const alarmLight = data[P.ALARM_LIGHT] === true || data[P.ALARM_LIGHT] === 'true'

      // 更新时间
      const now = new Date()
      const timeStr = now.getHours().toString().padStart(2, '0') + ':' +
                      now.getMinutes().toString().padStart(2, '0') + ':' +
                      now.getSeconds().toString().padStart(2, '0')

      this.setData({
        temperature: data[P.TEMPERATURE] !== undefined ? data[P.TEMPERATURE] : '--',
        humidity: data[P.HUMIDITY] !== undefined ? data[P.HUMIDITY] : '--',
        light: data[P.LIGHT] !== undefined ? data[P.LIGHT] : '--',
        curtainPos: data[P.CURTAIN] !== undefined ? data[P.CURTAIN] : '--',
        alarmTem: alarmTem,
        alarmHum: alarmHum,
        alarmLight: alarmLight,
        mode: api.normalizeMode(data[P.MODE], this.data.mode),
        online: api.isDeviceOnline(data),
        lastUpdate: timeStr,
        refreshCount: this.data.refreshCount + 1
      })
    }).catch(err => {
      console.error('查询设备属性失败:', err)
      this.setData({ online: false })
    })
  },

  /**
   * 切换运行模式（自动/手动）
   * 对应 key.c 中 KEY4 长按的功能
   * 作者：程磊
   * 时间：2026-04-27 16:35:27 +08:00
   */
  setMode(e) {
    const newMode = parseInt(e.currentTarget.dataset.mode)
    if (newMode === this.data.mode) return

    const P = CONFIG.PROPS
    wx.showLoading({ title: '切换中...' })

    api.setProperty({ [P.MODE]: newMode }).then((res) => {
      this.setData({ mode: newMode })
      wx.hideLoading()
      wx.showToast({
        title: res && res.pendingConfirm ? '已下发' : (newMode === 1 ? '已切换自动模式' : '已切换手动模式'),
        icon: 'success'
      })
      setTimeout(() => {
        this.refreshData()
      }, 1500)
    }).catch(err => {
      wx.hideLoading()
      wx.showToast({ title: '切换失败', icon: 'none' })
      console.error('设置模式失败:', err)
    })
  }
})
