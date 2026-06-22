// pages/setting/setting.js - 参数设置页
// 功能：调整温度阈值、光照阈值、迟滞死区宽度
// 对应微改工程 key.c OLED 第3页：
//   KEY1 切换选中参数，KEY2 减小，KEY3 增大
//   Tem_Up (温度阈值 -20~60℃)，ldr_Down (光照阈值 0~1000 lx)

const api = require('../../utils/api.js')
const CONFIG = require('../../utils/config.js')

/**
 * 参数设置页面对象：负责温度阈值、光照阈值的读取、编辑和保存。
 * 作者：程磊
 * 时间：2026-04-27 16:35:27 +08:00
 */
Page({
  data: {
    // 当前阈值（从 OneNET 读取，对应 key.c 全局变量）
    temThreshold: 30,     // Tem_Up 温度阈值 ℃
    lightThreshold: 500,  // ldr_Down 光照阈值 lx
    deviceTemThreshold: 30,     // 设备当前上发的温度阈值
    deviceLightThreshold: 500,  // 设备当前上发的光照阈值

    // 联动算法说明
    algorithmDesc: [
      '夏季防晒：温度 ≥ 阈值 且 光照 > 阈值 → 关窗',
      '冬季采暖：温度 < 阈值 且 光照 > 阈值 → 开窗',
      '夜间隐私：光照 < 150 lx → 强制关窗',
      '迟滞死区：±100 lx 防止电机来回抖动'
    ],

    // 设置状态
    setting: false, // 是否正在保存中
    editing: false, // 用户正在编辑草稿值，暂停云端覆盖
    dirty: false    // 是否有未保存修改
  },

  _timer: null,

  onLoad() {
    this.loadThresholds()
  },

  onShow() {
    this.loadThresholds()
    this.startRefreshTimer()
  },

  onHide() {
    this.stopRefreshTimer()
  },

  onUnload() {
    this.stopRefreshTimer()
  },

  startRefreshTimer() {
    if (this._timer) return
    this._timer = setInterval(() => {
      this.loadThresholds()
    }, CONFIG.REFRESH_INTERVAL)
  },

  stopRefreshTimer() {
    if (this._timer) {
      clearInterval(this._timer)
      this._timer = null
    }
  },

  /**
   * 从 OneNET 读取当前阈值
   * 作者：程磊
   * 时间：2026-04-27 16:35:27 +08:00
   */
  loadThresholds() {
    if (this.data.setting) return

    api.queryProperty().then(data => {
      const P = CONFIG.PROPS
      const update = {}

      if (data[P.TEM_SET] !== undefined) {
        update.deviceTemThreshold = parseFloat(data[P.TEM_SET])
      }
      if (data[P.LDR_SET] !== undefined) {
        update.deviceLightThreshold = parseInt(data[P.LDR_SET])
      }

      // 用户编辑期间只记录设备值，不覆盖滑块草稿。
      if (!this.data.editing && !this.data.dirty) {
        if (update.deviceTemThreshold !== undefined) update.temThreshold = update.deviceTemThreshold
        if (update.deviceLightThreshold !== undefined) update.lightThreshold = update.deviceLightThreshold
      }

      if (Object.keys(update).length) this.setData(update)
    }).catch(() => {
      // 读取失败，保持默认值
    })
  },

  /**
   * 温度阈值调节（对应 KEY2/KEY3，步进 1℃）
   */
  onTemChange(e) {
    const rawValue = parseFloat(e.detail.value)
    const value = Math.max(-20, Math.min(60, rawValue))
    this.setData({
      temThreshold: value,
      editing: true,
      dirty: true
    })
  },

  /**
   * 光照阈值调节（对应 KEY2/KEY3，步进 50 lx）
   */
  onLightChange(e) {
    const rawValue = parseInt(e.detail.value)
    const value = Math.max(0, Math.min(1000, rawValue))
    this.setData({
      lightThreshold: value,
      editing: true,
      dirty: true
    })
  },

  /**
   * 保存阈值到设备
   * 对应 key.c 第3页的 KEY2/KEY3 操作
   * 作者：程磊
   * 时间：2026-04-27 16:35:27 +08:00
   */
  saveSettings() {
    if (!this.data.dirty) {
      wx.showToast({ title: '没有未保存修改', icon: 'none' })
      return
    }

    const P = CONFIG.PROPS
    this.setData({ setting: true })

    // 下发阈值到设备
    api.setProperty({
      [P.TEM_SET]: this.data.temThreshold,
      [P.LDR_SET]: this.data.lightThreshold
    }).then((res) => {
      wx.showToast({
        title: res && res.pendingConfirm ? '已下发' : '设置已保存',
        icon: 'success'
      })
      setTimeout(() => {
        this.setData({
          setting: false,
          editing: false,
          dirty: false
        })
        this.loadThresholds()
      }, 2500)
    }).catch(err => {
      this.setData({ setting: false })
      wx.showToast({
        title: '请通过设备按键调整',
        icon: 'none',
        duration: 3000
      })
      console.warn('阈值下发失败（物模型可能未定义此属性）:', err)
    })
  },

  /**
   * 放弃未保存草稿，恢复为设备最近一次上发的阈值
   */
  discardChanges() {
    this.setData({
      temThreshold: this.data.deviceTemThreshold,
      lightThreshold: this.data.deviceLightThreshold,
      editing: false,
      dirty: false
    })
    wx.showToast({ title: '已放弃修改', icon: 'none' })
  },

  /**
   * 复位阈值到默认值
   * 对应微改工程 key.c 中的默认值：Tem_Up=30, ldr_Down=500
   */
  resetDefaults() {
    wx.showModal({
      title: '确认复位',
      content: '将阈值恢复为默认值？\n温度：30℃\n光照：500 lx',
      success: (res) => {
        if (res.confirm) {
          this.setData({
            temThreshold: 30,
            lightThreshold: 500,
            editing: true,
            dirty: true
          })
        }
      }
    })
  }
})
