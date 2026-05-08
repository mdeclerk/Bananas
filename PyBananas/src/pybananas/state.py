from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum


class GameStateEnum(IntEnum):
    AIM = 0
    PEEK = 1
    FIRE = 2
    HIT = 3
    MISS = 4
    PAN_CAM = 5


class ProjectileStateEnum(IntEnum):
    NONE = 0
    FLYING = 1
    HIT_TERRAIN = 2
    HIT_PLAYER = 3
    OOB = 4


@dataclass(frozen=True, slots=True)
class Vec2:
    x: int
    y: int


@dataclass(frozen=True, slots=True)
class Vec2Fixed:
    x: int
    y: int


@dataclass(frozen=True, slots=True)
class Viewport:
    pos: Vec2
    target: Vec2
    min: Vec2
    max: Vec2


@dataclass(frozen=True, slots=True)
class Terrain:
    heights: tuple[int, ...]
    plateau_count: int
    size_px: Vec2
    shake: bool
    loaded_view_col: int


@dataclass(frozen=True, slots=True)
class Player:
    type: int
    pos: Vec2
    aim: Vec2
    lives: int
    score: int
    shake: bool


@dataclass(frozen=True, slots=True)
class Projectile:
    pos: Vec2Fixed
    vel: Vec2Fixed
    state: ProjectileStateEnum
    age: int


@dataclass(frozen=True, slots=True)
class GameState:
    viewport: Viewport
    terrain: Terrain
    players: tuple[Player, Player]
    projectile: Projectile
    state: GameStateEnum
    active_player: int
    state_timer: int
