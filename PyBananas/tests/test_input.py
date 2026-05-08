"""GameInput construction, composition, and array conversion."""

from __future__ import annotations

import numpy as np
import pytest

from pybananas import Button, GameInput


def test_default_is_noop() -> None:
    g = GameInput()
    assert g == GameInput.NOOP
    assert all(v is False for v in g)
    assert g.pressed() == ()
    assert tuple(g.to_array().tolist()) == (0,) * 8


def test_kwargs_construction() -> None:
    g = GameInput(a=True, up=True)
    assert g.a is True and g.up is True
    assert g.b is False and g.down is False
    assert set(g.pressed()) == {Button.A, Button.UP}


def test_or_combines_singletons() -> None:
    g = GameInput.A | GameInput.UP | GameInput.RIGHT
    assert g == GameInput(a=True, up=True, right=True)
    assert set(g.pressed()) == {Button.A, Button.UP, Button.RIGHT}


def test_or_is_idempotent() -> None:
    assert (GameInput.A | GameInput.A) == GameInput.A


def test_or_rejects_non_gameinput() -> None:
    with pytest.raises(TypeError):
        GameInput.A | [1, 0, 0, 0, 0, 0, 0, 0]  # type: ignore[operator]


def test_to_array_byte_order() -> None:
    """Array index must align with Button enum values."""
    arr = (GameInput.A | GameInput.START).to_array()
    assert arr[Button.A] == 1
    assert arr[Button.START] == 1
    assert arr.sum() == 2
    assert arr.dtype == np.uint8


def test_from_array_round_trip() -> None:
    cases = [GameInput.NOOP, GameInput.A, GameInput(a=True, up=True, b=True)]
    for original in cases:
        recovered = GameInput.from_array(original.to_array())
        assert recovered == original


def test_from_array_accepts_list_and_bool_array() -> None:
    g1 = GameInput.from_array([0, 0, 0, 0, 1, 0, 0, 0])
    assert g1 == GameInput.A
    g2 = GameInput.from_array(np.array([False, True, False, False, False, False, False, False]))
    assert g2 == GameInput.DOWN


def test_from_array_rejects_bad_shape_or_values() -> None:
    with pytest.raises(ValueError):
        GameInput.from_array([0, 0, 0])
    with pytest.raises(ValueError):
        GameInput.from_array([0, 0, 0, 0, 2, 0, 0, 0])


def test_from_buttons() -> None:
    g = GameInput.from_buttons(Button.A, Button.UP)
    assert g == GameInput(a=True, up=True)
    # Duplicates collapse:
    assert GameInput.from_buttons(Button.A, Button.A) == GameInput.A


def test_immutable() -> None:
    with pytest.raises(Exception):  # FrozenInstanceError
        GameInput.A.up = True  # type: ignore[misc]


def test_singletons_match_kwargs() -> None:
    assert GameInput.UP == GameInput(up=True)
    assert GameInput.DOWN == GameInput(down=True)
    assert GameInput.LEFT == GameInput(left=True)
    assert GameInput.RIGHT == GameInput(right=True)
    assert GameInput.A == GameInput(a=True)
    assert GameInput.B == GameInput(b=True)
    assert GameInput.START == GameInput(start=True)
    assert GameInput.SELECT == GameInput(select=True)
