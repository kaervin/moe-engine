#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h> 
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include<dirent.h>
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

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"

#define HASHTABLE_IMPLEMENTATION
#include "hashtable.h"

#define CONVERTER_IMPLEMENTATION
#include "converter.hpp"

#include "glfw3.h"
#include "GL/gl3w.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "UI/nuklear.h"
#include "UI/nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024


GLFWwindow* window;

typedef struct {
	hmm_vec3 l;
	hmm_vec2 r;
    
	float rot_x;
	float rot_y;
	float rot_z;
    
	int resolve;
} Key_Struct;

Key_Struct processInput(GLFWwindow *window)
{
	Key_Struct k;
    
    
	float rot_x = 0.0f;
	if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)	 { rot_x = rot_x + 1.0f;}
	if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)	 { rot_x = rot_x - 1.0f;}
    
	float rot_y = 0.0f;
	if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)	 { rot_y = rot_y + 1.0f;}
	if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)	 { rot_y = rot_y - 1.0f;}
    
	float rot_z = 0.0f;
	if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)	 { rot_z = rot_z + 1.0f;}
	if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)	 { rot_z = rot_z - 1.0f;}
    
	hmm_vec3 l = HMM_Vec3(0.0f,0.0f,0.0f);
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)	 { l = HMM_Vec3( l.x , l.y + 1.0f , l.z );}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)	 { l = HMM_Vec3( l.x , l.y - 1.0f , l.z );}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)	 { l = HMM_Vec3( l.x + 1.0f , l.y , l.z );}
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)	 { l = HMM_Vec3( l.x - 1.0f , l.y , l.z );}
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)	 { l = HMM_Vec3( l.x , l.y , l.z + 1.0f );}
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)	 { l = HMM_Vec3( l.x , l.y , l.z - 1.0f );}
    
	hmm_vec2 r = HMM_Vec2(0.0f,0.0f);	 
	if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)	 { r = HMM_Vec2( r.x , r.y + 1.0f );}
	if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)	 { r = HMM_Vec2( r.x , r.y - 1.0f );}
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { r = HMM_Vec2( r.x + 1.0f , r.y );}
	if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)	 { r = HMM_Vec2( r.x - 1.0f , r.y );}
    
	k.resolve = 0;
	if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)  { k.resolve = 1;}
    
    
    
	k.l = l;
	k.r = r;
	k.rot_x = rot_x;
	k.rot_y = rot_y;
	k.rot_z = rot_z;
    
	return k;
}

bool Init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
	glfwWindowHint (GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint (GLFW_DOUBLEBUFFER, GL_TRUE);
    
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "baka", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return false;
	}
	
	glfwMakeContextCurrent(window);
	
	if (gl3wInit()) {
		return -1;
	}
	
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetWindowSizeLimits(window, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetWindowPos(window, 400, 400);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	glfwSwapInterval(1);
	
	return true;
}
// A quick dataype for having somewhere to store one-off strings,
// gets cleaned every frame
typedef struct StringStack {
    char* chars;
    uint next_string;
} StringStack;


