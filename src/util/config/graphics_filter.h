#pragma once

#include "graphics_options.h"

namespace dxvk {

  // API-independent filtering policy. Keep the original sampler description
  // intact: GetSamplerState/GetDesc must continue reporting application state.
  struct GraphicsSamplerFilter {
    bool minLinear;
    bool magLinear;
    bool mipLinear;
    bool mipEnabled;
    uint32_t anisotropy;
  };

  inline GraphicsSamplerFilter applyGraphicsSamplerFilter(
      const GraphicsOptions& options, GraphicsSamplerFilter filter) {
    const bool preservePoint = options.keepPointFiltering
      && !filter.minLinear && !filter.magLinear;

    if (!preservePoint && options.filtering != GraphicsFilter::App) {
      filter.minLinear = options.filtering != GraphicsFilter::Point;
      filter.magLinear = filter.minLinear;
      filter.mipLinear = options.filtering == GraphicsFilter::Trilinear
        || options.filtering == GraphicsFilter::Anisotropic;
      filter.anisotropy = options.filtering == GraphicsFilter::Anisotropic
        ? options.anisotropy : 0u;
      filter.mipEnabled = true;
    }

    if (options.mipmapping == GraphicsMipmaps::Disabled) {
      filter.mipEnabled = false;
      filter.mipLinear = false;
    } else if (options.generatesMipmaps() && !preservePoint) {
      filter.mipEnabled = true;
    }

    return filter;
  }

}
