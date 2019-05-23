#include <stdio.h>

// opengl has to be initialized before using this function
#include "GL/gl3w.h"

#include "typos.h"

#define CUTE_SPHEREMESH_IMPLEMENTATION
#include "cute_spheremesh.h"

#include "HandmadeMath.h"
#include <float.h>

#ifndef MUSHISHI_H
#define MUSHISHI_H

typedef struct
{
	v3 position;
	v4 color;
} mush_vertex;

typedef struct
{
    v3 center;
    float radius;
	
    v4 color;
	
} mush_sphere;

typedef struct
{
    mush_vertex p;
    mush_vertex q;
	
} mush_segment;

typedef struct
{
    mush_vertex a;
    mush_vertex b;
    mush_vertex c;
	
} mush_triangle;

// the user has to allocate the arrays himself
typedef struct
{
    mush_sphere *spheres;
    int max_spheres;
    int num_spheres;
	
    mush_segment *segments;
    int max_segments;
    int num_segments;
	
    mush_triangle *triangles;
    int max_triangles;
    int num_triangles;
	
} mush_draw_list;

typedef struct
{
    unsigned int vbo_sphere, vao_sphere;
    unsigned int pos_location_sphere, view_location_sphere, proj_location_sphere, radius_location_sphere, color_location_sphere;
	
    unsigned int vbo_segment, vao_segment; 
    unsigned int view_location_segment, proj_location_segment;
	
    unsigned int vbo_triangle, vao_triangle; 
    // can just reuse segment view and proj location, since they share the same shader
	
    unsigned int shader_prog_sphere;
    unsigned int shader_prog_segment;
	
    int sphere_verts_count;
	
    mat4 view;
    mat4 projection;
} mush_context;

// these functions draw the thing next frame
int mush_draw_sphere(
mush_draw_list *list,
v3 center,
float radius,
v4 color);

int mush_draw_segment(
mush_draw_list *list,
v3 p,
v3 q,
v4 p_color,
v4 q_color);

int mush_draw_triangle(
mush_draw_list *list,
v3 a,
v3 b,
v3 c,
v4 color);

int mush_draw_cube(
mush_draw_list *list,
v3 cube_mid,
v3 cube_ex,
Quat cube_rot,
v4 p_color);

void mush_empty_list(mush_draw_list *list);

void mush_alloc_forms(mush_draw_list *list, int n);

#endif /* MUSHISHI_H */


// ----- implementation -----


#ifdef MUSHISHI_IMPLEMENTATION


void mush_alloc_forms(mush_draw_list *list, int n) {
	
	list->spheres = (mush_sphere*)malloc(sizeof(mush_sphere) * n);
	list->max_spheres = n;
	list->num_spheres = 0;
	
	list->segments = (mush_segment*)malloc(sizeof(mush_segment) * n);;
	list->max_segments = n;
	list->num_segments = 0;
	
	list->triangles = (mush_triangle*)malloc(sizeof(mush_triangle) * n);;
	list->max_triangles = n;
	list->num_triangles = 0;
	
};


int mush_draw_sphere(mush_draw_list *list, v3 center, float radius, v4 color)
{
	
    if ( (*list).num_spheres == (*list).max_spheres ) {
        return 0;
    }
	
    mush_sphere sph;
    sph.center = center;
    sph.radius = radius;
    sph.color = color;
    (*list).spheres[(*list).num_spheres] = sph;
	
    (*list).num_spheres++;
    return 1;
}

int mush_draw_segment(
mush_draw_list *list,
v3 p,
v3 q,
v4 p_color,
v4 q_color)
{
    if ( (*list).num_segments == (*list).max_segments ) {
        return 0;
    }
	
    mush_segment seg;
    seg.p.position = p;
    seg.p.color = p_color;
	
    seg.q.position = q;
    seg.q.color = q_color;
    (*list).segments[(*list).num_segments] = seg;
	
    (*list).num_segments++;
    return 1;
}

int mush_draw_triangle(
mush_draw_list *list,
v3 a,
v3 b,
v3 c,
v4 color)
{
    if ( (*list).num_triangles == (*list).max_triangles ) {
        return 0;
    }
	
    mush_triangle tri;
    tri.a.position = a;
    tri.b.position = b;
    tri.c.position = c;
	
	tri.a.color = color;
    tri.b.color = color;
    tri.c.color = color;
	
    (*list).triangles[(*list).num_triangles] = tri;
	
    (*list).num_triangles++;
    return 1;
}

