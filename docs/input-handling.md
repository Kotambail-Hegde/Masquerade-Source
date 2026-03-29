# Masquerade Input Handling Architecture

## Overview

Masquerade uses a layered, backend-agnostic input system designed around three goals:

- **Decoupling** — the emulator core never depends on SDL or any windowing library
- **Configurability** — key bindings are remappable per emulation platform at runtime
- **Timing accuracy** — input events are delivered to the emulator mid-frame rather than at a fixed sync point, enabling natural interrupt timing variance

---

## Layers

```
┌─────────────────────────────────────────────────────────┐
│                   OS / SDL3 Backend                     │
│         Generates SDL_Event on physical keypress        │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────┐
│              handleSDLEvent (Frontend Lambda)           │
│   Translates SDL_Event → EmuKey via KeyBindings::resolve│
│   Calls current_instance->onKeyEvent(key, action)       │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────┐
│         KeyBindings (Backend-Agnostic Mapping)          │
│   Maps int (backend keycode) → EmuKey per EMULATION_ID  │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────┐
│      abstractEmulation_t::onKeyEvent (Virtual)          │
│   Each emulator implements this to update its own state │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────┐
│           InputHintCallback (Mid-Frame Delivery)        │
│   Emulator calls this at natural points in its run loop │
│   Frontend polls SDL events inside the callback         │
│   Interrupt fires at current emulation cycle / LY       │
└─────────────────────────────────────────────────────────┘
```

---

## Components

### EmuKey

A backend-agnostic enumeration of logical gamepad inputs.

```cpp
enum class EmuKey
{
    UP, DOWN, LEFT, RIGHT,
    A, B, X, Y,
    START, SELECT,
    L, R,
    UNKNOWN
};

enum class EmuKeyAction { PRESSED, RELEASED };
```

`EmuKey` has no dependency on SDL, GLFW, or any windowing library. It represents what the emulator sees, not what the OS reported.

---

### KeyBindings

Maps backend key codes (stored as plain `int`) to `EmuKey` values, scoped per `EMULATION_ID`. The use of `int` for storage means the class itself has no dependency on any backend — only the code that populates bindings (i.e. `setDefault`) needs to know backend-specific constants like `SDL_SCANCODE_*`.

```cpp
class KeyBindings
{
public:
    // Populate with default bindings for a given platform
    void setDefault(EMULATION_ID id);

    // Resolve a backend keycode to an EmuKey for a given platform
    EmuKey resolve(EMULATION_ID id, int keyCode) const;

    // Rebind a key at runtime (for remapping UI)
    void rebind(EMULATION_ID id, EmuKey key, int newKeyCode);

    // Get the current backend keycode for a given EmuKey (for display in UI)
    int getCurrentCode(EMULATION_ID id, EmuKey key) const;

    // Persist bindings to / from config file
    void load(boost::property_tree::ptree& config, EMULATION_ID id);
    void save(boost::property_tree::ptree& config, EMULATION_ID id);

private:
    std::unordered_map<EMULATION_ID, std::unordered_map<int, EmuKey>, EmulationIDHash> bindings;
};
```

`EmulationIDHash` is required because `EMULATION_ID` is an `enum class` and `std::unordered_map` needs a hash specialisation for non-standard key types:

```cpp
struct EmulationIDHash
{
    std::size_t operator()(EMULATION_ID id) const
    {
        return std::hash<uint8_t>{}(static_cast<uint8_t>(id));
    }
};
```

---

### abstractEmulation_t (Base Class)

Every emulator platform inherits from this base. Two methods are relevant to input:

```cpp
class abstractEmulation_t
{
public:
    // Called by the frontend when a physical key event arrives
    virtual FLAG onKeyEvent(EmuKey key, EmuKeyAction action) = 0;

    // Called by the frontend to register a mid-frame input poll callback
    void setInputHintCallback(std::function<void()> cb)
    {
        inputHintCallback = cb;
    }

protected:
    std::function<void()> inputHintCallback = nullptr;
};
```

`onKeyEvent` is pure virtual — each emulator implements it to map `EmuKey` to its own internal input state. The base class never touches SDL or platform types.

---

### handleSDLEvent (Frontend Lambda)

Defined once in `Start()`, this lambda is the only place in the frontend where SDL event types are examined and dispatched. It handles all event categories: quit, window close, file drop, and key input.

