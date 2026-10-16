#version 330 core
uniform sampler2D uSky;
uniform mat4      uInvViewProj;
uniform vec3      uCamPos;
uniform float     uExposure;
out vec4 fragColor;

const float PI = 3.14159265359;

void main() {
    vec2 ndc = gl_FragCoord.xy * 2.0 - 1.0;
    vec4 nearH = uInvViewProj * vec4(ndc, -1.0, 1.0);
    vec4 farH  = uInvViewProj * vec4(ndc,  1.0, 1.0);
    vec3 nearP = nearH.xyz / nearH.w;
    vec3 farP  = farH.xyz  / farH.w;
    vec3 dir   = normalize(farP - nearP);

    float u = atan(dir.z, dir.x) / (2.0 * PI) + 0.5;

    // V: +Y direction -> top of HDR (v=0). stb loads rows top-down, GL
    // treats row 0 as v=0, so this is correct without a load-time flip.
    float v = 0.5 - asin(clamp(dir.y, -1.0, 1.0)) / PI;

    vec3 hdr = texture(uSky, vec2(u, v)).rgb;
    fragColor = vec4(hdr * uExposure, 1.0);
}
