#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform RaymarchUBO
{
    int width, height;
    float cameraNear, cameraFar;
    vec3 cameraPosition;
    mat4 cameraRotation;
} ubo;

float sdSphere(vec3 p, float s)
{
    return length(p) - s;
}

float sdBox(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float smin(float a, float b, float k)
{
    float h = max(k - abs(a-b) , 0.0) / k;
    return min(a, b) - h * h * h * k * (1.0 / 6.0);
}

float sdMandelBlub(vec3 p)
{
    const float power = 2.0;

    vec3 z = p;
    float dr = 1;
    float r;
    for(int i = 0; i < 15; i++)
    {
        r = length(z);
        if(r > 2)
            break;

        float theta = acos(z.z / r) * power;
        float phi = atan(z.y, z.x) * power;
        float zr = pow(r, power);
        dr = pow(r, power - 1) * power * dr + 1;

        z = zr * vec3(sin(theta) * cos(phi), sin(phi) * cos(theta), cos(theta));
        z += p;
    }
    return 0.5 * log(r) * r / dr;
}



float map(vec3 p)
{

   // p = fract(p) - 0.5;

    //vec3 boxPos = vec3(5, 0, 1);
    //float box = sdBox(p, vec3(0.1));

    float mandelBlub = sdMandelBlub(p - vec3(5, 0, 0));
    return mandelBlub;
}

vec3 palete(float t)
{
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.263, 0.416, 0.557);


    return a + b * cos(6.28318 * (c * t + d));
}

void main() { 
    vec2 resolution = vec2(ubo.width, ubo.height);
    vec2 uv = (gl_FragCoord.xy * 2.0 - resolution) / resolution.y;

    vec3 rayOrigin = vec3(ubo.cameraPosition.x, ubo.cameraPosition.y, ubo.cameraPosition.z);
    vec3 rayDirection = normalize(ubo.cameraRotation * vec4(vec3(uv.x, -uv.y, -1), 0)).xyz;
    vec3 finalColor = vec3(0);

    float t = 0.0;

    for(int i = 0; i < 180; i++)
    {
        vec3 p = rayOrigin + rayDirection * t;

        p.y += sin(i) * 0.35;
        p.x += cos(i) * 0.35;

        float d = map(p);
        t += d;

        if(d < 0.001) break;
        if(t > ubo.cameraFar) break;
    }

    finalColor = palete(t * 0.04);
    outColor = vec4(finalColor, 1);

    float cameraRange = ubo.cameraFar - ubo.cameraNear;
    gl_FragDepth = 0.999999; //t / cameraRange;
}   