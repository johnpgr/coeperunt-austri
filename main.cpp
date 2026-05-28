#include "core.h"

#if defined(OS_WINDOWS)
    #include "platform/platform_windows.cpp"
#elif defined(OS_LINUX)
    #include "platform/platform_linux.cpp"
#endif

Basic basic;
WindowApi window_api;
RenderApi render_api;

#if defined(OS_LINUX)
EXTERN_C char** environ;
#endif

START {
    #if defined(OS_LINUX)
    environ = envp;
    #endif

    platform_init(&basic, &window_api, &render_api);
    
    basic.printf("[System] Initializing window and graphics context...\n");
    
    PlatformWindow* windowptr = window_api.create("Unnamed Game", 800, 600);
    if (!windowptr) {
        basic.printf("[Error] Failed to create window!\n");
        basic.exit(1);
    }
    
    GraphicsContext* context = render_api.init(windowptr);
    if (!context) {
        basic.printf("[Error] Failed to initialize graphics context!\n");
        window_api.destroy(windowptr);
        basic.exit(1);
    }
    
    i32 width = 800;
    i32 height = 600;
    b8 quit = false;

    basic.printf("[System] Subsystem initialized successfully. Running frame loop (Window: %p, Width: %d, Height: %d)...\n", windowptr, width, height);
    
    while (!quit) {
        window_api.poll(windowptr, &width, &height, &quit);
        render_api.clear(context, 0.05f, 0.07f, 0.11f, 1.0f);
        render_api.swap(windowptr, context);
    }
    
    basic.printf("[System] Window close requested. Cleaning up resources...\n");
    
    render_api.destroy(context);
    window_api.destroy(windowptr);
    
    basic.printf("[System] Subsystem shutdown complete. Exiting cleanly with code %d.\n", 0);
    
    basic.exit(0);
}
