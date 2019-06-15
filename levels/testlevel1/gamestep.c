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

#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"

int num_counter;
#define PERF_GAMESTEP 1
#include "perf_count.h"

#define BAKA_IMPLEMENTATION
#include "baka.h"

#define MUSHISHI_IMPLEMENTATION
#include "mushishi.h"

#define RENDER_TYPES
#define RENDER_IMPLEMENTATION
#include "render_basic.h"

#include "fpstruct.h"
FP_Struct fp;

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

typedef struct Platform {
	v3 p1, p2;
	int direction;
	v3 vel;
} Platform;

#define MAX_PLATFORMS 256

typedef struct Platform_Collection {
	Entity_Collection entities;
	Platform *platforms;
} Platform_Collection;

Platform_Collection platform_collection;

#define MAX_PLAYERS 1024

typedef struct Enemy_Collection {
	Entity_Collection entities;
	PlayerState *enemies;
} Enemy_Collection;

Enemy_Collection enemy_collection;

#include "../edit.h"
#include "../player.h"

void new_enemy(Game_Struct * game, v3 pos) {
	
	int enemy_idx = create_base_entity_from_prototype_name(&game->assets, &game->ns_instances, &enemy_collection.entities, "gondola.entity");
	
	if (enemy_idx == -1) {
		printf("couldn't create new enemy\n");
		return;
	}
	
	Base_Entity *player_ent = &enemy_collection.entities.els[enemy_idx];
	PlayerState *player = &enemy_collection.enemies[enemy_idx];
	
	player_ent->transform.translation = vec3(0.0f, 0.0f, 0.0f);
	player_ent->transform.rotation = quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	player_ent->scale = vec3(1.0f, 1.0f, 1.0f);
	player->ar_gondola = game->ps.ar_gondola;
	
	player->character_point = pos;
	player->character_model_point = player->character_point;
	
	player->relative_character_ray_origin = vec3(0.0f, 3.0f, 0.0f);
	player->character_ray_direction = vec3(0.0f, -1.0f, 0.0f);
	player->inv_character_ray_direction = vec3(0.0f, -1.0f, 0.0f);
	
	player->character_control_direction = vec3(0.0f, 0.0f, 0.0f);
	player->character_move_direction = vec3(0.0f, 0.0f, 0.0f);
	player->character_facing_direction = vec3(1.0f, 0.0f, 0.0f);
	
	player->hitnormal = vec3(0.0f, 1.0f, 0.0f);
	
	player->player_capsule.p = vec3(0.0f, 1.4f, 0.0f);
	player->player_capsule.q = vec3(0.0f, -0.0f, 0.0f);
	player->player_capsule.radius = 1.2f;
	player->player_capsule.num_contacts = 0;
	
	player->player_left_foot_fabrik_mix = 0.0f;
	player->player_right_foot_fabrik_mix = 0.0f;
	player->last_adjusted_foot_delta_left = vec3(0.0f, 0.0f, 0.0f);
	player->last_adjusted_foot_delta_right = vec3(0.0f, 0.0f, 0.0f);
	
	player->player_capsule_position_relative_to_player_point = vec3(0.0f, 3.0f, 0.0f);
	player->player_capsule_rotation = quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	update_base_entity(&game->assets, &enemy_collection.entities, enemy_idx);
	
}

