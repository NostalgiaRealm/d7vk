#pragma once

#include <windows.h>

namespace dxvk {

  // Clear Wine's separate fullscreen presentation rectangle for this HWND.
  void ClearWindowPresentRect(HWND window);

}
