#version 330 core

uniform vec2 u_window_size;
uniform vec2 u_center;
uniform vec2 u_j;
uniform int u_generator;
uniform float u_dpp;
uniform int u_max_iter;
uniform float u_t;
uniform float u_dt;
uniform int u_n;

float pi = 3.1415927;

/* julia can't use vec2 directly because of precision problem. */
int julia(in vec2 init, in vec2 c, in int max_iter) {
    int iter = 0;
    vec2 z = init;
    for (iter = 0; iter < max_iter; iter++) {
        float x2 = z.x * z.x;
        float y2 = z.y * z.y;
        float xy = z.x * z.y;
        float x = x2 - y2 + c.x;
        float y = 2 * xy + c.y;
        if (x * x + y * y > 4.0) {
            break;
        }
        z.x = x;
        z.y = y;
    }
    return iter;
}

int juliams(in vec2 init, in vec2 c, in int n, in int max_iter) {
    int iter = 0;
    vec2 z = init;
    for (iter = 0; iter < max_iter; iter++) {
        float x2 = z.x * z.x;
        float y2 = z.y * z.y;
        float at = atan(z.y, z.x);
        float cs = cos(n * at);
        float ss = sin(n * at);
        float p2 = pow(x2 + y2, n / 2);
        float x = p2 * cs + c.x;
        float y = p2 * ss + c.y;
        if (x * x + y * y > 4.0) {
            break;
        }
        z.x = x;
        z.y = y;
    }
    return iter;
}

/* mandelbrot can't use vec2 directly because of precision problem. */
int mandelbrot(in vec2 lc, in int max_iter) {
    return julia(vec2(0.0), lc, max_iter);
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
    lc.y *= -1; // to get the same fractal then software renderer.
    int iter = 0;
    if (u_generator == 1) {
        iter = julia(lc, u_j, u_max_iter);
    } else if (u_generator == 2) {
        vec2 t = vec2(u_j.x * cos(u_t / (2 * pi)), u_j.y * sin(u_t / (2 * pi)));
        iter = juliams(lc, t, 2, u_max_iter);
    } else {
        iter = mandelbrot(lc, u_max_iter);
    }
    vec3 color = color(iter, u_max_iter);
    gl_FragColor = vec4(color, 1.0);
}
