#include "platform.h"

#if defined(OS_WINDOWS)

// Physically separate implementations included in Unity Build order
#include "../basic/basic_win32.cpp"
#include "../window/window_win32.cpp"
#include "../render/render_win32.cpp"

void platform_init(Basic* basic, WindowApi* window, RenderApi* render) NOEXCEPT {
    if (basic)  basic_init(basic);
    if (window) window_init(window);
    if (render) render_init(render);
}

#endif // OS_WINDOWS
