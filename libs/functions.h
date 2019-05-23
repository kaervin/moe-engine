#include <string>
#include <bounce/bounce.h>
#include "HandmadeMath.h"

#define NUM_BONES_PER_VERTEX 4

typedef uint32_t uint;

typedef struct hmm_transform {
    hmm_quaternion rotation;
    hmm_vec3 translation;
} hmm_transform;

struct Vertex {
	float x, y, z;
	float nx, ny, nz;
	float s0, t0;
};

struct S_Vertex {
	float x, y, z;
	float nx, ny, nz;
	float s0, t0;
	uint b_index[NUM_BONES_PER_VERTEX];
	float b_weight[NUM_BONES_PER_VERTEX];
};



// A node of an animatable Hierachy
struct Node {
	int mesh;
	int skin = -1;
	//b3Transform loc_transform;
	//b3Transform glo_transform;
	hmm_transform loc_transform;
	hmm_transform glo_transform;
	//b3Vec3 scale;
};


struct Mesh {
	uint num_v;
	uint ndx_v;

	uint num_i;
	uint ndx_i;
};

struct GenMesh {
	uint num_v;
	uint ndx_v;

	uint num_i;
	uint ndx_i;

	// skinned indicates if this indexes the v_verts or normal verts
	bool skinned;
};

struct GenMeshStack {
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

	// for debugging mostly, but if streaming or reordering of data becomes necessary
	// we need to index via name hash
	//std::string * names; 

	// meshes of all the loaded models
	// index into it to retrieve vertices and such
	GenMesh * meshes;

	uint num_meshes_stat = 0;
	uint num_meshes_skin = 0;

	// vertices of all the models
	Vertex * vertices;
	// and indices
	uint * indices;

	// vertices of all the models
	S_Vertex * s_vertices;
	// and indices
	uint * s_indices;
};


struct GenModel {
	int num_meshes = -1;
	int num_nodes = -1;
	int num_skin = -1;

	uint mesh_idx;
	uint node_idx;
	uint skin;
};

struct Skin {
	uint idx; // index and num into inv_binds
	uint num;
};

struct SkinVal {
	uint node;
	hmm_mat4 inv_bind;
};

struct SkinStack {
	uint max_skin;
	uint max_val;


	uint next_skin;
	uint next_val;


	Skin * skins;
	SkinVal * skin_vals;
};

struct NodeStack {
	uint max_n;
	uint next_n;

	//std::string * names; // for debugging mostly

	int * parent;
	Node * nodes;

};

struct GenModelReturn {
	uint num_models;
	GenModel * gma;
	GenMeshStack gms;
	NodeStack ns;
	SkinStack ss;
};

struct TranChannel {

	// target node
	uint target;
	// number of keyframes and indices to them in the stack
	uint len;
	uint time_idx;
	uint idx;
};

struct RotChannel {

	// target node
	uint target;
	// number of keyframes and indices to them in the stack
	uint len;
	uint time_idx;
	uint idx;
};

struct Anim {

	float last_time;

	uint num_tranch;
	uint num_rotch;

	uint tranch;
	uint rotch;
};


struct AnimDescr {
	int num_anims;
	int anims;
};




struct AnimStack {

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

	hmm_vec3 * trans;
	hmm_quaternion * rots;
};

struct AnimReturn {
	int num_descr;
	AnimDescr * descr;

	AnimStack as;
};


NodeStack make_nodestack(
	unsigned int max_n
	);


SkinStack make_skinstack(
	unsigned int max_skin,
	unsigned int max_val
	);

bool add_node(
	NodeStack * ns,
	Node n,
	int parent
	);


void print_nodestack( NodeStack * ns );

void print_genmeshstack( const GenMeshStack& ms );


void skintest();

AnimDescr * files_to_animationdescr(
	unsigned int num_files,
	const char* * files );


AnimReturn files_to_animation(
	unsigned int num_files,
	const char* * files );

GenModelReturn * files_to_genmodels(
	unsigned int num_files,
	const char* * files );

// return the root
int add_genmodel(
	NodeStack * from,
	NodeStack * to,
	GenModel gm
	);

void print_skinstack( const SkinStack& ss );

GenModelReturn load_model_ressource_test(
	const char * filename);

void write_model_ressource_file(
	GenModelReturn * gmr,
	const char * filename);

void write_animations_file(
	AnimReturn ar,
	const char * filename);


AnimReturn load_animation_test(
	const char * filename);