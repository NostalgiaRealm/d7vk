# D7VK XP validation

Validation is limited to **Kao the Kangaroo**, as requested. This is an experimental fork, not a claim of compatibility with every game or rendering pipeline.

## Environment

- Linux KDE Wayland/Xwayland; AMD Radeon RX 6700 XT.
- RADV, Mesa 26.2.4; Lutris Wine 11.15 amd64 runner.
- 32-bit Kao, D3D6 renderer, application configuration 1920×1080.
- Upstream source base: `d5388d4b7` from WinterSnowfall/d7vk.
- LLVM-MinGW 20261006 / Clang 23.1.3; release builds for x86 and x64.

## Build and source checks

Both architectures build `ddraw.dll`, `d3d8.dll`, `d3d9.dll`, `d3d10core.dll`, `d3d11.dll`, and `dxgi.dll`. PE architectures and exports were checked against the module definitions. Imports require Windows/UCRT libraries, with no separate MinGW C++ or threading runtime DLLs. D3D8 uses the included D3D9 DLL; D3D10Core uses D3D11 and DXGI.

The graphics-options test passes under Wine in both architectures. It checks application defaults, malformed/oversized configuration, all sampler override modes, point-sampler preservation, mip disabling, generated mip availability, and modern screen-relative attachment selection. `git diff --check` and `bash -n package-release.sh` pass. Existing compiler warnings remain; the builds do not use `-Werror`.

## Game checks

| Path / settings | Observed result |
| --- | --- |
| Original dgVoodoo 2.87.5 fixed + DXVK 3.1.1 | Baseline starts and displays the animated scene at 2560×1440. Original configuration requests 8× MSAA and AF16. |
| This fork's DDraw → Vulkan; 2560×1440, MSAA8, AF16 | Intro, main menu, save-selection map and a loaded snow level render. Movement and jumping respond. Logs show 1920×1080 logical resources and viewport → 2560×1440 physical, matching 8-sample color/depth attachments and AF16 sampler state. |
| DDraw; bilinear mip generation, MSAA4, depth32, preserve point filtering | Menu renders, clean exit. Log confirms 4 samples and added mip tails, including 256×256 textures extended from 1 to 9 levels. |
| DDraw; point mip generation, MSAA2, 2× resolution, forced vSync | Menu renders. Logs show 3840×2160 rasterization, 2-sample attachments and FIFO presentation. Minimum-24-bit depth uses D32 on this GPU. |
| DDraw; 3× and 4× resolution with MSAA2 | Menus render at 5760×3240 and 7680×4320 internally. The 4× case also exercises bilinear filtering and disabled mipmapping. |
| DDraw; MSAA off, custom resolution and 3× resolution | Complete menus render and exit cleanly. The 3× case also exercises forced point filtering and disabled mipmapping. |
| DDraw; forced centered window | Complete menu fits a 1920×1080 decorated client, centered at (320, 193) on the 2560×1440 desktop. Win32 and X11 geometry agree; Wine's fullscreen flag is cleared. |
| Original dgVoodoo with its resolution/AA/filter overrides disabled → this fork's D3D11/DXGI | Menu renders with this fork applying 2560×1440, MSAA8, AF16, depth32 and bilinear mip generation. Logs confirm multisampled attachments, resolved shader views and generated mip tails. |

The temporary D3D11 test uses the **same game**, not another title. Its original dgVoodoo configuration and DXVK DLLs are restored afterward. The final installed renderer is this fork's standalone DDraw DLL.

## Fixes found through testing

- Present-timing initialization could crash inside Wine's native query even without graphics overrides. Timing is now opt-in, with additional initialization fallback checks.
- A legacy surface copy could write 1440 physical rows into a 1080-row CPU buffer. Transfers are bounded by both surface descriptions.
- Depth/stencil scaling required separate aspect transfers, depth/stencil attachment shaders on formats without native blit support, and an MSAA-aware resolve/broadcast path. This also fixes missing scene geometry when MSAA is off.
- Modern render-target texture atlases using integer texel addressing were corrupted by unconditional scaling. Screen-aspect matching preserves these unrelated textures while scaling scene attachments.
- Wine's native DirectDraw uses a separate per-window fullscreen presentation rectangle; forced window mode must clear that state in addition to changing Win32 window styles.

## Limits of this validation

Only the 32-bit game was exercised; x64 and the other API frontends were compiled and reviewed. This is not a full campaign playthrough or a performance benchmark. There was no physical monitor refresh-rate measurement on the Wayland compositor and no Windows runtime test.

Unsupported mip-generation formats retain their original chains. Shared, sparse/tiled, CPU-accessible, UAV and application-authored multi-mip attachments can retain their original layout. Mixed incompatible attachments, shader integer texel coordinates, and unusual post-processing pipelines can still require application-specific work. Existing shader-visible multisample resources keep enough samples for the application's literal sample indices, including when a lower override is requested. See the README for the supported configuration and deployment combinations.

Local test logs and screenshots are retained in `.local/evidence/` and excluded from Git, along with backups and proprietary game/wrapper files. The repository and binary package contain no game assets or dgVoodoo binaries.
