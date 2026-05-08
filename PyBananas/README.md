# PyBananas

PyBananas is a Python wrapper of Bananas ROM for AI/ML training tasks.

## Usage

Install wheel packges:

```bash
pip install https://github.com/mdeclerk/Bananas/releases/latest/download/pybananas-0.1.0-py3-none-any.whl
```

Use in code:

```python
from pybananas import BananasEnv, GameInput, GameStateEnum

with BananasEnv() as env:
    obs = env.reset()                              # (144, 160) uint8
    for _ in range(100):
        obs = env.step(GameInput.A | GameInput.UP)
        state = env.game_state()
    print(f"P1: {state.players[0].score} pts, {state.players[0].lives} lives")
```

## Pixi Build Workflow

Install [Pixi](https://pixi.sh) as dev environment and task runner.

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