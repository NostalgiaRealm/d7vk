#pragma once

#include "graphics_options.h"
#include "../util_window.h"
#include <windows.h>

namespace dxvk {

  inline void ApplyGraphicsWindowMode(HWND window, GraphicsWindow mode, uint32_t width, uint32_t height) {
#ifdef _WIN32
    if (mode == GraphicsWindow::App || !window || IsIconic(window))
      return;

    static thread_local bool adjustingWindow = false;
    if (adjustingWindow)
      return;
    struct AdjustmentGuard {
      bool& active;
      explicit AdjustmentGuard(bool& value) : active(value) { active = true; }
      ~AdjustmentGuard() { active = false; }
    } guard(adjustingWindow);

    // Native DirectDraw may retain an exclusive WineD3D swapchain so that
    // flipping surfaces remain valid. Wine keeps its fullscreen presentation
    // rectangle separately from the HWND style and client rectangle; clear
    // that per-window override before applying the requested window geometry.
    ClearWindowPresentRect(window);

    // Games and native DirectDraw can maximize/reconfigure the HWND after
    // device creation. Restore first: SetWindowPos alone does not leave the
    // window manager's maximized state on Wine or native Windows.
    if (IsZoomed(window))
      ShowWindow(window, SW_RESTORE);

    LONG_PTR oldStyle = GetWindowLongPtrW(window, GWL_STYLE);
    LONG_PTR oldExStyle = GetWindowLongPtrW(window, GWL_EXSTYLE);
    LONG_PTR style = (oldStyle & ~(WS_POPUP | WS_MAXIMIZE)) | WS_OVERLAPPEDWINDOW;
    LONG_PTR exStyle = oldExStyle & ~WS_EX_TOPMOST;
    bool changedStyle = style != oldStyle || exStyle != oldExStyle;
    if (style != oldStyle)
      SetWindowLongPtrW(window, GWL_STYLE, style);
    if (exStyle != oldExStyle)
      SetWindowLongPtrW(window, GWL_EXSTYLE, exStyle);

    RECT rect = { 0, 0, LONG(width), LONG(height) };
    AdjustWindowRectEx(&rect, DWORD(style), GetMenu(window) != nullptr, DWORD(exStyle));
    RECT current = {};
    if (!GetWindowRect(window, &current))
      return;
    UINT flags = SWP_NOACTIVATE;
    if (changedStyle)
      flags |= SWP_FRAMECHANGED;
    if (!(oldExStyle & WS_EX_TOPMOST))
      flags |= SWP_NOZORDER;
    int x = 0, y = 0;
    if (mode == GraphicsWindow::Centered) {
      MONITORINFO monitor = { sizeof(MONITORINFO) };
      if (GetMonitorInfoW(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &monitor)) {
        x = monitor.rcWork.left + (monitor.rcWork.right - monitor.rcWork.left - (rect.right - rect.left)) / 2;
        y = monitor.rcWork.top + (monitor.rcWork.bottom - monitor.rcWork.top - (rect.bottom - rect.top)) / 2;
      }
    } else {
      flags |= SWP_NOMOVE;
    }
    if (mode == GraphicsWindow::Centered && current.left == x && current.top == y)
      flags |= SWP_NOMOVE;
    if (current.right - current.left == rect.right - rect.left
     && current.bottom - current.top == rect.bottom - rect.top)
      flags |= SWP_NOSIZE;

    // Present rechecks the override, but only actual changes send window
    // messages. Avoid a WM_SIZE/WM_WINDOWPOSCHANGED storm every frame.
    if (!changedStyle && (flags & SWP_NOMOVE) && (flags & SWP_NOSIZE))
      return;
    SetWindowPos(window, HWND_NOTOPMOST, x, y, rect.right - rect.left, rect.bottom - rect.top, flags);
#else
    (void)window; (void)mode; (void)width; (void)height;
#endif
  }

}
