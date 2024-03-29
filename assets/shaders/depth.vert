#version 460

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 LSNormal;
layout (location = 2) in vec3 VIColor;
layout (location = 3) in vec2 TexCoords;

layout (std140, set = 0, binding = 0) uniform UBO
{
    mat4 DepthVP;
} u_ubo;

layout (std140, set = 1, binding = 0) uniform Model
{
    mat4 Mat;
} u_model;

void main()
{
    gl_Position = u_ubo.DepthVP * u_model.Mat * vec4(a_Position, 1.f);
}