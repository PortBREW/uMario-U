# uMario-U
A port of uMario (a remake of SMB NES with 16:9 widescreen) to the Nintendo Wii U.
## Install

### Aroma (recommended)

Aroma's bundled `homebrew_on_menu` plugin scans `sd:/wiiu/apps/` for `.rpx` and `.wuhb` files and lists them **directly on the Wii U Menu**, so no Homebrew Launcher is involved.

1. Copy the contents of `dist/` to the root of your SD card. You should end up with:
   ```
   sd:/wiiu/apps/uMarioU/uMarioU.wuhb
   sd:/wiiu/apps/uMarioU/meta.xml      (optional)
   sd:/wiiu/apps/uMarioU/icon.png      (optional)
   ```
2. Put the SD card back in the console and boot into Aroma.
3. The app appears on the Wii U Menu as **uMario U**. If it does not, reboot into Aroma; the plugin only rescans at boot.

`dist/uMarioU-sd.zip` is the same layout, ready to extract. The `.wuhb` is self-contained: executable, all game data, icon, TV splash and GamePad splash are inside the one file, mounted at `/vol/content`.

**Note:** a bare `.wuhb` dropped anywhere outside `sd:/wiiu/apps/` is ignored, and Tiramisu's Homebrew Launcher does not list `.wuhb` files at all. Both are the usual reasons an app "doesn't appear".

### Tiramisu / Homebrew Launcher (legacy)

Build the dist with the `.rpx` and loose data included:

```bash
./tools/make_dist.sh --with-rpx
```

Then copy the contents of `dist/` to the SD card, giving:
```
sd:/wiiu/apps/uMarioU/uMarioU.rpx
sd:/wiiu/apps/uMarioU/meta.xml
sd:/wiiu/apps/uMarioU/icon.png
sd:/wiiu/apps/uMarioU/uNext/files/images/*.bmp
sd:/wiiu/apps/uMarioU/uNext/files/sounds/*.wav
sd:/wiiu/apps/uMarioU/imagery/portbrew.png
```

The port probes `/vol/content` first, then the SD card paths `/wiiu/apps/uMarioU`, `/wiiu/apps/uMario U` and the SD root, so a flattened layout also works.
## Controls

| Action | GamePad | Wii U Pro / Classic | Wii Remote (+ Nunchuk) |
| --- | --- | --- | --- |
| Move | D-pad / Left stick | D-pad / Left stick | D-pad / Nunchuk stick |
| Jump | **A**, **Y**, ZL, L | **A**, ZR | **A**, **2**, Z |
| Run / fire | **B**, **X**, ZR, R | **B**, ZL | **B**, **1**, C |
| Pause / confirm | **+** | **+** | **+** |
| Pause / back | **−** | **−** | **−** |

**+** pauses during gameplay (as in the NES original); both **+** and **−** pause. From the pause menu, select *Quit to Homescreen* to return to the Wii U menu.
## Build from source
Requires the `devkitpro/devkitppc` Docker image (devkitPPC + wut + wiiu portlibs). No host toolchain is needed.

```bash
docker run --rm -v "$PWD":/w -w /w devkitpro/devkitppc:latest bash -lc '
  export DEVKITPRO=/opt/devkitpro
  /opt/devkitpro/portlibs/wiiu/bin/powerpc-eabi-cmake \
      -S. -Bbuild \
      -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/WiiU.cmake \
      -DCMAKE_BUILD_TYPE=Release
  cmake --build build -j$(nproc)
'
```
Outputs `build/uMarioU.rpx` and `build/uMarioU.wuhb`, then package the SD layout:
```bash
./tools/make_dist.sh              # Aroma: .wuhb only
./tools/make_dist.sh --with-rpx  # also ship .rpx + loose data
```
Result lands in `dist/`, plus `dist/uMarioU-sd.zip`.
## Credits
* Łukasz Jakowski - Original uMario developer
* WeegeeDEVELOPER - uMario Vita developer (which this fork is based off of their fork)
* Built with devKitPro, wut & SDL2
## Known limitations
* No touch-screen or motion controls; input is buttons only.
* The GamePad and TV always show the same image; there is no asymmetric two-screen mode.
* Saving is not implemented (the original game has none either).
* The `content/` tree is ~24 MB, dominated by 22 MB of uncompressed WAV audio.
