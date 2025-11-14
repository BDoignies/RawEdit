layout(r32f, binding = 0) uniform image2D outTex;

void main() 
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    imageStore(outTex, pos, vec4(0, 0, 0, 0));
}
