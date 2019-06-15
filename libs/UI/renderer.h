#ifndef RENDERER_H
#define RENDERER_H

#include <GLFW/glfw3.h>
#include "microui.h"
#include "atlas.inl"

void r_init(void);
void r_draw_rect(mu_Rect rect, mu_Color color);
void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color);
void r_draw_icon(int id, mu_Rect rect, mu_Color color);
int r_get_text_width(const char *text, int len);
int r_get_text_height(void);
void r_set_clip_rect(mu_Rect rect);
void r_clear(mu_Color color);
void r_present(void);

#endif
#ifdef RENDERER_IMPLEMENTATION

#define BUFFER_SIZE 163840

#define UI_MAX_VERTEX_BUFFER 512 * 10240
#define UI_MAX_ELEMENT_BUFFER 128 * 10240


struct ui_vertex {
    float position[2];
    float uv[2];
	char col[4];
};

static GLuint  index_buf[BUFFER_SIZE *  6];
static struct ui_vertex ui_verts[BUFFER_SIZE];

static int buf_idx;
static int index_idx;

struct ui_glfw_device {
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_uv;
    GLint attrib_col;
    GLint uniform_tex;
    GLint uniform_proj;
    GLuint font_tex;
};


static struct ui_glfw {
    GLFWwindow *win;
    int width, height;
    int display_width, display_height;
	struct ui_glfw_device ogl;
} glfwui;

const GLchar *vertex_shader =
"#version 420 core\n"
"uniform mat4 ProjMtx;\n"
"in vec2 Position;\n"
"in vec2 TexCoord;\n"
"in vec4 Color;\n"
"out vec2 Frag_UV;\n"
"out vec4 Frag_Color;\n"
"void main() {\n"
"   Frag_UV = TexCoord;\n"
"   Frag_Color = Color;\n"
"   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
"}\n";

const GLchar *fragment_shader =
"#version 420 core\n"
"precision mediump float;\n"
"uniform sampler2D Texture;\n"
"in vec2 Frag_UV;\n"
"in vec4 Frag_Color;\n"
"out vec4 Out_Color;\n"
"void main(){\n"
"   Out_Color = Frag_Color;\n"
"   Out_Color = Out_Color * texture(Texture, Frag_UV.st);\n"
"   //Out_Color = pow(Frag_Color * texture(Texture, Frag_UV.st), vec4(2.2));\n"
"   //vec4 temp_color = pow(Frag_Color * texture(Texture, Frag_UV.st), vec4(2.2));\n"
"   //Out_Color = vec4(1.0, 1.0, temp_color.x, 1.0);\n"
"   //Out_Color.g = pow(Frag_Color * texture(Texture, vec2(0, 1)), vec4(2.2)).a;\n"
"   //Out_Color.b = Frag_UV.s;\n"
"   //Out_Color.r = texture(Texture, Frag_UV.st).a;\n"
"   //Out_Color.a = 1.0;//pow(Frag_Color * texture(Texture, vec2(0, 1)), vec4(2.2)).r;\n"
"}\n";


// function to load shaders returns a shader program id
unsigned int loadshaders_ui(const char* vShaderCode, const char* fShaderCode) {
    
	char infoLog[1024];
	
    unsigned int vertex, fragment;
    int success;
    
    // vertexShader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
        printf("%s", infoLog);
        return 0;
    }
    // fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // check for shader compile errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("%s", infoLog);
        return 0;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 1024, NULL, infoLog);
        printf("%s", infoLog);
		return 0;
	}
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return shaderProgram;
}


static unsigned char *atlas_rgba;

void create_atlas_rgba() {
	atlas_rgba = malloc(ATLAS_WIDTH * ATLAS_HEIGHT * 4);
	for (int i = 0; i < ((ATLAS_WIDTH*ATLAS_HEIGHT)*4); i+=4) {
		atlas_rgba[i+0] = 255;
		atlas_rgba[i+1] = 255;
		atlas_rgba[i+2] = 255;
		atlas_rgba[i+3] = atlas_texture[i/4];
	}
}