void ui_show_genmodel(struct nk_context *ctx, GenModel *gm, NodeStack *ns, StringStack *ss, GenMeshStack *gms) {
	
	if (nk_begin(ctx, "info", nk_rect(5, 160, 250, 300),
				 NK_WINDOW_BORDER|NK_WINDOW_SCALABLE))
	{
		nk_layout_row_dynamic(ctx, 20, 1);
		
		for(int i = 0; i < gm->num_nodes ; i++) {
			
			const char* index_string = &ss->chars[ss->next_string];
			ss->next_string += 1+sprintf(&ss->chars[ss->next_string], "node id: %i", i);
			
			const char* parent_string = &ss->chars[ss->next_string];
			ss->next_string += 1+sprintf(&ss->chars[ss->next_string], " par id: %i", ns->parent[gm->node_idx + i]);
			
			int name_index = ns->name_indices[gm->node_idx+i];
			const char* node_name = (const char*)ns->names[name_index].chars;
			
			const char*  mesh_string = &ss->chars[ss->next_string];
			ss->next_string += 1+sprintf(&ss->chars[ss->next_string], "  mesh id: %i", ns->nodes[gm->node_idx + i].mesh);
			
			const char* skin_string = &ss->chars[ss->next_string];ss->next_string += 1+sprintf(&ss->chars[ss->next_string], "  skin id: %i", ns->nodes[gm->node_idx + i].skin);
			
			const char* texture_asset_string = &ss->chars[ss->next_string];ss->next_string += 1+sprintf(&ss->chars[ss->next_string], "  texture_asset: type %i nr %i", ns->texture_assets[gm->node_idx + i].type, ns->texture_assets[gm->node_idx + i].nr);
			
			const char* transf_transl_string = &ss->chars[ss->next_string];ss->next_string += 1+sprintf(&ss->chars[ss->next_string], "  loc: %f %f %f", ns->nodes[gm->node_idx + i].loc_transform.translation.X, ns->nodes[gm->node_idx + i].loc_transform.translation.Y, ns->nodes[gm->node_idx + i].loc_transform.translation.Z);
			
			const char* object_transl_string = &ss->chars[ss->next_string];ss->next_string += 1+sprintf(&ss->chars[ss->next_string], "  obj: %f %f %f", ns->nodes[gm->node_idx + i].object_transform.translation.X, ns->nodes[gm->node_idx + i].object_transform.translation.Y, ns->nodes[gm->node_idx + i].object_transform.translation.Z);
			
			nk_label(ctx, index_string, NK_TEXT_LEFT);
			nk_label(ctx, node_name, NK_TEXT_LEFT);
			nk_label(ctx, parent_string, NK_TEXT_LEFT);
			
			nk_label(ctx, mesh_string, NK_TEXT_LEFT);
			if(ns->nodes[gm->node_idx + i].mesh != -1) {
				const char* mesh_name = (const char *)gms->names[gm->mesh_idx + ns->nodes[gm->node_idx + i].mesh].chars;
				
				nk_label(ctx, mesh_name, NK_TEXT_LEFT);
			}
			
			
			nk_label(ctx, skin_string, NK_TEXT_LEFT);
			nk_label(ctx, texture_asset_string, NK_TEXT_LEFT);
			nk_label(ctx, transf_transl_string, NK_TEXT_LEFT);
			nk_label(ctx, object_transl_string, NK_TEXT_LEFT);
			
		}
	}
	nk_end(ctx);
}

struct ui_selected {
    int anim1, anim2;
    float mix;
};

// returns the index of the selected animation, shows the names of the animations
void ui_animation_selection(struct nk_context *ctx, AnimReturn *ar, struct ui_selected * selected) {
    
    if (nk_begin(ctx, "animation", nk_rect(5, 5, 250, 150),
                 NK_WINDOW_BORDER))
    {
        nk_layout_row_dynamic(ctx, 20, 2);
        
        nk_label(ctx,"mix:", NK_TEXT_LEFT);
        nk_slider_float(ctx, 0.0f, &selected->mix, 1.0f, 0.1f);
        
        nk_label(ctx,"anim 1", NK_TEXT_LEFT);
        nk_label(ctx,"anim 2", NK_TEXT_LEFT);
        
        for (int i = 0; i < ar->descr.num_anims; i++) {
            
            const char *animation_name1 = (const char*)ar->as.anims[ar->descr.anims+i].name.chars;
            if (nk_option_label(ctx, animation_name1, selected->anim1 == i)) {
                selected->anim1 = i;
            }
            const char *animation_name2 = (const char*)ar->as.anims[ar->descr.anims+i].name.chars;
            if (nk_option_label(ctx, animation_name2, selected->anim2 == i)) {
                selected->anim2 = i;
            }
        }
    }
    nk_end(ctx);
}


