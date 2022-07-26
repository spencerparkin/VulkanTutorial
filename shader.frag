#version 450

// The prefix here is how we specify the index of the framebuffer.
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main()
{
    //outColor = vec4(fragColor, 1.0);
    outColor = texture(texSampler, fragTexCoord);
}