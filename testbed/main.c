#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h> 
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdlib.h>

typedef unsigned int uint;

#define PERF_MAIN 1
//#define PERF_GAMESTEP 0
#include "perf_count.h"

#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"

#define BAKA_IMPLEMENTATION
#include "baka.h"

#define MUSHISHI_IMPLEMENTATION
#include "mushishi.h"

#define RENDER_TYPES
#define RENDER_IMPLEMENTATION
#include "render_basic.h"

#define TYPOS_IMPLEMENTATION
#include "typos.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

#define TERRAIN_IMPLEMENTATION
#include "terrain.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glfw3.h"
#include "GL/gl3w.h"

#include <assert.h>
#define RENDERER_IMPLEMENTATION
#include "UI/renderer.h"
#include "UI/microui.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define MAX_NAME_LENGTH 128
#define MAX_NUM_POINTS 1024
#define MAX_GAME_POINT_CHARS 512 * 1024

#define LEVEL_DIR "../levels/"
#define LEVEL_END ".level"
// --- types ---

#include "level_static.h"


#include "fpstruct.h"
FP_Struct fp_struct;
#include "ffill.h"
// --- globals ---


v4 Green4 = {0.0f, 1.0f, 0.0f, 1.0};
v4 Yellow4 = {1.0f, 1.0f, 0.0f, 1.0};
v4 Red4 = {1.0f, 0.0f, 0.0f, 1.0};
v4 Blue4 = {0.0f, 0.0f, 1.0f, 1.0};

struct texture_asset ta_face4;
struct texture_asset ta_angry;

mush_draw_list *dbg_list;
mush_draw_list *dbg_list_non_cleared;
mush_draw_list *dbg_list_static;

#include "random_stuff.h"

void * dl_handle = NULL;

char * dl_error_string;

void (*stepfunc) (Game_Struct*);

void (*init_stepfunc) (Game_Struct * game);

Game_Struct game;

void example_func() {
	printf("hello world\n");
}

void load_level_lib(char * level_name) {
	printf("level to be loaded: %s\n", level_name);
	// ---
	printf("%p\n", animate_model_test);
	printf("%p\n", solve_IK_fabrik);
	printf("%p\n", get_model_from_crc32);
	
	game.sample_func_p = example_func;
	fill_function_pointers();
	game.fp_struct = fp_struct;
	
	FILE *fp;
	int status;
	char level_path[PATH_MAX];
	char path[PATH_MAX];
	path[0] = '\0';
	level_path[0] = '\0';
	
	strcpy(level_path, LEVEL_DIR);
	strcat(level_path, level_name);
	
	strcpy(path, level_path);
	strcat(path, "/buildlib.sh");
	
	fp = popen(path, "r");
	if (fp == NULL) {
		printf("couldn't use popen\n");
	}
	
	while (fgets(path, PATH_MAX, fp) != NULL) {
		printf("%s", path);
	}
	
	status = pclose(fp);
	if (status == -1) {
	} else {
		if (WIFEXITED(status)) {
			printf("successfuly compiled\n");
		} else {
			printf("compilation failed\n");
			return;
		}
	}
	
	path[0] = '\0';
	strcpy(path, level_path);
	strcat(path, "/shared.so");
	
	if(dl_handle != NULL) {
		dlclose(dl_handle);
	}
	dl_handle = dlopen(path, RTLD_NOW);
	
	char * dl_error_string = dlerror();
	printf("derror:\n%s\n", dl_error_string);
	if (dl_error_string != NULL) {
		printf("dlerror:\n%s\n", dl_error_string);
		glfwTerminate();
		return;
	}
	
	stepfunc = (void (*)(Game_Struct*)) dlsym(dl_handle, "gamestep");
	if (stepfunc == NULL) {
		dl_error_string = dlerror();
		printf("dlerror:\n%s\n", dl_error_string);
		glfwTerminate();
		return;
	}
	
	init_stepfunc = (void (*)(Game_Struct * game)) dlsym(dl_handle, "init_step");
	
	if (init_stepfunc == NULL) {
		dl_error_string = dlerror();
		printf("dlerror:\n%s\n", dl_error_string);
		glfwTerminate();
		return;
	}
	
	init_stepfunc(&game);
}