int mush_draw_cube(
mush_draw_list *list,
v3 cube_mid,
v3 cube_ex,
Quat cube_rot,
v4 p_color)
{
	
    v3 ex_pos = add_v3(cube_mid, cube_ex);
    v3 ex_neg = sub_v3(cube_mid, cube_ex);
	
	
    // right side points
    v3 right_top_front    = vec3(1.0, 1.0, 1.0);
    v3 right_top_back     = vec3(1.0, 1.0, -1.0);
    v3 right_bottom_front = vec3(1.0, -1.0, 1.0);
    v3 right_bottom_back  = vec3(1.0, -1.0, -1.0);
	
    // left side points
    v3 left_top_front    = vec3(-1.0, 1.0, 1.0);
    v3 left_top_back     = vec3(-1.0, 1.0, -1.0);
    v3 left_bottom_front = vec3(-1.0, -1.0, 1.0);
    v3 left_bottom_back  = vec3(-1.0, -1.0, -1.0);
	
	
    v3 p2s[12];
    v3 q2s[12];
	
    p2s[0] = right_top_front;
    q2s[0] = left_top_front;
	
    p2s[1] = right_top_back;
    q2s[1] = left_top_back;
	
    p2s[2] = right_bottom_front;
    q2s[2] = left_bottom_front;
	
    p2s[3] = right_bottom_back;
    q2s[3] = left_bottom_back;
	
	
    p2s[4] = right_top_front;
    q2s[4] = right_top_back;
	
    p2s[5] = right_bottom_front;
    q2s[5] = right_bottom_back;
	
    p2s[6] = right_top_front;
    q2s[6] = right_bottom_front;
	
    p2s[7] = right_top_back;
    q2s[7] = right_bottom_back;
	
	
    p2s[8] = left_top_front;
    q2s[8] = left_top_back;
	
    p2s[9] = left_bottom_front;
    q2s[9] = left_bottom_back;
	
    p2s[10] = left_top_front;
    q2s[10] = left_bottom_front;
	
    p2s[11] = left_top_back;
    q2s[11] = left_bottom_back;
	
    for (int i = 0; i < 12; ++i)
    {
        v3 tp = p2s[i];
        tp = mul_v3(cube_ex, tp);
        tp = rotate_vec3_quat(tp, cube_rot);
        tp = add_v3(cube_mid, tp);
		
        v3 tq = q2s[i];
        tq = mul_v3(cube_ex, tq);
        tq = rotate_vec3_quat(tq, cube_rot);
        tq = add_v3(cube_mid, tq);
		
        int suc = mush_draw_segment(list, tp, tq, p_color, p_color);
        if (suc == 0) {
            return 0;
        }
    }
	
    return 1;
}

const char * vs_shader_line =
"#version 330 core\n"
"layout (location = 0) in vec4 vert;\n"
"layout (location = 1) in vec4 color;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec4 out_color;\n"
"void main() {\n"
"	out_color = color;\n"
"	gl_Position = projection * view * vec4(vert.xyz, 1.0);\n"
"}\n";

const char * fs_shader_line =
"#version 330 core\n"
"in vec4 out_color;"
"out vec4 FragColor;"
"void main() {"
"	FragColor = out_color;"
"}";

const char * vs_shader_sphere =
"#version 330 core\n"
"layout (location = 0) in vec3 vert;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform vec3 position;\n"
"uniform float radius;\n"
"void main() {\n"
"	vec3 postpos = position + (radius * vert);\n"
"	gl_Position = projection * view * vec4(postpos, 1.0);\n"
"}\n";

const char * fs_shader_sphere =
"#version 330 core\n"
"uniform vec4 color;\n"
"out vec4 FragColor;"
"void main() {"
"	FragColor = color;"
"}";

