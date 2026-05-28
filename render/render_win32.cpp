#include "../platform/platform.h"

#if defined(OS_WINDOWS)

#define COBJMACROS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

// Linker directives for Windows
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct GraphicsContext {
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swap_chain;
    ID3D11RenderTargetView* render_target_view;
    i32 cached_width;
    i32 cached_height;
    
    // Quad Rendering Resources
    ID3D11VertexShader* vertex_shader;
    ID3D11PixelShader* pixel_shader;
    ID3D11InputLayout* input_layout;
    ID3D11Buffer* instance_buffer;
    ID3D11Buffer* constant_buffer;
    ID3D11ShaderResourceView* texture_view;
    ID3D11SamplerState* sampler_state;
    ID3D11BlendState* blend_state;
    ID3D11RasterizerState* rasterizer_state;
    ID3D11DepthStencilState* depth_state;
};

// Global static instances for no-stdlib single-window architecture
static GraphicsContext g_context = {};

static void __win32_destroy_graphics(GraphicsContext* context) noexcept;

#define QUAD_SHADER_PATH "assets/shaders/quads.hlsl"

// Max quads enqueued per frame
#define MAX_QUADS 10000
static RenderCommandQuad g_quad_instances[MAX_QUADS];
static i32 g_quad_count = 0;