static const char key_map[256] = {
	[ GLFW_KEY_LEFT_SHIFT          & 0xff ] = MU_KEY_SHIFT,
	[ GLFW_KEY_RIGHT_SHIFT         & 0xff ] = MU_KEY_SHIFT,
	[ GLFW_KEY_LEFT_CONTROL        & 0xff ] = MU_KEY_CTRL,
	[ GLFW_KEY_RIGHT_CONTROL       & 0xff ] = MU_KEY_CTRL,
	[ GLFW_KEY_LEFT_ALT            & 0xff ] = MU_KEY_ALT,
	[ GLFW_KEY_RIGHT_ALT           & 0xff ] = MU_KEY_ALT,
	[ GLFW_KEY_ENTER               & 0xff ] = MU_KEY_RETURN,
	[ GLFW_KEY_BACKSPACE           & 0xff ] = MU_KEY_BACKSPACE,
};


static int text_width(mu_Font font, const char *text, int len) {
	if (len == -1) { len = strlen(text); }
	return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
	return r_get_text_height();
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	mu_input_mousemove(game.mu_ctx, xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	
    if (action == GLFW_PRESS) {
		mu_input_mousedown(game.mu_ctx, xpos, ypos, button+1);
	}
	
    if (action == GLFW_RELEASE) {
		mu_input_mouseup(game.mu_ctx, xpos, ypos, button+1);
	}
	
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	mu_input_scroll(game.mu_ctx, 0, yoffset * -30);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
	int c = key_map[key & 0xff];
	if (c && action == GLFW_PRESS) { mu_input_keydown(game.mu_ctx, c); }
	if (c && action == GLFW_RELEASE) { mu_input_keyup(game.mu_ctx, c);   }
	
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	char itext[5];
	memcpy(itext, &codepoint, 4);
	itext[4] = 0;
	mu_input_text(game.mu_ctx, itext);
}


bool Init(Game_Struct *game)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
	glfwWindowHint (GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint (GLFW_DOUBLEBUFFER, GL_TRUE);
	//glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
	
	game->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "baka", NULL, NULL);
	if (!game->window) {
		glfwTerminate();
		return false;
	}
	
	glfwMakeContextCurrent(game->window);
	
	if (gl3wInit()) {
		return -1;
	}
	
	if (GL_ARB_multi_draw_indirect) {
		printf("GL_ARB_multi_draw_indirect\n");
	}
	
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetWindowSizeLimits(game->window, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetWindowPos(game->window, 200, 200);
	glfwSetInputMode(game->window, GLFW_STICKY_MOUSE_BUTTONS, 1);
	glfwSetInputMode(game->window, GLFW_STICKY_KEYS, 1);
	glfwSwapInterval(1);
	
	printf("here\n");
	
	// --- more init stuff
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glEnable(GL_FRAMEBUFFER_SRGB);
	
	printf( "gl version %s \n", glGetString(GL_VERSION));
	
	int texture_array_layers = 0;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &texture_array_layers);
	printf("max array texture depth %i \n", texture_array_layers);
	
	int max_texture_image_units;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_image_units);
	printf("max image texture units %i\n", max_texture_image_units);
	
	// joystick stuff
	int joy1 = glfwJoystickPresent(GLFW_JOYSTICK_1);
	printf("joystick present: %i\n", joy1);
	
	ui_glfw3_device_create(game->window);
	r_init();
	
	/* init microui */
	game->mu_ctx = malloc(sizeof(mu_Context));
	mu_init(game->mu_ctx);
	game->mu_ctx->text_width = text_width;
	game->mu_ctx->text_height = text_height;
	
	glfwSetCursorPosCallback(game->window, cursor_position_callback);
	glfwSetMouseButtonCallback(game->window, mouse_button_callback);
	glfwSetKeyCallback(game->window, key_callback);
	glfwSetCharCallback(game->window, character_callback);
	glfwSetScrollCallback(game->window, scroll_callback);
	
	return true;
}


// TODO:

// A N I M A T I O N S - animation currently just get malloc'd for themselves, which isn't horrible for now, but having them in one memory place may actually be pretty neat, and grabbing them via some identifier, like model-ressources currently, would be pretty nice too

// IDEA: for the ground and possibly other large structures, where we want tiled textures,
// we could create a new renderer type, which has a texture atlas of tiles
// and a map texture, which basically tells us at which point which tile should be drawn with which proportion
// NOTE: I wonder if this is even necessary for the current game idea
// we probably don't even need the terrain at all, although in a sense it could be used to cheapen physics calculations of some irregular objects. Probably unnecessary though



