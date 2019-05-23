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

#define DEBUG_UI 1

#ifdef DEBUG_UI

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

#endif

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define MAX_NAME_LENGTH 128

#define LEVEL_DIR "../levels/"
#define LEVEL_END ".level"

v4 Green4 = {0.0f, 1.0f, 0.0f, 1.0};
v4 Yellow4 = {1.0f, 1.0f, 0.0f, 1.0};
v4 Red4 = {1.0f, 0.0f, 0.0f, 1.0};
v4 Blue4 = {0.0f, 0.0f, 1.0f, 1.0};

struct texture_asset ta_face4;
struct texture_asset ta_angry;
struct texture_asset ta_material;

mush_draw_list *dbg_list;
mush_draw_list *dbg_list_non_cleared;
mush_draw_list *dbg_list_static;

#include "level_static.h"

#include "random_stuff.h"

#include "../edit.h"
#include "../player.h"


void init_step(Game_Struct * game, struct texture_asset ta_face4_p, struct texture_asset ta_angry_p) {
	
	strcpy(game->static_level_file_name, "1");
	
	dbg_list = &game->dbg_list;
	dbg_list_static = &game->dbg_list_static;
	dbg_list_non_cleared = &game->dbg_list_non_cleared;
	
	bool ok = false;
	
	// load some textures and assign their identifiers
	// the way this works will be changed probably, this is just for now
	
	unsigned int texture_ident = map_lookup_key(&game->assets.tex_ass_man.map, &ok, crc32_cstring("Material.png"));
	ta_material = to_texture_asset(texture_ident);
	printf("Material texture_ident %u %u %u %u\n", ok, texture_ident, ta_material.type, ta_material.nr);
	
	texture_ident = map_lookup_key(&game->assets.tex_ass_man.map, &ok, crc32_cstring("Face.png"));
	ta_face4 = to_texture_asset(texture_ident);
	printf("face texture_ident %u %u %u %u\n", ok, texture_ident, ta_face4.type, ta_face4.nr);
	
	texture_ident = map_lookup_key(&game->assets.tex_ass_man.map, &ok, crc32_cstring("Angry.png"));
	ta_angry = to_texture_asset(texture_ident);
	printf("face texture_ident %u %u %u %u\n", ok, texture_ident, ta_angry.type, ta_angry.
		   nr);
	
	game->ns_instances.next_n = 0;
	
	reset_level_static(&game->level_static);
	load_level_static(&game->level_static, "../levels/testlevel1/1.level", &game->assets, &game->ns_instances);
	
	//game->ps.gondola_model = get_model_from_crc32(&game->assets.model_memory, &ok, crc32_cstring("gondola.moe"));
	if (!ok) {
		printf("couldn't load gondola.moe\n");
	}
	
	bool succ = instantiate_genmodel(
		game->assets.model_memory.node_stack,
		game->ps.gondola_model,
		&game->ns_instances,
		&game->ps.instantiated_gondola);
	
	if (!succ) {
		printf("couldn't instantiate gondola\n");
	}
	
	set_node_texture_asset_by_name(&game->ns_instances, &game->ps.instantiated_gondola, "Plane", ta_face4);
	set_node_texture_asset_by_name(&game->ns_instances, &game->ps.instantiated_gondola, "Cube", ta_material);
	
	game->ps.gondola_transform.translation = vec3(5.0f, 0.0f, 0.0f);
	game->ps.gondola_transform.rotation = quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	game->ps.gondola_scale = vec3(1.0f, 1.0f, 1.0f);
}


// the gamestep is basically where all the game interaction happens
// it can be reloaded at runtime, which is pretty great, just remember to put the necessary stuff into the init step
void gamestep(Game_Struct * game) {
	// reset the string_stack
	game->string_stack.next_string = 0;
	
	game->keys = processInput(game->window, game->keys);
	
	if (game->keys.F10_key.initial == 1) {
		game->level_editing_mode = !game->level_editing_mode;
	}
	
	if (game->keys.F1_key.initial == 1) {
		game->es.editor_mode = SETTINGS;
		game->level_editing_mode = true;
	}
	if (game->keys.F2_key.initial == 1) {
		game->es.editor_mode = SELECTION;
		game->level_editing_mode = true;
	}
	if (game->keys.F3_key.initial == 1) {
		game->es.editor_mode = NEW_OBJECT;
		game->level_editing_mode = true;
	}
	
	if (game->keys.p_key.initial == 1) {
		//game->low_fps_mode= !game->low_fps_mode;
		game->should_reload_lib = 1;
	}
	
	game->current_time = glfwGetTime();
	float dt = game->current_time - game->last_time;
	
	game->last_time = game->current_time;
	
	if (game->level_editing_mode) {
		editor_step(game, dt);
		
		game->dbg_ctx.view = HMM_LookAt(game->es.editor_point, add_v3(game->es.editor_point, game->es.editor_forward), vec3(0.0f,1.0f,0.0f));
		
	} else {
		// do several game steps per frame
		dt = dt / 4.0f;
		
		game->ps = player_step(game, dt);
		game->keys.r_key.initial = false;
		
		game->ps = player_step(game, dt);
		game->ps = player_step(game, dt);
		
		mush_empty_list(dbg_list);
		mush_empty_list(dbg_list_non_cleared);
		
		game->ps = player_step(game, dt);
		
		game->es.editor_point = add_v3(game->cs.camera_point, game->ps.character_model_point);
		game->es.editor_forward = normalize_v3(sub_v3(add_v3(game->ps.character_model_point, vec3(0.0f, 6.0f, 0.0f)), game->es.editor_point));
		
		game->dbg_ctx.view = HMM_LookAt(add_v3(game->cs.camera_point, game->ps.character_model_point), add_v3(game->ps.character_model_point, vec3(0.0f, 6.0f, 0.0f)), vec3(0.0f,1.0f,0.0f));
	}
	
	game->rendering_context.view = game->dbg_ctx.view;
	game->terra.view = game->dbg_ctx.view;
	game->rendering_context.player_pos = game->ps.shadow_point;
	
}
