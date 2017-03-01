#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) out vec4 out_Color;

void main()
{
	out_Color = vec4(1.0, 1.0, 1.0, 1.0);
}