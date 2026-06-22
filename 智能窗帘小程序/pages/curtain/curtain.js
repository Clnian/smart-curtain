// pages/curtain/curtain.js - 窗帘控制页
// 功能：窗帘可视化、滑动控制、快捷开/关/半开、操作日志
// 对应微改工程：key.c 手动模式下 KEY2/KEY3 控制电机，motor.c 驱动步进电机

const api = require('../../utils/api.js')
const CONFIG = require('../../utils/config.js')

/**
 * 窗帘控制页面对象：负责窗帘状态刷新、滑动控制和手动模式提醒。
 * 作者：程磊
 * 时间：2026-04-27 16:35:27 +08:00
 */
Page({
  data: {
    curtainValue: 0,      // 窗帘关闭度：0=全开，100=全关
    mode: 1,              // 1=自动联动，0=手动控制
    isMoving: false,       // 是否正在运动
    manualSwitchNotice: '',
    logs: []               // 操作日志
  },

  _timer: null,
  _noticeTimer: null,

  onLoad() {
    this.refreshCurtain()
  },

  onShow() {
    this.refreshCurtain()
    this.startRefreshTimer()
  },

  onHide() {
    this.stopRefreshTimer()
  },

  onUnload() {
    this.stopRefreshTimer()
    this.clearManualSwitchNotice()
  },

  startRefreshTimer() {
    if (this._timer) return
    this._timer = setInterval(() => {
      this.refreshCurtain()
    }, CONFIG.REFRESH_INTERVAL)
  },

  stopRefreshTimer() {
    if (this._timer) {
      clearInterval(this._timer)
      this._timer = null
    }
  },

  showManualSwitchNotice(message) {
    if (this._noticeTimer) {
      clearTimeout(this._noticeTimer)
      this._noticeTimer = null
    }

    this.setData({ manualSwitchNotice: message })
    this._noticeTimer = setTimeout(() => {
      this.setData({ manualSwitchNotice: '' })
      this._noticeTimer = null
    }, 4500)
  },

  clearManualSwitchNotice() {
    if (this._noticeTimer) {
      clearTimeout(this._noticeTimer)
      this._noticeTimer = null
    }
    this.setData({ manualSwitchNotice: '' })
  },

  /**
   * 刷新窗帘状态
   * 从 OneNET 读取 curtain 属性值
   * 作者：程磊
   * 时间：2026-04-27 16:35:27 +08:00
   */
  refreshCurtain() {
    api.queryProperty().then(data => {
      const update = {}
      if (data[CONFIG.PROPS.CURTAIN] !== undefined) {
        update.curtainValue = parseInt(data[CONFIG.PROPS.CURTAIN])
      }
      if (data[CONFIG.PROPS.MODE] !== undefined) {
        update.mode = api.normalizeMode(data[CONFIG.PROPS.MODE], this.data.mode)
      }
      if (Object.keys(update).length) this.setData(update)
    }).catch(err => {
      console.error('读取窗帘状态失败:', err)
    })
  },

  /**
   * 滑动条拖动中 - 仅更新 UI，不发送指令
   */
  onSliderChanging(e) {
    this.setData({ curtainValue: e.detail.value })
  },

  /**
   * 滑动条松手 - 发送控制指令
   */
  onSliderChange(e) {
    const value = e.detail.value
    this.requestCurtainControl(value)
  },

  /**
   * 快捷按钮：全开/25%/50%/75%/全关
   */
  setCurtain(e) {
    const value = parseInt(e.currentTarget.dataset.value)
    this.setData({ curtainValue: value })
    this.requestCurtainControl(value)
  },

  /**
   * 用户主动控制窗帘时，统一切换到手动模式再下发窗帘位置。
   * 这样不依赖弹窗确认，也避免自动联动立即覆盖手动指令。
   * 作者：程磊
   * 时间：2026-04-27 16:35:27 +08:00
   */
  requestCurtainControl(value) {
    this.sendManualCurtainCommand(value)
  },

  /**
   * 下发手动模式 + 窗帘控制指令到 STM32。
   * 作者：程磊
   * 时间：2026-04-27 16:35:27 +08:00
   */
  sendManualCurtainCommand(value) {
    const P = CONFIG.PROPS
    const wasAuto = this.data.mode === 1
    wx.showLoading({ title: '切换手动...' })
    this.setData({ isMoving: true })
    this.showManualSwitchNotice('正在切换为手动模式并控制窗帘...')

    api.setProperty({
      [P.MODE]: 0,
      [P.CURTAIN]: value
    }).then((res) => {
      const label = value === 0 ? '全开' : value === 100 ? '全关' : '关闭到 ' + value + '%'
      const notice = wasAuto
        ? '已自动切换为手动模式，窗帘' + label
        : '当前为手动模式，窗帘' + label
      this.setData({ mode: 0, isMoving: false })
      this.showManualSwitchNotice(notice)
      this.addLog('已切换为手动模式')
      this.addLog('窗帘设为 ' + label)
      wx.hideLoading()
      wx.showToast({
        title: res && res.pendingConfirm ? '已切手动并下发' : '已切换手动',
        icon: 'success'
      })
      setTimeout(() => {
        this.refreshCurtain()
      }, 1500)
    }).catch(err => {
      wx.hideLoading()
      this.setData({ isMoving: false })
      this.showManualSwitchNotice('下发失败，请稍后重试')
      wx.showToast({ title: '下发失败', icon: 'none' })
      console.error('设置窗帘失败:', err)
      this.refreshCurtain()
    })
  },

  /**
   * 添加操作日志（最多保留 20 条）
   */
  addLog(msg) {
    const now = new Date()
    const timeStr = now.getHours().toString().padStart(2, '0') + ':' +
                    now.getMinutes().toString().padStart(2, '0') + ':' +
                    now.getSeconds().toString().padStart(2, '0')

    const logs = this.data.logs
    logs.unshift({ time: timeStr, msg: msg })
    if (logs.length > 20) logs.pop()
    this.setData({ logs: logs })
  }
})
