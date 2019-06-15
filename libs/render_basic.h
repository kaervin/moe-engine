#include "HandmadeMath.h"
#include <float.h>
#include "typos.h"
//#include "hashtable.h"
#include "stb_image.h"

#define FUN

#ifdef RENDER_TYPES

#define NUM_BONES_PER_VERTEX 4

// --- model data definitions ---

typedef struct Vertex {
	float x, y, z;
	float nx, ny, nz;
	float s0, t0;
} Vertex;

typedef struct S_Vertex {
	float x, y, z;
	float nx, ny, nz;
	float s0, t0;
	uint b_index[NUM_BONES_PER_VERTEX];
	float b_weight[NUM_BONES_PER_VERTEX];
} S_Vertex;

// Ressource_Names are for storing names of stuff like Nodes and Meshes
// they are 100 bytes long plus a null-terminator, so 101 in total
typedef struct Resource_Name {
	char chars[101];
} Resource_Name;

// TODO: we temporarily just pair a mesh with a texture
// this way we can't vary textures between meshes, which is bad, but this is just a test
// restructure it in a way, where every instanced nodestack gets its own extra place for these
// this will be when we split prototype and instanced nodestacks
// hopefully this will make things a little less confusing and the API clearer

typedef struct Texture_Asset_Manager {
	Map map;
	unsigned char *textures[7];
	unsigned int next_texture[7];
} Texture_Asset_Manager;

typedef struct texture_asset {
	unsigned int type, nr;
} texture_asset;

typedef struct GenMesh {
	uint num_v;
	uint ndx_v;
    
	uint num_i;
	uint ndx_i;
    
	// skinned indicates if this indexes the s_verts or normal verts
	bool skinned;
} GenMesh;

typedef struct GenMeshStack {
	uint max_m;
	uint max_v;
	uint max_i;
	uint max_sv;
	uint max_si;
	// next index we can use
	uint next_m;
	uint next_v;
	uint next_i;
	uint next_sv;
	uint next_si;
    
    // Names of the individual meshes
    Resource_Name * names; 
    
	// meshes of all the loaded models
	// index into it to retrieve vertices and such
	GenMesh * meshes;
    
	// vertices of all the models
	Vertex * vertices;
	// and indices
	uint * indices;
    
	// vertices of all the models
	S_Vertex * s_vertices;
	// and indices
	uint * s_indices;
} GenMeshStack;

// A node of an animatable Hierachy
// basically structures meshes and their skeletons
typedef struct Node {
	int mesh;
	int skin;
    
	// TODO: we might potentially get rid of one transform, 
	// since they aren't ever really needed at the same time I think
    // Had a later look at it: I don't see how we could get rid of one, 
    // but maybe I saw something back then that mighht have been a good idea, so I leave the todo for now
	// 25th of November: actually thinking about it local transforms are set by the animation
    // after that we transfer those with multiplication of the parent into object transforms
    // maybe saving it into loc_transform instead we could actually not need object transform
    // It really doesn't seem to be used after that process.
	// TODO2 splitting this data up could be a simple speed up as well. This way we are touching too much data we don't need
    Transform loc_transform;
    
	Transform object_transform;
} Node;

// Using a Nodestack as a universal tool for loading, but also for instances might be a bit confusing
// So maybe splitting the data_structure into two, a prototypical nodestack, which gets loaded
// and an instanced nodestack in which the instances are saved could be beneficial
// data which will be similar across many Nodes can be easily reused this way too, which is beneficial for the cache
// but maybe flexibility will suffer a bit.
// TODO: maybe do split them
typedef struct NodeStack {
	uint max_n;
	uint next_n;
    
    // The situation with the names for Nodes is a bit more complicated, since
    // we instantiate the nodes, and don't want to copy names unnecessarily.
    // What we do is have a name_number, which is an index into the resource names
    // this way different instantiated nodes can share one common name
    // This name index actually also shows us from what node in the loaded nodestack a specific node was instantiated from, maybe we could use this for something actually
    // for now we'll leave it at the name 'name_indices' though we might change that when the use changes 
    Resource_Name * names;
    int * name_indices;
    
	texture_asset * texture_assets;
	int * parent;
	Node * nodes;
    
} NodeStack;

typedef struct Skin {
	uint idx; // index and num into inv_binds
	uint num;
} Skin;

typedef struct SkinVal {
	uint node;
	mat4 inv_bind;
} SkinVal;

typedef struct SkinStack {
	uint max_skin;
	uint max_val;
    
	uint next_skin;
	uint next_val;
    
	Skin * skins;
	SkinVal * skin_vals;
} SkinStack;

typedef struct GenModel {
	int num_meshes;
	int num_nodes;
	int num_skin;
    
	uint mesh_idx;
	uint node_idx;
	uint skin;
} GenModel;

typedef struct GenModelPack {
	GenModel gm;
	GenMeshStack gms;
	NodeStack ns;
	SkinStack ss;
} GenModelPack;

typedef struct ModelMemory {
	GenModel *genmodels;
	uint next_genmodel;
	uint max_genmodel;
	Map model_map;
	
	GenMeshStack mesh_stack;
	NodeStack node_stack;
	SkinStack skin_stack;
} ModelMemory;
// --- animation descriptions


typedef struct TranChannel {
    
	// target node
	uint target;
	// number of keyframes and indices to them in the stack
	uint len;
	uint time_idx;
	uint idx;
} TranChannel;

typedef struct RotChannel {
    
	// target node
	uint target;
	// number of keyframes and indices to them in the stack
	uint len;
	uint time_idx;
	uint idx;
} RotChannel;

typedef struct Anim {
    Resource_Name name;
    
	float last_time;
    
	uint num_tranch;
	uint num_rotch;
    
	uint tranch;
	uint rotch;
} Anim;

typedef struct AnimDescr {
	int num_anims;
	int anims;
} AnimDescr;

// TODO: Currently animations and models are seperate, this way we can reuse animations for different models
// This makes everything a bit more complicated though, so think about if it should continue to be this way 
// The names are a bit confusing currently too, so I should fix that in the next compression phase
// The whole interface is a bit shitty I think. Just not obvious enough, especially the whole AnimReturn thing
// Why does it even need to exist? Maybe I had a good idea sometime, but it's not obvious
// But since I'm lazy I'll take care of that tomorrow, lol
typedef struct AnimStack {
    
	uint max_anims;
    
	uint max_tranch;
	uint max_rotch;
    
	uint max_times;
    
	uint max_tran;
	uint max_rot;
    
	uint next_anim;
	Anim * anims;
    
	uint next_tranch;
	TranChannel * tranchs;
    
	uint next_rotch;
	RotChannel * rotchs;
    
	uint next_time;
	float * times;
    
	uint next_trans;
	uint next_rot;
    
	v3 * trans;
	Quat * rots;
} AnimStack;

typedef struct AnimReturn {
	AnimDescr descr;
    
	AnimStack as;
} AnimReturn;

typedef struct AnimationState {
	uint anim_index;
	float time;
} AnimationState;

enum ASplaytype {
	DONT_PLAY,
	PLAY_ONCE,
	LOOP,
};

typedef struct CombinedAnimationState {
	AnimationState animation_states[4];
	v4 weight;
} CombinedAnimationState;

// --- model rendering definitions

// the basic batched opengl draw command struct
typedef struct DrawElementsIndirectCommand {
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint baseVertex;
	uint baseInstance;
} DrawElementsIndirectCommand;

typedef struct SSBO_Data {
	v4 position;
	Quat transform;
    v4 scale;
	
	// TODO: the texture identifier could be one value instead, so we can optimize that
	// most GPU stuff could need some optimization eventually anyway
	int texture_type;
	int texture_nr;
} SSBO_Data; 


typedef struct SSBO_Data_Skin {
	v4 position;
	Quat transform;
	int root_joint;
	// TODO: the texture identifier could be one value instead, so we can optimize that
	// most GPU stuff could need some optimization evantually anyway
	int texture_type;
	int texture_nr;
	float __padding;
} SSBO_Data_Skin;


typedef struct SSBO_Joint {
	mat4 jointMat;
} SSBO_Joint;


typedef struct RenderInfo_Stat {
	
	unsigned int shaderProgram;
	
	int viewLoc;
	int projectionLoc;
	
	unsigned int vbo, vao, indirect, ssbo_pos, ebo;
	
	int ssbo_data_len;
	SSBO_Data * ssbo_data;
} RenderInfo_Stat;

typedef struct RenderInfo_Skin {
	
	unsigned int shaderProgram;
	
	int viewLoc;
	int projectionLoc;
	
	unsigned int vbo, vao, indirect, ssbo_pos, ssbo_joint_pos, ebo;
	
	int ssbo_data_len;
	SSBO_Data_Skin * ssbo_data_skin;
	
	SSBO_Joint * ssbo_joint;
} RenderInfo_Skin;

typedef struct Rendering_Context_Gen {
	
	GenMeshStack gms;
	
	unsigned int gl_texture_ids[7];
	
	// write to these to change them
	mat4 view;
	mat4 projection;
	
	v3 player_pos;
	
	RenderInfo_Stat info_stat;
	RenderInfo_Skin info_skin;
} Rendering_Context_Gen;

#endif

#ifndef RENDER_H
#define RENDER_H

ModelMemory init_model_memory();

GenModelPack load_modelpack(
const char * filename);


AnimReturn load_animation_test(
const char * filename);

void animate_nodes(
NodeStack * ns,
AnimStack * as,
GenModel gm,
AnimDescr ad,
uint ai,
float time );

void animte_model(
NodeStack * ns,
AnimStack * as,
GenModel gm,
AnimationState animstate);


void animate_model(
NodeStack * ns_src,
GenModel gm_src,
NodeStack * ns_result,
GenModel gm_result,
AnimStack * as,
CombinedAnimationState c_animstate);

RenderInfo_Skin prepare_skin(
GenMeshStack gms,
uint ssbo_data_len);

RenderInfo_Stat prepare_stat(
GenMeshStack gms,
uint ssbo_data_len);

Rendering_Context_Gen prepare_ressources_gen(
GenMeshStack gms,
Texture_Asset_Manager *texture_asset_manager,
uint ssbo_data_len);

void render_genmodels(
Rendering_Context_Gen rectx,
NodeStack ns,
SkinStack ss,
GenModel * gma,
v3 * model_scales,
uint num_models);

void render_Info_Skin(
Rendering_Context_Gen rectx,
NodeStack ns,
SkinStack ss,
GenModel * gma,
Transform * world_transforms,
v3 * model_scales,
uint num_models);

void render_Info_Stat(
Rendering_Context_Gen rectx,
NodeStack ns,
SkinStack ss,
GenModel * gma,
Transform * world_transforms,
v3 * model_scales,
uint num_models);

GenModel load_model_into(ModelMemory *mm, const char *filename);


Texture_Asset_Manager gather_texture_assets();


#endif /* RENDER_H */


#ifdef RENDER_IMPLEMENTATION

