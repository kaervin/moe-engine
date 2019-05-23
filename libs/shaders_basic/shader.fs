#version 420 core

in vec3 normal;
in vec2 TexCoord;
in float lambert;
in flat int texture_type;
in flat int texture_nr;
in vec3 fragment_world_pos;

uniform sampler2DArray texture_arrays[7];

uniform sampler2DArray tex_256;

uniform vec3 player_pos;

out vec4 FragColor;

void main() {
	//FragColor = texture(tex, vec3(TexCoord, 1.0));
	//FragColor = vec4( (normal+1.0)/2 ,1.0);
	vec3 ambient = vec3(0.0,0.0,0.0);


	float gamma = 2.2;
	vec4 tex_color = texture(texture_arrays[texture_type], vec3(TexCoord, texture_nr));
	tex_color = pow(tex_color, vec4(gamma));

	vec4 color = tex_color * 0.7;
	color = lambert*color*0.4 + color;

	// compute the distance between the player pos and the fragment coord
	//FragColor = color;
	FragColor = vec4(color.xyz, tex_color.a);

	vec3 cp = fragment_world_pos - player_pos;
	float cpls = dot(cp, cp);
	
	if (cpls < 2.0 && fragment_world_pos.y < player_pos.y+2.5) {
		
		float fac = 0.7 + (cpls/2.0) *0.3;
		
vec3 shadow_color = color.xyz*fac;
		FragColor = vec4(shadow_color, tex_color.a);
	}

}
