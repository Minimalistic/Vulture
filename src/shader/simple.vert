#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 in_color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texcoord;

layout (location = 0) out vec4 out_color;

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	mat4 modelMatrix[INSTANCE_COUNT];
	mat4 viewMatrix;
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(void)
{
	out_color = in_color;
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix[gl_InstanceIndex] * position;
}