FUN bool instantiate_genmodel(
NodeStack ns_src,
GenModel gm_src,
NodeStack * ns_dest,
GenModel * gm_dest);


FUN void set_node_texture_asset_by_name(NodeStack *ns, GenModel *genmodel, const char* node_name, texture_asset tex);

FUN struct texture_asset to_texture_asset(unsigned int value);

FUN static void animate_model_test(
NodeStack * ns_src,
GenModel gm_src,
NodeStack * ns_result,
GenModel gm_result,
AnimStack * as,
CombinedAnimationState c_animstate);

FUN void solve_IK_fabrik(NodeStack *ns, GenModel *model, uint target_node, uint chain_length, v3 target_pos_delta, uint pole_target, float mix);

FUN GenModel get_model_from_crc32(ModelMemory *mm, bool *ok, uint key);


FUN void calculate_object_transforms(NodeStack ns, GenModel * gma, Transform * world_transforms,uint num_models);


void set_node_texture_asset_by_name(NodeStack *ns, GenModel *genmodel, const char* node_name, texture_asset tex) {
	for(int i = 0; i < genmodel->num_nodes; i++) {
		int current_node_name_index = ns->name_indices[genmodel->node_idx+i];
		Resource_Name * current_node_name = &ns->names[current_node_name_index];
		
		if (!strcmp(node_name, current_node_name->chars)) {
			ns->texture_assets[genmodel->node_idx+i] = tex;
			break;
		}
	}
}


struct texture_asset to_texture_asset(unsigned int value) {
	struct texture_asset a;
	a.nr = value & 0x000000FF;
	a.type = (value >> 8);
	return a;
}

Texture_Asset_Manager gather_texture_assets() {
	
	printf("\ngathering textures\n");
	
	Texture_Asset_Manager tax;
	
	tax.map = init_map();
	
	int allocation_size[7];
	for (int i = 0; i < 7; i++) {
		allocation_size[i] = 0;
	}
	// add space for the 'missing texture' texture
	allocation_size[0] = 16*16*4;
	
	int texwidth, texheight, texn;
	
	DIR *textures_directory;
	struct dirent *directory_entry;
	textures_directory = opendir("../textures");
	
	while(NULL != (directory_entry = readdir(textures_directory))) {
		if (directory_entry->d_type != DT_REG) {
			continue;
		}
		
		char dirnamebuffer[strlen(directory_entry->d_name) + sizeof("../textures/")];
		sprintf(dirnamebuffer, "../textures/%s", directory_entry->d_name);
		
		if (stbi_info(dirnamebuffer, &texwidth, &texheight, &texn)) {
			
			if (texwidth != texheight) {
				printf("texture is not quadratic, skipped\n");
				continue;
			}
			
			if (!(texwidth == 16 || texwidth == 32 || texwidth == 64 || texwidth == 128 || texwidth == 256 || texwidth == 512 || texwidth == 1024)) {
				printf("texturesize %i is not supported, skipped\n", texwidth);
				continue;
			}
			
			int size_index = ((int)log2(texwidth)) - 4; // -4 sice the first log2 value of 16 is 4
			allocation_size[size_index] += texwidth*texwidth*4;
		}
	}
	
	for (int i = 0; i < 7; i++) {
		tax.textures[i] = (unsigned char*)malloc(allocation_size[i]);
		tax.next_texture[i] = 0;
	}
	// add the 'missing texture' texture, which is just magenta
	for(int i = 0; i < 16*16*4; i = i+4) {
		tax.textures[0][i] = 0xFF;
		tax.textures[0][i+1] = 0x00;
		tax.textures[0][i+2] = 0xFF;
		tax.textures[0][i+3] = 0xFF;
	}
	tax.next_texture[0] = 1;
	
	rewinddir(textures_directory);
	while(NULL != (directory_entry = readdir(textures_directory))) {
		if (directory_entry->d_type != DT_REG) {
			continue;
		}
		
		char dirnamebuffer[strlen(directory_entry->d_name) + sizeof("../textures/")];
		
		sprintf(dirnamebuffer, "../textures/%s", directory_entry->d_name);
		printf("\t%s\n", dirnamebuffer);
		
		unsigned char * texture_data = stbi_load(dirnamebuffer, &texwidth, &texheight, &texn, 4);
		
		if (texture_data == NULL) {
			continue;
		}
		
		if (texwidth != texheight) {
			continue;
		}
		
		if (!(texwidth == 16 || texwidth == 32 || texwidth == 64 || texwidth == 128 || texwidth == 256 || texwidth == 512 || texwidth == 1024)) {
			continue;
		}
		
		unsigned int data_size = texwidth * texwidth * 4;
		unsigned int size_index = ((int)log2(texwidth)) - 4;
		
		int nxt_data = tax.next_texture[size_index] * data_size;
		memcpy(&tax.textures[size_index][nxt_data], texture_data, data_size);
		
		printf("\tsize_index: %u, next_tex: %u\n", size_index, tax.next_texture[size_index]);
		
		unsigned int value = (size_index << 8) + (tax.next_texture[size_index] & 0x000000FF);
		unsigned int key = crc32_cstring(directory_entry->d_name);
		
		printf("\tcrc32 %s %u %u\n", directory_entry->d_name, key, value);
		
		unsigned int ok = map_insert_kv(&tax.map, key, value);
		printf("\tinsert ok %u\n\n", ok);
		
		tax.next_texture[size_index]++;
	}
	
	closedir(textures_directory);
	
	printf("gathered textures; next_textures:\n");
	for (int i = 0; i < 7; i++) {
		printf("\tnext_texture %i: %i\n", i, tax.next_texture[i]);
	}
	printf("\n");
	return tax;
}



ModelMemory init_model_memory() {
	
	ModelMemory mm;
	
	GenMeshStack mesh_stack;
	NodeStack node_stack;
	SkinStack skin_stack;
	
	// TODO: for now we just give it some values
	// we could figure out the necessary size by doing a prepass on all the files,
	// but for testing this is enough
	
	
	
	mesh_stack.max_m = 1000;
	mesh_stack.max_v = 1024 << 8;
	mesh_stack.max_i = 1024 << 8;
	mesh_stack.max_sv = 1024 << 8;
	mesh_stack.max_si = 1024 << 8;
	
	node_stack.max_n = 1000;
	
	skin_stack.max_skin = 1000;
	skin_stack.max_val = 1000;
	
	mesh_stack.next_m = 0;
	mesh_stack.next_v = 0;
	mesh_stack.next_i = 0;
	mesh_stack.next_sv = 0;
	mesh_stack.next_si = 0;
	
	node_stack.next_n = 0;
	
	skin_stack.next_skin = 0;
	skin_stack.next_val = 0;
	
	
	mesh_stack.names = (Resource_Name*)malloc( sizeof(Resource_Name)*mesh_stack.max_m );
	mesh_stack.meshes = (GenMesh*) malloc( sizeof(GenMesh)*mesh_stack.max_m );
	mesh_stack.vertices = (Vertex*) malloc( sizeof(Vertex)*mesh_stack.max_v );
	mesh_stack.indices = (uint*) malloc( sizeof(uint)*mesh_stack.max_i );
	mesh_stack.s_vertices = (S_Vertex*) malloc( sizeof(S_Vertex)*mesh_stack.max_sv );
	mesh_stack.s_indices = (uint*) malloc( sizeof(uint)*mesh_stack.max_si);
	
	node_stack.names = (Resource_Name*) malloc( sizeof(Resource_Name)*node_stack.max_n );
	node_stack.name_indices = (int*) malloc( sizeof(int)*node_stack.max_n );
	node_stack.texture_assets = (texture_asset*)malloc(sizeof(texture_asset)*node_stack.max_n);
	node_stack.parent = (int*) malloc( sizeof(int)*node_stack.max_n );
	node_stack.nodes = (Node*) malloc( sizeof(Node)*node_stack.max_n );
	
	skin_stack.skins = (Skin*) malloc( sizeof(Skin)*skin_stack.max_skin );
	skin_stack.skin_vals = (SkinVal*)  malloc( sizeof(SkinVal)*skin_stack.max_val );
	
	memset(node_stack.texture_assets, 0, sizeof(texture_asset)*node_stack.max_n);
	
	mm.mesh_stack = mesh_stack;
	mm.node_stack = node_stack;
	mm.skin_stack = skin_stack;
	
	mm.next_genmodel = 0;
	mm.max_genmodel = 1024;
	mm.genmodels = (GenModel*)malloc(sizeof(GenModel)*mm.max_genmodel);
	mm.model_map = init_map();
	
	return mm;
	
}

GenModel load_model_into(ModelMemory *mm, const char *filename) {
	
	GenMeshStack *gms = &mm->mesh_stack;
	NodeStack *ns = &mm->node_stack;
	SkinStack *ss = &mm->skin_stack;
	
	FILE * mfile = fopen( filename, "rb" );
	
	// find out the length of what we are going to load
	int numbers[8];
	
	int rlen = fread(numbers, sizeof(int), 8, mfile);
	
	int number_of_meshes = numbers[0];
	int number_of_vertices_static = numbers[1];
	int number_of_indices_static = numbers[2];
	int number_of_vertices_skinned = numbers[3];
	int number_of_indices_skinned = numbers[4];
	int number_of_nodes = numbers[5];
	int number_of_skins = numbers[6];
	int number_of_skin_values = numbers[7];
	
	GenModel model;
	
	model.num_meshes = number_of_meshes;
	model.num_nodes = number_of_nodes;
	model.num_skin = number_of_skins;
	
	model.mesh_idx = gms->next_m;
	model.node_idx  = ns->next_n;
	model.skin = ss->next_skin;
	
    fread(&gms->names[gms->next_m], sizeof(Resource_Name), number_of_meshes, mfile);
	fread(&gms->meshes[gms->next_m], sizeof(GenMesh), number_of_meshes, mfile );
	
	fread(&gms->vertices[gms->next_v], sizeof(Vertex), number_of_vertices_static, mfile );
	fread(&gms->indices[gms->next_i], sizeof(uint), number_of_indices_static, mfile );
	
	fread(&gms->s_vertices[gms->next_sv], sizeof(S_Vertex), number_of_vertices_skinned, mfile );
	fread(&gms->s_indices[gms->next_si], sizeof(uint), number_of_indices_skinned, mfile );
	
	fread(&ns->names[ns->next_n], sizeof(Resource_Name), number_of_nodes, mfile );
	fread(&ns->name_indices[ns->next_n], sizeof(int), number_of_nodes, mfile );
	fread(&ns->parent[ns->next_n], sizeof(int), number_of_nodes, mfile );
	fread(&ns->nodes[ns->next_n], sizeof(Node), number_of_nodes, mfile );
	
	fread(&ss->skins[ss->next_skin], sizeof(Skin), number_of_skins, mfile );
	fread(&ss->skin_vals[ss->next_val], sizeof(SkinVal), number_of_skin_values, mfile );
	
	fclose(mfile);
	
	for (int i = 0; i < number_of_meshes; i++) {
		int mesh_index = i + gms->next_m;
		
		if( gms->meshes[mesh_index].skinned ) {
			gms->meshes[mesh_index].ndx_v = gms->next_sv;
			gms->next_sv = gms->next_sv + gms->meshes[mesh_index].num_v;
			
			gms->meshes[mesh_index].ndx_i = gms->next_si;
			gms->next_si = gms->next_si + gms->meshes[mesh_index].num_i;
		} else {
			gms->meshes[mesh_index].ndx_v = gms->next_v;
			gms->next_v = gms->next_v + gms->meshes[mesh_index].num_v;
			
			gms->meshes[mesh_index].ndx_i = gms->next_i;
			gms->next_i = gms->next_i + gms->meshes[mesh_index].num_i;
		}
	}
	
	for(int i = 0; i < number_of_nodes; i++) {
		ns->name_indices[ns->next_n + i] += ns->next_n;
	}
	
	for (int i = 0; i < number_of_skins; i++) {
		ss->skins[ss->next_skin + i].idx += ss->next_val;
	}
	
	gms->next_m += number_of_meshes;/*
 gms->next_v += number_of_vertices_static;
 gms->next_i += number_of_indices_static;
 gms->next_sv += number_of_vertices_skinned;
 gms->next_si += number_of_indices_skinned;
 */
	ns->next_n += number_of_nodes;
	
	ss->next_skin += number_of_skins;
	ss->next_val += number_of_skin_values;
	
	return model;
}

