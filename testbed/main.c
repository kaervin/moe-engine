#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h> 
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>
#include <dlfcn.h>

#define BAKA_IMPLEMENTATION
#include "baka.h"

#define MUSHISHI_IMPLEMENTATION
#include "mushishi.h"

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

#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"

#include "glfw3.h"
#include "GL/gl3w.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_MEMSET
//#define NK_IMPLEMENTATION

//#define NK_KEYSTATE_BASED_INPUT
#include "UI/nuklear.h"
#define NK_GLFW_GL3_IMPLEMENTATION
#include "UI/nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define MAX_NAME_LENGTH 128

#define LEVEL_DIR "../levels/"
#define LEVEL_END ".level"
// --- types ---

#include "level_static.h"


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

void (*init_stepfunc) (Game_Struct * game, struct texture_asset ta_face4_p, struct texture_asset ta_angry_p);

Game_Struct game;

void load_level_lib(char * level_name) {
	
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
	
	
	init_stepfunc = (void (*)(Game_Struct * game, struct texture_asset ta_face4_p, struct texture_asset ta_angry_p)) dlsym(dl_handle, "init_step");
	
	if (init_stepfunc == NULL) {
		dl_error_string = dlerror();
		printf("dlerror:\n%s\n", dl_error_string);
		glfwTerminate();
		return;
	}
	
	init_stepfunc(&game, ta_face4, ta_angry);
	
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
	
	// --- Nuklear UI stuff
	struct nk_colorf bg;
	game->ctx = nk_glfw3_init(game->window, NK_GLFW3_INSTALL_CALLBACKS);
	{
		struct nk_font_atlas *atlas;
		nk_glfw3_font_stash_begin(&atlas);
		nk_glfw3_font_stash_end();
	}
	
	bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
	
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
	
	game.assets.tex_ass_man = gather_texture_assets();
	
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
	baka_StaticObjects prototype_static_objects = init_StaticObjectsStack(10000);
	
	// here we actually gather and create all the prototypes from the entities directory
	PrototypeStack prototypes = gather_static_entity_prototypes(&model_memory, &phys_mesh_stack, &prototype_static_objects,&game.assets.tex_ass_man, &prototype_ns);
	
	// prepare the graphics buffers
	// the second arguement is basically the maximum amount of nodes
	game.rendering_context = prepare_ressources_gen(model_memory.mesh_stack, &game.assets.tex_ass_man, 30000);
	
	game.rendering_context.projection = BuildProjectionMatrix((float)WINDOW_WIDTH/(float)WINDOW_HEIGHT,0.9f,0.2f,10000.0f);
	
	game.assets.prototypes = prototypes;
	game.assets.prototype_static_objects = prototype_static_objects;
	game.assets.model_memory = model_memory;
	
	// ns instances is basically the memory-arena/stack where all the instantiated game-objects are stored
	// this is whats primarily worked with in the actual game loop
	game.ns_instances.max_n = 1000;
	game.ns_instances.next_n = 0;
	game.ns_instances.nodes = (Node *)malloc( sizeof(Node) * 1000 );
	game.ns_instances.parent = (int *)malloc( sizeof(int) * 1000 );
	game.ns_instances.texture_assets = (texture_asset *)malloc( sizeof(texture_asset)*1000 );
	game.ns_instances.names = model_memory.node_stack.names; // NOTE: for now we just do this
	// When we systemize everything in the compression phase, I will think of something better
	game.ns_instances.name_indices = (int *)malloc(sizeof(int)*1000);
	
	game.level_static = init_level_static();
	
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
	
	game.cs.camera_point = vec3(0.0f, 0.0f, 25.0f);
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
	
	// Where we save our strings, currently used for the UI, 
	// may become useful for other stuff too
	game.string_stack.chars = (char *)malloc(sizeof(char)*10000);
	game.string_stack.next_string = 0;
	
	// Initialize stuff in the player struct
	// TODO: move this out of here. this should completely be the responsobilty of the loaded game code
	game.ps.character_point = vec3(0.0f, 5.0f, 0.0f);
	game.ps.character_model_point = game.ps.character_point;
	game.ps.character_downwards_speed = 0.0f;
	
	game.ps.relative_character_ray_origin = vec3(0.0f, 3.0f, 0.0f);
	game.ps.character_ray_direction = vec3(0.0f, -1.0f, 0.0f);
	game.ps.inv_character_ray_direction = vec3(0.0f, -1.0f, 0.0f);
	
	game.ps.character_control_direction = vec3(0.0f, 0.0f, 0.0f);
	game.ps.character_move_direction = vec3(0.0f, 0.0f, 0.0f);
	game.ps.character_facing_direction = vec3(1.0f, 0.0f, 0.0f);
	
	game.ps.hitnormal = vec3(0.0f, 1.0f, 0.0f);
	
	game.ps.player_capsule.p = vec3(0.0f, 1.4f, 0.0f);
	game.ps.player_capsule.q = vec3(0.0f, -0.0f, 0.0f);
	game.ps.player_capsule.radius = 1.0f;
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
	game.es.new_object_prototype_index = -1;
	game.es.show_dbg = false;
	game.es.plane_height = 0.0f;
	
	// init some test collision capsules
	game.num_cap = 10;
	baka_capsule caps[10];
	v3 cap_positions[10];
	Quat cap_rotations[10];
	
	game.caps = caps;
	game.cap_positions = cap_positions;
	game.cap_rotations = cap_rotations;
	
	// NOTE: spot 0 is reserved for the player capsule to be copied into
	// have to think about a good way to make this better...
	// anyway, this is pretty temporary in the first place anyway
	for (int i = 1; i < 10; i++) {
		game.caps[i].p = vec3(0.0, -1.0, 0.0);
		game.caps[i].q = vec3(0.0, 1.0, 0.0);
		game.caps[i].radius = 0.5 * i;
		game.caps[i].num_contacts = 0;
		
		game.cap_positions[i] = vec3(0.0, 0.0, -5.0 * i);
		game.cap_rotations[i] = quaternion(0.0, 0.0, 0.0, 1.0);
	}
	
	game.low_fps_mode = 0;
	strcpy(game.level_to_be_loaded, "testlevel1");
	game.should_reload_lib = 1;
	
	// load the game code from the dynamic library
	
	printf("start while \n\n");
	
	while (!glfwWindowShouldClose(game.window)) {
		
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
			game.level_static.genmodels,
			game.level_static.scales,
			game.level_static.next_level_entity);
		
		render_genmodels(
			game.rendering_context,
			game.ns_instances,
			game.assets.model_memory.skin_stack,
			&game.ps.instantiated_gondola,
			&game.ps.gondola_scale,
			1);
		
		
		render_skeleton(&game.ns_instances, &game.ps.instantiated_gondola);
		
		// NUKLEAR GUI TEST STUFF
		nk_glfw3_new_frame();
		
		if (game.es.show_dbg) {
			glDisable(GL_CULL_FACE);
			mush_render(game.dbg_ctx, dbg_list_non_cleared);
			
			glClear(GL_DEPTH_BUFFER_BIT);
			
			mush_render(game.dbg_ctx, dbg_list);
			mush_render(game.dbg_ctx, dbg_list_static);
		}
		
		mush_empty_list(dbg_list);
		mush_empty_list(dbg_list_non_cleared);
		
		nk_glfw3_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
		
		glfwSwapBuffers(game.window);			
		if (game.low_fps_mode) {
			sleep(1);
		}
		glfwPollEvents();
		
		
	}
	glfwTerminate();	
	return 0;
}