void init_step(Game_Struct * game) {
	printf("perf address: %p\n", perf_measurements_gamestep);
	printf("perf num: %i\n", num_counter);
	
	perf_measurements_gamestep = game->gamestep_perf_msr;
	game->num_gamestep_perf_msr = num_counter;
	fp = game->fp_struct;
	
	strcpy(game->static_level_file_name, "1");
	
	dbg_list = &game->dbg_list;
	dbg_list_static = &game->dbg_list_static;
	dbg_list_non_cleared = &game->dbg_list_non_cleared;
	
	game->ns_instances.next_n = 0;
	
	game->game_points.next_point = 0;
	game->game_points.point_names.next_string = 0;
	reinit_map(&game->game_points.map);
	
	reset_level_static(&game->level_static);
	load_level_static(&game->level_static, &game->assets, &game->ns_instances, &game->game_points, "../levels/testlevel1/1.level");
	
	bool ok = false;
	
	uint testpoint1_id = map_lookup_key(&game->game_points.map, &ok, crc32_cstring("testpoint1"));
	Game_Point testpoint1 = game->game_points.points[testpoint1_id];
	
	uint testpoint2_id = map_lookup_key(&game->game_points.map, &ok, crc32_cstring("testpoint2"));
	Game_Point testpoint2 = game->game_points.points[testpoint2_id];
	
	platform_collection.platforms = malloc(sizeof(Platform)*MAX_PLATFORMS);
	platform_collection.entities = init_entity_collection(MAX_PLATFORMS);
	
	int ent_idx = create_base_entity_from_prototype_name(&game->assets, &game->ns_instances, &platform_collection.entities, "cube_obb.entity");
	
	Base_Entity *plat_ent = &platform_collection.entities.els[ent_idx];
	Platform *plat = &platform_collection.platforms[ent_idx];
	
	plat->p1 = testpoint1.point;
	plat->p2 = testpoint2.point;
	plat->direction = 0;
	plat->vel = vec3(0.0f, 0.0f, 0.0f);
	plat_ent->transform.translation = testpoint1.point;
	plat_ent->transform.rotation = quaternion(0,0,0,1.0f);
	plat_ent->scale = vec3(6.0f, 1.0f, 6.0f);
	
	update_base_entity(&game->assets, &platform_collection.entities, ent_idx);
	
	enemy_collection.enemies = malloc(sizeof(PlayerState)*MAX_PLAYERS);
	enemy_collection.entities = init_entity_collection(MAX_PLAYERS);
	
	create_gondola(game);
	
	for (int i = 0; i < 50; i++) {
		new_enemy(game, vec3(i*1.0f - 10.0f, 5.0f, 5.0f));
	}
}


