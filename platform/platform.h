#ifndef PLATFORM_H
#define PLATFORM_H

#include "../core.h"
#include "../basic/basic.h"
#include "../window/window.h"
#include "../render/render.h"

// ============================================================================
// Platform Initialization Interface
// ============================================================================
EXTERN_C void platform_init(Basic* basic, WindowApi* window, RenderApi* render) NOEXCEPT;

#endif // PLATFORM_H
