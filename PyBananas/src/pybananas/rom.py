from __future__ import annotations

import struct
from pathlib import Path

from .state import (
    GameState,
    GameStateEnum,
    Player,
    Projectile,
    ProjectileStateEnum,
    Terrain,
    Vec2,
    Vec2Fixed,
    Viewport,
)

GAME_STATE_SIZE = 93

# Field offsets within g_game (relative to the resolved _g_game address).
# Derived from SDCC GBZ80 ABI applied to game_t in src/game.h.
OFF_VIEWPORT = 0x00
OFF_TERRAIN = 0x10
OFF_PLAYER0 = 0x30
OFF_PLAYER1 = 0x3C
OFF_PROJECTILE = 0x48
OFF_STATE = 0x5A
OFF_ACTIVE_PLAYER = 0x5B
OFF_STATE_TIMER = 0x5C
OFF_PLAYER0_LIVES = OFF_PLAYER0 + 9

BANANAS_ROM = Path(__file__).resolve().parent / "rom" / "bananas.gb"


def validate_rom(rom: Path = BANANAS_ROM) -> Path:
    """Validate ROM and .sym exist. Returns the rom path."""
    if not rom.exists():
        raise FileNotFoundError(
            f"ROM not found: {rom}\n"
            "Run `pixi run build-rom` to compile it."
        )
    sym = rom.with_suffix(".sym")
    if not sym.exists():
        raise FileNotFoundError(
            f"Symbol file not found: {sym}\n"
            "Run `pixi run build-rom` to compile with symbols."
        )
    return rom


def resolve_symbol(rom: Path, name: str) -> int:
    """Look up `name` in the .sym file (`bank:addr name`)."""
    sym = rom.with_suffix(".sym")
    for line in sym.read_text().splitlines():
        parts = line.strip().split()
        if len(parts) >= 2 and parts[-1] == name and ":" in parts[0]:
            return int(parts[0].split(":", 1)[1], 16)
    raise RuntimeError(f"could not resolve {name} in {sym}")


def decode_game_state(buf: bytes | bytearray | memoryview) -> GameState:
    if len(buf) < GAME_STATE_SIZE:
        raise ValueError(f"buf too small: {len(buf)} < {GAME_STATE_SIZE}")
    b = bytes(buf)
    return GameState(
        viewport=_decode_viewport(b, OFF_VIEWPORT),
        terrain=_decode_terrain(b, OFF_TERRAIN),
        players=(
            _decode_player(b, OFF_PLAYER0),
            _decode_player(b, OFF_PLAYER1),
        ),
        projectile=_decode_projectile(b, OFF_PROJECTILE),
        state=GameStateEnum(b[OFF_STATE]),
        active_player=b[OFF_ACTIVE_PLAYER],
        state_timer=b[OFF_STATE_TIMER],
    )


def _decode_vec2(buf: bytes, off: int) -> Vec2:
    x, y = struct.unpack_from("<hh", buf, off)
    return Vec2(x, y)


def _decode_vec2fixed(buf: bytes, off: int) -> Vec2Fixed:
    x, y = struct.unpack_from("<ii", buf, off)
    return Vec2Fixed(x, y)


def _decode_viewport(buf: bytes, off: int) -> Viewport:
    return Viewport(
        pos=_decode_vec2(buf, off + 0),
        target=_decode_vec2(buf, off + 4),
        min=_decode_vec2(buf, off + 8),
        max=_decode_vec2(buf, off + 12),
    )


def _decode_terrain(buf: bytes, off: int) -> Terrain:
    heights = tuple(buf[off : off + 24])
    plateau_count = buf[off + 24]
    size_px = _decode_vec2(buf, off + 25)
    shake = bool(buf[off + 29])
    (loaded_view_col,) = struct.unpack_from("<H", buf, off + 30)
    return Terrain(
        heights=heights,
        plateau_count=plateau_count,
        size_px=size_px,
        shake=shake,
        loaded_view_col=loaded_view_col,
    )


def _decode_player(buf: bytes, off: int) -> Player:
    ptype = buf[off]
    pos = _decode_vec2(buf, off + 1)
    aim = _decode_vec2(buf, off + 5)
    lives = buf[off + 9]
    score = buf[off + 10]
    shake = bool(buf[off + 11])
    return Player(type=ptype, pos=pos, aim=aim, lives=lives, score=score, shake=shake)


def _decode_projectile(buf: bytes, off: int) -> Projectile:
    pos = _decode_vec2fixed(buf, off + 0)
    vel = _decode_vec2fixed(buf, off + 8)
    state = buf[off + 16]
    age = buf[off + 17]
    return Projectile(pos=pos, vel=vel, state=ProjectileStateEnum(state), age=age)
