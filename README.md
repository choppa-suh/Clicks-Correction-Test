# Click Correction

Adds a **Click Correction** button to the Geometry Dash pause menu. When it is on,
a click that comes in a little too early (while you are still in the air) is
remembered and applied the moment you land, instead of being thrown away.

* **Pause menu button** (green "CC" circle, top-right of the pause screen) opens a
  small panel with an on/off toggle and the max-frames slider.
* **Max Correction Frames** (1-30, default 4) is the largest number of physics
  frames a click may be early and still be corrected. At 240 TPS, 4 frames is
  about 17 ms.

## How it works

Every jump press and release goes through `GJBaseGameLayer::handleButton`. If the
player is on a mode that needs the ground (cube, ball, robot, spider) and is in
the air, the click is stored together with an expiry frame. Each physics step
(`GJBaseGameLayer::processCommands`) the mod checks whether the player has
landed:

| Situation on landing | What happens |
| --- | --- |
| Button already released | The click is replayed for one frame, so the jump / flip happens |
| Button still held | Nothing is injected, the game's own hold-to-jump handling covers it |
| Window ran out | The click is discarded |

Clicks are not buffered while touching an orb, dashing, or in ship / UFO / wave /
swing modes, because those either react in mid-air or may already have used the
click.

## Settings

| Setting | Default | Description |
| --- | --- | --- |
| Enable Click Correction | off | Master switch (also toggled from the pause menu) |
| Max Correction Frames | 4 | Size of the correction window, 1 to 30 frames |
| Show Pause Menu Button | on | Hide the button if you only want the setting page |

## Building

1. Install the [Geode SDK and CLI](https://docs.geode-sdk.org/getting-started/) and
   set the `GEODE_SDK` environment variable.
2. Edit `mod.json` and change `id`, `developer` and `repository` to your own.
3. Build:

```sh
geode build
```

The resulting `.geode` file ends up in `build/`; copy it into your Geode `mods`
folder (or run `geode package install <file>`).

The included GitHub workflow (`.github/workflows/multi-platform.yml`) builds for
Windows, macOS, Android and iOS when you push the project to GitHub.

## Project layout

```
click-correction/
  CMakeLists.txt
  mod.json
  about.md
  changelog.md
  logo.png
  src/
    ClickCorrection.hpp          settings helpers
    hooks/GJBaseGameLayer.cpp    click buffering logic
    hooks/PauseLayer.cpp         pause menu button
    ui/CorrectionPopup.{hpp,cpp} toggle + slider popup
```

## Notes and limitations

* Targets Geometry Dash **2.2081** with Geode **5.x**.
* A "frame" here is one physics step, not one rendered frame. If you use a TPS
  bypass above 240, the same number of frames lasts a shorter amount of time.
* The mod only forgives clicks that are **early**. It cannot rewind the game to
  rescue a click that is late.
* Using it in rated or leaderboard-style challenges may be considered unfair by
  the community; use it where it is allowed.

## License

MIT, see `LICENSE`.