ModelMemory gather_model_memory() {
	
	printf("gathering models\n");
	
	ModelMemory mm = init_model_memory();
	
	DIR *models_directory;
	struct dirent *directory_entry;
	models_directory = opendir("../models");
	
	while(NULL != (directory_entry = readdir(models_directory))) {
		if (directory_entry->d_type != DT_REG) {
			continue;
		}
		
		if (!compare_tail(directory_entry->d_name, ".moe")) {
			continue;
		}
		
		char dirnamebuffer[strlen(directory_entry->d_name) + sizeof("../models/")];
		sprintf(dirnamebuffer, "../models/%s", directory_entry->d_name);
		printf("\t%s\n", dirnamebuffer);
		
		GenModel next_model = load_model_into(&mm, dirnamebuffer);
		
		if (mm.next_genmodel >= mm.max_genmodel) {
			printf("error when gathering models: mm.next_genmodel >= mm.max_genmodel\n");
			return mm;
		}
		mm.genmodels[mm.next_genmodel] = next_model;
		
		unsigned int value = mm.next_genmodel;
		unsigned int key = crc32_cstring(directory_entry->d_name);
		mm.next_genmodel++;
		
		printf("\tcrc32 %s %u %u\n", directory_entry->d_name, key, value);
		
		unsigned int ok = map_insert_kv(&mm.model_map, key, value);
		printf("\tinsert ok %u\n\n", ok);
	}
	
	return mm;
}

GenModel get_model_from_crc32(ModelMemory *mm, bool *ok, uint key) {
	uint genmodel_index = map_lookup_key(&mm->model_map, ok, key);
	if(*ok) {
		return mm->genmodels[genmodel_index];
	}
	return mm->genmodels[0];
}

// this one is just a one-off function creating it's memory itself
// can be used for when there is only one model to load, so mostly for testing purposes
GenModelPack load_modelpack(const char * filename) {
	
	FILE * mfile = fopen( filename, "rb" );
	
	int numbers[8];
	
	int rlen = fread(numbers, sizeof(int), 8, mfile);
	
	int number_of_meshes = numbers[0];
	int number_of_vertices_static = numbers[1];
	int number_of_indices_static = numbers[2];
	int number_of_vertices_skinned = numbers[3];
	int number_of_indices_skinned = numbers[4];
	int number_of_nodes = numbers[5];
	int number_of_skins = numbers[6];
	int number_of_skin_values = numbers[7];
	
	GenModelPack gmr;
	
	gmr.gm.num_meshes = number_of_meshes;
	gmr.gm.num_nodes = number_of_nodes;
	gmr.gm.num_skin = number_of_skins;
	
	gmr.gm.mesh_idx = 0;
	gmr.gm.node_idx  = 0;
	gmr.gm.skin = 0;
	
	gmr.gms.next_m = number_of_meshes;
	gmr.gms.next_v = number_of_vertices_static;
	gmr.gms.next_i = number_of_indices_static;
	gmr.gms.next_sv = number_of_vertices_skinned;
	gmr.gms.next_si = number_of_indices_skinned;
	
	gmr.ns.next_n = number_of_nodes;
	
	gmr.ss.next_skin = number_of_skins;
	gmr.ss.next_val = number_of_skin_values;
	
	// TODO: the following could be one huge malloc
	// or rather we might as well just copy the whole data into memory with mmap and set pointers
	// this is just temporary to see if it works
	
	gmr.gms.names = (Resource_Name*)malloc(sizeof(Resource_Name) * number_of_meshes);
	gmr.gms.meshes = (GenMesh*) malloc( sizeof(GenMesh) * number_of_meshes );
	
	gmr.gms.vertices = (Vertex*) malloc( sizeof(Vertex) * number_of_vertices_static );
	gmr.gms.indices = (uint*) malloc( sizeof(uint) * number_of_indices_static );
	
	gmr.gms.s_vertices = (S_Vertex*) malloc( sizeof(S_Vertex) * number_of_vertices_skinned );
	gmr.gms.s_indices = (uint*) malloc( sizeof(uint) * number_of_indices_skinned );
	
	gmr.ns.names = (Resource_Name*) malloc( sizeof(Resource_Name)*number_of_nodes );
	gmr.ns.name_indices = (int*) malloc( sizeof(int)*number_of_nodes );
	gmr.ns.parent = (int*) malloc( sizeof(int)*number_of_nodes );
	gmr.ns.nodes = (Node*) malloc( sizeof(Node)*number_of_nodes );
	
	gmr.ss.skins = (Skin*) malloc( sizeof(Skin) * number_of_skins );
	gmr.ss.skin_vals = (SkinVal*)  malloc( sizeof(SkinVal) * number_of_skin_values );
	
    fread(gmr.gms.names, sizeof(Resource_Name), number_of_meshes, mfile);
	fread(gmr.gms.meshes, sizeof(GenMesh), number_of_meshes, mfile );
	
	fread(gmr.gms.vertices, sizeof(Vertex), number_of_vertices_static, mfile );
	fread(gmr.gms.indices, sizeof(uint), number_of_indices_static, mfile );
	
	fread(gmr.gms.s_vertices, sizeof(S_Vertex), number_of_vertices_skinned, mfile );
	fread(gmr.gms.s_indices, sizeof(uint), number_of_indices_skinned, mfile );
	
	fread(gmr.ns.names, sizeof(Resource_Name), number_of_nodes, mfile );
	fread(gmr.ns.name_indices, sizeof(int), number_of_nodes, mfile );
	fread(gmr.ns.parent, sizeof(int), number_of_nodes, mfile );
	fread(gmr.ns.nodes, sizeof(Node), number_of_nodes, mfile );
	
	fread(gmr.ss.skins, sizeof(Skin), number_of_skins, mfile );
	fread(gmr.ss.skin_vals, sizeof(SkinVal), number_of_skin_values, mfile );
	
	fclose( mfile );
	
	return gmr;
}

AnimReturn load_animation_test(
const char * filename)
{
	FILE * afile = fopen( filename, "rb" );
	
	int numbers[6];
	
	int rlen = fread(numbers, sizeof(int), 6, afile);
	
	int number_of_animations = numbers[0];
	int number_of_translation_channels = numbers[1];
	int number_of_rotation_channels = numbers[2];
	int number_of_times = numbers[3];
	int number_of_translations = numbers[4];
	int number_of_rotations = numbers[5];
	
	AnimReturn ar;
	
	ar.descr.num_anims = number_of_animations;
	ar.descr.anims = 0;
	
	ar.as.next_anim = number_of_animations;
	
	ar.as.next_tranch = number_of_translation_channels;
	ar.as.next_rotch = number_of_rotation_channels;
	
	ar.as.next_time = number_of_times;
	ar.as.next_trans = number_of_translations;
	ar.as.next_rot = number_of_rotations;
	
	ar.as.anims = (Anim*)malloc(sizeof(Anim) * number_of_animations);
	
	ar.as.tranchs = (TranChannel*)malloc(sizeof(TranChannel) * number_of_translation_channels);
	ar.as.rotchs = (RotChannel*)malloc(sizeof(RotChannel) * number_of_rotation_channels);
	
	ar.as.times = (float*)malloc( sizeof(float) * number_of_times);
	ar.as.trans = (v3*)malloc( sizeof(v3) * number_of_translations);
	ar.as.rots = (Quat*)malloc( sizeof(Quat) * number_of_rotations);
	
	fread(ar.as.anims, sizeof(Anim), number_of_animations, afile );
	
	fread(ar.as.tranchs, sizeof(TranChannel), number_of_translation_channels, afile );
	fread(ar.as.rotchs, sizeof(RotChannel), number_of_rotation_channels, afile );
	
	fread(ar.as.times, sizeof(float), number_of_times, afile );
	fread(ar.as.trans, sizeof(v3), number_of_translations, afile );
	fread(ar.as.rots, sizeof(Quat), number_of_rotations, afile );
	
	fclose( afile );
	
	return ar;
}

// from https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer
/* Size of each input chunk to be
   read and allocate for. */
#ifndef	 READALL_CHUNK
#define	 READALL_CHUNK	262144
#endif

#define	 READALL_OK			 0	/* Success */
#define	 READALL_INVALID	-1	/* Invalid parameters */
#define	 READALL_ERROR		-2	/* Stream error */
#define	 READALL_TOOMUCH	-3	/* Too much input */
#define	 READALL_NOMEM		-4	/* Out of memory */

/* This function returns one of the READALL_ constants above.
   If the return value is zero == READALL_OK, then:
  (*dataptr) points to a dynamically allocated buffer, with
  (*sizeptr) chars read from the file.
  The buffer is allocated for one extra char, which is NUL,
  and automatically appended after the data.
   Initial values of (*dataptr) and (*sizeptr) are ignored.
*/
int readall(FILE *in, char **dataptr, size_t *sizeptr)
{
	char  *data = NULL, *temp;
	size_t size = 0;
	size_t used = 0;
	size_t n;
    
	/* None of the parameters can be NULL. */
	if (in == NULL || dataptr == NULL || sizeptr == NULL)
		return READALL_INVALID;
    
	/* A read error already occurred? */
	if (ferror(in))
		return READALL_ERROR;
    
	while (1) {
        
		if (used + READALL_CHUNK + 1 > size) {
			size = used + READALL_CHUNK + 1;
            
			/* Overflow check. Some ANSI C compilers
      may optimize this away, though. */
			if (size <= used) {
				free(data);
				return READALL_TOOMUCH;
			}
            
			temp = (char *)realloc(data, size);
			if (temp == NULL) {
				free(data);
				return READALL_NOMEM;
			}
			data = temp;
		}
        
		n = fread(data + used, 1, READALL_CHUNK, in);
		if (n == 0)
			break;
        
		used += n;
	}
    
	if (ferror(in)) {
		free(data);
		return READALL_ERROR;
	}
    
	temp = (char *)realloc(data, used + 1);
	if (temp == NULL) {
		free(data);
		return READALL_NOMEM;
	}
	data = temp;
	data[used] = '\0';
    
	*dataptr = data;
	*sizeptr = used;
    
	return READALL_OK;
}


