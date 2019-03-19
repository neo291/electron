'use strict'

const bindings = process.electronBinding('mouse')
const { EventEmitter } = require('events')
const { mouse, Mouse } = bindings

// Only one mouse object permitted.
module.exports = mouse

// Mouse is an EventEmitter.
Object.setPrototypeOf(Mouse.prototype, EventEmitter.prototype)
EventEmitter.call(mouse)
