#pragma once

#include "config.h"
#include "../log/log.h"

#include <charconv>
#include <cstdint>

namespace dxvk {

  enum class GraphicsFilter { App, Point, Bilinear, Trilinear, Anisotropic };
  enum class GraphicsMipmaps { App, Disabled, Point, Bilinear };
  enum class GraphicsWindow { App, Windowed, Centered };

  /** Shared, opt-in graphics overrides. Defaults preserve upstream behaviour. */
  struct GraphicsOptions {
    GraphicsFilter filtering = GraphicsFilter::App;
    uint32_t anisotropy = 1;
    bool keepPointFiltering = false;
    GraphicsMipmaps mipmapping = GraphicsMipmaps::App;
    uint32_t resolutionScale = 1;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t refreshRate = 0;
    GraphicsWindow windowMode = GraphicsWindow::App;
    int32_t msaa = -1;
    uint32_t depthBits = 0;
    bool forceVSync = false;

    GraphicsOptions() = default;

    explicit GraphicsOptions(const Config& config) {
      auto filter = Config::toLower(config.getOption<std::string>("d7vk.filtering", "app"));
      if (filter == "point") filtering = GraphicsFilter::Point;
      else if (filter == "bilinear") filtering = GraphicsFilter::Bilinear;
      else if (filter == "trilinear") filtering = GraphicsFilter::Trilinear;
      else if (filter == "2" || filter == "4" || filter == "8" || filter == "16") {
        filtering = GraphicsFilter::Anisotropic;
        anisotropy = uint32_t(std::stoi(filter));
      } else if (!filter.empty() && filter != "app") {
        Logger::warn("D7VK: Invalid d7vk.filtering; using app");
      }
      keepPointFiltering = config.getOption<bool>("d7vk.keepPointFiltering", false);

      auto mips = Config::toLower(config.getOption<std::string>("d7vk.mipmapping", "app"));
      if (mips == "disabled") mipmapping = GraphicsMipmaps::Disabled;
      else if (mips == "autogen_point") mipmapping = GraphicsMipmaps::Point;
      else if (mips == "autogen_bilinear") mipmapping = GraphicsMipmaps::Bilinear;
      else if (!mips.empty() && mips != "app") Logger::warn("D7VK: Invalid d7vk.mipmapping; using app");

      auto resolution = Config::toLower(config.getOption<std::string>("d7vk.resolution", "unforced"));
      if (resolution == "2x" || resolution == "3x" || resolution == "4x") {
        resolutionScale = resolution[0] - '0';
      } else if (!resolution.empty() && resolution != "unforced") {
        uint32_t w = 0, h = 0;
        const char* first = resolution.data();
        const char* last = first + resolution.size();
        auto x = std::from_chars(first, last, w);
        auto y = x.ptr < last && *x.ptr == 'x'
          ? std::from_chars(x.ptr + 1, last, h) : x;
        if (x.ec == std::errc() && y.ec == std::errc() && y.ptr == last
            && w > 0 && h > 0 && w <= 16384 && h <= 16384) {
          width = w;
          height = h;
        } else {
          Logger::warn("D7VK: Invalid d7vk.resolution; using unforced");
        }
      }
      int rate = config.getOption<int32_t>("d7vk.refreshRate", 0);
      if (rate >= 0 && rate <= 1000) refreshRate = uint32_t(rate);
      else Logger::warn("D7VK: Invalid d7vk.refreshRate; using app");

      auto window = Config::toLower(config.getOption<std::string>("d7vk.windowMode", "app"));
      if (window == "windowed") windowMode = GraphicsWindow::Windowed;
      else if (window == "centered") windowMode = GraphicsWindow::Centered;
      else if (!window.empty() && window != "app") Logger::warn("D7VK: Invalid d7vk.windowMode; using app");

      msaa = config.getOption<int32_t>("d7vk.msaa", -1);
      if (msaa != -1 && msaa != 0 && msaa != 2 && msaa != 4 && msaa != 8) {
        Logger::warn("D7VK: Invalid d7vk.msaa; using app");
        msaa = -1;
      }
      int depth = config.getOption<int32_t>("d7vk.depthBits", 0);
      if (depth == 0 || depth == 24 || depth == 32) depthBits = uint32_t(depth);
      else Logger::warn("D7VK: Invalid d7vk.depthBits; using app");
      forceVSync = config.getOption<bool>("d7vk.forceVSync", false);
    }

    bool scalesResolution() const { return width || resolutionScale != 1; }
    bool generatesMipmaps() const {
      return mipmapping == GraphicsMipmaps::Point || mipmapping == GraphicsMipmaps::Bilinear;
    }

    // Programmable renderers also use render-target textures as glyph atlases
    // or lookup tables addressed with integer texel coordinates. Only scale
    // screen-relative attachments; allow one pixel of rounding in fractional
    // resolution passes such as half-resolution effects on odd-sized windows.
    static bool screenRelative(uint32_t width, uint32_t height,
                               uint32_t screenWidth, uint32_t screenHeight) {
      if (!screenWidth || !screenHeight)
        return false;
      if (width == screenWidth && height == screenHeight)
        return true;
      if (width < 2 || height < 2 || screenWidth < 2 || screenHeight < 2)
        return false;
      uint64_t a = uint64_t(width) * screenHeight;
      uint64_t b = uint64_t(height) * screenWidth;
      uint64_t tolerance = screenWidth > screenHeight ? screenWidth : screenHeight;
      return (a > b ? a - b : b - a) <= tolerance;
    }
  };

}
