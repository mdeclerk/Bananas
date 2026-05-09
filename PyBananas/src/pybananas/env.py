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
    """Headless PyBoy environment for the Bananas ROM.

    Observations are grayscale ``uint8`` arrays with shape ``(144, 160)`` and
    values ``0..3`` where ``0`` is lightest. Actions are either ``GameInput``
    values or length-8 bool/0-1 sequences in ``(UP, DOWN, LEFT, RIGHT, A, B,
    START, SELECT)`` order.
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
        """Create a Bananas environment.

        Args:
            rom_path: Optional path to a ``.gb`` ROM. Defaults to the packaged
                Bananas ROM.
            window: PyBoy window backend. Use ``"null"`` for headless training.
            scale: Display scale used when a visible PyBoy window is enabled.
            frame_skip: Emulator frames advanced by each ``step`` call.
            skip_splash: If true, advance from the splash screen to gameplay.
            sound: Whether PyBoy should emulate sound.
            no_input: Whether PyBoy should ignore host keyboard input.

        Raises:
            FileNotFoundError: If the ROM or matching ``.sym`` file is missing.
            ValueError: If ``frame_skip`` is less than 1.
        """
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
        """Start a new randomized episode and return the first observation.

        Reloads the boot state, idles for a random number of splash-screen
        frames, then presses START. This varies the hardware DIV register used
        by the game seed so terrain changes between episodes.

        Returns:
            Grayscale ``uint8`` observation with shape ``(144, 160)`` and
            values ``0..3``.
        """
        self._pyboy.load_state(io.BytesIO(self._boot_state))
        warmup = int(self._rng.integers(0, SPLASH_WARMUP_MAX))
        self._pyboy.tick(count=warmup, render=False)
        skip_through_splash(self._pyboy, self._g_game_addr)
        self._pyboy.tick(count=SETTLE_FRAMES, render=True)
        return self.observe()

    def step(self, action) -> np.ndarray:
        """Apply an action for one environment step.

        Args:
            action: ``GameInput`` or any length-8 bool/0-1 sequence in
                ``(UP, DOWN, LEFT, RIGHT, A, B, START, SELECT)`` order.

        Returns:
            Next grayscale ``uint8`` observation with shape ``(144, 160)``.

        Raises:
            ValueError: If ``action`` is not a valid 8-button mask.
        """
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
        """Return the current frame without advancing the emulator.

        Returns:
            Grayscale ``uint8`` observation with shape ``(144, 160)`` and
            values ``0..3``.
        """
        return (255 - self._pyboy.screen.ndarray[..., 0]) >> 6

    # ---------------------------------------------------------------- state

    def game_state(self) -> GameState:
        """Return the decoded game state from emulator memory."""
        return decode_game_state(self.g_game_bytes())

    def g_game_bytes(self) -> bytes:
        """Return raw bytes for the ROM's ``g_game`` state struct."""
        return bytes(self._pyboy.memory[self._g_game_addr : self._g_game_addr + GAME_STATE_SIZE])

    @property
    def g_game_addr(self) -> int:
        """Memory address of the ROM's ``g_game`` state struct."""
        return self._g_game_addr

    # ----------------------------------------------------------- emulation

    def tick(self, count: int = 1, render: bool = True) -> bool:
        """Advance the emulator by ``count`` frames.

        Args:
            count: Number of emulator frames to advance.
            render: Whether PyBoy should render frames while ticking.

        Returns:
            ``False`` when the emulator window has been closed, otherwise
            ``True``.
        """
        return self._pyboy.tick(count=count, render=render)

    def set_emulation_speed(self, speed: int) -> None:
        """Set target emulation speed.

        Args:
            speed: PyBoy speed multiplier. ``1`` is real-time; ``0`` is
                unlimited.
        """
        self._pyboy.set_emulation_speed(speed)

    # -------------------------------------------------------------- lifecycle

    def close(self) -> None:
        """Stop the PyBoy emulator and release environment resources."""
        if self._pyboy is not None:
            self._pyboy.stop()
            self._pyboy = None  # type: ignore[assignment]

    def __enter__(self) -> BananasEnv:
        """Return this environment for ``with`` statement use."""
        return self

    def __exit__(
        self,
        exc_type: type[BaseException] | None,
        exc: BaseException | None,
        tb: TracebackType | None,
    ) -> None:
        """Close the environment when leaving a ``with`` block."""
        self.close()


# --- shared splash-skip ----------------------------------------------------
# Used by both BananasEnv (headless) and play.main (interactive SDL window).


def skip_through_splash(pyboy: PyBoy, g_game_addr: int) -> None:
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
