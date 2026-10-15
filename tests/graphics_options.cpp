#include "../src/util/config/graphics_options.h"
#include "../src/util/config/graphics_filter.h"

#include <cstdlib>
#include <iostream>

namespace dxvk {
  Logger Logger::s_instance("graphics-options-test");
}

using namespace dxvk;

static void check(bool result, const char* message) {
  if (!result) {
    std::cerr << "FAIL: " << message << '\n';
    std::exit(1);
  }
}

int main() {
  const GraphicsOptions defaults{Config()};
  check(defaults.filtering == GraphicsFilter::App && defaults.mipmapping == GraphicsMipmaps::App,
    "default texture behaviour remains application controlled");
  check(!defaults.scalesResolution() && defaults.refreshRate == 0 && defaults.msaa == -1
    && defaults.depthBits == 0 && defaults.windowMode == GraphicsWindow::App && !defaults.forceVSync,
    "default rendering behaviour remains application controlled");

  Config config;
  config.setOption("d7vk.filtering", "16");
  config.setOption("d7vk.keepPointFiltering", "True");
  config.setOption("d7vk.mipmapping", "autogen_bilinear");
  config.setOption("d7vk.resolution", "2560x1440");
  config.setOption("d7vk.refreshRate", "144");
  config.setOption("d7vk.windowMode", "centered");
  config.setOption("d7vk.msaa", "8");
  config.setOption("d7vk.depthBits", "32");
  config.setOption("d7vk.forceVSync", "True");
  const GraphicsOptions forced(config);
  check(forced.filtering == GraphicsFilter::Anisotropic && forced.anisotropy == 16
    && forced.keepPointFiltering && forced.generatesMipmaps(), "Kao texture preset");
  check(forced.width == 2560 && forced.height == 1440 && forced.refreshRate == 144
    && forced.msaa == 8 && forced.depthBits == 32 && forced.forceVSync
    && forced.windowMode == GraphicsWindow::Centered, "Kao rendering preset");

  for (auto value : {"2x", "3x", "4x"}) {
    config.setOption("d7vk.resolution", value);
    auto parsed = GraphicsOptions(config);
    check(parsed.resolutionScale == uint32_t(value[0] - '0') && !parsed.width && !parsed.height,
      "integer resolution multipliers");
  }
  for (auto value : {"0x1440", "2560x0", "-1x1080", "2560x1440junk", "9999999999999999999x2",
      "16385x1080", "2560", "2xx", "", "1920x1080x60"}) {
    config.setOption("d7vk.resolution", value);
    check(!GraphicsOptions(config).scalesResolution(), "reject malformed or oversized resolution");
  }
  for (auto value : {"-2", "1", "3", "16", "9999"}) {
    config.setOption("d7vk.msaa", value);
    check(GraphicsOptions(config).msaa == -1, "reject unsupported MSAA requests");
  }
  config.setOption("d7vk.depthBits", "16");
  config.setOption("d7vk.refreshRate", "-1");
  config.setOption("d7vk.filtering", "123");
  config.setOption("d7vk.windowMode", "invalid");
  config.setOption("d7vk.mipmapping", "invalid");
  auto invalid = GraphicsOptions(config);
  check(invalid.depthBits == 0 && invalid.refreshRate == 0 && invalid.filtering == GraphicsFilter::App
    && invalid.windowMode == GraphicsWindow::App && invalid.mipmapping == GraphicsMipmaps::App,
    "invalid overrides fall back to application behaviour");

  GraphicsOptions policy;
  const GraphicsSamplerFilter point = { false, false, false, false, 0 };
  const GraphicsSamplerFilter linear = { true, true, false, true, 0 };
  policy.filtering = GraphicsFilter::Anisotropic;
  for (uint32_t af : { 2u, 4u, 8u, 16u }) {
    policy.anisotropy = af;
    auto sampler = applyGraphicsSamplerFilter(policy, point);
    check(sampler.minLinear && sampler.magLinear && sampler.mipLinear
      && sampler.anisotropy == af, "force AF even for point textures when opt-out is disabled");
    policy.keepPointFiltering = true;
    sampler = applyGraphicsSamplerFilter(policy, point);
    check(!sampler.minLinear && !sampler.magLinear && !sampler.mipLinear
      && sampler.anisotropy == 0 && !sampler.mipEnabled, "preserve point-sampled HUD");
    sampler = applyGraphicsSamplerFilter(policy, linear);
    check(sampler.anisotropy == af, "preserving point textures still enhances linear textures");
    policy.keepPointFiltering = false;
  }
  policy.filtering = GraphicsFilter::Point;
  auto sampler = applyGraphicsSamplerFilter(policy, linear);
  check(!sampler.minLinear && !sampler.magLinear && !sampler.mipLinear && !sampler.anisotropy,
    "force point filtering");
  policy.filtering = GraphicsFilter::Bilinear;
  sampler = applyGraphicsSamplerFilter(policy, point);
  check(sampler.minLinear && sampler.magLinear && !sampler.mipLinear && !sampler.anisotropy,
    "force bilinear filtering");
  policy.filtering = GraphicsFilter::Trilinear;
  sampler = applyGraphicsSamplerFilter(policy, point);
  check(sampler.minLinear && sampler.magLinear && sampler.mipLinear && !sampler.anisotropy,
    "force trilinear filtering");
  policy.mipmapping = GraphicsMipmaps::Disabled;
  sampler = applyGraphicsSamplerFilter(policy, linear);
  check(!sampler.mipEnabled && !sampler.mipLinear, "mipmap disable takes precedence over trilinear");
  policy.filtering = GraphicsFilter::App;
  policy.mipmapping = GraphicsMipmaps::Bilinear;
  sampler = applyGraphicsSamplerFilter(policy, point);
  check(sampler.mipEnabled, "generated mipmaps become available to the sampler");
  check(GraphicsOptions::screenRelative(1920, 1080, 1920, 1080)
    && GraphicsOptions::screenRelative(960, 540, 1920, 1080)
    && GraphicsOptions::screenRelative(960, 540, 1921, 1081),
    "scale full and fractional screen targets with integer rounding");
  check(!GraphicsOptions::screenRelative(256, 256, 1920, 1080)
    && !GraphicsOptions::screenRelative(512, 128, 1920, 1080)
    && !GraphicsOptions::screenRelative(1, 1, 1920, 1080)
    && !GraphicsOptions::screenRelative(1920, 1080, 0, 0),
    "preserve texture atlases, lookup tables and unknown screen layouts");
  std::cout << "All graphics option tests passed\n";
}