// the gamestep is basically where all the game interaction happens
// it can be reloaded at runtime, which is pretty great, just remember to put the necessary stuff into the init step
void gamestep(Game_Struct * game) {
	START_TIME;
	
	game->next_model_to_render = 0;
	
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
	
	game->current_time = glfwGetTime();
	float dt = game->current_time - game->last_time;
	dt = 0.016f;
	
	game->last_time = game->current_time;
	
	if (game->level_editing_mode) {
		editor_step(game, dt);
		
		game->dbg_ctx.view = HMM_LookAt(game->es.editor_point, add_v3(game->es.editor_point, game->es.editor_forward), vec3(0.0f,1.0f,0.0f));
		
	} else {
		// do several player steps per frame
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
		
		enemy_collection_step(game, dt*4.0f);
		
		// enemy-enemy and player-enemy collision
		
		baka_capsule test_res_cap[enemy_collection.entities.num+1];
		v3 test_res_pos[enemy_collection.entities.num+1];
		Quat test_res_rot[enemy_collection.entities.num+1];
		v3 test_res_disp[enemy_collection.entities.num+1];
		
		test_res_cap[0] = game->ps.player_capsule;
		test_res_pos[0] = game->ps.character_point;
		test_res_rot[0] = quaternion(0,0,0,1);
		
		for (int i = 0; i < enemy_collection.entities.num; i++) {
			test_res_cap[i+1] = enemy_collection.enemies[i].player_capsule;
			test_res_pos[i+1] = enemy_collection.enemies[i].character_point;
			test_res_rot[i+1] = quaternion(0,0,0,1);
		}
		
		for (int i = 0; i < enemy_collection.entities.num; i++) {
			test_res_cap[i].num_dyn_contacts = 0;
		}
		
		new_character_resolution(test_res_cap, test_res_pos, test_res_rot, test_res_disp, enemy_collection.entities.num+1);
		
		for (int i = 0; i < enemy_collection.entities.num+1; i++) {
			
			test_res_disp[i] = baka_resolve_capsule_position_dynamic(&test_res_cap[i], test_res_pos[i]);
			
			// draw the contacts
			for (int j = 0; j < test_res_cap[i].num_dyn_contacts; j++)
			{
				mush_draw_sphere(dbg_list, test_res_cap[i].dyn_contacts[j].point, 0.2f, Yellow4);
			}
		}
		
		for (int i = 1; i < enemy_collection.entities.num+1; i++) {
			test_res_disp[i].y = 0.0f;
			test_res_disp[i] = mul_v3f(test_res_disp[i], 0.2f);
			// TODO: here we can also restrict the capsule movement, like we do with the player capsule
			test_res_pos[i] = add_v3(test_res_pos[i], test_res_disp[i]);
		}
		
		test_res_disp[0] = mul_v3f(test_res_disp[0], 0.2f);//0.8f);
		if (test_res_disp[0].y > 0.0f) {
			//cap_displacement[0].y = HMM_MIN(cap_displacement[0].y, 0.1f);
			if (game->ps.character_move_direction.y < 0.0f) {
				game->ps.character_move_direction.y -= game->ps.character_move_direction.y * 0.04f;
			}
		}
		v3 disp_cps = move_player_capsule_edit(game->ps, test_res_disp[0]);
		//game->ps.character_point = test_res_pos[0];
		game->ps.character_point = add_v3(game->ps.character_point, disp_cps);
		
		for (int i = 0; i < enemy_collection.entities.num; i++) {
			enemy_collection.enemies[i].character_point = test_res_pos[i+1];
		}
	}
	
	bool ok;
	uint testpoint2idx = map_lookup_key(&game->game_points.map, &ok, crc32_cstring("testpoint2"));
	if (ok) {
		Game_Point gp = game->game_points.points[testpoint2idx];
		
		mush_draw_sphere(
			dbg_list,
			gp.point,
			1.0f,
			vec4(1.0f,1.0,0.0f,1.0f));
	}
	
	calculate_entity_collection_transforms(game->ns_instances, &game->level_static.entities);
	
	for (int i = 0; i < game->level_static.entities.num; i++) {
		game->models_to_render[game->next_model_to_render] = game->level_static.entities.els[i].model;
		game->models_to_render_scales[game->next_model_to_render] = game->level_static.entities.els[i].scale;
		game->next_model_to_render++;
	}
	
	// render the enemy_collection
	
	calculate_entity_collection_transforms(game->ns_instances, &enemy_collection.entities);
	
	for (int i = 0; i < enemy_collection.entities.num; i++) {
		game->models_to_render[game->next_model_to_render] = enemy_collection.entities.els[i].model;
		game->models_to_render_scales[game->next_model_to_render] = enemy_collection.entities.els[i].scale;
		game->next_model_to_render++;
	}
	
	
	// render gondola 
	game->models_to_render[game->next_model_to_render] = game->ps.instantiated_gondola;
	game->models_to_render_scales[game->next_model_to_render] = game->ps.gondola_scale;
	game->next_model_to_render++;
	
	
	// render the platform
	calculate_object_transforms(
		game->ns_instances,
		&platform_collection.entities.els[0].model,
		&platform_collection.entities.els[0].transform,
		1);
	
	game->models_to_render[game->next_model_to_render] = platform_collection.entities.els[0].model;
	game->models_to_render_scales[game->next_model_to_render] = platform_collection.entities.els[0].scale;
	game->next_model_to_render++;
	
	game->rendering_context.view = game->dbg_ctx.view;
	game->terra.view = game->dbg_ctx.view;
	game->rendering_context.player_pos = game->ps.shadow_point;
	END_TIME;
}

int num_counter = __COUNTER__;
Perf_Measurement perf_measurements[__COUNTER__];