void ui_glfw3_device_create(GLFWwindow *win)
{
    glfwui.win = win;
	
    GLint status;
    
    struct ui_glfw_device *dev = &glfwui.ogl;
    
	dev->prog = loadshaders_ui(vertex_shader, fragment_shader);
	
	
    int err = glGetError();
	printf("%x\n", err);
	assert(err == 0);
	
	dev->uniform_tex = glGetUniformLocation(dev->prog, "Texture");
    dev->uniform_proj = glGetUniformLocation(dev->prog, "ProjMtx");
    dev->attrib_pos = glGetAttribLocation(dev->prog, "Position");
    dev->attrib_uv = glGetAttribLocation(dev->prog, "TexCoord");
    dev->attrib_col = glGetAttribLocation(dev->prog, "Color");
	
    err = glGetError();
	printf("%x\n", err);
	assert(err == 0);
	
    {
        /* buffer setup */
        GLsizei vs = sizeof(struct ui_vertex);
        size_t vp = offsetof(struct ui_vertex, position);
        size_t vt = offsetof(struct ui_vertex, uv);
        size_t vc = offsetof(struct ui_vertex, col);
		
        glGenBuffers(1, &dev->vbo);
        glGenBuffers(1, &dev->ebo);
        glGenVertexArrays(1, &dev->vao);
		
        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);
		
        glEnableVertexAttribArray((GLuint)dev->attrib_pos);
        glEnableVertexAttribArray((GLuint)dev->attrib_uv);
        glEnableVertexAttribArray((GLuint)dev->attrib_col);
		
        glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }
	
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    err = glGetError();
	printf("%x\n", err);
	assert(err == 0);
	
	/* init texture */
	create_atlas_rgba();
	
	err = glGetError();
	printf("%i", err);
	assert(err == 0);
	
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_WIDTH, ATLAS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, atlas_rgba);
	
	glfwui.ogl.font_tex = id;
	
}

void r_init() {
	
    struct GLFWwindow *win = glfwui.win;
	
    glfwGetWindowSize(win, &glfwui.width, &glfwui.height);
    glfwGetFramebufferSize(win, &glfwui.display_width, &glfwui.display_height);
    
	int err = glGetError();
	printf("%i", err);
	assert(err == 0);
	
	glEnable(GL_SCISSOR_TEST);
    
	buf_idx = 0;
	index_idx = 0;
}


static void flush(void) {
    int err;
	
	if (buf_idx == 0) { return; }
	
	struct ui_glfw_device *dev = &glfwui.ogl;
	
    GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (GLfloat)glfwui.width;
    ortho[1][1] /= (GLfloat)glfwui.height;
	
    /* setup global state */
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
	glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    
	glDisable(GL_SCISSOR_TEST);
	
	glActiveTexture(GL_TEXTURE0);
	glUseProgram(dev->prog);
	
    glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);
    glUniform1i(dev->uniform_tex, 0);
    glViewport(0,0,(GLsizei)glfwui.display_width,(GLsizei)glfwui.display_height);
    
	void *vertices, *elements;
	
	/* allocate vertex and element buffer */
	glBindVertexArray(dev->vao);
    glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);
	glBufferData(GL_ARRAY_BUFFER, UI_MAX_VERTEX_BUFFER, NULL, GL_STREAM_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, UI_MAX_ELEMENT_BUFFER, NULL, GL_STREAM_DRAW);
	
	/* load draw vertices & elements directly into vertex + element buffer */
	vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	memcpy(vertices, ui_verts, sizeof(struct ui_vertex)*buf_idx);
	memcpy(elements, index_buf, sizeof(GLuint)*index_idx);
	
	glBindTexture(GL_TEXTURE_2D, dev->font_tex);
	glDrawElements(GL_TRIANGLES, index_idx, GL_UNSIGNED_INT, 0);
	
    /* default OpenGL state */
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
	
	/*
 glViewport(0, 0, width, height);
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 glOrtho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();
 
 glTexCoordPointer(2, GL_FLOAT, 0, tex_buf);
 glVertexPointer(2, GL_FLOAT, 0, vert_buf);
 glColorPointer(4, GL_UNSIGNED_BYTE, 0, color_buf);
 glDrawElements(GL_TRIANGLES, buf_idx * 6, GL_UNSIGNED_INT, index_buf);
 
 glMatrixMode(GL_MODELVIEW);
 glPopMatrix();
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 */
	
	
	
	
	buf_idx = 0;
	index_idx = 0;
}


