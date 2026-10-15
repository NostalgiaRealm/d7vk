#pragma once

#include "dxvk_context.h"

namespace dxvk {

  // Transfer between physical render targets and logical-size CPU images. The
  // backend also handles multisampled destinations through a rendering pass.
  inline void BlitResolutionImage(DxvkContext* ctx,
      const Rc<DxvkImage>& dst, const Rc<DxvkImage>& src,
      VkImageSubresourceLayers layers, VkOffset3D dstOffset, VkExtent3D dstExtent,
      VkOffset3D srcOffset, VkExtent3D srcExtent,
      const VkImageSubresourceLayers* sourceLayers = nullptr) {
    ctx->blitResolutionImage(dst, src, layers, dstOffset, dstExtent,
      sourceLayers ? *sourceLayers : layers, srcOffset, srcExtent);
  }

}
