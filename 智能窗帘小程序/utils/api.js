/**
 * OneNET 平台 API 工具函数
 *
 * 通过 OneNET HTTP API 与 STM32 设备通信
 * 依赖 config.js 中的 TOKEN、PRODUCT_ID、DEVICE_NAME
 *
 * 提供 2 个核心能力：
 *   1. queryProperty()  — 查询设备最新属性数据
 *   2. setProperty()    — 下发属性设置指令（控制设备）
 */

const CONFIG = require('./config.js')

/**
 * 统一解析 OneNET 返回的 mode 值。
 * 兼容 1/0、"1"/"0"、true/false、"true"/"false"。
 *
 * 作者：程磊
 * 时间：2026-04-27 16:35:27 +08:00
 *
 * @param {*} value - OneNET 属性值
 * @param {number} fallback - 无法识别时使用的模式
 * @returns {number} 1=自动，0=手动
 */
function normalizeMode(value, fallback) {
  if (value === undefined || value === null || value === '') return fallback
  if (value === true || value === 1) return 1
  if (value === false || value === 0) return 0

  const text = String(value).trim().toLowerCase()
  if (text === 'true' || text === 'auto' || text === '自动') return 1
  if (text === 'false' || text === 'manual' || text === 'hand' || text === '手动') return 0

  const num = parseInt(text, 10)
  return isNaN(num) ? fallback : (num ? 1 : 0)
}

/**
 * 查询设备最新属性数据
 * GET https://iot-api.heclouds.com/thingmodel/query-device-property
 *
 * 返回 Promise，resolve(data) 为属性键值对象
 *   例如: { Tmp: 26.0, Hum: 46.0, ldr: 140, curtain: 100, mode: 1, ... }
 *
 * 作者：程磊
 * 时间：2026-04-27 16:35:27 +08:00
 *
 * OneNET 实际返回格式:
 * { code: 0, data: [ {identifier:"Tmp", value:"26.0", ...}, ... ], msg:"succ" }
 */
function parsePropertyTime(item) {
  const raw = item && (item.time || item.update_time || item.updateTime || item.ts || item.timestamp)
  if (raw === undefined || raw === null || raw === '') return 0

  if (typeof raw === 'number') {
    return raw < 1000000000000 ? raw * 1000 : raw
  }

  const text = String(raw).trim()
  if (/^\d+$/.test(text)) {
    const num = parseInt(text, 10)
    return num < 1000000000000 ? num * 1000 : num
  }

  const parsed = Date.parse(text.replace(/-/g, '/'))
  return isNaN(parsed) ? 0 : parsed
}

function isDeviceOnline(data) {
  if (!data || !data._meta || !data._meta.lastReportTime) return false
  return Date.now() - data._meta.lastReportTime <= CONFIG.ONLINE_TIMEOUT
}

function queryProperty() {
  return new Promise((resolve, reject) => {
    wx.request({
      url: CONFIG.API_BASE + CONFIG.API_QUERY,
      method: 'GET',
      header: {
        'Content-Type': 'application/json',
        'Authorization': CONFIG.TOKEN
      },
      data: {
        product_id: CONFIG.PRODUCT_ID,
        device_name: CONFIG.DEVICE_NAME
      },
      success(res) {
        if (res.statusCode === 200 && res.data && res.data.code === 0) {
          // 扁平化：把数组转成 { Tmp: "26.0", Hum: "46.0", ... }
          const list = res.data.data
          if (list && list.length) {
            const flat = {}
            list.forEach(item => {
              flat[item.identifier] = item.value
            })
            resolve(flat)
          } else {
            resolve({})
          }
        } else {
          reject(new Error('查询失败，code=' + (res.data && res.data.code) + ' msg=' + (res.data && res.data.msg)))
        }
      },
      fail(err) {
        reject(err)
      }
    })
  })
}

/**
 * 设置设备属性（下发指令到 STM32）
 * POST https://iot-api.heclouds.com/thingmodel/set-device-property
 *
 * @param {Object} params - 要设置的属性，例如：
 *   { mode: 0 }         — 切换手动模式
 *   { curtain: 100 }    — 全关窗帘
 *   { mode: 0, curtain: 50 } — 同时设置
 *
 * 作者：程磊
 * 时间：2026-04-27 16:35:27 +08:00
 */
function setProperty(params) {
  return new Promise((resolve, reject) => {
    wx.request({
      url: CONFIG.API_BASE + CONFIG.API_SET,
      method: 'POST',
      header: {
        'Content-Type': 'application/json',
        'Authorization': CONFIG.TOKEN
      },
      data: {
        product_id: CONFIG.PRODUCT_ID,
        device_name: CONFIG.DEVICE_NAME,
        params: params
      },
      success(res) {
        console.log('[API] 设置响应 statusCode:', res.statusCode, 'data:', JSON.stringify(res.data))
        if (res.statusCode === 200 && res.data && res.data.code === 0) {
          resolve(res.data)
        } else if (res.statusCode === 200 && res.data && res.data.code === 10411) {
          // OneNET 返回 10411 表示等待设备同步确认超时。
          // 实测 MQTT 指令仍可能已经到达 STM32 并被执行，因此先按“已受理”处理，
          // 页面后续再通过 queryProperty 刷新真实设备状态。
          resolve(Object.assign({}, res.data, {
            accepted: true,
            pendingConfirm: true
          }))
        } else {
          reject(new Error('设置失败，code=' + (res.data && res.data.code) + ' msg=' + (res.data && res.data.msg)))
        }
      },
      fail(err) {
        reject(err)
      }
    })
  })
}

module.exports = {
  queryProperty,
  setProperty,
  normalizeMode,
  isDeviceOnline
}
