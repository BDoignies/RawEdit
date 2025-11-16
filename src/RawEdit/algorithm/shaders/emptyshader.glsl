#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba16f, binding = 0) uniform readonly  image2D inTex;
layout(rgba16f, binding = 1) uniform writeonly image2D outTex;
uniform float exposure;

void main() 
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    vec4 inColor = imageLoad(inTex, pos);
    vec4 outColor = clamp(vec4(inColor.xyz * exposure, 1.f), 0.f, 1.f);
    imageStore(outTex, pos, outColor);
}