// function to load shaders returns a shader program id
unsigned int loadshaders_from_file(const char* vertexpath, const char* fragmentpath, char* infoLog) {
    
    
	FILE * vertexfile = fopen( vertexpath, "rb" );
	FILE * fragmentfile = fopen( fragmentpath, "rb" );
    
	char * vertexfile_dataptr;
	size_t vertexfile_sizeptr;
    
	char * fragmentfile_dataptr;
	size_t fragmentfile_sizeptr;
    
    
	int vret = readall(vertexfile, &vertexfile_dataptr, &vertexfile_sizeptr);
	int fret = readall(fragmentfile, &fragmentfile_dataptr, &fragmentfile_sizeptr);
    
	printf("%i\n", vret);
	printf("%i\n", fret);
    
	if (vret != READALL_OK || fret != READALL_OK ) {
		return 0;
	}
    
	const char * vertexshader_code = vertexfile_dataptr;
	const char * fragmentshader_code = fragmentfile_dataptr;
    
	unsigned int vertex, fragment;
	int success;
	
	// vertexShader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexshader_code, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		printf("%s", infoLog);
		return 0;
	}
	// fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentshader_code, NULL);
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
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s", infoLog);
		return 0;
	}
	glDeleteShader(vertex);
	glDeleteShader(fragment);
    
	fclose(vertexfile);
	fclose(fragmentfile);
    
	return shaderProgram;
}



RenderInfo_Stat prepare_stat(
GenMeshStack gms,
uint ssbo_data_len)
{
    
	RenderInfo_Stat info_stat;
	
	unsigned int vbo, vao, indirect, ssbo_pos, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &indirect);
	glGenBuffers(1, &ssbo_pos);
	glGenBuffers(1, &ebo);
	
	// bind vao and associate everything with it
	glBindVertexArray(vao);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		(sizeof(unsigned int) * gms.next_i),
		gms.indices,
		GL_STATIC_DRAW );
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glBufferData(
		GL_ARRAY_BUFFER,
		(sizeof(Vertex) * gms.next_v),
		gms.vertices,
		GL_STATIC_DRAW );
	
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	
	// normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	// texture coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6* sizeof(float)));
	glEnableVertexAttribArray(2);
	
	//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	char infoLog[512];
    
	printf("loading shader\n");
    
	// TODO: temporarily we just hardcode the shader directory 
    
	unsigned int shaderProgram = loadshaders_from_file("../libs/shaders_basic/stat_shader.vs","../libs/shaders_basic/shader.fs", infoLog);
    
	printf("loaded shader\n");
    
	info_stat.viewLoc = glGetUniformLocation(shaderProgram, "view");
	info_stat.projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	
	info_stat.shaderProgram = shaderProgram;
	info_stat.vbo = vbo;
	info_stat.vao = vao;
	info_stat.indirect = indirect;
	info_stat.ssbo_pos = ssbo_pos;
	info_stat.ebo = ebo;
	info_stat.ssbo_data = (SSBO_Data*) malloc( sizeof(SSBO_Data)*ssbo_data_len );
	info_stat.ssbo_data_len = ssbo_data_len;
	
	return info_stat;
}

RenderInfo_Skin prepare_skin(
GenMeshStack gms,
uint ssbo_data_len)
{
	RenderInfo_Skin info_skin;
	
	unsigned int vbo, vao, indirect, ssbo_pos, ssbo_joint_pos, ebo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &indirect);
	glGenBuffers(1, &ssbo_pos);
	glGenBuffers(1, &ssbo_joint_pos);
	glGenBuffers(1, &ebo);
	
	// bind vao and associate everything with it
	glBindVertexArray(vao);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		(sizeof(unsigned int) * gms.next_si),
		gms.s_indices,
		GL_STATIC_DRAW );
	
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos);
	
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_joint_pos);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glBufferData(
		GL_ARRAY_BUFFER,
		(sizeof(S_Vertex) * gms.next_sv),
		gms.s_vertices,
		GL_STATIC_DRAW );
	
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(S_Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	
	// normals
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(S_Vertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	// texture coords
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(S_Vertex), (void*)(6* sizeof(float)));
	glEnableVertexAttribArray(2);
	
	glVertexAttribPointer(3,
                          4,
                          GL_UNSIGNED_INT,
                          GL_FALSE,
                          sizeof(S_Vertex),
                          (void*)(8* sizeof(float)));
    
	glEnableVertexAttribArray(3);
	
	glVertexAttribPointer(
		4,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(S_Vertex),
		(void*)( 8*sizeof(float) + 4*sizeof(uint)));
    
	glEnableVertexAttribArray(4);
	
	//glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    
	char infoLog[512];
    
	unsigned int shaderProgram = loadshaders_from_file("../libs/shaders_basic/dyn_shader.vs","../libs/shaders_basic/dyn_shader.fs", infoLog);
	
	info_skin.shaderProgram = shaderProgram;
	
	
	info_skin.viewLoc = glGetUniformLocation(shaderProgram, "view");
	info_skin.projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	
	info_skin.vbo = vbo;
	info_skin.vao = vao;
	info_skin.indirect = indirect;
	info_skin.ssbo_pos = ssbo_pos;
	info_skin.ssbo_joint_pos = ssbo_joint_pos;
	info_skin.ebo = ebo;
	info_skin.ssbo_data_skin = (SSBO_Data_Skin*) malloc( sizeof(SSBO_Data_Skin)*ssbo_data_len );
	info_skin.ssbo_data_len = ssbo_data_len;
	
	info_skin.ssbo_joint = (SSBO_Joint*) malloc( sizeof(SSBO_Joint)*ssbo_data_len );
	
	return info_skin;
}


Rendering_Context_Gen prepare_ressources_gen(
GenMeshStack gms,
Texture_Asset_Manager *texture_asset_manager,
uint ssbo_data_len)
{
	Rendering_Context_Gen retctx;
	
	glGenTextures(7, retctx.gl_texture_ids);
	
	unsigned int texwidth = 16;
	for(int i = 0; i < 7; i++) {
		unsigned int layercount = texture_asset_manager->next_texture[i]; 
		
		glBindTexture(GL_TEXTURE_2D_ARRAY, retctx.gl_texture_ids[i]);
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, texwidth, texwidth, layercount);
		
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, texwidth, texwidth, layercount, GL_RGBA, GL_UNSIGNED_BYTE, texture_asset_manager->textures[i]);
		
		glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		
		texwidth = texwidth << 1;
	}
	
	RenderInfo_Stat info_stat = prepare_stat(gms, ssbo_data_len);
	RenderInfo_Skin info_skin = prepare_skin(gms, ssbo_data_len);
    
	retctx.info_stat = info_stat;
	retctx.info_skin = info_skin;
	
	retctx.gms = gms;
	
	//b3Transform start_transform = b3Transform(b3Quat(0.0f,0.0f,0.0f,1.0f),b3Vec3(0.0f,0.0f,5.0f));
	//retctx.view = Convert44(b3Inverse(start_transform));
	//retctx.projection = BuildProjectionMatrix(1.0f, glm::radians(45.0f), 0.2f,  10000.0f);
	
	return retctx;
}

mat4 Convert44(Transform transform) {
    
	mat4 retmat = quaternionToMat4(transform.rotation);
    
	retmat.Elements[3][0] = transform.translation.X;
	retmat.Elements[3][1] = transform.translation.Y;
	retmat.Elements[3][2] = transform.translation.Z;
    
	return retmat;
}

