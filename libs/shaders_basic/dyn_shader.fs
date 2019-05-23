#version 420 core

in vec3 normal;
in vec2 TexCoord;
in float lambert;
in flat int texture_type;
in flat int texture_nr;

uniform sampler2DArray texture_arrays[7];

uniform sampler2DArray tex_256;

out vec4 FragColor;

void main() {

	float gamma = 2.2;	

	vec4 tex_color = texture(texture_arrays[texture_type], vec3(TexCoord, texture_nr));
	
	tex_color = pow(tex_color, vec4(gamma));

	vec4 color = tex_color * 0.8;
	color = lambert*color*0.2  + color;
	
	FragColor = color;

	FragColor = vec4(color.xyz, tex_color.a);
}
