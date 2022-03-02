#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(binding = 2) uniform sampler texSampler; 
layout(binding = 3) uniform texture2D textures[126];

layout(push_constant) uniform PER_OBJECT {
	int imageIndex;
} pushConstant;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(sampler2D(textures[pushConstant.imageIndex], texSampler), fragTexCoord);
}