// TODO: Add scaling when you have some test_models
void render_Info_Stat(
Rendering_Context_Gen rectx,
NodeStack ns,
SkinStack ss,
GenModel * gma,
Transform * world_transforms,
v3 * model_scales,
uint num_models)
{
	printf("starting render info stat\n\n");
	
	
	// for ease of writing we just reassign some simple stuff
	GenMeshStack gms = rectx.gms;
	
	// first count how often a specific mesh will be rendered
	uint num_meshes = gms.next_m;
	uint num_nodes = ns.next_n;
	
	mat4 view = rectx.view;
	mat4 projection = rectx.projection;
	int viewLoc = rectx.info_stat.viewLoc;
	int projectionLoc = rectx.info_stat.projectionLoc;
	
	unsigned int shaderProgram = rectx.info_stat.shaderProgram;
	
	unsigned int vbo = rectx.info_stat.vbo;
	unsigned int vao = rectx.info_stat.vao;
	unsigned int indirect = rectx.info_stat.indirect;
	unsigned int ssbo_pos = rectx.info_stat.ssbo_pos;
	unsigned int ebo = rectx.info_stat.ebo;
	
	
	
	DrawElementsIndirectCommand deicommand[num_meshes];
	uint num_drawn_meshes = 0;
	
	// first count how often a specific mesh will be rendered
	int mesh_id_count[num_meshes]; // how often the mesh i at index i will be drawn
	int mesh_id_base[num_meshes]; // the baseinstance 
	int mesh_instance_count[num_meshes];
	
	for (int i = 0; i < num_meshes; ++i) {
		mesh_id_count[i] = 0;
		mesh_id_base[i] = 0;
		mesh_instance_count[i] = 0;
	}
	
	// count all the meshes, if it is skinned don't count it
	printf("first step\ncount the meshes\n");
	for (int i = 0; i < num_models; ++i)
	{
		for (int j = 0; j < gma[i].num_nodes; ++j)
		{
			int nidxj = gma[i].node_idx + j;
			
			int mesh_id = ns.nodes[nidxj].mesh;
			if ( mesh_id == -1 || gms.meshes[mesh_id + gma[i].mesh_idx].skinned == true ) {
				continue;
			}
			mesh_id_count[mesh_id + gma[i].mesh_idx]++;
			num_drawn_meshes++;		
		}
	}
	
	for (int i = 0; i < num_meshes; i++) {
		printf("mesh %i , drawn %i times\n", i, mesh_id_count[i]);
	}
	printf("now assemble the deicommand\n");
	// TODO: this is the broblem the gms.meshes are relative to their models mesh_idx
	// but we don't add that offest
	// too lazu to fix, did enough for today
	for (int i = 0; i < num_meshes; i++) {
		if (i != 0) {
			mesh_id_base[i] = mesh_id_base[i-1] + mesh_id_count[i-1]; 
		}
		deicommand[i].indexCount = gms.meshes[i].num_i;
		deicommand[i].firstIndex = gms.meshes[i].ndx_i;
		
		deicommand[i].baseVertex = gms.meshes[i].ndx_v;
		
		deicommand[i].instanceCount = mesh_id_count[i];
		deicommand[i].baseInstance =  mesh_id_base[i];
	}
	
	for (int i = 0; i < num_meshes; i++) {
		printf("deicommand %i\n", i);
		printf("  indexCount %i\n", deicommand[i].indexCount);
		printf("  firstIndex  %i\n", deicommand[i].firstIndex);
		printf("  baseVertex %i\n", deicommand[i].baseVertex);
		printf("  instanceCount %i\n", deicommand[i].instanceCount);
		printf("  baseInstance %i\n", deicommand[i].baseInstance);
	}
	
	for (int i = 0; i < num_meshes; i++) {
		printf("mesh_id_base %i: %i\n", i, mesh_id_base[i]);
	}
	SSBO_Data * ssbo_data = rectx.info_stat.ssbo_data;
	
	if (num_drawn_meshes > rectx.info_stat.ssbo_data_len ) {
		printf("num_drawn_meshes > rendering_ctxt.ssbo_data_len\n");
	}
	
	printf("now in the final step\n");
	for (int i = 0; i < num_models; ++i)
	{
		for (int j = 0; j < gma[i].num_nodes; ++j)
		{
			int nidxj = gma[i].node_idx + j;
			
			int relative_mesh_id = ns.nodes[nidxj].mesh;
			int mesh_id = ns.nodes[nidxj].mesh + gma[i].mesh_idx;
			if ( relative_mesh_id == -1 || gms.meshes[mesh_id].skinned == true ) {
				continue;
			}
			
			int ssboidx = mesh_id_base[mesh_id] + mesh_instance_count[mesh_id];
			
			printf("mesh_id: %i  ssboidx: %i\n", mesh_id,  ssboidx);
			
			Transform world_transform = ns.nodes[nidxj].object_transform; //mul_transform(world_transforms[i],ns.nodes[nidxj].object_transform);
            
			ssbo_data[ssboidx].position = vec4(
				world_transform.translation.X,
				world_transform.translation.Y,
				world_transform.translation.Z,
				0.0f);
            
			ssbo_data[ssboidx].transform = world_transform.rotation;
			
			printf("ssbo_data[%i].transform %f %f %f\n", ssboidx, ssbo_data[ssboidx].position.X, ssbo_data[ssboidx].position.Y, ssbo_data[ssboidx].position.Z);
			
			//ssbo_data[ssboidx].texture_type = gms.meshes[mesh_id].texid.texture_type;
			//ssbo_data[ssboidx].texture_nr = gms.meshes[mesh_id].texid.texture_nr;
			
			ssbo_data[ssboidx].texture_type = ns.texture_assets[nidxj].type;ssbo_data[ssboidx].texture_nr = ns.texture_assets[nidxj].nr;
			
			mesh_instance_count[mesh_id]++;
		}
	}
	
	glUseProgram(shaderProgram);
	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.X.X);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection.X.X);
	
	glBindVertexArray(vao);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO_Data)*num_drawn_meshes, ssbo_data, GL_DYNAMIC_DRAW);
	
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand)*num_meshes, deicommand, GL_STATIC_DRAW);
	
	
	for(int i = 0; i < 7; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D_ARRAY, rectx.gl_texture_ids[i]);
	}
	
	int texture_arrays_loc = glGetUniformLocation(shaderProgram, "texture_arrays");
	
	int gl_1ivalues[7];
	for(int i = 0; i < 7; i++) {
		gl_1ivalues[i] = i;
	}
	glUniform1iv(texture_arrays_loc, 7, gl_1ivalues);
	
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, num_meshes, 0);
}


// TODO: Add scaling when you have some test_models
void render_Info_Stat_oldgl(
Rendering_Context_Gen rectx,
NodeStack ns,
SkinStack ss,
GenModel * gma,
v3 * model_scales,
uint num_models)
{
	//printf("starting render info stat\n\n");
	
	
	// for ease of writing we just reassign some simple stuff
	GenMeshStack gms = rectx.gms;
	
	// first count how often a specific mesh will be rendered
	uint num_meshes = gms.next_m;
	uint num_nodes = ns.next_n;
	
	mat4 view = rectx.view;
	mat4 projection = rectx.projection;
	int viewLoc = rectx.info_stat.viewLoc;
	int projectionLoc = rectx.info_stat.projectionLoc;
	
	unsigned int shaderProgram = rectx.info_stat.shaderProgram;
	
	unsigned int vbo = rectx.info_stat.vbo;
	unsigned int vao = rectx.info_stat.vao;
	unsigned int indirect = rectx.info_stat.indirect;
	unsigned int ssbo_pos = rectx.info_stat.ssbo_pos;
	unsigned int ebo = rectx.info_stat.ebo;
	
	
	
	DrawElementsIndirectCommand deicommand[num_meshes];
	uint num_drawn_meshes = 0;
	
	// first count how often a specific mesh will be rendered
	int mesh_id_count[num_meshes]; // how often the mesh i at index i will be drawn
	int mesh_id_base[num_meshes]; // the baseinstance 
	int mesh_instance_count[num_meshes];
	
	for (int i = 0; i < num_meshes; i++) {
		mesh_id_count[i] = 0;
		mesh_id_base[i] = 0;
		mesh_instance_count[i] = 0;
	}
	
	// count all the meshes, if it is skinned don't count it
	//printf("first step\ncount the meshes\n");
	for (int i = 0; i < num_models; ++i)
	{
		for (int j = 0; j < gma[i].num_nodes; ++j)
		{
			int nidxj = gma[i].node_idx + j;
			
			int mesh_id = ns.nodes[nidxj].mesh;
			if ( mesh_id == -1 || gms.meshes[mesh_id + gma[i].mesh_idx].skinned == true ) {
				continue;
			}
			mesh_id_count[mesh_id + gma[i].mesh_idx]++;
			num_drawn_meshes++;		
		}
	}
	/*
 for (int i = 0; i < num_meshes; i++) {
  printf("mesh %i , drawn %i times\n", i, mesh_id_count[i]);
 }
 printf("now assemble the deicommand\n");
 */
	for (int i = 0; i < num_meshes; i++) {
		if (i != 0) {
			mesh_id_base[i] = mesh_id_base[i-1] + mesh_id_count[i-1]; 
		}
		deicommand[i].indexCount = gms.meshes[i].num_i;
		deicommand[i].firstIndex = gms.meshes[i].ndx_i;
		
		deicommand[i].baseVertex = gms.meshes[i].ndx_v;
		
		deicommand[i].instanceCount = mesh_id_count[i];
		deicommand[i].baseInstance =  mesh_id_base[i];
	}
	/*
 for (int i = 0; i < num_meshes; i++) {
  printf("deicommand %i\n", i);
  printf("  indexCount %i\n", deicommand[i].indexCount);
  printf("  firstIndex  %i\n", deicommand[i].firstIndex);
  printf("  baseVertex %i\n", deicommand[i].baseVertex);
  printf("  instanceCount %i\n", deicommand[i].instanceCount);
  printf("  baseInstance %i\n", deicommand[i].baseInstance);
 }
 
 for (int i = 0; i < num_meshes; i++) {
  printf("mesh_id_base %i: %i\n", i, mesh_id_base[i]);
 }
*/
	SSBO_Data * ssbo_data = rectx.info_stat.ssbo_data;
	
	if (num_drawn_meshes > rectx.info_stat.ssbo_data_len ) {
		printf("num_drawn_meshes > rendering_ctxt.ssbo_data_len\n");
	}
	// TODO: this way we are iterating too many times, for every model we check all the nodes, which is dumb as fuck (but works for now, lol)
	//printf("now in the final step\n");
	for (int i = 0; i < num_models; ++i)
	{
		for (int j = 0; j < gma[i].num_nodes; ++j)
		{
			int nidxj = gma[i].node_idx + j;
			
			int relative_mesh_id = ns.nodes[nidxj].mesh;
			int mesh_id = ns.nodes[nidxj].mesh + gma[i].mesh_idx;
			if ( relative_mesh_id == -1 || gms.meshes[mesh_id].skinned == true ) {
				continue;
			}
			
			int ssboidx = mesh_id_base[mesh_id] + mesh_instance_count[mesh_id];
			
			//printf("mesh_id: %i  ssboidx: %i\n", mesh_id,  ssboidx);
			
			Transform world_transform = ns.nodes[nidxj].object_transform; //mul_transform(world_transforms[i],ns.nodes[nidxj].object_transform);
            
			ssbo_data[ssboidx].position = vec4(
				world_transform.translation.X,
				world_transform.translation.Y,
				world_transform.translation.Z,
				0.0f);
            
			ssbo_data[ssboidx].transform = world_transform.rotation;
			
			ssbo_data[ssboidx].scale = vec4(
				model_scales[i].X,
				model_scales[i].Y,
				model_scales[i].Z,
				0.0f);
			
			//printf("ssbo_data[%i].transform %f %f %f\n", ssboidx, ssbo_data[ssboidx].position.X, ssbo_data[ssboidx].position.Y, ssbo_data[ssboidx].position.Z);
			
			//ssbo_data[ssboidx].texture_type = gms.meshes[mesh_id].texid.texture_type;
			//ssbo_data[ssboidx].texture_nr = gms.meshes[mesh_id].texid.texture_nr;
			
			ssbo_data[ssboidx].texture_type = ns.texture_assets[nidxj].type;ssbo_data[ssboidx].texture_nr = ns.texture_assets[nidxj].nr;
			
			mesh_instance_count[mesh_id]++;
		}
	}
	
	glUseProgram(shaderProgram);
	
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.X.X);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection.X.X);
	
	glBindVertexArray(vao);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO_Data)*num_drawn_meshes, ssbo_data, GL_DYNAMIC_DRAW);
	
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand)*num_meshes, deicommand, GL_STATIC_DRAW);
	
	
	
	for(int i = 0; i < 7; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D_ARRAY, rectx.gl_texture_ids[i]);
	}
	
	int texture_arrays_loc = glGetUniformLocation(shaderProgram, "texture_arrays");
	
	int gl_1ivalues[7];
	for(int i = 0; i < 7; i++) {
		gl_1ivalues[i] = i;
	}
	glUniform1iv(texture_arrays_loc, 7, gl_1ivalues);
	
	int newBaseInstanceLoc = glGetUniformLocation(shaderProgram, "newBaseInstance");
	glUniform1i(newBaseInstanceLoc, 0);
	
	v4 player_pos_projected = HMM_MultiplyMat4ByVec4(projection, HMM_MultiplyMat4ByVec4(view, vec4v(rectx.player_pos, 1.0f)));
	
	int player_pos_loc = glGetUniformLocation(shaderProgram, "player_pos");
	glUniform3f(player_pos_loc, rectx.player_pos.x, rectx.player_pos.y, rectx.player_pos.z);
	
	
	for (int i = 0; i < num_meshes; i++) {
		glUniform1i(newBaseInstanceLoc, deicommand[i].baseInstance);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, deicommand[i].indexCount, GL_UNSIGNED_INT, (void*)(deicommand[i].firstIndex * sizeof(GL_UNSIGNED_INT)), deicommand[i].instanceCount, deicommand[i].baseVertex);
		
	}
}


