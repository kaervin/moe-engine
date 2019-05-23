#ifndef TERRAIN_H
#define TERRAIN_H

typedef struct Terrain {
	float spacing;
	float top, left;
	float *depth;
	int num_width, num_height;
	
	mat4 view;
	mat4 projection;
	
	v3 *render_vertices;
	unsigned int *render_indices;
	unsigned int vao, vbo, ebo, shader, view_location, proj_location;
} Terrain;

#endif /* TERRAIN_H */
#ifdef TERRAIN_IMPLEMENTATION


const char * vs_shader_terra =
"#version 330 core\n"
"layout (location = 0) in vec3 vert;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec4 out_color;\n"
"void main() {\n"
"    float color_fac = 0.1+(vert.y+20.0)/1200.0;\n"
"	out_color = vec4(0.1, 0.1, 0.1, 0.0) +  vec4(color_fac*0.7, color_fac*0.3, color_fac*0.4, 1.0);\n"
"	gl_Position = projection * view * vec4(vert.xyz, 1.0);\n"
"}\n";

const char * fs_shader_terra =
"#version 330 core\n"
"in vec4 out_color;"
"out vec4 FragColor;"
"void main() {"
"    float gamma = 2.2;"
"    FragColor = pow(out_color, vec4(gamma));"
//"	FragColor = out_color;"
"}";


Terrain terra_new(int width, int height, float spacing, float top, float left) {
	Terrain terra;
	
	terra.depth = (float*)malloc(sizeof(float)*width*height);
	terra.render_vertices = (v3*)malloc(sizeof(v3)*width*height);
	terra.render_indices = (unsigned int*)malloc(sizeof(unsigned int)*(width-1)*(height-1)*6);
	terra.num_width = width;
	terra.num_height = height;
	terra.spacing = spacing;
	terra.top = top;
	terra.left = left;
	
	
	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++) {
			terra.depth[i*width + j] = 0.0f;
			terra.render_vertices[i*width +j] = vec3(j*spacing+top, terra.depth[i*width+j], i*spacing+left);
		}
	}
	
	int next_index = 0;
	for (int i = 0; i < terra.num_height-1; i++) {
		for (int j = 0; j < terra.num_width-1; j++) {
			// A B
			// C D
			unsigned int indexA = i*terra.num_width + j;
			unsigned int indexB = i*terra.num_width + j+1;
			unsigned int indexC = (i+1)*terra.num_width + j;
			unsigned int indexD = (i+1)*terra.num_width + j+1;
			
			terra.render_indices[next_index] = indexA;
			next_index++;
			terra.render_indices[next_index] = indexD;
			next_index++;
			terra.render_indices[next_index] = indexB;
			next_index++;
			terra.render_indices[next_index] = indexA;
			next_index++;
			terra.render_indices[next_index] = indexC;
			next_index++;
			terra.render_indices[next_index] = indexD;
			next_index++;
		}
	}
	
	
	
	glGenVertexArrays(1, &terra.vao);
	glGenBuffers(1, &terra.vbo);
	glGenBuffers(1, &terra.ebo);
	
	glBindVertexArray(terra.vao);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terra.ebo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int)*(width-1)*(height-1)*6,
		terra.render_indices,
		GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, terra.vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(float)*3*width*height,
		terra.render_vertices,
		GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	
    char infoLog[1024];
	terra.shader = loadshaders(vs_shader_terra,fs_shader_terra, infoLog);
	
	terra.view_location = glGetUniformLocation(terra.shader, "view");
    terra.proj_location = glGetUniformLocation(terra.shader, "projection");
	
	return terra;
}


Terrain terra_new_from_image(const char* filename, float spacing, float top, float left) {
	Terrain terra;
	
	int width;
	int height;
	int texn;
	unsigned short* map_data = stbi_load_16(filename, &width, &height, &texn, 4);
	
	terra.num_width = width;
	terra.num_height = height;
	terra.spacing = spacing;
	terra.top = top;
	terra.left = left;
	
	
	if (map_data == NULL) {
		printf("couldn't load the heightmap");
		return terra;
	}
	
	terra.depth = (float*)malloc(sizeof(float)*width*height);
	terra.render_vertices = (v3*)malloc(sizeof(v3)*width*height);
	terra.render_indices = (unsigned int*)malloc(sizeof(unsigned int)*(width-1)*(height-1)*6);
	
	
	for(int i = 0; i < height; i++){
		for(int j = 0; j < width; j++) {
			terra.depth[i*width + j] = ((float)map_data[4*i*width + 4*j + 3])/(1<<6)-10.0f;
			terra.render_vertices[i*width +j] = vec3(j*spacing+top, terra.depth[i*width+j], i*spacing+left);
		}
	}
	
	int next_index = 0;
	for (int i = 0; i < terra.num_height-1; i++) {
		for (int j = 0; j < terra.num_width-1; j++) {
			// A B
			// C D
			unsigned int indexA = i*terra.num_width + j;
			unsigned int indexB = i*terra.num_width + j+1;
			unsigned int indexC = (i+1)*terra.num_width + j;
			unsigned int indexD = (i+1)*terra.num_width + j+1;
			
			terra.render_indices[next_index] = indexA;
			next_index++;
			terra.render_indices[next_index] = indexD;
			next_index++;
			terra.render_indices[next_index] = indexB;
			next_index++;
			terra.render_indices[next_index] = indexA;
			next_index++;
			terra.render_indices[next_index] = indexC;
			next_index++;
			terra.render_indices[next_index] = indexD;
			next_index++;
		}
	}
	
	
	
	glGenVertexArrays(1, &terra.vao);
	glGenBuffers(1, &terra.vbo);
	glGenBuffers(1, &terra.ebo);
	
	glBindVertexArray(terra.vao);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terra.ebo);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int)*(width-1)*(height-1)*6,
		terra.render_indices,
		GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, terra.vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(float)*3*width*height,
		terra.render_vertices,
		GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	
    char infoLog[1024];
	terra.shader = loadshaders(vs_shader_terra,fs_shader_terra, infoLog);
	
	terra.view_location = glGetUniformLocation(terra.shader, "view");
    terra.proj_location = glGetUniformLocation(terra.shader, "projection");
	
	return terra;
}

