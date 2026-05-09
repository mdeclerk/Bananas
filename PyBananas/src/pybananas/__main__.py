from __future__ import annotations

from .env import BananasEnv

WINDOW = "SDL2"
SCALE = 3
SPEED = 1
SOUND = True


def main() -> None:
    with BananasEnv(
        window=WINDOW, scale=SCALE, sound=SOUND, no_input=False, skip_splash=False,
    ) as env:
        env.set_emulation_speed(SPEED)
        print("Bananas is running. Focus the PyBoy window to play; close it to exit.")
        while env.tick():
            pass


if __name__ == "__main__":
    main()