```cpp
auto handleSDLEvent = [&](SDL_Event& e)
{
    ImGui_ImplSDL3_ProcessEvent(&e);

    if (e.type == SDL_EVENT_QUIT)
        done = true;

    if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && 
        e.window.windowID == SDL_GetWindowID(window))
        done = true;

    if (e.type == SDL_EVENT_DROP_FILE)
    {
        // ... file drop handling ...
    }

    if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP)
    {
        EmuKey key = keyBindings.resolve(
            current_instance->getEmulationID(),
            (int)e.key.scancode
        );
        EmuKeyAction act = (e.type == SDL_EVENT_KEY_DOWN)
                           ? EmuKeyAction::PRESSED
                           : EmuKeyAction::RELEASED;
        if (key != EmuKey::UNKNOWN)
            current_instance->onKeyEvent(key, act);
    }
};
```

By centralising all event dispatch in one lambda, both the **main loop** and the **mid-frame hint callback** use identical logic with no duplication.

---

### InputHintCallback (Mid-Frame Input Delivery)

The hint callback is the mechanism that gives input events their natural timing. It is registered once after initialisation:

```cpp
current_instance->setInputHintCallback([&]()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
        handleSDLEvent(e);
});
```

The emulator calls `inputHintCallback()` from inside its run loop at points where input is naturally sampled. Because `SDL_PollEvent` is called **inside the emulation loop** rather than at the top of the render frame, events arrive at whatever cycle the emulator is currently executing. This means:

- A keypress that the OS delivers mid-frame is processed at the cycle the emulator happens to be at when it next calls the hint
- The resulting interrupt (if any) fires at that cycle, not at a fixed VBLANK or frame-start position
- Different keypresses at different moments yield different internal timing — matching the behaviour of real hardware where keypresses are asynchronous

The main loop also calls `handleSDLEvent` for events that arrive between hint callbacks:

```cpp
SDL_Event event;
while (SDL_PollEvent(&event))
    handleSDLEvent(event);
```

Since `SDL_PollEvent` drains the queue, events consumed by the hint callback mid-frame are not seen again in the main loop and vice versa. No events are dropped or double-processed.

---

## Data Flow: Physical Keypress to Emulator State

```
Physical key pressed on keyboard
        │
        ▼
OS delivers event to SDL event queue
        │
        ▼
SDL_PollEvent() called inside inputHintCallback (mid-frame)
        │
        ▼
handleSDLEvent() examines event.type
        │
        ▼
keyBindings.resolve(EMULATION_ID, SDL_Scancode) → EmuKey
        │
        ▼
current_instance->onKeyEvent(EmuKey, EmuKeyAction)
        │
        ▼
Emulator updates internal key state
        │
        ▼
Emulator fires interrupt (if applicable) at current cycle
```

---

## Adding a New Platform

To add input support for a new emulator platform:

**1. Add default bindings in `KeyBindings::setDefault`:**
```cpp
void KeyBindings::setDefault(EMULATION_ID id)
{
    if (id == EMULATION_ID::MY_NEW_PLATFORM_ID)
    {
        bindings[id] = {
            { SDL_SCANCODE_Z,      EmuKey::A      },
            { SDL_SCANCODE_X,      EmuKey::B      },
            // ...
        };
    }
}
```

**2. Implement `onKeyEvent` in the new emulator class:**
```cpp
FLAG MyEmulator_t::onKeyEvent(EmuKey key, EmuKeyAction action)
{
    bool pressed = (action == EmuKeyAction::PRESSED);
    switch (key)
    {
    case EmuKey::A:     myInputState.buttonA = pressed; break;
    case EmuKey::START: myInputState.start   = pressed; break;
    // ...
    default: return NO;
    }
    return YES;
}
```

**3. Call `inputHintCallback` from inside the emulator run loop:**
```cpp
void MyEmulator_t::runOneCycle()
{
    // ... execute one CPU step ...

    if (inputHintCallback)
        inputHintCallback();
}
```

No changes to the frontend, `KeyBindings`, or `EmuKey` are needed.

---

## Switching Backends

Because `KeyBindings` stores key codes as plain `int` and `EmuKey` has no backend dependency, switching from SDL3 to GLFW (or any other backend) requires changes in exactly two places:

1. **`KeyBindings::setDefault`** — replace `SDL_SCANCODE_*` constants with the new backend's equivalents
2. **`handleSDLEvent`** — replace SDL event types and field names with the new backend's equivalents

Everything else — `EmuKey`, `EmuKeyAction`, `KeyBindings::resolve`, `abstractEmulation_t::onKeyEvent`, all emulator implementations — remains completely unchanged.