// TODO: both render_Info_Skin and render_Info_Stat can be combined in some way to optimize I think
void render_Info_Skin_oldgl(
Rendering_Context_Gen rectx,
NodeStack ns,
SkinStack ss,
GenModel * gma,
v3 * model_scales,
uint num_models)
{
	
	// for ease of writing
	GenMeshStack gms = rectx.gms;
	
	unsigned int num_meshes = gms.next_m; 
	unsigned int num_nodes = ns.next_n;
	
	//GLuint texture = rendering_ctxt.texture;
	mat4 view = rectx.view;
	mat4 projection = rectx.projection;
	int viewLoc = rectx.info_skin.viewLoc;
	int projectionLoc = rectx.info_skin.projectionLoc;
	
	unsigned int shaderProgram = rectx.info_skin.shaderProgram;
	
	unsigned int vbo = rectx.info_skin.vbo;
	unsigned int vao = rectx.info_skin.vao;
	unsigned int indirect = rectx.info_skin.indirect;
	unsigned int ssbo_pos = rectx.info_skin.ssbo_pos;
	unsigned int ssbo_joint_pos = rectx.info_skin.ssbo_joint_pos;
	unsigned int ebo = rectx.info_skin.ebo;
	
	DrawElementsIndirectCommand deicommand[num_meshes];
	
	unsigned int num_drawn_meshes = 0;
	
	// lets try it by just counting all the elements
	int mesh_id_count[num_meshes];
	int mesh_id_base[num_meshes];
	int mesh_instance_count[num_meshes];
	
	for (int i = 0; i < num_meshes; i++) {
		mesh_id_count[i] = 0;
		mesh_id_base[i] = 0;
		mesh_instance_count[i] = 0;
	}
	
	for (int i = 0; i < num_models; ++i)
	{
		for (int j = 0; j < gma[i].num_nodes; ++j)
		{
			int nidxj = gma[i].node_idx + j;
			
			int mesh_id = ns.nodes[nidxj].mesh;
			if ( mesh_id == -1 || gms.meshes[mesh_id + gma[i].mesh_idx].skinned == false ) {
				continue;
			}
			mesh_id_count[mesh_id + gma[i].mesh_idx]++;
			num_drawn_meshes++;		
		}
	}
	
	
	for (int i = 0; i < num_meshes; i++) {
		if (i != 0) {
			mesh_id_base[i] = mesh_id_base[i-1] + mesh_id_count[i-1]; 
		}
		//std::cout << "model " << i << '\n';
		
		deicommand[i].indexCount = gms.meshes[i].num_i;
		deicommand[i].firstIndex = gms.meshes[i].ndx_i;
		
		deicommand[i].baseVertex = gms.meshes[i].ndx_v;
		
		deicommand[i].instanceCount = mesh_id_count[i];
		deicommand[i].baseInstance =  mesh_id_base[i];
	}
	
	SSBO_Data_Skin * ssbo_data_skin = rectx.info_skin.ssbo_data_skin;
	SSBO_Joint * ssbo_joint = rectx.info_skin.ssbo_joint;
	int next_joint = 0; 
	
	if (num_drawn_meshes > rectx.info_skin.ssbo_data_len ) {
		printf("num_drawn_meshes > rendering_ctxt.ssbo_data_len\n");
	}	
	
	for (int i = 0; i < num_models; i++) {
		
		int joint_root = next_joint;
		
		for (int j = 0; j < gma[i].num_nodes; ++j)
		{
			int c = gma[i].node_idx + j;
			
			int relative_mesh_id = ns.nodes[c].mesh;
			int mesh_id = relative_mesh_id + gma[i].mesh_idx;
			
			if ( relative_mesh_id == -1 || ns.nodes[c].skin == -1 || gms.meshes[mesh_id].skinned == false ) {
				continue;
			}
            
			Skin sk = ss.skins[ns.nodes[c].skin + gma[i].skin];
            
			joint_root = next_joint;
			
			for (int k = 0; k < sk.num; k++) {
				
				SkinVal sv = ss.skin_vals[sk.idx+k];
                
				// the index into this tree that represents the transform at index k
				// basically root + relative index 
				int nidx = gma[i].node_idx + sv.node;
				
				// transform of the joint k
				mat4 glo_node = Convert44(ns.nodes[nidx].object_transform);
				
				mat4 calcm = HMM_MultiplyMat4(glo_node, sv.inv_bind);
				
				ssbo_joint[next_joint].jointMat = calcm;
				next_joint = next_joint+1;
			}
			
			
			// add the transform to the right place
			// TODO: this is wrong, we are adding it to the wrong place
			int ni = mesh_id_base[mesh_id] + mesh_instance_count[mesh_id];
			ssbo_data_skin[ni].root_joint = joint_root;
            
			// TODO: remove this stuff here, skinned meshes don't need any position data transmitted
			//Transform world_transform = mul_transform(world_transforms[i], ns.nodes[c].object_transform);
            
			//ssbo_data_skin[ni].position = vec4(world_transform.translation.X,world_transform.translation.Y,world_transform.translation.Z,0.0f); 
            
			//ssbo_data_skin[ni].transform = world_transform.rotation;
            
			//ssbo_data_skin[ni].texture_type = gms.meshes[mesh_id].texid.texture_type;
			//ssbo_data_skin[ni].texture_nr = gms.meshes[mesh_id].texid.texture_nr;
			
			ssbo_data_skin[ni].texture_type = ns.texture_assets[c].type;
			ssbo_data_skin[ni].texture_nr = ns.texture_assets[c].nr;
			
			/*
            ssbo_data_skin[ni].scale = vec4(
 model_scales[i].X,
 model_scales[i].Y,
 model_scales[i].Z,
 0.0f);
            */
			mesh_instance_count[mesh_id] = mesh_instance_count[mesh_id] + 1;
		}
	}
    
	glUseProgram(shaderProgram);
    
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.X.X);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection.X.X);
	
	glBindVertexArray(vao);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO_Data_Skin)*num_drawn_meshes, ssbo_data_skin, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssbo_pos);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_joint_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO_Joint)*next_joint, ssbo_joint, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssbo_joint_pos);
	
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand)*num_meshes, deicommand, GL_STATIC_DRAW);
	
	
	for(int i = 0; i < 7; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D_ARRAY, rectx.gl_texture_ids[i]);
	}
	
	int texture_arrays_loc = glGetUniformLocation(shaderProgram, "texture_arrays");
	
	int gl_1ivalues[7];
	for(int i = 0; i < 7; i++) {
		gl_1ivalues[i] = i;
	}
	glUniform1iv(texture_arrays_loc, 7, gl_1ivalues);
	
	
	// NOTE temporary texture stuff
	//int tex_256_loc = glGetUniformLocation(shaderProgram, "tex_256");
	//glUniform1i(tex_256_loc, 4);
	//glActiveTexture(GL_TEXTURE0 + 4);
	//glBindTexture(GL_TEXTURE_2D_ARRAY, rectx.gl_texture_ids[4]);
	/*
 int tex_1024_loc = glGetUniformLocation(shaderProgram, "tex_1024");
 glUniform1i(tex_1024_loc, 2);
 glActiveTexture(GL_TEXTURE0 + 2);
 glBindTexture(GL_TEXTURE_2D_ARRAY, rectx.textures_1024_id);
 //glBindSampler(0, linearFiltering);
 */
	int newBaseInstanceLoc = glGetUniformLocation(shaderProgram, "newBaseInstance");
	glUniform1i(newBaseInstanceLoc, 0);
	
	for (int i = 0; i < num_meshes; i++) {
		glUniform1i(newBaseInstanceLoc, deicommand[i].baseInstance);
		glDrawElementsInstancedBaseVertex(GL_TRIANGLES, deicommand[i].indexCount, GL_UNSIGNED_INT, (void*)(deicommand[i].firstIndex * sizeof(GL_UNSIGNED_INT)), deicommand[i].instanceCount, deicommand[i].baseVertex);
		
	}
}