void terra_smooth_depths(Terrain terra) {
	
	// take 4 samples and smooth
	
	for(int i = 2; i < terra.num_height-2; i++){
		for(int j = 2; j < terra.num_width-2; j++) {
			
			int num_width = terra.num_width;
			
			int index = i*num_width + j;
			
			float dl = terra.depth[i*num_width + j-2] * 0.25f;
			float dr = terra.depth[i*num_width + j+2] * 0.25f;
			float du = terra.depth[(i-2)*num_width + j] * 0.25f;
			float dd = terra.depth[(i+2)*num_width + j] * 0.25f;
			
			float smoothed_depth = dl + dr + du + dd;
			
			terra.depth[index] = smoothed_depth*0.5f + terra.depth[index]*0.5f;
		}
	}
	
}

void terra_rebuild_vertices(Terrain terra) {
	for(int i = 0; i < terra.num_height; i++){
		for(int j = 0; j < terra.num_width; j++) {
			terra.render_vertices[i*terra.num_width +j].Y = terra.depth[i*terra.num_width+j];
		}
	}
	
	glBindVertexArray(terra.vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, terra.vbo);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(float)*3*terra.num_width*terra.num_height,
		terra.render_vertices,
		GL_STATIC_DRAW);
	
	
}

void terra_render(Terrain terra) {
	
	glUseProgram(terra.shader);
	
    glUniformMatrix4fv(terra.view_location, 1, GL_FALSE, &terra.view.X.X);
    glUniformMatrix4fv(terra.proj_location, 1, GL_FALSE, &terra.projection.X.X);
	
	glBindVertexArray(terra.vao);
	glDrawElements(GL_TRIANGLES, (terra.num_width-1)*(terra.num_height-1)*6, GL_UNSIGNED_INT, (void*)0);
	
}

// returns the depth at a specific point in the space relative to the terrains top left corner
// x and y are in world coordinates
float terra_depth_at_point(Terrain terra, float x, float y) {
	x -= terra.left;
	y -= terra.top;
	
	x = x / terra.spacing;
	y = y / terra.spacing;
	
	int ix = (int)x;
	int iy = (int)y;
	
	if (ix < 0) {
		ix = 0;
	}
	if (iy < 0) {
		iy = 0;
	}
	if (ix >= terra.num_width) {
		ix = terra.num_width-2;
	}
	if (iy >= terra.num_height) {
		iy = terra.num_width-2;
	}
	
	float interpolation_x = x - ix;
	float interpolation_y = y - iy;
	
	// A B
	// B C
	float depthA, depthB, depthC;
	depthA = terra.depth[iy*terra.num_width+ix];
	depthC = terra.depth[(iy+1)*terra.num_width+(ix+1)];
	
	if (interpolation_x > interpolation_y) {
		// A B
		//   C
		depthB = terra.depth[iy*terra.num_width+(ix+1)];
		
		float ABlerp = (depthB-depthA)*interpolation_x;
		float BClerp = (depthC-depthB)*interpolation_y;
		
		return depthA+ABlerp+BClerp;
		
	} else {
		// A
		// B C
		depthB = terra.depth[(iy+1)*terra.num_width+ix];
		float ABlerp = (depthB-depthA)*interpolation_y;
		float BClerp = (depthC-depthB)*interpolation_x;
		
		return depthA+ABlerp+BClerp;
	}
}

// returns the depth at a specific point in the space relative to the terrains top left corner
// x and y are in world coordinates
// also stores normal of the hit, TODO: store normal of the hit and not just the upwards vector
float terra_depth_normal_at_point(Terrain terra, float x, float y, v3 *normal) {
	*normal = vec3(0.0f, 1.0f, 0.0f);
	
	x -= terra.left;
	y -= terra.top;
	
	x = x / terra.spacing;
	y = y / terra.spacing;
	
	int ix = (int)x;
	int iy = (int)y;
	
	if (ix < 0) {
		ix = 0;
	}
	if (iy < 0) {
		iy = 0;
	}
	if (ix >= terra.num_width) {
		ix = terra.num_width-2;
	}
	if (iy >= terra.num_height) {
		iy = terra.num_width-2;
	}
	
	float interpolation_x = x - ix;
	float interpolation_y = y - iy;
	
	// A B
	// B C
	float depthA, depthB, depthC;
	depthA = terra.depth[iy*terra.num_width+ix];
	depthC = terra.depth[(iy+1)*terra.num_width+(ix+1)];
	
	if (interpolation_x > interpolation_y) {
		// A B
		//   C
		depthB = terra.depth[iy*terra.num_width+(ix+1)];
		
		float ABlerp = (depthB-depthA)*interpolation_x;
		float BClerp = (depthC-depthB)*interpolation_y;
		
		return depthA+ABlerp+BClerp;
		
	} else {
		// A
		// B C
		depthB = terra.depth[(iy+1)*terra.num_width+ix];
		float ABlerp = (depthB-depthA)*interpolation_y;
		float BClerp = (depthC-depthB)*interpolation_x;
		
		return depthA+ABlerp+BClerp;
	}
}

#endif 