static GraphicsContext* __win32_init_graphics(PlatformWindow* window) noexcept {
    if (!window) return nullptr;

    core::printf("[System] Win32 renderer init start.\n");
    
    // Opaque cast or get HWND (it's the first member of PlatformWindow)
    HWND hwnd = *(HWND*)window;
    i32 width = *(i32*)((char*)window + sizeof(HWND));
    i32 height = *(i32*)((char*)window + sizeof(HWND) + sizeof(i32));
    
    DXGI_SWAP_CHAIN_DESC sc_desc = {};
    sc_desc.BufferCount = 1;
    sc_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sc_desc.BufferDesc.Width = width;
    sc_desc.BufferDesc.Height = height;
    sc_desc.BufferDesc.RefreshRate.Numerator = 60;
    sc_desc.BufferDesc.RefreshRate.Denominator = 1;
    sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sc_desc.OutputWindow = hwnd;
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

    core::printf("[System] D3D11 device and swap chain created.\n");
    
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

    core::printf("[System] Backbuffer render target created.\n");
    
    g_context.cached_width = width;
    g_context.cached_height = height;

    FileContent shader_source = api.fs.read_entire_file(QUAD_SHADER_PATH);
    if (!shader_source.data || shader_source.size == 0) {
        core::printf("[Error] Failed to read shader file: %s\n", QUAD_SHADER_PATH);
        __win32_destroy_graphics(&g_context);
        g_context = {};
        return nullptr;
    }

    core::printf(
        "[System] Shader source loaded: %s (%u bytes).\n",
        QUAD_SHADER_PATH,
        (u32)shader_source.size
    );
    
    // Compile and create Vertex Shader
    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* error_blob = nullptr;
    core::printf("[System] Compiling vertex shader...\n");
    HRESULT hr_shader = D3DCompile(
        shader_source.data,
        shader_source.size,
        nullptr,
        nullptr,
        nullptr,
        "vs_main",
        "vs_5_0",
        0,
        0,
        &vs_blob,
        &error_blob
    );
    
    if (FAILED(hr_shader)) {
        if (error_blob) {
            core::printf("[Error] VS Compile Error: %s\n", (const char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        api.fs.free_file_content(shader_source);
        __win32_destroy_graphics(&g_context);
        g_context = {};
        return nullptr;
    }

    core::printf("[System] Vertex shader compiled.\n");
    
    hr = g_context.device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &g_context.vertex_shader);
    if (FAILED(hr)) {
        vs_blob->Release();
        api.fs.free_file_content(shader_source);
        __win32_destroy_graphics(&g_context);
        g_context = {};
        return nullptr;
    }

    core::printf("[System] Vertex shader created.\n");
    
    // Create Input Layout matching RenderCommandQuad layout
    D3D11_INPUT_ELEMENT_DESC layout_desc[] = {
        { "INSTANCE_POS",      0, DXGI_FORMAT_R32G32_FLOAT,       0, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_SCALE",    0, DXGI_FORMAT_R32G32_FLOAT,       0, 8,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_ORIGIN",   0, DXGI_FORMAT_R32G32_FLOAT,       0, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_UV_POS",   0, DXGI_FORMAT_R32G32_FLOAT,       0, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_UV_SZ",    0, DXGI_FORMAT_R32G32_FLOAT,       0, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_ROTATION", 0, DXGI_FORMAT_R32_FLOAT,          0, 40, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "INSTANCE_COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
    
    hr = g_context.device->CreateInputLayout(
        layout_desc,
        sizeof(layout_desc) / sizeof(layout_desc[0]),
        vs_blob->GetBufferPointer(),
        vs_blob->GetBufferSize(),
        &g_context.input_layout
    );
    vs_blob->Release();
    
    if (FAILED(hr)) {
        api.fs.free_file_content(shader_source);
        __win32_destroy_graphics(&g_context);
        g_context = {};
        return nullptr;
    }

    core::printf("[System] Input layout created.\n");
    
    // Compile and create Pixel Shader
    ID3DBlob* ps_blob = nullptr;
    core::printf("[System] Compiling pixel shader...\n");
    hr_shader = D3DCompile(
        shader_source.data,
        shader_source.size,
        nullptr,
        nullptr,
        nullptr,
        "ps_main",
        "ps_5_0",
        0,
        0,
        &ps_blob,
        &error_blob
    );
    
    if (FAILED(hr_shader)) {
        if (error_blob) {
            core::printf("[Error] PS Compile Error: %s\n", (const char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        api.fs.free_file_content(shader_source);
        __win32_destroy_graphics(&g_context);
        g_context = {};
        return nullptr;
    }

    core::printf("[System] Pixel shader compiled.\n");
    
    hr = g_context.device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &g_context.pixel_shader);
    ps_blob->Release();
    if (FAILED(hr)) {
        api.fs.free_file_content(shader_source);
        __win32_destroy_graphics(&g_context);
        g_context = {};
        return nullptr;
    }

    core::printf("[System] Pixel shader created.\n");

    api.fs.free_file_content(shader_source);
    
    // Create Instance Buffer (Dynamic Vertex Buffer)
    D3D11_BUFFER_DESC inst_desc = {};
    inst_desc.ByteWidth = MAX_QUADS * sizeof(RenderCommandQuad);
    inst_desc.Usage = D3D11_USAGE_DYNAMIC;
    inst_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    inst_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    hr = g_context.device->CreateBuffer(&inst_desc, nullptr, &g_context.instance_buffer);
    if (FAILED(hr)) return nullptr;

    core::printf("[System] Instance buffer created.\n");
    
    // Create Constant Buffer for projection matrix
    D3D11_BUFFER_DESC const_desc = {};
    const_desc.ByteWidth = 64; // float4x4 is 64 bytes
    const_desc.Usage = D3D11_USAGE_DYNAMIC;
    const_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    const_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    hr = g_context.device->CreateBuffer(&const_desc, nullptr, &g_context.constant_buffer);
    if (FAILED(hr)) return nullptr;

    core::printf("[System] Constant buffer created.\n");
    
    // Create Sampler State ( crisp Nearest-Neighbor filtering for pixel art sprites )
    D3D11_SAMPLER_DESC samp_desc = {};
    samp_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samp_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samp_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samp_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samp_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samp_desc.MinLOD = 0;
    samp_desc.MaxLOD = D3D11_FLOAT32_MAX;
    
    hr = g_context.device->CreateSamplerState(&samp_desc, &g_context.sampler_state);
    if (FAILED(hr)) return nullptr;

    core::printf("[System] Sampler state created.\n");
    
    // Create Alpha Blend State
    D3D11_BLEND_DESC blend_desc = {};
    blend_desc.AlphaToCoverageEnable = FALSE;
    blend_desc.IndependentBlendEnable = FALSE;
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    hr = g_context.device->CreateBlendState(&blend_desc, &g_context.blend_state);
    if (FAILED(hr)) return nullptr;

    core::printf("[System] Blend state created.\n");
    
    // Create Rasterizer State (Disable Backface Culling for 2D batcher)
    D3D11_RASTERIZER_DESC rast_desc = {};
    rast_desc.FillMode = D3D11_FILL_SOLID;
    rast_desc.CullMode = D3D11_CULL_NONE;
    rast_desc.DepthClipEnable = TRUE;
    
    hr = g_context.device->CreateRasterizerState(&rast_desc, &g_context.rasterizer_state);
    if (FAILED(hr)) return nullptr;

    core::printf("[System] Rasterizer state created.\n");
    
    // Create Depth Stencil State (Disable Depth Testing for 2D painters algorithm)
    D3D11_DEPTH_STENCIL_DESC depth_desc = {};
    depth_desc.DepthEnable = FALSE;
    depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depth_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    
    hr = g_context.device->CreateDepthStencilState(&depth_desc, &g_context.depth_state);
    if (FAILED(hr)) return nullptr;

    core::printf("[System] Depth state created.\n");

    core::printf("[System] Win32 renderer init complete.\n");
    
    return &g_context;
}

// Internal helper to handle resize dynamically inside clear/swap operations
static void __win32_handle_resize(PlatformWindow* window, GraphicsContext* context) {
    i32 width = *(i32*)((char*)window + sizeof(HWND));
    i32 height = *(i32*)((char*)window + sizeof(HWND) + sizeof(i32));
    
    if (width != context->cached_width || height != context->cached_height) {
        if (context->render_target_view) {
            context->render_target_view->Release();
            context->render_target_view = nullptr;
        }
        
        context->swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        
        ID3D11Texture2D* backbuffer = nullptr;
        HRESULT hr = context->swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer);
        if (SUCCEEDED(hr)) {
            context->device->CreateRenderTargetView(backbuffer, nullptr, &context->render_target_view);
            backbuffer->Release();
        }
        
        context->cached_width = width;
        context->cached_height = height;
    }
}

static void __win32_clear_screen(GraphicsContext* context, f32 r, f32 g, f32 b, f32 a) noexcept {
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

static b8 __win32_upload_texture(GraphicsContext* context, const u8* pixels, i32 width, i32 height) noexcept {
    if (!context || !context->device || !pixels || width <= 0 || height <= 0) return FALSE;
    
    // Release existing texture view if any
    if (context->texture_view) {
        context->texture_view->Release();
        context->texture_view = nullptr;
    }
    
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    
    D3D11_SUBRESOURCE_DATA subresource = {};
    subresource.pSysMem = pixels;
    subresource.SysMemPitch = width * 4;
    subresource.SysMemSlicePitch = 0;
    
    ID3D11Texture2D* texture = nullptr;
    HRESULT hr = context->device->CreateTexture2D(&desc, &subresource, &texture);
    if (FAILED(hr)) return FALSE;
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;
    
    hr = context->device->CreateShaderResourceView(texture, &srv_desc, &context->texture_view);
    texture->Release();
    
    return SUCCEEDED(hr);
}

// Generic Render Command Buffer submission, decoding, and GPU execution
static void __win32_submit_frame(PlatformWindow* window, GraphicsContext* context, RenderCommandQueue queue) noexcept {
    if (!window || !context || !context->context) return;
    
    // Automatically handle client window layout resizes
    __win32_handle_resize(window, context);
    
    g_quad_count = 0;
    
    // 1. Process CPU render command queue (immediate buffer decoder)
    usize offset = 0;
    while (offset < queue.size) {
        RenderCommandHeader* header = (RenderCommandHeader*)(queue.buffer + offset);
        
        switch (header->kind) {
            case RENDER_COMMAND_CLEAR: {
                RenderCommandClear* clear_cmd = (RenderCommandClear*)(queue.buffer + offset + sizeof(RenderCommandHeader));
                __win32_clear_screen(context, clear_cmd->r, clear_cmd->g, clear_cmd->b, clear_cmd->a);
            } break;
            
            case RENDER_COMMAND_QUAD: {
                RenderCommandQuad* quad_cmd = (RenderCommandQuad*)(queue.buffer + offset + sizeof(RenderCommandHeader));
                if (g_quad_count < MAX_QUADS) {
                    g_quad_instances[g_quad_count++] = *quad_cmd;
                }
            } break;
        }
        
        offset += header->size;
    }
    
    // 2. Execute GPU drawing calls if we have batched quads
    if (g_quad_count > 0) {
        // Map dynamic instanced data vertex buffer
        D3D11_MAPPED_SUBRESOURCE mapped_inst = {};
        HRESULT hr = context->context->Map(context->instance_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_inst);
        if (SUCCEEDED(hr)) {
            RenderCommandQuad* dest = (RenderCommandQuad*)mapped_inst.pData;
            core::memcpy(dest, g_quad_instances, g_quad_count * sizeof(RenderCommandQuad));
            context->context->Unmap(context->instance_buffer, 0);
        } else {
            return;
        }
        
        // Map dynamic orthographic projection matrix constant buffer
        f32 width = (f32)context->cached_width;
        f32 height = (f32)context->cached_height;
        f32 projection[16] = {0};
        projection[0] = 2.0f / width;
        projection[5] = -2.0f / height; // Flipped Y so Y points down!
        projection[10] = 1.0f;
        projection[12] = -1.0f;
        projection[13] = 1.0f; // Shift to top-left
        projection[15] = 1.0f;
        
        D3D11_MAPPED_SUBRESOURCE mapped_const = {};
        hr = context->context->Map(context->constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_const);
        if (SUCCEEDED(hr)) {
            f32* dest = (f32*)mapped_const.pData;
            core::memcpy(dest, projection, sizeof(projection));
            context->context->Unmap(context->constant_buffer, 0);
        } else {
            return;
        }
        
        // Bind all pipeline states
        UINT stride = sizeof(RenderCommandQuad);
        UINT offset_vb = 0;
        context->context->IASetInputLayout(context->input_layout);
        context->context->IASetVertexBuffers(0, 1, &context->instance_buffer, &stride, &offset_vb);
        context->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        context->context->VSSetShader(context->vertex_shader, nullptr, 0);
        context->context->VSSetConstantBuffers(0, 1, &context->constant_buffer);
        
        context->context->PSSetShader(context->pixel_shader, nullptr, 0);
        if (context->texture_view) {
            context->context->PSSetShaderResources(0, 1, &context->texture_view);
        }
        context->context->PSSetSamplers(0, 1, &context->sampler_state);
        
        context->context->OMSetBlendState(context->blend_state, nullptr, 0xFFFFFFFF);
        context->context->OMSetDepthStencilState(context->depth_state, 0);
        context->context->RSSetState(context->rasterizer_state);
        
        // Draw all instanced quads
        context->context->DrawInstanced(6, g_quad_count, 0, 0);
        
        // Reset batch count for the next frame
        g_quad_count = 0;
    }
    
    // 3. Present backbuffer (VSync enabled)
    context->swap_chain->Present(1, 0);
}

static void __win32_destroy_graphics(GraphicsContext* context) noexcept {
    if (context) {
        if (context->vertex_shader) {
            context->vertex_shader->Release();
            context->vertex_shader = nullptr;
        }
        if (context->pixel_shader) {
            context->pixel_shader->Release();
            context->pixel_shader = nullptr;
        }
        if (context->input_layout) {
            context->input_layout->Release();
            context->input_layout = nullptr;
        }
        if (context->instance_buffer) {
            context->instance_buffer->Release();
            context->instance_buffer = nullptr;
        }
        if (context->constant_buffer) {
            context->constant_buffer->Release();
            context->constant_buffer = nullptr;
        }
        if (context->texture_view) {
            context->texture_view->Release();
            context->texture_view = nullptr;
        }
        if (context->sampler_state) {
            context->sampler_state->Release();
            context->sampler_state = nullptr;
        }
        if (context->blend_state) {
            context->blend_state->Release();
            context->blend_state = nullptr;
        }
        if (context->rasterizer_state) {
            context->rasterizer_state->Release();
            context->rasterizer_state = nullptr;
        }
        if (context->depth_state) {
            context->depth_state->Release();
            context->depth_state = nullptr;
        }
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

#endif // OS_WINDOWS
