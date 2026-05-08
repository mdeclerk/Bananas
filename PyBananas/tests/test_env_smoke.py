"""End-to-end smoke test: boot the ROM, reset, step, check state."""

from __future__ import annotations

import numpy as np

from pybananas import ACTION_SIZE, BananasEnv, GameStateEnum


def test_boot_reset_step() -> None:
    with BananasEnv() as env:
        obs = env.reset()
        assert obs.shape == (144, 160)
        assert obs.dtype == np.uint8

        for _ in range(50):
            obs = env.step([0] * ACTION_SIZE)
        assert obs.shape == (144, 160)

        gs = env.game_state()
        assert gs.players[0].lives == 3
        assert gs.state in set(GameStateEnum)
