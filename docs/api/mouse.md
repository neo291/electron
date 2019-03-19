# mouse

> Retrieves and emits global mouse events.

Process: [Main](../glossary.md#main-process), [Renderer](../glossary.md#renderer-process)

`mouse` is an [EventEmitter](https://nodejs.org/api/events.html#events_class_eventemitter).

An example of receiving global mouse click events:

```javascript
const { mouse } = require('electron')

mouse.on('click', (event, mouseEvent) => {
  console.log(mouseEvent)
})
```

## Events

The `mouse` module emits the following events:

### Event: 'click'

Returns:

* `event` Event
* `mouseEvent` [MouseEvent](structures/mouse-event.md)

Emitted when mouse click has been received.

### Event: 'dblclick'

Returns:

* `event` Event
* `mouseEvent` [MouseEvent](structures/mouse-event.md)

Emitted when mouse double click has been received.

### Event: 'down'

Returns:

* `event` Event
* `mouseEvent` [MouseEvent](structures/mouse-event.md)

Emitted when mouse button down has been received.

### Event: 'move'

Returns:

* `event` Event
* `mouseEvent` [MouseEvent](structures/mouse-event.md)

Emitted when mouse move has been received.

### Event: 'up'

Returns:

* `event` Event
* `mouseEvent` [MouseEvent](structures/mouse-event.md)

Emitted when mouse button up has been received.

## Methods

The `mouse` module has the following methods:

### `mouse.isSupported()`

Returns `Boolean`

Is global mouse events supported.

### `mouse.setFullMoveResolution(value)`

* `value` Boolean

If `value` is set to `true` the `mouse` object will send all consecutive `move` events in the queue, otherwise, if set to `false` sends only the last of a series of consecutive move events avoiding some overhead when not required. Defualt `false`

### `mouse.hasFullMoveResolution()`

Returns `Boolean`

Mouse has the full move resolution.
