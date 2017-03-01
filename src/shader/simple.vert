#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

layout (binding = 1) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix;
	mat4 viewMatrix;
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(void)
{
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * position;
}

