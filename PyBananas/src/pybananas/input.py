"""Button enum + GameInput: composable, named-button replacement for raw arrays."""

from __future__ import annotations

from dataclasses import dataclass, fields
from enum import IntEnum
from typing import ClassVar, Iterator

import numpy as np

# Field order MUST match Button. to_array()/from_array()/Button rely on this.
BUTTON_NAMES: tuple[str, ...] = (
    "up", "down", "left", "right", "a", "b", "start", "select",
)
ACTION_SIZE = len(BUTTON_NAMES)


class Button(IntEnum):
    UP = 0
    DOWN = 1
    LEFT = 2
    RIGHT = 3
    A = 4
    B = 5
    START = 6
    SELECT = 7


@dataclass(frozen=True, slots=True)
class GameInput:
    """Immutable, composable controller-button set.

    Construct one of three ways:

        GameInput()                                 # no buttons (alias: GameInput.NOOP)
        GameInput(a=True, up=True)                  # kwargs
        GameInput.A | GameInput.UP                  # combine named singletons

    `BananasEnv.step` accepts both `GameInput` and raw 8-element arrays.
    """

    up: bool = False
    down: bool = False
    left: bool = False
    right: bool = False
    a: bool = False
    b: bool = False
    start: bool = False
    select: bool = False

    # Named singletons (assigned after class body — see bottom of file).
    NOOP:   ClassVar["GameInput"]
    UP:     ClassVar["GameInput"]
    DOWN:   ClassVar["GameInput"]
    LEFT:   ClassVar["GameInput"]
    RIGHT:  ClassVar["GameInput"]
    A:      ClassVar["GameInput"]
    B:      ClassVar["GameInput"]
    START:  ClassVar["GameInput"]
    SELECT: ClassVar["GameInput"]

    @classmethod
    def from_buttons(cls, *buttons: Button) -> "GameInput":
        """`GameInput.from_buttons(Button.A, Button.UP)`."""
        kwargs: dict[str, bool] = {}
        for btn in buttons:
            kwargs[BUTTON_NAMES[Button(btn).value]] = True
        return cls(**kwargs)

    @classmethod
    def from_array(cls, arr) -> "GameInput":
        """Inverse of `to_array()`; accepts any 8-element bool/0-1 sequence."""
        a = np.asarray(arr)
        if a.shape != (ACTION_SIZE,):
            raise ValueError(f"expected shape ({ACTION_SIZE},), got {a.shape}")
        if a.dtype.kind not in "biu" or ((a != 0) & (a != 1)).any():
            raise ValueError(f"values must be bool/0/1, got dtype={a.dtype}")
        return cls(**{name: bool(a[i]) for i, name in enumerate(BUTTON_NAMES)})

    def to_array(self) -> np.ndarray:
        """Length-8 uint8 array in (UP, DOWN, LEFT, RIGHT, A, B, START, SELECT) order."""
        return np.array(
            [getattr(self, name) for name in BUTTON_NAMES],
            dtype=np.uint8,
        )

    def pressed(self) -> tuple[Button, ...]:
        """Buttons currently down, in canonical order."""
        return tuple(
            Button(i) for i, name in enumerate(BUTTON_NAMES) if getattr(self, name)
        )

    def __or__(self, other: "GameInput") -> "GameInput":
        if not isinstance(other, GameInput):
            return NotImplemented
        return GameInput(
            **{name: getattr(self, name) or getattr(other, name) for name in BUTTON_NAMES}
        )

    def __iter__(self) -> Iterator[bool]:
        # Lets `np.asarray(game_input)` work transparently.
        return (getattr(self, name) for name in BUTTON_NAMES)


# --- named singletons --------------------------------------------------------
# Defined after the class so `cls(...)` is callable. slots=True only affects
# instance attributes; class-level assignment is unaffected.

GameInput.NOOP   = GameInput()
GameInput.UP     = GameInput(up=True)
GameInput.DOWN   = GameInput(down=True)
GameInput.LEFT   = GameInput(left=True)
GameInput.RIGHT  = GameInput(right=True)
GameInput.A      = GameInput(a=True)
GameInput.B      = GameInput(b=True)
GameInput.START  = GameInput(start=True)
GameInput.SELECT = GameInput(select=True)


# Sanity: dataclass field order must match BUTTON_NAMES exactly.
assert tuple(f.name for f in fields(GameInput)) == BUTTON_NAMES, (
    "GameInput dataclass field order drifted from BUTTON_NAMES"
)