mush_context mush_init() {
	
    mush_context ctx;
	
    // init sphere vbo
	
    // sphere stuff 
    int num_subdivisions = 2;
    int sphere_bytes_scratch = spheremesh_bytes_required3(num_subdivisions);
    void* sphere_scratch = malloc(sphere_bytes_scratch);
    int sphere_verts_count;
    float* sphere_floats = spheremesh_generate_verts3(
        sphere_scratch,
        num_subdivisions,
        &sphere_verts_count);
	
    free(sphere_scratch);
	
    glGenVertexArrays(1, &ctx.vao_sphere);
    glGenBuffers(1, &ctx.vbo_sphere);
	
    glBindVertexArray(ctx.vao_sphere);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.vbo_sphere);
	
    glBufferData(
        GL_ARRAY_BUFFER,
        (sizeof(float) * 3 * sphere_verts_count),
        sphere_floats,
        GL_STATIC_DRAW );
	
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glEnableVertexAttribArray(0);
	
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
    char infoLog[512];
	
    ctx.shader_prog_sphere = loadshaders(vs_shader_sphere,fs_shader_sphere, infoLog);
	
    // --- segment stuff ---
	
    glGenVertexArrays(1, &ctx.vao_segment);
    glGenBuffers(1, &ctx.vbo_segment);
	
    glBindVertexArray(ctx.vao_segment);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.vbo_segment);
	
    // positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mush_vertex), (void*)0);
    glEnableVertexAttribArray(0);
	
    // colors
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(mush_vertex), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(1);
	
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
    // --- triangle stuff ---
	
    glGenVertexArrays(1, &ctx.vao_triangle);
    glGenBuffers(1, &ctx.vbo_triangle);
	
    glBindVertexArray(ctx.vao_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.vbo_triangle);
	
    // positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mush_vertex), (void*)0);
    glEnableVertexAttribArray(0);
	
    // colors
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(mush_vertex), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(1);
	
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
	
    ctx.shader_prog_segment = loadshaders(vs_shader_line,fs_shader_line, infoLog);
	
    // --- ---
	
    ctx.sphere_verts_count = sphere_verts_count;
	
    ctx.pos_location_sphere = glGetUniformLocation(ctx.shader_prog_sphere, "position");
    ctx.view_location_sphere = glGetUniformLocation(ctx.shader_prog_sphere, "view");
    ctx.proj_location_sphere = glGetUniformLocation(ctx.shader_prog_sphere, "projection");
    ctx.radius_location_sphere = glGetUniformLocation(ctx.shader_prog_sphere, "radius");
    ctx.color_location_sphere = glGetUniformLocation(ctx.shader_prog_sphere, "color");
	
    ctx.view_location_segment = glGetUniformLocation(ctx.shader_prog_segment, "view");
    ctx.proj_location_segment = glGetUniformLocation(ctx.shader_prog_segment, "projection");
	
    return ctx;
}

void mush_empty_list(mush_draw_list *list) {
    (*list).num_spheres = 0;
    (*list).num_segments = 0;
    (*list).num_triangles = 0;
}

void mush_render(mush_context dbg_ctx, mush_draw_list *list)
{
	
    glUseProgram(dbg_ctx.shader_prog_sphere);
	
    glUniformMatrix4fv(dbg_ctx.view_location_sphere, 1, GL_FALSE, &dbg_ctx.view.Elements[0][0]);
    glUniformMatrix4fv(dbg_ctx.proj_location_sphere, 1, GL_FALSE, &dbg_ctx.projection.Elements[0][0]);
	
    glBindVertexArray(dbg_ctx.vao_sphere);
	
	
    //printf("num_spheres: %i \n", (*list).num_spheres);
    //printf("sphere_verts_count: %i \n", dbg_ctx.sphere_verts_count);
	
    for (int i = 0; i < (*list).num_spheres ; ++i)
    {
        mush_sphere sphi = (*list).spheres[i];
		
        //printf("sphi x: %f, y: %f, z: %f \n", sphi.center.X, sphi.center.Y, sphi.center.Z);
		
        //glUniformMatrix4fv(dbg_ctx.pos_location, 1, GL_FALSE, &dbg_ctx.projection.x.x);
        glUniform3f(dbg_ctx.pos_location_sphere, sphi.center.X, sphi.center.Y, sphi.center.Z);
        glUniform1f(dbg_ctx.radius_location_sphere, sphi.radius);
        glUniform4f(dbg_ctx.color_location_sphere, sphi.color.X, sphi.color.Y, sphi.color.Z, sphi.color.W);
		
		
        glDrawArrays( GL_TRIANGLES, 0, dbg_ctx.sphere_verts_count * 3);
    }
	
    glUseProgram(dbg_ctx.shader_prog_segment);
	
    glUniformMatrix4fv(dbg_ctx.view_location_segment, 1, GL_FALSE, &dbg_ctx.view.Elements[0][0]);
    glUniformMatrix4fv(dbg_ctx.proj_location_segment, 1, GL_FALSE, &dbg_ctx.projection.Elements[0][0]);
	
    glBindVertexArray(dbg_ctx.vao_segment);
    glBindBuffer(GL_ARRAY_BUFFER, dbg_ctx.vbo_segment);
	
    //printf("num_segments: %i \n", (*list).num_segments);
    //printf("sphere_verts_count: %i \n", dbg_ctx.sphere_verts_count);
	
	
    glBufferData(
        GL_ARRAY_BUFFER,
        (sizeof(mush_vertex) * 2 * (*list).num_segments),
        (*list).segments,
        GL_DYNAMIC_DRAW );
	
    glDrawArrays( GL_LINES, 0, 2 * (*list).num_segments);
	
    // --- triangles ---
	
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	
    glBindVertexArray(dbg_ctx.vao_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, dbg_ctx.vbo_triangle);
	
    glBufferData(
        GL_ARRAY_BUFFER,
        (sizeof(mush_vertex) * 3 * (*list).num_triangles),
        (*list).triangles,
        GL_DYNAMIC_DRAW );
	
    glDrawArrays( GL_TRIANGLES, 0, 3 * (*list).num_triangles);
	
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}



#endif /* MUSHISHI_IMPLEMENTATION */
