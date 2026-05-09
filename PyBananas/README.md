# PyBananas

PyBananas is a Python wrapper of Bananas ROM for AI/ML training tasks.

## Install PIP Wheel

```bash
pip install https://github.com/mdeclerk/Bananas/releases/latest/download/pybananas-0.1.1-py3-none-any.whl
```

## Quickstart

```python
from pybananas import BananasEnv, GameInput, GameStateEnum

with BananasEnv() as env:
    obs = env.reset()                              # (144, 160) uint8
    for _ in range(100):
        obs = env.step(GameInput.A | GameInput.UP)
        state = env.game_state()
    print(f"P1: {state.players[0].score} pts, {state.players[0].lives} lives")
```

## PyBananas API


| API | Purpose |
| --- | --- |
| `BananasEnv(window="null", frame_skip=4, ...)` | Create a headless training environment. |
| `env.reset()` | Start a randomized episode and return the first observation. |
| `env.step(action)` | Apply `GameInput` or an 8-value button array and return the next observation. |
| `env.observe()` | Read the current frame without stepping. |
| `env.game_state()` | Read decoded scores, lives, terrain, projectile, and turn state. |
| `env.tick(count=1, render=True)` | Manually advance emulator frames. |
| `env.set_emulation_speed(speed)` | Set emulator speed; `0` means unlimited. |
| `env.close()` | Stop the emulator. |

Observations are grayscale `uint8` arrays with shape `(144, 160)` and values
`0..3`. Action arrays use `(UP, DOWN, LEFT, RIGHT, A, B, START, SELECT)` order.

## Pixi Build Environment

Install [Pixi](https://pixi.sh) as dev environment and task runner for PyBananas.

```bash
cd PyBananas
pixi install
pixi run build-rom          # build ROM files into src/PyBananas/rom
pixi run clean-rom          # clean ROM files
pixi run build-wheel        # build whl file
pixi run test               # run tests
pixi run play               # play in PyBoy for testing
```

Commands are also available as VS Code tasks via **Terminal → Run Task**.