static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
	if (buf_idx == BUFFER_SIZE) {
		flush();
	}
	int element_idx = buf_idx;
	
	/* update texture buffer */
	float x = src.x / (float) ATLAS_WIDTH;
	float y = src.y / (float) ATLAS_HEIGHT;
	float w = src.w / (float) ATLAS_WIDTH;
	float h = src.h / (float) ATLAS_HEIGHT;
	
	// 1
	ui_verts[buf_idx].position[0] = dst.x;
	ui_verts[buf_idx].position[1] = dst.y;
	
	ui_verts[buf_idx].uv[0] = x;
	ui_verts[buf_idx].uv[1] = y;
	
	ui_verts[buf_idx].col[0] = color.r;
	ui_verts[buf_idx].col[1] = color.g;
	ui_verts[buf_idx].col[2] = color.b;
	ui_verts[buf_idx].col[3] = color.a;
	buf_idx++;
	// 2
	ui_verts[buf_idx].position[0] = dst.x + dst.w;
	ui_verts[buf_idx].position[1] = dst.y;
	
	ui_verts[buf_idx].uv[0] = x + w;
	ui_verts[buf_idx].uv[1] = y;
	
	
	ui_verts[buf_idx].col[0] = color.r;
	ui_verts[buf_idx].col[1] = color.g;
	ui_verts[buf_idx].col[2] = color.b;
	ui_verts[buf_idx].col[3] = color.a;
	buf_idx++;
	// 3
	ui_verts[buf_idx].position[0] = dst.x;
	ui_verts[buf_idx].position[1] = dst.y + dst.h;
	
	ui_verts[buf_idx].uv[0] = x;
	ui_verts[buf_idx].uv[1] = y + h;
	
	ui_verts[buf_idx].col[0] = color.r;
	ui_verts[buf_idx].col[1] = color.g;
	ui_verts[buf_idx].col[2] = color.b;
	ui_verts[buf_idx].col[3] = color.a;
	buf_idx++;
	// 4
	ui_verts[buf_idx].position[0] = dst.x + dst.w;
	ui_verts[buf_idx].position[1] = dst.y + dst.h;
	
	ui_verts[buf_idx].uv[0] = x + w;
	ui_verts[buf_idx].uv[1] = y + h;
	
	ui_verts[buf_idx].col[0] = color.r;
	ui_verts[buf_idx].col[1] = color.g;
	ui_verts[buf_idx].col[2] = color.b;
	ui_verts[buf_idx].col[3] = color.a;
	buf_idx++;
	
	/* update index buffer */
	index_buf[index_idx + 0] = element_idx + 0;
	index_buf[index_idx + 1] = element_idx + 1;
	index_buf[index_idx + 2] = element_idx + 2;
	index_buf[index_idx + 3] = element_idx + 2;
	index_buf[index_idx + 4] = element_idx + 3;
	index_buf[index_idx + 5] = element_idx + 1;
	index_idx +=6;
}


void r_draw_rect(mu_Rect rect, mu_Color color) {
    push_quad(rect, atlas[ATLAS_WHITE], color);
}


void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
    mu_Rect dst = { pos.x, pos.y, 0, 0 };
	for (const char *p = text; *p; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		mu_Rect src = atlas[ATLAS_FONT + chr];
		dst.w = src.w;
		dst.h = src.h;
		push_quad(dst, src, color);
		dst.x += dst.w;
	}
}


void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
    mu_Rect src = atlas[id];
	int x = rect.x + (rect.w - src.w) / 2;
	int y = rect.y + (rect.h - src.h) / 2;
	push_quad(mu_rect(x, y, src.w, src.h), src, color);
}


int r_get_text_width(const char *text, int len) {
	int res = 0;
	for (const char *p = text; *p && len--; p++) {
		if ((*p & 0xc0) == 0x80) { continue; }
		int chr = mu_min((unsigned char) *p, 127);
		res += atlas[ATLAS_FONT + chr].w;
	}
	return res;
}


int r_get_text_height(void) {
	return 18;
}


void r_set_clip_rect(mu_Rect rect) {
    flush();
	glScissor(rect.x, glfwui.height - (rect.y + rect.h), rect.w, rect.h);
}


void r_clear(mu_Color clr) {
	flush();
	glClearColor(clr.r / 255., clr.g / 255., clr.b / 255., clr.a / 255.);
	glClear(GL_COLOR_BUFFER_BIT);
}


void r_present(void) {
    flush();
	//SDL_GL_SwapWindow(window);
}


#endif
