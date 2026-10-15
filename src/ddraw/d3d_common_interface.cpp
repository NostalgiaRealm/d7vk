#include "d3d_common_interface.h"

#include "d3d_common_material.h"

namespace dxvk {

  std::atomic<D3DMATERIALHANDLE> D3DCommonInterface::s_materialHandle = 0;

  D3DCommonInterface::D3DCommonInterface()
    : m_d3d9Intf ( d3d9::Direct3DCreate9(D3D_SDK_VERSION) ) {
  }

  D3DCommonInterface::~D3DCommonInterface() {
  }

  HRESULT STDMETHODCALLTYPE D3DCommonInterface::QueryInterface(REFIID riid, void** ppvObject) {
    *ppvObject = ref(this);
    return S_OK;
  }

  D3DCommonMaterial* D3DCommonInterface::GetCommonMaterialFromHandle(D3DMATERIALHANDLE handle) {
    if (unlikely(handle == 0))
      return nullptr;

    auto materialsIter = s_materials.find(handle);
    if (unlikely(materialsIter == s_materials.end())) {
      Logger::warn(str::format("D3DCommonInterface::GetCommonMaterialFromHandle: Unknown handle: ", handle));
      return nullptr;
    }

    return materialsIter->second;
  }

  void D3DCommonInterface::EmplaceMaterial(D3DMATERIALHANDLE handle, D3DCommonMaterial* commonMaterial) {
    s_materials.emplace(std::piecewise_construct,
                        std::forward_as_tuple(handle),
                        std::forward_as_tuple(commonMaterial));
  }

  void D3DCommonInterface::ReleaseMaterialHandle(D3DMATERIALHANDLE handle) {
    auto materialsIter = s_materials.find(handle);

    if (likely(materialsIter != s_materials.end()))
      s_materials.erase(materialsIter);
  }

  d3d9::D3DMULTISAMPLE_TYPE D3DCommonInterface::GetMultiSampleType(
      d3d9::D3DFORMAT backBufferFormat, uint32_t requestedSamples) const {
    for (uint32_t samples = requestedSamples; samples >= 2; samples /= 2) {
      auto type = d3d9::D3DMULTISAMPLE_TYPE(samples);
      bool supported = SUCCEEDED(m_d3d9Intf->CheckDeviceMultiSampleType(
        0, d3d9::D3DDEVTYPE_HAL, backBufferFormat, TRUE, type, nullptr));

      // A depth surface may be attached after device creation. Choose a count
      // usable by all of our advertised depth formats, so those attachments
      // do not end up with a different sample count from the colour buffer.
      for (auto format : { d3d9::D3DFMT_D16, d3d9::D3DFMT_D24X8, d3d9::D3DFMT_D24S8 }) {
        if (SUCCEEDED(m_d3d9Intf->CheckDeviceFormat(0, d3d9::D3DDEVTYPE_HAL,
            backBufferFormat, D3DUSAGE_DEPTHSTENCIL, d3d9::D3DRTYPE_SURFACE, format)))
          supported &= SUCCEEDED(m_d3d9Intf->CheckDeviceMultiSampleType(
            0, d3d9::D3DDEVTYPE_HAL, format, TRUE, type, nullptr));
      }

      if (supported) {
        Logger::info(str::format("D7VK: Using ", samples, "x MSAA (requested ", requestedSamples, "x)"));
        if (samples != requestedSamples)
          Logger::warn("D7VK: Requested MSAA count is unsupported; using the reported lower count");
        return type;
      }
    }
    Logger::warn(str::format("D7VK: ", requestedSamples, "x MSAA unsupported; antialiasing disabled"));
    return d3d9::D3DMULTISAMPLE_NONE;
  }

}
