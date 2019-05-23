#version 420 core
#extension GL_ARB_shader_draw_parameters : enable
#extension GL_ARB_shader_storage_buffer_object : enable

struct SSBO_Data_Skin {
	vec4 position;
	vec4 transform;
	int root_joint;
	int texture_type;
	int texture_nr;
	int pad1;
};

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 joints;
layout (location = 4) in vec4 weights;

layout (std430, binding = 6) buffer ssbo_pos_buffer
{
	SSBO_Data_Skin ssbo_data_skin[];
};

layout (std430, binding = 7) buffer ssbo_joint_buffer
{
	mat4 joint_mat[];
};

out vec3 normal;
out vec2 TexCoord;
out float lambert;
out flat int texture_type;
out flat int texture_nr;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int newBaseInstance;

void main() {

	vec3 light = normalize(vec3(-1.0,0.2,1.0));

	vec4 p = ssbo_data_skin[newBaseInstance + gl_InstanceID].position;
	//p = vec4(5.0, 0.0, 0.0, 0.0);


	mat4 testview = mat4(
		vec4(1.0, 0.0, 0.0, 0.0),
		vec4(0.0, 1.0, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(-1.0, -1.0, -10.0, 1.0));

	vec4 q = ssbo_data_skin[newBaseInstance + gl_InstanceID].transform;

	int root = ssbo_data_skin[newBaseInstance + gl_InstanceID].root_joint;

    mat4 skinMat =
        weights.x * joint_mat[root+uint(joints.x)] +
        weights.y * joint_mat[root+uint(joints.y)] +
        weights.z * joint_mat[root+uint(joints.z)] +
        weights.w * joint_mat[root+uint(joints.w)];

    vec4 npos =  skinMat * (vec4(aPos, 1.0));
	
	//npos = vec4(npos.xyz, 1.0);

	vec4 viewv = vec4(5.0, 3.0, 20.0, 0.0);

	gl_Position = projection * ((view *( npos )));

	TexCoord = aTexCoord;
	normal = normalize(skinMat * vec4(aNormal,0.0)).xyz;
	lambert = max(dot(light,normal), 0.0);

	texture_nr = ssbo_data_skin[newBaseInstance + gl_InstanceID].texture_nr;
	texture_type = ssbo_data_skin[newBaseInstance + gl_InstanceID].texture_type;
}