// TODO: both render_Info_Skin and render_Info_Stat can be combined in some way to optimize I think
void render_Info_Skin(
Rendering_Context_Gen rectx,
NodeStack ns,
SkinStack ss,
GenModel * gma,
Transform * world_transforms,
v3 * model_scales,
uint num_models)
{
	
	// for ease of writing
	GenMeshStack gms = rectx.gms;
	
	unsigned int num_meshes = gms.next_m; 
	unsigned int num_nodes = ns.next_n;
	
	//GLuint texture = rendering_ctxt.texture;
	mat4 view = rectx.view;
	mat4 projection = rectx.projection;
	int viewLoc = rectx.info_skin.viewLoc;
	int projectionLoc = rectx.info_skin.projectionLoc;
	
	unsigned int shaderProgram = rectx.info_skin.shaderProgram;
	
	unsigned int vbo = rectx.info_skin.vbo;
	unsigned int vao = rectx.info_skin.vao;
	unsigned int indirect = rectx.info_skin.indirect;
	unsigned int ssbo_pos = rectx.info_skin.ssbo_pos;
	unsigned int ssbo_joint_pos = rectx.info_skin.ssbo_joint_pos;
	unsigned int ebo = rectx.info_skin.ebo;
	
	DrawElementsIndirectCommand deicommand[num_meshes];
	
	unsigned int num_drawn_meshes = 0;
	
	// lets try it by just counting all the elements
	int mesh_id_count[num_meshes];
	int mesh_id_base[num_meshes];
	int mesh_instance_count[num_meshes];
	
	for (int i = 0; i < num_meshes; ++i) {
		mesh_id_count[i] = 0;
		mesh_id_base[i] = 0;
		mesh_instance_count[i] = 0;
	}
	/*
 // count all the meshes
 for (size_t i = 0; i < num_nodes; i++) {
  int mesh_id = ns.nodes[i].mesh;
  if ( mesh_id == -1 || gms.meshes[mesh_id].skinned == false ) {
   continue;
  }
  mesh_id_count[ns.nodes[i].mesh]++;
  num_drawn_meshes++;
 }
 */
	for (int i = 0; i < num_models; ++i)
	{
		for (int j = 0; j < gma[i].num_nodes; ++j)
		{
			int nidxj = gma[i].node_idx + j;
			
			int mesh_id = ns.nodes[nidxj].mesh;
			if ( mesh_id == -1 || gms.meshes[mesh_id + gma[i].mesh_idx].skinned == false ) {
				continue;
			}
			mesh_id_count[mesh_id + gma[i].mesh_idx]++;
			num_drawn_meshes++;		
		}
	}
	
	
	for (int i = 0; i < num_meshes; i++) {
		if (i != 0) {
			mesh_id_base[i] = mesh_id_base[i-1] + mesh_id_count[i-1]; 
		}
		//std::cout << "model " << i << '\n';
		
		deicommand[i].indexCount = gms.meshes[i].num_i;
		deicommand[i].firstIndex = gms.meshes[i].ndx_i;
		
		deicommand[i].baseVertex = gms.meshes[i].ndx_v;
		
		deicommand[i].instanceCount = mesh_id_count[i];
		deicommand[i].baseInstance =  mesh_id_base[i];
	}
	
	SSBO_Data_Skin * ssbo_data_skin = rectx.info_skin.ssbo_data_skin;
	SSBO_Joint * ssbo_joint = rectx.info_skin.ssbo_joint;
	int next_joint = 0; 
	
	if (num_drawn_meshes > rectx.info_skin.ssbo_data_len ) {
		printf("num_drawn_meshes > rendering_ctxt.ssbo_data_len\n");
	}	
	
	for (int i = 0; i < num_models; i++) {
		
		int joint_root = next_joint;
		
		for (int j = 0; j < gma[i].num_nodes; ++j)
		{
			int c = gma[i].node_idx + j;
			;
			int relative_mesh_id = ns.nodes[c].mesh; // TODO: here it indices wrongly again
			int mesh_id = relative_mesh_id + gma[i].mesh_idx;
			
			if ( relative_mesh_id == -1 || ns.nodes[c].skin == -1 || gms.meshes[mesh_id].skinned == false ) {
				continue;
			}
            
			Skin sk = ss.skins[ns.nodes[c].skin];
            
			Transform inv_mesh_trans;
			
			inv_mesh_trans.translation = vec3(-ns.nodes[c].object_transform.translation.X,
											  -ns.nodes[c].object_transform.translation.Y,
											  -ns.nodes[c].object_transform.translation.Z);
            
			inv_mesh_trans.rotation = inverse_quat(ns.nodes[c].object_transform.rotation);
			
			mat4 mesh_joint = Convert44(ns.nodes[c].object_transform);
			
			mat4 inv_mesh_joint = Convert44(inv_mesh_trans);
			
			joint_root = next_joint;
			
			// now calculate every joint
			// TODO: temporarily we potentially calculate stuff more than necessary
			// fix this
			// although blender _always_ exports stuff in a way where never anything "unnecessary" can happen
			// and that is not a good thing
			// TODO: write more helpful TODO messages
            // TODO: joint matrices could be two quaternions instead
            // at least I remember reading about that somewhere
            // would certainly be less to send to the card, and maybe would be less costly to calculate
			for (int k = 0; k < sk.num; k++) {
				
				SkinVal sv = ss.skin_vals[sk.idx+k];
                
				// the index into this tree that represents the transform at index k
				// basically root + relative index 
				int nidx = gma[i].node_idx + sv.node;
				
				// transform of the joint k
				mat4 glo_node = Convert44(ns.nodes[nidx].object_transform);
				
				mat4 calcm = HMM_MultiplyMat4(glo_node, sv.inv_bind);
				
				//calcm = HMM_MultiplyMat4(inv_mesh_joint, calcm);
                
				ssbo_joint[next_joint].jointMat = calcm;
				next_joint = next_joint+1;
			}
			
			
			// add the transform to the right place
			// TODO: this is wrong, we are adding it to the wrong place
			int ni = mesh_id_base[mesh_id] + mesh_instance_count[mesh_id];
			ssbo_data_skin[ni].root_joint = joint_root;
            
			Transform world_transform = mul_transform(
				world_transforms[i],
				ns.nodes[c].object_transform);			  
            
			ssbo_data_skin[ni].position = vec4(
				world_transform.translation.X,
				world_transform.translation.Y,
				world_transform.translation.Z,
				0.0f); 
            
			ssbo_data_skin[ni].transform = world_transform.rotation;
            
			//ssbo_data_skin[ni].texture_type = gms.meshes[mesh_id].texid.texture_type;
			//ssbo_data_skin[ni].texture_nr = gms.meshes[mesh_id].texid.texture_nr;
			
			ssbo_data_skin[ni].texture_type = ns.texture_assets[c].type;
			ssbo_data_skin[ni].texture_nr = ns.texture_assets[c].nr;
			
			/*
            ssbo_data_skin[ni].scale = vec4(
 model_scales[i].X,
 model_scales[i].Y,
 model_scales[i].Z,
 0.0f);
            */
			mesh_instance_count[mesh_id] = mesh_instance_count[mesh_id] + 1;
		}
	}
    
	glUseProgram(shaderProgram);
    
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view.X.X);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection.X.X);
	
	glBindVertexArray(vao);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO_Data_Skin)*num_drawn_meshes, ssbo_data_skin, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssbo_pos);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_joint_pos);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(SSBO_Joint)*next_joint, ssbo_joint, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssbo_joint_pos);
	
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand)*num_meshes, deicommand, GL_STATIC_DRAW);
	
	
	for(int i = 0; i < 7; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D_ARRAY, rectx.gl_texture_ids[i]);
	}
	
	int texture_arrays_loc = glGetUniformLocation(shaderProgram, "texture_arrays");
	
	int gl_1ivalues[7];
	for(int i = 0; i < 7; i++) {
		gl_1ivalues[i] = i;
	}
	glUniform1iv(texture_arrays_loc, 7, gl_1ivalues);
	
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, num_meshes, 0);
}

void render_genmodels(
Rendering_Context_Gen rectx,
NodeStack ns,
SkinStack ss,
GenModel * gma,
v3 * model_scales,
uint num_models)
{
	START_TIME;
	render_Info_Skin_oldgl(
		rectx,
		ns,
		ss,gma,
		model_scales,
		num_models);
	
	render_Info_Stat_oldgl(
		rectx,
		ns,
		ss,gma,
		model_scales,
		num_models);
	END_TIME;
}


// TODO: this can be optimized I think
void calc_glo(NodeStack ns, GenModel dm, Transform world_transform) {
    
	ns.nodes[dm.node_idx].object_transform = ns.nodes[dm.node_idx].loc_transform;
	//ns.nodes[dm.node_idx].object_transform.translation = add_v3(ns.nodes[dm.node_idx].object_transform.translation, world_transform.translation);
	
	ns.nodes[dm.node_idx].object_transform = mul_transform(world_transform, ns.nodes[dm.node_idx].object_transform);
	
	for (int i = 1; i < dm.num_nodes; ++i)
	{
		int par = ns.parent[dm.node_idx+i];
		
		Transform par_glo_trans = ns.nodes[dm.node_idx + par].object_transform;
        
		ns.nodes[dm.node_idx+i].object_transform =
			mul_transform(par_glo_trans,ns.nodes[dm.node_idx+i].loc_transform);
	}
}


void calculate_object_transforms(NodeStack ns, GenModel * gma, Transform * world_transforms, uint num_models) {
	START_TIME;
	for (int i = 0; i < num_models; ++i)
	{
		calc_glo(ns, gma[i], world_transforms[i]);
	}
	END_TIME;
}

// TODO: add a "last index", from which we can search fore the next frame
void animate_nodes(
NodeStack * ns,
AnimStack * as,
GenModel gm,
AnimDescr ad,
uint ai,
float time )
{
	
	// select the right animation
	Anim anim = as->anims[ad.anims+ai];
	
	//std::cout << time << '\n';
	
	for (int i = 0; i < anim.num_tranch; ++i)
	{
		TranChannel tch = as->tranchs[anim.tranch+i];
		
		float chtime = 0;
		
		int tidx = -1;
        
		// find the right time
		for (int j = 0; j < tch.len; ++j)
		{
			chtime = as->times[tch.time_idx+j];
			
			if ( chtime > time ) {
				break;
			}
			
			tidx = j;
		}
		
		// if time is before our first frame, or target is not in models range
		// we can just do nothing
		if ( tidx == -1 || tch.target >= gm.num_nodes ) { continue; }
		
		int target = gm.node_idx+tch.target;
		
		// time is after our last frame, so just stick to the last frame
		if ( tidx == tch.len-1) {
			
			// get our vec3
			v3 nv3 = as->trans[tch.idx+tidx];
			
			ns->nodes[target].loc_transform.translation = nv3;
			
		} else {
			
			// get our vec3s
			v3 pv3 = as->trans[tch.idx+tidx];
			v3 nv3 = as->trans[tch.idx+tidx+1];
            
			// frametimes
			float ptime = as->times[tch.time_idx+tidx];
			float ntime = as->times[tch.time_idx+tidx+1];
			
			if ( ptime == ntime ) {
				ns->nodes[target].loc_transform.translation = nv3;
				continue;
			}
			float iv = (time - ptime) / (ntime - ptime);
			
			ns->nodes[target].loc_transform.translation = lerp_vec3(pv3,nv3,iv);
			
			v3 test = ns->nodes[target].loc_transform.translation;
            
		}
	}
	
	for (int i = 0; i < anim.num_rotch; ++i)
	{
		RotChannel rch = as->rotchs[anim.rotch+i];
		
		float chtime = 0;
		
		int tidx = -1;
		
		// find the right time
		for (int j = 0; j < rch.len; ++j)
		{
			chtime = as->times[rch.time_idx+j];
			
			if ( chtime > time ) {
				break;
			}
			
			tidx = j;
		}
		
		// time is before our first frame, so we can just do nothing
		if ( tidx == -1 || rch.target >= gm.num_nodes ) { continue; }
		
		int target = gm.node_idx+rch.target;
		
		// time is after our last frame, so just stick to the last frame
		if ( tidx == rch.len-1) {
			
			Quat nq = as->rots[rch.idx+tidx];
			
			ns->nodes[target].loc_transform.rotation = nq;
			
		} else {
			
			// get our quats
			Quat pq = as->rots[rch.idx+tidx];
			Quat nq = as->rots[rch.idx+tidx+1];
			
			// frametimes
			float ptime = as->times[rch.time_idx+tidx];
			float ntime = as->times[rch.time_idx+tidx+1];
			
			if ( ptime == ntime ) {
				ns->nodes[target].loc_transform.rotation = nq;
				continue;
			}
			
			float iv = (time - ptime) / (ntime - ptime);
			
			ns->nodes[target].loc_transform.rotation = lerp_quat(pq,nq,iv);
		}
	}
}

