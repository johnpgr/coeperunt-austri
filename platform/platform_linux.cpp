#include "platform.h"

#if defined(OS_LINUX)

// Physically separate implementations included in Unity Build order
#include "../basic/basic_linux.cpp"
#include "../window/window_linux.cpp"
#include "../render/render_linux.cpp"

void platform_init(Basic* basic, WindowApi* window, RenderApi* render) NOEXCEPT {
    if (basic)  basic_init(basic);
    if (window) window_init(window);
    if (render) render_init(render);
}

#endif // OS_LINUX