int main(int argc, char *argv[]) {
	// for 4coder to have smooth "terminal" output
	setvbuf(stdout, NULL, _IONBF, 0);
	
	if (!Init(&game)) { return -1;}
	
	
	
	// gather all the physical obj defintions
	PhysicsMeshStack phys_mesh_stack = gather_physics_meshes();
	
	// The Modelmemory contains all the Data pertaining to possibly multiple models.
	// the GenModels themselves contain the indices and lengths that pertain to that specific model
	// for example the Nodes that belong to a specific model are stored contiguously in the NodeStack
	// starting at node_idx with a length of num_nodes
	// to 'instantiate' such a model then, one would copy the portion of the NodeStack into a different one reserved for instances (here called ns_instances)
	// this can be done with the function instantiate_genmodel
	
	ModelMemory model_memory = gather_model_memory();
	
	bool ok;
	GenModel gondola_model = get_model_from_crc32(&model_memory, &ok, crc32_cstring("gondola.moe"));
	AnimReturn ar_gondola = load_animation_test("../models/gondola.moa");
	
	
	// the prototype NodeStack is for storing the Nodes of the prototypical Objects,
	// meaning the objects from which the actual ingame objects are instanced
	NodeStack prototype_ns;
	prototype_ns.max_n = 1000;
	prototype_ns.next_n = 0;
	prototype_ns.nodes = (Node *)malloc( sizeof(Node) * 1000 );
	prototype_ns.parent = (int *)malloc( sizeof(int) * 1000 );
	prototype_ns.texture_assets = (texture_asset *)malloc( sizeof(texture_asset)*1000 );
	prototype_ns.names = model_memory.node_stack.names; // NOTE: for now we just do this
	// When I systemize everything in the compression phase, I will think of something better
	prototype_ns.name_indices = (int *)malloc(sizeof(int)*1000);
	
	// prototype_static_objects is for storing the physics of the static level objects
	baka_Shape_Stack prototype_static_objects = baka_init_shape_stack(10000);
    
    
    Texture_Asset_Manager texa = gather_texture_assets();
	
    game.assets.tex_ass_man = texa;
	// here we actually gather and create all the prototypes from the entities directory
	PrototypeStack prototypes = gather_static_entity_prototypes(&model_memory, &phys_mesh_stack, &prototype_static_objects,&game.assets.tex_ass_man, &prototype_ns);
	game.assets.prototypes = prototypes;
	// prepare the graphics buffers
	// the second arguement is basically the maximum amount of nodes
	game.rendering_context = prepare_ressources_gen(model_memory.mesh_stack, &game.assets.tex_ass_man, 30000);
	
	game.rendering_context.projection = BuildProjectionMatrix((float)WINDOW_WIDTH/(float)WINDOW_HEIGHT,0.9f,0.2f,10000.0f);
	
	//game.assets.prototypes = prototypes;
	game.assets.prototype_static_objects = prototype_static_objects;
	game.assets.model_memory = model_memory;
	
	// ns instances is basically the memory-arena/stack where all the instantiated game-objects are stored
	// this is whats primarily worked with in the actual game loop
	game.ns_instances.max_n = 10000;
	game.ns_instances.next_n = 0;
	game.ns_instances.nodes = (Node *)malloc( sizeof(Node) * 10000 );
	game.ns_instances.parent = (int *)malloc( sizeof(int) * 10000 );
	game.ns_instances.texture_assets = (texture_asset *)malloc( sizeof(texture_asset)*10000 );
	game.ns_instances.names = model_memory.node_stack.names; // NOTE: for now we just do this
	// When we systemize everything in the compression phase, I will think of something better
	game.ns_instances.name_indices = (int *)malloc(sizeof(int)*10000);
	
	game.level_static = init_level_static();
	
	game.next_model_to_render = 0;
	game.max_models_to_render = 10000;
	game.models_to_render = malloc(sizeof(GenModel)*game.max_models_to_render);
	game.models_to_render_scales = malloc(sizeof(v3)*game.max_models_to_render);
	
	game.game_points.point_names.chars = malloc(MAX_GAME_POINT_CHARS);
	game.game_points.point_names.max_char = MAX_GAME_POINT_CHARS;
	game.game_points.point_names.next_string = 0;
	game.game_points.next_point = 0;
	game.game_points.max_point = MAX_NUM_POINTS;
	game.game_points.points = malloc(sizeof(Game_Point)*MAX_NUM_POINTS);
	game.game_points.map = init_map();
	
	// create some terrain
	game.terra = terra_new_from_image("../heightmaps/heightmap16.png", 10.0f, -250.0f*10.0f, -250.0f*10.0f);
	terra_smooth_depths(game.terra);
	terra_rebuild_vertices(game.terra);
	
	
	// --- some Graphics debug stuff
	// init the context and create space for the specific debug shapes
	game.dbg_ctx = mush_init();
	
	dbg_list = &(game.dbg_list);
	dbg_list_non_cleared = &(game.dbg_list_non_cleared);
	dbg_list_static = &(game.dbg_list_static);
	
	mush_alloc_forms(dbg_list, 100000);
	mush_alloc_forms(dbg_list_non_cleared, 100000);
	mush_alloc_forms(dbg_list_static, 100000);
	
	game.dbg_ctx.projection = game.rendering_context.projection;
	
	game.terra.projection = game.dbg_ctx.projection;
	
	game.cs.camera_point = vec3(0.0f, 5.0f, 25.0f);
	game.cs.camera_right = vec3(1.0f, 0.0f, 0.0f);
	game.cs.projection = game.dbg_ctx.projection;
	game.cs.inv_projection = inverse_projection_matrix(game.dbg_ctx.projection);
	
	game.dbg_ctx.view = HMM_LookAt( vec3(10.0f, 3.0f, 20.0f), vec3(10.0f,3.0f,0.0f), vec3(0.0f,1.0f,0.0f));
	
	// draw coordinate-sys
	mush_draw_segment(dbg_list_static,vec3(0.0f, 0.0f, 0.0f),vec3(1.0f, 0.0f, 0.0f),vec4(1.0f, 0.0f, 0.0f, 1.0f),vec4(1.0f, 0.0f, 0.0f, 1.0f));
	mush_draw_segment(dbg_list_static,vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 1.0f, 0.0f),vec4(0.0f, 1.0f, 0.0f, 1.0f),vec4(0.0f, 1.0f, 0.0f, 1.0f));
	mush_draw_segment(dbg_list_static,vec3(0.0f, 0.0f, 0.0f),vec3(0.0f, 0.0f, 1.0f),vec4(0.0f, 0.0f, 1.0f, 1.0f),vec4(0.0f, 0.0f, 1.0f, 1.0f));
	
	
	game.level_editing_mode = 0;
	game.current_time = 0.0;
	game.last_time = 0.0;
	
	// Initialize stuff in the player struct
	// TODO: move this out of here. this should completely be the responsobilty of the loaded game code
	game.ps.character_point = vec3(0.0f, 5.0f, 0.0f);
	game.ps.character_model_point = game.ps.character_point;
	
	game.ps.relative_character_ray_origin = vec3(0.0f, 3.0f, 0.0f);
	game.ps.character_ray_direction = vec3(0.0f, -1.0f, 0.0f);
	game.ps.inv_character_ray_direction = vec3(0.0f, -1.0f, 0.0f);
	
	game.ps.character_control_direction = vec3(0.0f, 0.0f, 0.0f);
	game.ps.character_move_direction = vec3(0.0f, 0.0f, 0.0f);
	game.ps.character_facing_direction = vec3(1.0f, 0.0f, 0.0f);
	
	game.ps.hitnormal = vec3(0.0f, 1.0f, 0.0f);
	
	game.ps.player_capsule.p = vec3(0.0f, 1.4f, 0.0f);
	game.ps.player_capsule.q = vec3(0.0f, -0.0f, 0.0f);
	game.ps.player_capsule.radius = 1.2f;
	game.ps.player_capsule.num_contacts = 0;
	
	game.ps.player_left_foot_fabrik_mix = 0.0f;
	game.ps.player_right_foot_fabrik_mix = 0.0f;
	game.ps.last_adjusted_foot_delta_left = vec3(0.0f, 0.0f, 0.0f);
	game.ps.last_adjusted_foot_delta_right = vec3(0.0f, 0.0f, 0.0f);
	
	game.ps.player_capsule_position_relative_to_player_point = vec3(0.0f, 3.0f, 0.0f);
	game.ps.player_capsule_rotation = quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	
	game.ps.gondola_model = gondola_model;
	game.ps.ar_gondola = ar_gondola;
	
	game.ps.previous_time = 0.0f;
	
	// Init stuff in the Edit struct
	game.es.edit_object_index = -1;
	game.es.edit_point_index = -1;
	game.es.new_object_prototype_index = -1;
	game.es.show_dbg = false;
	game.es.plane_height = 0.0f;
	
	game.low_fps_mode = 0;
	strcpy(game.level_to_be_loaded, "testlevel1");
	game.should_reload_lib = 1;
	game.es.show_perf_times = 0;
	// load the game code from the dynamic library
	
	printf("start while \n\n");
	bool should_show_performance_times = false;
	
	while (!glfwWindowShouldClose(game.window)) {
		START_TIME;
		mu_begin(game.mu_ctx);
		
		glClearColor(0.03, 0.03, 0.32, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		
		
		if (game.should_reload_lib) {
			game.should_reload_lib = 0;
			load_level_lib(game.level_to_be_loaded);
		}
		
		stepfunc(&game);
		
		game.terra.view = game.dbg_ctx.view;
		terra_render(game.terra);
		
		// as the name implies, render_genmodels renders our models
		// TODO: rework render_genmodels and all of these functions to use the new data structures, be more concise, and remove stuff that we don't use anymore (scales, translations)
		
		render_genmodels(
			game.rendering_context,
			game.ns_instances,
			game.assets.model_memory.skin_stack,
			game.models_to_render,
			game.models_to_render_scales,
			game.next_model_to_render);
		
		render_skeleton(&game.ns_instances, &game.ps.instantiated_gondola);
		
		if (game.es.show_dbg) {
			glDisable(GL_CULL_FACE);
			mush_render(game.dbg_ctx, dbg_list_non_cleared);
			
			glClear(GL_DEPTH_BUFFER_BIT);
			
			mush_render(game.dbg_ctx, dbg_list);
			mush_render(game.dbg_ctx, dbg_list_static);
		}
		
		mush_empty_list(dbg_list);
		mush_empty_list(dbg_list_non_cleared);
		
		
		if (game.es.show_perf_times) {
			static mu_Container window;
			
			/* init window manually so we can set its position and size */
			if (!window.inited) {
				mu_init_window(game.mu_ctx, &window, 0);
				window.rect = mu_rect(WINDOW_WIDTH-600, 5, 250, 150);
			}
			
			if (mu_begin_window_ex(game.mu_ctx, &window, "Perf", MU_OPT_NOCLOSE | MU_OPT_NOTITLE)) {
				
				mu_layout_row(game.mu_ctx, 1, (int[]) { 300 }, 0);
				
				char temp_buf[1024];
				
				for (int i = 0; i < __COUNTER__; i++) {
					Perf_Measurement p = perf_measurements[i];
					if (p.fnc_name == NULL) continue;
					sprintf(temp_buf, "%f %lu %f %s\n", p.time, p.hit, (p.time/p.hit), p.fnc_name);
					mu_label(game.mu_ctx, temp_buf);
				}
				for (int i = 0; i < game.num_gamestep_perf_msr; i++) {
					Perf_Measurement p = game.gamestep_perf_msr[i];
					if (p.fnc_name == NULL) continue;
					sprintf(temp_buf, "%f %lu %f %s\n", p.time, p.hit, (p.time/p.hit), p.fnc_name);
					mu_label(game.mu_ctx, temp_buf);
				}
				
				for (int i = 0; i < __COUNTER__; i++) {
					perf_measurements[i].time = 0.0f;
					perf_measurements[i].rdtsc_stamp = 0;
					perf_measurements[i].fnc_name = NULL;
					perf_measurements[i].hit = 0;
				}
				for (int i = 0; i < game.num_gamestep_perf_msr; i++) {
					game.gamestep_perf_msr[i].time = 0.0f;
					game.gamestep_perf_msr[i].rdtsc_stamp = 0;
					game.gamestep_perf_msr[i].fnc_name = NULL;
					game.gamestep_perf_msr[i].hit = 0;
				}
				mu_end_window(game.mu_ctx);
			}
			/*
   printf("main stuff %p\n", perf_measurements);
   
   printf("gamestep stuff %p\n", game.gamestep_perf_msr);
   */
		}
		
		mu_end(game.mu_ctx);
		
		//r_clear(mu_color(bg[0], bg[1], bg[2], 255));
		mu_Command *cmd = NULL;
		
		while (mu_next_command(game.mu_ctx, &cmd)) {
			switch (cmd->type) {
				case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
				case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
				case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
				case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
			}
		}
		
		r_present();
		
		END_TIME;
		glfwSwapBuffers(game.window);			
		if (game.low_fps_mode) {
			sleep(1);
		}
		glfwPollEvents();
		
	}
	glfwTerminate();	
	return 0;
}

Perf_Measurement perf_measurements[__COUNTER__];