void render_skeleton(mush_draw_list *dbg_list, NodeStack *ns, GenModel *skeleton) {
    
    // Render our NodeHierachy,
    // We might also consider just rendering the skeleton by looking at skins or sonething
    // but it's not necessary right now
    
    for(int i = 0; i < skeleton->num_nodes; i++) {
        int nidx = skeleton->node_idx + i;
        Node* current_node = &ns->nodes[nidx];
        
        hmm_vec3 startpoint = current_node->object_transform.translation;
        hmm_quaternion rotation = current_node->object_transform.rotation;
        
        hmm_vec3 endpoint = HMM_Vec3(0.0f, 1.0f, 0.0f);
        endpoint = rotate_vec3_quat(endpoint, rotation);
        endpoint = endpoint + startpoint;
        
        mush_draw_sphere(
            dbg_list,
            startpoint,
            0.1f,
            HMM_Vec4(0.0f, 0.0f, 1.0f, 0.5f));
        
        mush_draw_segment(
            dbg_list,
            startpoint,
            endpoint,
            HMM_Vec4(1.0f, 0.0f, 0.0f, 1.0f),
            HMM_Vec4(1.0f, 0.0f, 0.0f, 0.1f));
    } 
}

// TODO 
// T E X T U R E S
// model some world stuff

// IDEA: for the ground and possibly other large structures, where we want tiled textures,
// we could create a new renderer type, which has a texture atlas of tiles
// and a map texture, which basically tells us at which point which tile should be drawn with which proportion

