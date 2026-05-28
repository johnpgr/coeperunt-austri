#include "render.h"

#if defined(OS_WINDOWS)

#define COBJMACROS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

// Linker directives for Windows
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

struct GraphicsContext {
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swap_chain;
    ID3D11RenderTargetView* render_target_view;
    i32 cached_width;
    i32 cached_height;
};

// Global static instances for no-stdlib single-window architecture
static GraphicsContext g_context = {};

static GraphicsContext* __win32_init_graphics(PlatformWindow* window) NOEXCEPT {
    if (!window || !window->hwnd) return nullptr;
    
    DXGI_SWAP_CHAIN_DESC sc_desc = {};
    sc_desc.BufferCount = 1;
    sc_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sc_desc.BufferDesc.Width = window->width;
    sc_desc.BufferDesc.Height = window->height;
    sc_desc.BufferDesc.RefreshRate.Numerator = 60;
    sc_desc.BufferDesc.RefreshRate.Denominator = 1;
    sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_desc.OutputWindow = window->hwnd;
    sc_desc.SampleDesc.Count = 1;
    sc_desc.Windowed = TRUE;
    
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL out_feature_level;
    
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        feature_levels,
        1,
        D3D11_SDK_VERSION,
        &sc_desc,
        &g_context.swap_chain,
        &g_context.device,
        &out_feature_level,
        &g_context.context
    );
    
    if (FAILED(hr)) {
        return nullptr;
    }
    
    ID3D11Texture2D* backbuffer = nullptr;
    hr = g_context.swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
    if (SUCCEEDED(hr)) {
        hr = g_context.device->CreateRenderTargetView(backbuffer, nullptr, &g_context.render_target_view);
        backbuffer->Release();
    }
    
    if (FAILED(hr)) {
        if (g_context.context) g_context.context->Release();
        if (g_context.swap_chain) g_context.swap_chain->Release();
        if (g_context.device) g_context.device->Release();
        g_context = {};
        return nullptr;
    }
    
    g_context.cached_width = window->width;
    g_context.cached_height = window->height;
    
    return &g_context;
}

// Internal helper to handle resize dynamically inside clear/swap operations
static void __win32_handle_resize(PlatformWindow* window, GraphicsContext* context) {
    if (window->width != context->cached_width || window->height != context->cached_height) {
        if (context->render_target_view) {
            context->render_target_view->Release();
            context->render_target_view = nullptr;
        }
        
        context->swap_chain->ResizeBuffers(0, window->width, window->height, DXGI_FORMAT_UNKNOWN, 0);
        
        ID3D11Texture2D* backbuffer = nullptr;
        HRESULT hr = context->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
        if (SUCCEEDED(hr)) {
            context->device->CreateRenderTargetView(backbuffer, nullptr, &context->render_target_view);
            backbuffer->Release();
        }
        
        context->cached_width = window->width;
        context->cached_height = window->height;
    }
}

static void __win32_clear_screen(GraphicsContext* context, f32 r, f32 g, f32 b, f32 a) NOEXCEPT {
    if (context && context->context && context->render_target_view) {
        float clear_color[4] = { r, g, b, a };
        context->context->ClearRenderTargetView(context->render_target_view, clear_color);
        
        context->context->OMSetRenderTargets(1, &context->render_target_view, nullptr);
        
        D3D11_VIEWPORT vp = {};
        vp.Width = (float)context->cached_width;
        vp.Height = (float)context->cached_height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        context->context->RSSetViewports(1, &vp);
    }
}

static void __win32_swap_buffers(PlatformWindow* window, GraphicsContext* context) NOEXCEPT {
    if (window && context && context->swap_chain) {
        // Automatically check if the window resized and adjust our buffers before presenting!
        __win32_handle_resize(window, context);
        
        context->swap_chain->Present(1, 0); // VSync enabled
    }
}

static void __win32_destroy_graphics(GraphicsContext* context) NOEXCEPT {
    if (context) {
        if (context->render_target_view) {
            context->render_target_view->Release();
            context->render_target_view = nullptr;
        }
        if (context->swap_chain) {
            context->swap_chain->Release();
            context->swap_chain = nullptr;
        }
        if (context->context) {
            context->context->Release();
            context->context = nullptr;
        }
        if (context->device) {
            context->device->Release();
            context->device = nullptr;
        }
    }
}

void render_init(RenderApi* render) NOEXCEPT {
    if (render) {
        render->init    = __win32_init_graphics;
        render->clear   = __win32_clear_screen;
        render->swap    = __win32_swap_buffers;
        render->destroy = __win32_destroy_graphics;
    }
}

#endif // OS_WINDOWS
