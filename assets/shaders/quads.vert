layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_scale;
layout(location = 2) in vec2 in_origin;
layout(location = 3) in vec2 in_uv_pos;
layout(location = 4) in vec2 in_uv_size;
layout(location = 5) in float in_rotation;
layout(location = 6) in vec4 in_color;

out vec2 frag_uv;
out vec4 frag_color;

uniform mat4 projection;

void main() {
    const vec2 quad_vertices[6] = vec2[](
        vec2(0.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );
    
    vec2 local_pos = quad_vertices[gl_VertexID];
    vec2 local_pivot = local_pos - in_origin;
    vec2 local_scaled = local_pivot * in_scale;
    
    float cos_r = cos(in_rotation);
    float sin_r = sin(in_rotation);
    vec2 local_rotated;
    local_rotated.x = local_scaled.x * cos_r - local_scaled.y * sin_r;
    local_rotated.y = local_scaled.x * sin_r + local_scaled.y * cos_r;
    
    vec2 world_pos = in_pos + local_rotated;
    gl_Position = projection * vec4(world_pos, 0.0, 1.0);
    
    frag_uv = in_uv_pos + local_pos * in_uv_size;
    frag_color = in_color;
}