void animate_skeleton(
Transform * transforms,
int num_of_transforms,
AnimStack * as,
uint ai,
float time )
{
	
	// select the right animation
	Anim anim = as->anims[ai];
	
	//std::cout << time << '\n';
	
	for (int i = 0; i < anim.num_tranch; ++i)
	{
		TranChannel tch = as->tranchs[anim.tranch+i];
		
		float chtime = 0;
		
		int tidx = -1;
        
		// find the right time
		for (int j = 0; j < tch.len; ++j)
		{
			chtime = as->times[tch.time_idx+j];
			
			if ( chtime > time ) {
				break;
			}
			
			tidx = j;
		}
		
		// if time is before our first frame, or target is not in models range
		// we can just do nothing
		if ( tidx == -1 || tch.target >= num_of_transforms ) { continue; }
		
		int target = tch.target;
		
		// time is after our last frame, so just stick to the last frame
		if ( tidx == tch.len-1) {
			
			transforms[target].translation = as->trans[tch.idx+tidx];
			
		} else {
			
			// get our vec3s
			v3 pv3 = as->trans[tch.idx+tidx];
			v3 nv3 = as->trans[tch.idx+tidx+1];
            
			// frametimes
			float ptime = as->times[tch.time_idx+tidx];
			float ntime = as->times[tch.time_idx+tidx+1];
			
			if ( ptime == ntime ) {
				transforms[target].translation = nv3;
				continue;
			}
			float iv = (time - ptime) / (ntime - ptime);
			
			transforms[target].translation = lerp_vec3(pv3,nv3,iv);
            
		}
	}
	
	for (int i = 0; i < anim.num_rotch; ++i)
	{
		RotChannel rch = as->rotchs[anim.rotch+i];
		
		float chtime = 0;
		
		int tidx = -1;
		
		// find the right time
		for (int j = 0; j < rch.len; ++j)
		{
			chtime = as->times[rch.time_idx+j];
			
			if ( chtime > time ) {
				break;
			}
			
			tidx = j;
		}
		
		// time is before our first frame, so we can just do nothing
		if ( tidx == -1 || rch.target >= num_of_transforms ) { continue; }
		
		int target = rch.target;
		
		// time is after our last frame, so just stick to the last frame
		if ( tidx == rch.len-1) {
			
			transforms[target].rotation = as->rots[rch.idx+tidx];
			
		} else {
			
			// get our quats
			Quat pq = as->rots[rch.idx+tidx];
			Quat nq = as->rots[rch.idx+tidx+1];
			
			// frametimes
			float ptime = as->times[rch.time_idx+tidx];
			float ntime = as->times[rch.time_idx+tidx+1];
			
			if ( ptime == ntime ) {
				transforms[target].rotation = nq;
				continue;
			}
			
			float iv = (time - ptime) / (ntime - ptime);
			
			transforms[target].rotation = lerp_quat(pq,nq,iv);
		}
	}
}


void animate_model(
NodeStack * ns_src,
GenModel gm_src,
NodeStack * ns_result,
GenModel gm_result,
AnimStack * as,
CombinedAnimationState c_animstate)
{
    
	// TODO IMPORTANT: decide on how to manage the default state of the nodes
	// it could be a good idea for the nodes to always be somehow in the bind pose as a default,
	// so that when switching between animation some nodes won't keep the transform of their previous state
	// or in general non-animated nodes are safe
    
	// TODO: this can be optimized greatly, this is just the easiest way for testing 
    
	Transform transforms[4][gm_result.num_nodes];
    
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < gm_src.num_nodes; ++j)
		{
			transforms[i][j] = ns_src->nodes[gm_src.node_idx+j].loc_transform;
		}
	}
    
	for (int i = 0; i < 4; ++i)
	{
		animate_skeleton(
			transforms[i],
			gm_result.num_nodes,
			as,
			c_animstate.animation_states[i].anim_index,
			c_animstate.animation_states[i].time );
	}
    
	for (int j = 0; j < gm_result.num_nodes; ++j)
	{
		ns_result->nodes[gm_result.node_idx+j].loc_transform.translation =
			mul_v3f(transforms[0][j].translation, c_animstate.weight.Elements[0]);
        
		ns_result->nodes[gm_result.node_idx+j].loc_transform.rotation =
			mul_quatF(transforms[0][j].rotation, c_animstate.weight.Elements[0]);
	}
    
	for (int i = 1; i < 4; ++i)
	{
		for (int j = 0; j < gm_result.num_nodes; ++j)
		{
            
			ns_result->nodes[gm_result.node_idx+j].loc_transform.translation =add_v3(mul_v3f(transforms[i][j].translation, c_animstate.weight.Elements[i]),
																					 ns_result->nodes[gm_result.node_idx+j].loc_transform.translation);
            
			ns_result->nodes[gm_result.node_idx+j].loc_transform.rotation = HMM_NLerp(ns_result->nodes[gm_result.node_idx+j].loc_transform.rotation,c_animstate.weight.Elements[i],transforms[i][j].rotation);
            
		}		 
	}
}

// TODO: this is just a test with 2 mixed animations
// should be careful with those weights when expanding it
static void animate_model_test(
NodeStack * ns_src,
GenModel gm_src,
NodeStack * ns_result,
GenModel gm_result,
AnimStack * as,
CombinedAnimationState c_animstate)
{
    START_TIME;
	//printf("entering animate_models_test\n");
	
	//printf("animation_states selected %i , %i\n",c_animstate.animation_states[0].anim_index, c_animstate.animation_states[1].anim_index );
	
	
	Transform transforms[2][gm_result.num_nodes];
    
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < gm_src.num_nodes; ++j)
		{
			transforms[i][j] = ns_src->nodes[gm_src.node_idx+j].loc_transform;
		}
	}
    
	for (int i = 0; i < 2; ++i)
	{
		//printf("animation_state %i , %i\n", i, c_animstate.animation_states[i].anim_index);
		
		animate_skeleton(
			transforms[i],
			gm_result.num_nodes,
			as,
			c_animstate.animation_states[i].anim_index,
			c_animstate.animation_states[i].time );
	}
    
    
	for (int j = 0; j < gm_result.num_nodes; ++j)
	{
        
		ns_result->nodes[gm_result.node_idx+j].loc_transform.translation = 
            add_v3(
			mul_v3f(transforms[0][j].translation, c_animstate.weight.Elements[0]),
			mul_v3f(transforms[1][j].translation, c_animstate.weight.Elements[1]));
        
		ns_result->nodes[gm_result.node_idx+j].loc_transform.rotation = 
            lerp_quat(
			transforms[1][j].rotation,
			transforms[0][j].rotation,
			c_animstate.weight.Elements[0]);
        
	}
	END_TIME;
}

// TODO: could have a lighter version of genmodel extra for the purpose of just node-hierachies
// the extra information might be unnecessary most of the time
bool instantiate_genmodel(
NodeStack ns_src,
GenModel gm_src,
NodeStack * ns_dest,
GenModel * gm_dest)
{
	if (gm_src.num_nodes > (ns_dest->max_n - ns_dest->next_n)) {
		return false;
	}
    
	(*gm_dest) = gm_src;
	gm_dest->node_idx = ns_dest->next_n;
    
	for (int i = 0; i < gm_src.num_nodes; ++i)
	{
        ns_dest->name_indices[ns_dest->next_n] = ns_src.name_indices[gm_src.node_idx+i];
		ns_dest->nodes[ns_dest->next_n] = ns_src.nodes[gm_src.node_idx+i];
		ns_dest->texture_assets[ns_dest->next_n] = ns_src.texture_assets[gm_src.node_idx+i];
		ns_dest->parent[ns_dest->next_n] = ns_src.parent[gm_src.node_idx+i];
		ns_dest->next_n++;
	}
	return true;
}

// http://www.andreasaristidou.com/publications/papers/FABRIK.pdf
void solve_IK_fabrik(NodeStack *ns, GenModel *model, uint target_node, uint chain_length, v3 target_pos_delta, uint pole_target, float mix) {
	
	int chain[chain_length];
	v3 chain_pos[chain_length];
	v3 old_chain_pos[chain_length];
	// add the nodes to the chain
	chain[chain_length-1] = model->node_idx + target_node; 
	
	for (int i = chain_length-2; i >= 0; i--) {
		int this_node_index = model->node_idx + ns->parent[chain[i+1]];
		chain[i] = this_node_index;
	}
	
	for (int i = 0; i < chain_length; i++) {
		chain_pos[i] = ns->nodes[chain[i]].object_transform.translation;
		old_chain_pos[i] = ns->nodes[chain[i]].object_transform.translation;
	}
	
	v3 target_pos = add_v3(chain_pos[chain_length-1], target_pos_delta);
	
	// calculate the distances between the nodes
	float distances[chain_length-1];
	float combined_distance = 0.0f;
	for (int i = 0; i < chain_length-1; i++) {
		v3 parent_pos = chain_pos[i+1];
		v3 child_pos = chain_pos[i];
		
		v3 pos_dif_vec = sub_v3(child_pos, parent_pos);
		distances[i] = HMM_LengthVec3(pos_dif_vec); 
		combined_distance += distances[i];
	}
	
	// distance between root and target
	v3 root_pos = chain_pos[0];
	float root_to_target_dist = HMM_LengthVec3(sub_v3(root_pos, target_pos));
	
	if (root_to_target_dist > combined_distance) {
		// we do nothing, since we mainly want to adjust our position so that it doesn't penetrate geometry
		// the original paper has some steps here that try to make the chain "reach" for the target
		return;
	}
	
	float target_difference = HMM_LengthVec3(sub_v3(chain_pos[chain_length-1],target_pos));
	
	if (target_difference <= 0.01f) {
		return;
	}
	
	//we set the position of the knee to the pole target, so that everything works nicely
	v3 pole_target_pos = ns->nodes[model->node_idx + pole_target].object_transform.translation;
	pole_target_pos = lerp_vec3(chain_pos[chain_length-2], pole_target_pos, 1.0f);
	chain_pos[chain_length-2] = pole_target_pos;
	
	int iteration = 0;
	while(target_difference > 0.01f) {
		iteration++;
		if (iteration > 500) {
			printf("over 500 iterations of FABRIK\n");
			break;
		}
		chain_pos[chain_length-1] = target_pos;
		//printf("doing this here!\n");
		// forwards reaching
		for (int i = chain_length-2; i >= 0; i--) {
			float node_distance = HMM_LengthVec3(sub_v3(chain_pos[i+1],chain_pos[i]));
			float delta_distance = distances[i] / node_distance;
			chain_pos[i] = lerp_vec3(chain_pos[i+1], chain_pos[i], delta_distance);
		}
		chain_pos[0] = root_pos;
		// backwards reaching
		for (int i = 0; i < chain_length-1; i++) {
			float node_distance = HMM_LengthVec3(sub_v3(chain_pos[i+1],chain_pos[i]));
			float delta_distance = distances[i] / node_distance;
			chain_pos[i+1] = lerp_vec3(chain_pos[i], chain_pos[i+1], delta_distance);
		}
		target_difference = HMM_LengthVec3(sub_v3(chain_pos[chain_length-1],target_pos));
	}
	
	// now mix the new positions back and calculate the rotations
	for (int i = 0; i < chain_length; i++) {
		chain_pos[i] = lerp_vec3(old_chain_pos[i], chain_pos[i], mix);
		ns->nodes[chain[i]].object_transform.translation = chain_pos[i];
	}
	
	for (int i = 0; i < chain_length-1; i++) {
		// get the quaternion that rotates to the childs direction
		v3 child_direction = sub_v3(chain_pos[i+1], chain_pos[i]);
		v3 old_child_direction = sub_v3(old_chain_pos[i+1], old_chain_pos[i]);
		
		Quat new_rot = quaternion_rotation_from_vectors(old_child_direction, child_direction);
		
		new_rot = mul_quat(new_rot, ns->nodes[chain[i]].object_transform.rotation);
		
		ns->nodes[chain[i]].object_transform.rotation = new_rot;
	}
	
	return;
}



#endif /* RENDER_IMPLEMENTATION */
