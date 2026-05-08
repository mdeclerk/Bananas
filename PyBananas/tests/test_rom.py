"""Round-trip a synthetic g_game blob through decode_game_state.

Catches struct-layout drift between the C side (game.h, player.h,
viewport.h, terrain.h, projectile.h) and the Python decoder offsets.
"""

from __future__ import annotations

import struct

from pybananas.rom import (
    GAME_STATE_SIZE,
    OFF_ACTIVE_PLAYER,
    OFF_PLAYER0,
    OFF_PLAYER1,
    OFF_PROJECTILE,
    OFF_STATE,
    OFF_STATE_TIMER,
    OFF_TERRAIN,
    OFF_VIEWPORT,
    decode_game_state,
)
from pybananas.state import GameStateEnum, ProjectileStateEnum


def _build_blob() -> bytes:
    buf = bytearray(GAME_STATE_SIZE)

    # viewport: pos(1,2) target(3,4) min(5,6) max(7,8)
    struct.pack_into("<hhhhhhhh", buf, OFF_VIEWPORT, 1, 2, 3, 4, 5, 6, 7, 8)

    # terrain: heights[24] = 0..23, plateau_count=5, size_px=(160, 120),
    # shake=1, loaded_view_col=42
    for i in range(24):
        buf[OFF_TERRAIN + i] = i
    buf[OFF_TERRAIN + 24] = 5
    struct.pack_into("<hh", buf, OFF_TERRAIN + 25, 160, 120)
    buf[OFF_TERRAIN + 29] = 1
    struct.pack_into("<H", buf, OFF_TERRAIN + 30, 42)

    # player0: type=0 pos=(10,11) aim=(-1,2) lives=3 score=7 shake=0
    buf[OFF_PLAYER0] = 0
    struct.pack_into("<hhhh", buf, OFF_PLAYER0 + 1, 10, 11, -1, 2)
    buf[OFF_PLAYER0 + 9] = 3
    buf[OFF_PLAYER0 + 10] = 7
    buf[OFF_PLAYER0 + 11] = 0

    # player1: type=1 pos=(140,80) aim=(0,-3) lives=2 score=4 shake=1
    buf[OFF_PLAYER1] = 1
    struct.pack_into("<hhhh", buf, OFF_PLAYER1 + 1, 140, 80, 0, -3)
    buf[OFF_PLAYER1 + 9] = 2
    buf[OFF_PLAYER1 + 10] = 4
    buf[OFF_PLAYER1 + 11] = 1

    # projectile: pos=(0x1000, 0x2000) vel=(-100, 50) state=FLYING age=12
    struct.pack_into("<iiii", buf, OFF_PROJECTILE, 0x1000, 0x2000, -100, 50)
    buf[OFF_PROJECTILE + 16] = ProjectileStateEnum.FLYING
    buf[OFF_PROJECTILE + 17] = 12

    buf[OFF_STATE] = GameStateEnum.FIRE
    buf[OFF_ACTIVE_PLAYER] = 1
    buf[OFF_STATE_TIMER] = 25

    return bytes(buf)


def test_decode_round_trip() -> None:
    gs = decode_game_state(_build_blob())

    assert gs.viewport.pos.x == 1 and gs.viewport.pos.y == 2
    assert gs.viewport.max.x == 7 and gs.viewport.max.y == 8

    assert gs.terrain.heights == tuple(range(24))
    assert gs.terrain.plateau_count == 5
    assert gs.terrain.size_px.x == 160 and gs.terrain.size_px.y == 120
    assert gs.terrain.shake is True
    assert gs.terrain.loaded_view_col == 42

    p0, p1 = gs.players
    assert p0.type == 0 and p0.lives == 3 and p0.score == 7 and p0.shake is False
    assert p0.pos.x == 10 and p0.aim.x == -1
    assert p1.type == 1 and p1.lives == 2 and p1.score == 4 and p1.shake is True
    assert p1.aim.y == -3

    assert gs.projectile.pos.x == 0x1000 and gs.projectile.vel.y == 50
    assert gs.projectile.state == ProjectileStateEnum.FLYING
    assert gs.projectile.age == 12

    assert gs.state == GameStateEnum.FIRE
    assert gs.active_player == 1
    assert gs.state_timer == 25


def test_decode_rejects_short_buffer() -> None:
    import pytest

    with pytest.raises(ValueError):
        decode_game_state(bytes(GAME_STATE_SIZE - 1))