// arguments: 
// 0: in_name - the name that gets converted
// 1: out_name - output name without file extension
int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        fprintf( stderr, "no file names given");
        return 1;
    }
    
    char *in_name = argv[1];
    char *out_name = argv[2];
    
    char out_name_animationpack[strlen(out_name)+5];
    char out_name_modelpack[strlen(out_name)+5];
    
    snprintf(out_name_modelpack,strlen(out_name)+5,"%s.moe", out_name);
    
    snprintf(out_name_animationpack,strlen(out_name)+5,"%s.moa", out_name);
    
    
    // The GenModelPack contains all the Data pertaining to possibly multiple models.
    // the GenModels themselves contain the indices and lengths that pertain to that specific model
    // for example the Nodes that belong to a specific model are stored contiguously in the NodeStack
    // starting at node_idx with a length of num_nodes
    // to 'instantiate' such a model then, one would copy the portion of the NodeStack into a different one reserved for instances (here called ns_instances)
    // this can be done with the function instantiate_genmodel
    
    printf("loading model in: %s\n", in_name);
    
	CombinedModelPack cmb_pack = file_to_combinedmodelpack(in_name);
	
    int has_animation = 0;
    
	AnimReturn ar_from_gltf = cmb_pack.animreturn;
	if ( ar_from_gltf.as.next_anim != 0 ) {
		has_animation = 1;
        printf("has animation\n");
        
        write_animations_file(ar_from_gltf,out_name_animationpack);
    }
    
	GenModelPack gmr_from_gltf = cmb_pack.genmodelpack;
	
    printf("loaded model\n");
	
	// --- try to save the modelreturn
	write_model_ressource_file(&gmr_from_gltf, out_name_modelpack);
	
	if (!Init()) { return -1;}
    
    // --- Nuklear UI stuff
    struct nk_colorf bg;
    struct nk_context *ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
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
	printf( "gl version %s \n", glGetString(GL_VERSION));
	int texture_array_layers = 0;
	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &texture_array_layers);
	printf("max array texture depth %i \n", texture_array_layers);
    
	// --- rendering init stuff
    
	
	ModelMemory model_memory = init_model_memory();
	GenModel test_model = load_model_into(&model_memory, out_name_modelpack);
	
	
	AnimReturn ar;
	if(has_animation) {
		ar = load_animation_test(out_name_animationpack);
		printf("loaded animationpack\n");
	}
	
	
	Texture_Asset_Manager texture_asset_manager = gather_texture_assets();
	
	for (int i = 0; i < 7; i++) {
		printf("next_texture %i: %i\n", i, texture_asset_manager.next_texture[i]);
	}
	
	bool ok;
	unsigned int texture_ident;
	
	texture_ident = map_lookup_key(&texture_asset_manager.map, &ok, crc32_cstring("Material.png"));
	struct texture_asset ta_material = to_texture_asset(texture_ident);
	printf("Material texture_ident %u %u %u %u\n", ok, texture_ident, ta_material.type, ta_material.nr);
	
	texture_ident = map_lookup_key(&texture_asset_manager.map, &ok, crc32_cstring("Face2.png"));
	struct texture_asset ta_face4 = to_texture_asset(texture_ident);
	printf("face texture_ident %u %u %u %u\n", ok, texture_ident, ta_face4.type, ta_face4.nr);
	
	// the second arguement is basically the maximum amount of nodes
	Rendering_Context_Gen rendering_context = prepare_ressources_gen(model_memory.mesh_stack, &texture_asset_manager, 30000);
	
	rendering_context.projection = BuildProjectionMatrix(
		1.0f,
		0.9f,
		0.2f,
		10000.0f);
    
	NodeStack ns_instances;
	ns_instances.max_n = 100;
	ns_instances.next_n = 0;
	ns_instances.nodes = (Node *)malloc( sizeof(Node) * 100 );
	ns_instances.parent = (int *)malloc( sizeof(int) * 100 );
    ns_instances.texture_assets = (texture_asset *)malloc( sizeof(texture_asset)*100 );
	ns_instances.names = model_memory.node_stack.names; // NOTE: for now we just do this
    // When we systemize everything in the compression phase, I will think of something better
    ns_instances.name_indices = (int *)malloc(sizeof(int)*100);
    
    
	GenModel instantiated_genmodel;
	hmm_transform world_transform;
    hmm_vec3 object_scale;
    
	bool succ = instantiate_genmodel(
		model_memory.node_stack,
		test_model,
		&ns_instances,
		&instantiated_genmodel);
	
	set_node_texture_asset_by_name(&ns_instances, &test_model, "Plane", ta_face4);
	set_node_texture_asset_by_name(&ns_instances, &test_model, "Cube", ta_material);
	
	if (!succ) {
		printf("succ failed to instantiate\n");
	}
	
	object_scale = HMM_Vec3(1.0f, 1.0f, 1.0f);
	
    
    // --- some Graphics debug stuff
    // init the context and create space for the specific debug shapes
	mush_context dbg_ctx = mush_init();
    
	mush_draw_list dbg_list;
	mush_draw_list dbg_list_static;
    
	mush_alloc_forms(&dbg_list, 1000);
	mush_alloc_forms(&dbg_list_static, 1000);
	
	dbg_ctx.projection = BuildProjectionMatrix(
		1.0f,
		0.9f,
		0.2f,
		10000.0f);
    
	hmm_vec3 camera_point = HMM_Vec3(0.0f, 0.0f, 10.0f);
	hmm_vec3 camera_right = HMM_Vec3(1.0f, 0.0f, 0.0f);
    
	dbg_ctx.view = HMM_LookAt(camera_point, HMM_Vec3(0.0f,0.0f,0.0f), HMM_Vec3(0.0f,1.0f,0.0f));
    
	Key_Struct keys;
    
	// draw coordinate-sys
	mush_draw_segment(
		&dbg_list_static,
		HMM_Vec3(0.0f, 0.0f, 0.0f),
		HMM_Vec3(1.0f, 0.0f, 0.0f),
		HMM_Vec4(1.0f, 0.0f, 0.0f, 1.0f),
		HMM_Vec4(1.0f, 0.0f, 0.0f, 1.0f));
	mush_draw_segment(
		&dbg_list_static,
		HMM_Vec3(0.0f, 0.0f, 0.0f),
		HMM_Vec3(0.0f, 1.0f, 0.0f),
		HMM_Vec4(0.0f, 1.0f, 0.0f, 1.0f),
		HMM_Vec4(0.0f, 1.0f, 0.0f, 1.0f));
	mush_draw_segment(
		&dbg_list_static,
		HMM_Vec3(0.0f, 0.0f, 0.0f),
		HMM_Vec3(0.0f, 0.0f, 1.0f),
		HMM_Vec4(0.0f, 0.0f, 1.0f, 1.0f),
		HMM_Vec4(0.0f, 0.0f, 1.0f, 1.0f));
    
    
	int framecounter = 0;
	int last_resolve = 0;
	int do_jump = 0;
	int jumpcounter = 0;
    
	struct timespec prev;
    
	double currentTime = 0.0;
    
    // Where we save our strings, currently used for the UI, 
    // don't know how useful this will be for other stuff
    StringStack string_stack;
    string_stack.chars = (char *)malloc(sizeof(char)*10000);
    string_stack.next_string = 0;
    
    
    struct ui_selected selected_animation = {0,0,0.1f};
    
	while (!glfwWindowShouldClose(window)) {
        
        // clean the string_stack
        string_stack.next_string = 0;
        
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		
		keys = processInput(window);
        
		currentTime = glfwGetTime();
        
		// time stuf 
        
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC,&now);
		float elapsed = (float)((double)(now.tv_sec - prev.tv_sec) + ((double)(now.tv_nsec - prev.tv_nsec) * (1.0 / 1000000.0)));
		prev = now;
        
		// adjust camera
        
		// first rotate around y axis
		hmm_quaternion quat_right = HMM_QuaternionFromAxisAngle(HMM_Vec3(0.0f,1.0f,0.0f), keys.r.x*-0.05);
        
		camera_point = rotate_vec3_quat(camera_point, quat_right);
		camera_right = rotate_vec3_quat(camera_right, quat_right);
        
		// then rotate around camera right
		hmm_quaternion quat_up = HMM_QuaternionFromAxisAngle(camera_right, keys.r.y*-0.05);
        
		camera_point = rotate_vec3_quat(camera_point, quat_up);
        
		dbg_ctx.view =HMM_LookAt(HMM_AddVec3(camera_point, HMM_Vec3(0.0f, 2.0f, 0.0f)), HMM_Vec3(0.0f,2.0f,0.0f),HMM_Vec3(0.0f,1.0f,0.0f));
		
		rendering_context.view = dbg_ctx.view;
        
		if(has_animation) {
			// select the animation
			ui_animation_selection(ctx, &ar, &selected_animation);
			
			float animtime = fmod(currentTime, ar.as.anims[selected_animation.anim1].last_time); 
			
			
			// the combinedanimationstate is a way to mix animations
			CombinedAnimationState c_animstate;
			for (int j = 0; j < 4; ++j)
			{
				c_animstate.animation_states[j].anim_index = 0;
				c_animstate.animation_states[j].time = animtime;				
			}
			c_animstate.weight = HMM_Vec4(selected_animation.mix,1-selected_animation.mix, 0.0f, 0.0f);
			c_animstate.animation_states[0].anim_index = selected_animation.anim1;
			c_animstate.animation_states[1].anim_index = selected_animation.anim2;
			
			animate_model_test(
				&(model_memory.node_stack),
				test_model,
				&ns_instances,
				instantiated_genmodel,
				&(ar.as),
				c_animstate);
		}
		
		world_transform.translation = HMM_Vec3(0.0f, 0.0f, 0.0f);
		world_transform.rotation = HMM_QuaternionFromAxisAngle(HMM_Vec3(1.0f,1.0f,1.0f), 0.0f);
		
        
        // calculate_object_transforms calculates the node-local transforms according to the hierachy
        calc_glo(
			ns_instances,
			instantiated_genmodel,
			world_transform);
        
		GenModel instances_to_render[1];
		instances_to_render[0] = instantiated_genmodel;
		
		hmm_transform transforms_to_render[1];
		transforms_to_render[0] = world_transform;
		
		hmm_vec3 scale_to_render[1];
		scale_to_render[0] = object_scale;
		
        // as the name implies, render_genmodels renders our models
		render_genmodels(
			rendering_context,
			ns_instances,
			model_memory.skin_stack,
			instances_to_render,
			scale_to_render,
			1);
		
        
        glClear(GL_DEPTH_BUFFER_BIT);
        
        render_skeleton(&dbg_list, &ns_instances, &instantiated_genmodel);
        
        mush_render(dbg_ctx, &dbg_list);
        mush_render(dbg_ctx, &dbg_list_static);
        mush_empty_list(&dbg_list);
        
        // NUKLEAR GUI TEST STUFF
        nk_glfw3_new_frame();
        ui_show_genmodel(ctx, &instantiated_genmodel, &ns_instances, & string_stack, &model_memory.mesh_stack);
        nk_glfw3_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
        glfwSwapBuffers(window);			
		
		//sleep(1);
        
		glfwPollEvents();
		framecounter++;
	}
	glfwTerminate();	
	return 0;
}