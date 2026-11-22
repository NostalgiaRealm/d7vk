# D7VK XP changelog

Changes specific to this fork. Upstream changes are described in the
[WinterSnowfall/d7vk releases](https://github.com/WinterSnowfall/d7vk/releases).

## XP 0.2

Upstream sync. No new graphics overrides; the `d7vk.*` settings and their defaults are the same as in XP 0.1.

- Rebased the fork from upstream `d5388d4b7` (pre-v2.4) onto upstream **D7VK v2.5** (`9fb6a2d50`). Upstream rebases its `devel` branch onto newer DXVK, so the fork's branch was replayed on top of it instead of merged; XP 0.1 stays available under the `xp-0.1` tag.
- Brings in upstream v2.4 and v2.5, among others: Glide compatibility mode, the legacy mode filter and legacy aspect ratio options, proxied gamma ramps, D3D9 adapter groups, reworked present timing and latency handling in the DXVK backend, reworked D3D9 texture mapping and upload handling, arm64x build support, and many DirectDraw surface and mode enumeration fixes.
- Graphics overrides ported to the changed upstream code:
  - D3D9 display mode changes keep the forced resolution/refresh fallback and now respect adapter group heads.
  - The present-timing fallback (opt-in timing, ordinary presentation when the timing domain is unavailable) was re-applied to upstream's rewritten presenter.
  - `ddraw.backBufferResize` still defaults to on only when a forced render size is configured.
  - Scaled-resource readback and forced mip tails were adapted to upstream's reworked D3D9 texture lock and upload code.
- `package-release.sh`: keeps building both architectures with all bundled wrappers and the `--llvm` option, and gains upstream's `--build-arm64x` / `--arm64x-only` options. The 64-bit DLLs are packaged in `x64/` as before.
- The release archive now includes this changelog.

## XP 0.1

- Initial release, based on upstream `d5388d4b7`.
- Configurable overrides shared by the D3D3/5/6/7 renderer and the bundled D3D8/9/10/11 wrappers: texture filtering, point-filter preservation, mipmapping, internal resolution, refresh rate, window mode, MSAA, depth precision and forced vSync.
- Present timing made opt-in, with initialization fallbacks.
- Kao the Kangaroo preset.
