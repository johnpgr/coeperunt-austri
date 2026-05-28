cbuffer FrameConstants : register(b0) {
    row_major float4x4 projection;
};

struct VSInput {
    float2 pos       : INSTANCE_POS;
    float2 scale     : INSTANCE_SCALE;
    float2 origin    : INSTANCE_ORIGIN;
    float2 uv_pos    : INSTANCE_UV_POS;
    float2 uv_size   : INSTANCE_UV_SZ;
    float  rotation  : INSTANCE_ROTATION;
    float4 color     : INSTANCE_COLOR;
};

struct PSInput {
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
    float4 color    : COLOR0;
};

Texture2D main_texture : register(t0);
SamplerState main_sampler : register(s0);

PSInput vs_main(VSInput input, uint vertex_id : SV_VertexID) {
    PSInput output;
    static const float2 quad_vertices[6] = {
        float2(0.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 0.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(1.0f, 1.0f)
    };
    float2 local_pos = quad_vertices[vertex_id];
    float2 local_pivot = local_pos - input.origin;
    float2 local_scaled = local_pivot * input.scale;
    float cos_r = cos(input.rotation);
    float sin_r = sin(input.rotation);
    float2 local_rotated;
    local_rotated.x = local_scaled.x * cos_r - local_scaled.y * sin_r;
    local_rotated.y = local_scaled.x * sin_r + local_scaled.y * cos_r;
    float2 world_pos = input.pos + local_rotated;
    output.position = mul(float4(world_pos, 0.0f, 1.0f), projection);
    output.uv = input.uv_pos + local_pos * input.uv_size;
    output.color = input.color;
    return output;
}

float4 ps_main(PSInput input) : SV_Target {
    float4 tex_color = main_texture.Sample(main_sampler, input.uv);
    if (tex_color.a == 0.0f) {
        discard;
    }
    return tex_color * input.color;
}
