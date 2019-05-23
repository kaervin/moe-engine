#version 420 core
#extension GL_ARB_shader_draw_parameters : require
#extension GL_ARB_shader_storage_buffer_object : enable

struct SSBO_Data {
	vec4 position;
	vec4 transform;
	vec4 scale;
	int texture_type;
	int texture_nr;
};

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (std430, binding = 3) buffer ssbo_pos_buffer
{
	SSBO_Data ssbo_data[];
};

out vec3 normal;
out vec2 TexCoord;
out float lambert;
out flat int texture_type;
out flat int texture_nr;
out vec3 fragment_world_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 player_pos;

uniform int newBaseInstance;

void main() {

	vec3 light = normalize(vec3(-1.0,0.2,1.0));

	vec3 scaled_pos = aPos * ssbo_data[newBaseInstance + gl_InstanceID].scale.xyz;

	vec4 p = ssbo_data[newBaseInstance + gl_InstanceID].position;

	vec4 q = ssbo_data[newBaseInstance + gl_InstanceID].transform;

	vec3 t = 2 * cross(q.xyz, scaled_pos);
	vec3 nv = scaled_pos + q.w * t + cross(q.xyz, t);
	vec4 post = p + vec4(nv, 1.0);

	gl_Position = projection * view * post;
	
	TexCoord = aTexCoord;


	vec3 t_normal = 2 * cross(q.xyz, aNormal);
	vec3 nv_normal = aNormal + q.w * t_normal + cross(q.xyz, t_normal);

	normal = normalize(nv_normal);
	lambert = max(dot(light,normal), 0.0);

	texture_nr = ssbo_data[newBaseInstance + gl_InstanceID].texture_nr;
	texture_type = ssbo_data[newBaseInstance + gl_InstanceID].texture_type;

	fragment_world_pos = vec3(post);
}
