from __future__ import annotations

import io
import warnings
from pathlib import Path
from types import TracebackType

import numpy as np

warnings.filterwarnings(
    "ignore",
    message=r"Using SDL2 binaries from pysdl2-dll.*",
    category=UserWarning,
)

from pyboy import PyBoy

from .rom import (
    BANANAS_ROM,
    GAME_STATE_SIZE,
    OFF_PLAYER0_LIVES,
    OFF_STATE,
    decode_game_state,
    resolve_symbol,
    validate_rom,
)
from .input import ACTION_SIZE, BUTTON_NAMES, Button, GameInput
from .state import GameState, GameStateEnum

DMG_PALETTE = (0xFFFFFF, 0xAAAAAA, 0x555555, 0x000000)

BOOT_FRAMES = 60
SPLASH_MAX_FRAMES = 600
SPLASH_WARMUP_MAX = 256  # random idle frames before pressing START
SETTLE_FRAMES = 10  # let DISPLAY_ON + PPU produce a full gameplay frame after AIM


class BananasEnv:
    """PyBoy wrapper for bananas.gb.

    Observation: grayscale (144, 160) uint8 in [0, 3] (0 = lightest).
    Action: a `GameInput` (preferred) or any length-8 bool/0-1 sequence over
    (UP, DOWN, LEFT, RIGHT, A, B, START, SELECT).
    Step: presses the action mask for `frame_skip` ticks, then releases it.
    """

    def __init__(
        self,
        rom_path: Path | str | None = None,
        window: str = "null",
        scale: int = 3,
        frame_skip: int = 4,
        skip_splash: bool = True,
        sound: bool = False,
        no_input: bool = True,
    ) -> None:
        if rom_path is not None:
            rom = validate_rom(Path(rom_path))
        else:
            rom = validate_rom()
        if frame_skip < 1:
            raise ValueError(f"frame_skip must be >= 1, got {frame_skip}")

        self._frame_skip = frame_skip
        self._g_game_addr = resolve_symbol(rom, "_g_game")

        self._pyboy = PyBoy(
            str(rom),
            window=window,
            scale=scale,
            sound_emulated=sound,
            no_input=no_input,
            ram_file=io.BytesIO(),
            rtc_file=io.BytesIO(),
            color_palette=DMG_PALETTE,
            symbols=str(rom.with_suffix(".sym")),
        )
        self._rng = np.random.default_rng()

        self._pyboy.tick(count=BOOT_FRAMES, render=True)

        # Save boot state (pre-splash) so reset() can replay with a fresh seed.
        boot_buf = io.BytesIO()
        self._pyboy.save_state(boot_buf)
        self._boot_state = boot_buf.getvalue()

        if skip_splash:
            skip_through_splash(self._pyboy, self._g_game_addr)
            self._pyboy.tick(count=SETTLE_FRAMES, render=True)

    # ------------------------------------------------------------------ core

    def reset(self) -> np.ndarray:
        """Reload the boot state, idle a random number of frames on the splash
        screen so the hardware DIV register varies, then press START.  The game
        seeds its RNG from DIV_REG at the moment START is pressed, producing
        unique terrain each episode."""
        self._pyboy.load_state(io.BytesIO(self._boot_state))
        warmup = int(self._rng.integers(0, SPLASH_WARMUP_MAX))
        self._pyboy.tick(count=warmup, render=False)
        skip_through_splash(self._pyboy, self._g_game_addr)
        self._pyboy.tick(count=SETTLE_FRAMES, render=True)
        return self.observe()

    def step(self, action) -> np.ndarray:
        mask = _action_mask(action)
        pressed = [name for bit, name in zip(mask, BUTTON_NAMES, strict=True) if bit]
        for name in pressed:
            self._pyboy.button_press(name)
        try:
            self._pyboy.tick(count=self._frame_skip, render=True)
        finally:
            for name in pressed:
                self._pyboy.button_release(name)
        return self.observe()

    def observe(self) -> np.ndarray:
        return (255 - self._pyboy.screen.ndarray[..., 0]) >> 6

    # ---------------------------------------------------------------- state

    def game_state(self) -> GameState:
        return decode_game_state(self.g_game_bytes())

    def g_game_bytes(self) -> bytes:
        return bytes(self._pyboy.memory[self._g_game_addr : self._g_game_addr + GAME_STATE_SIZE])

    @property
    def g_game_addr(self) -> int:
        return self._g_game_addr

    # ----------------------------------------------------------- emulation

    def tick(self, count: int = 1, render: bool = True) -> bool:
        """Advance the emulator by *count* frames.

        Returns ``False`` when the emulator window has been closed.
        """
        return self._pyboy.tick(count=count, render=render)

    def set_emulation_speed(self, speed: int) -> None:
        """Set the target emulation speed (1 = real-time, 0 = unlimited)."""
        self._pyboy.set_emulation_speed(speed)

    # -------------------------------------------------------------- lifecycle

    def close(self) -> None:
        if self._pyboy is not None:
            self._pyboy.stop()
            self._pyboy = None  # type: ignore[assignment]

    def __enter__(self) -> BananasEnv:
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc: BaseException | None,
        tb: TracebackType | None,
    ) -> None:
        self.close()


# --- shared splash-skip ----------------------------------------------------
# Used by both BananasEnv (headless) and play.main (interactive SDL window).


def skip_through_splash(pyboy: PyBoy, g_game_addr: int) -> None:
    """Pulse START until g_game.state == AIM and players[0].lives == 3."""
    for _ in range(0, SPLASH_MAX_FRAMES, 8):
        if _in_game(pyboy, g_game_addr):
            return
        pyboy.button_press("start")
        pyboy.tick(count=4, render=True)
        pyboy.button_release("start")
        pyboy.tick(count=4, render=True)
    last = pyboy.memory[g_game_addr + OFF_STATE]
    raise RuntimeError(
        f"could not skip splash in {SPLASH_MAX_FRAMES} frames (last state byte: {last:#x})"
    )


def _in_game(pyboy: PyBoy, g_game_addr: int) -> bool:
    state = pyboy.memory[g_game_addr + OFF_STATE]
    lives = pyboy.memory[g_game_addr + OFF_PLAYER0_LIVES]
    return state == GameStateEnum.AIM and lives == 3


def _action_mask(action) -> np.ndarray:
    if isinstance(action, GameInput):
        return action.to_array().astype(bool)
    arr = np.asarray(action)
    if arr.shape != (ACTION_SIZE,):
        raise ValueError(f"action must have shape ({ACTION_SIZE},), got {arr.shape}")
    if arr.dtype.kind not in "biu" or ((arr != 0) & (arr != 1)).any():
        raise ValueError(f"action values must be bool/0/1, got dtype={arr.dtype}")
    return arr.astype(bool)
