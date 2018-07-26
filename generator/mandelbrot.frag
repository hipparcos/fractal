#version 330 core

uniform vec2 u_window_size;
uniform vec2 u_center;
uniform float u_dpp;
uniform int u_max_iter;

/* mandelbrot can't use vec2 directly because of precision problem. */
int mandelbrot(in vec2 lc, in int max_iter) {
    int iter = 0;
    vec2 z = vec2(0.0);
    for (iter = 0; iter < max_iter; iter++) {
        float x2 = z.x * z.x;
        float y2 = z.y * z.y;
        float xy = z.x * z.y;
        float x = x2 - y2 + lc.x;
        float y = 2 * xy + lc.y;
        if (x * x + y * y > 4.0) {
            break;
        }
        z.x = x;
        z.y = y;
    }
    return iter;
}

vec3 color(in int iter, in int max_iter) {
    if (iter >= max_iter) {
        iter = 0;
    }
    float ratio = float(iter) / float(max_iter);
    return vec3(ratio, ratio, ratio);
}

vec2 global_to_local(in vec2 px) {
    return u_center + (px - u_window_size * 0.5) * u_dpp;
}

void main() {
    vec2 px = gl_FragCoord.xy;
    vec2 lc = global_to_local(px);
    int iter = mandelbrot(lc, u_max_iter);
    vec3 color = color(iter, u_max_iter);
    gl_FragColor = vec4(color, 1.0);
}
