#version 450

const vec3 gLightDirection = vec3(-1, -1, -1);

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform RaymarchUBO
{
    int width, height;
    float cameraNear, cameraFar;
    vec3 cameraPosition;
    float time;
    mat4 cameraRotation;
} ubo;

float randNoise(vec3 co){
    return fract(sin(dot(co, vec3(12.9898, 78.233, 42.9962))) * 43758.5453);
}

float randNoise2(vec3 p) 
{
    vec3 u = floor(p);
    vec3 v = fract(p);
    vec3 s = smoothstep(0.0, 1.0, v);
    
    float a = randNoise(u);
    float b = randNoise(u + vec3(1.0, 0.0, 0.0));
    float c = randNoise(u + vec3(0.0, 1.0, 0.0));
    float d = randNoise(u + vec3(1.0, 1.0, 0.0));
    float e = randNoise(u + vec3(0.0, 0.0, 1.0));
    float f = randNoise(u + vec3(1.0, 0.0, 1.0));
    float g = randNoise(u + vec3(0.0, 1.0, 1.0));
    float h = randNoise(u + vec3(1.0, 1.0, 1.0));
    
    return mix(mix(mix(a, b, s.x), mix(c, d, s.x), s.y),
               mix(mix(e, f, s.x), mix(g, h, s.x), s.y),
               s.z);
}


float fbm(vec3 p) {
    vec3 q = p + ubo.time * 0.01 * vec3(1.0, -0.2, -1.0);
    float g = randNoise2(q);

    float f = 0.0;
    float scale = 1.1;
    float factor = 2.02;

    for (int i = 0; i < 6; i++) {
        f += scale * randNoise2(q);
        q *= factor;
        factor += 0.21;
        scale *= 0.5;
    }

    return f;
}



float sdSphere(vec3 p, float radius)
{
    return length(p) - radius;
}

float sdBox( vec3 p, vec3 b )
{
    vec3 q = abs(p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}


float scene(vec3 p)
{
    //p.xz = fract(p.xz) - 0.5;
    p.y -= 2.0;
    float box = sdBox(p, vec3(10.0, 0.5, 10.0));
    float noise = fbm(p);

    return -box + noise;
}


vec3 getNormal(vec3 p) {
  vec2 e = vec2(.01, 0);

  vec3 n = scene(p) - vec3(
    scene(p-e.xyy),
    scene(p-e.yxy),
    scene(p-e.yyx));

  return normalize(n);
}


void rayMarch(vec3 ro, vec3 rd, out vec4 color)
{
    const int marchSteps = 5;
    const float stepSize = 0.8;
    float t = 0.0;
    vec3 p = ro + rd * t;
    vec4 res = vec4(0.0);
    for(int i = 0; i < marchSteps; i++)
    {
        float d = scene(p);

        if(d > 0.0)
        {
            vec3 normal = getNormal(p);

            float diffuse = clamp((scene(p) - scene(p + 0.3 * -gLightDirection)) / 0.3, 0.0, 1.0 );
            vec3 lin = vec3(0.60,0.60,0.75) * 1.1 + 0.8 * vec3(1.0,0.6,0.3) * diffuse;
            vec4 mixColor = vec4(mix(vec3(1.0, 1.0, 1.0), vec3(0.0, 0.0, 0.0), d), d );
            mixColor.rgb *= lin;

            mixColor.rgb *= mixColor.a;
            res += mixColor * (1.0 - res.a);
        }

        t += stepSize;
        p = ro + rd * t;
    }

    color = res;
}

void main() { 
    vec2 resolution = vec2(ubo.width, ubo.height);
    vec2 uv = (gl_FragCoord.xy * 2.0 - resolution) / resolution.y;

    vec3 rayOrigin = vec3(0, 0, 0);
    vec3 rayDirection = normalize(ubo.cameraRotation * vec4(vec3(uv.x, -uv.y, -1), 0)).xyz;

    const vec4 baseColor = vec4(0.18, 0.27, 0.47, 1.0);
    float red = -0.22 * rayDirection.y + baseColor.x; 
    float green = -0.25 * rayDirection.y + baseColor.y; 
    float blue = -0.19 * rayDirection.y + baseColor.z; 
    vec4 skyColor = vec4(red, green, blue, 1);
    vec4 cloudColor = vec4(0);
    rayMarch(rayOrigin, rayDirection, cloudColor);
    outColor = cloudColor + skyColor;

    

    float cameraRange = ubo.cameraFar - ubo.cameraNear;
    gl_FragDepth = 0.0;
}   