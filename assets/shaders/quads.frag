in vec2 frag_uv;
in vec4 frag_color;

out vec4 out_color;

uniform sampler2D main_texture;

void main() {
    vec4 tex_color = texture(main_texture, frag_uv);
    if (tex_color.a == 0.0) {
        discard;
    }
    out_color = tex_color * frag_color;
}
