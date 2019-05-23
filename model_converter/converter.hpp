#include <iostream>

#ifndef CONVERTER_H
#define CONVERTER_H

typedef struct CombinedModelPack {
	GenModelPack genmodelpack;
	AnimReturn animreturn;
} CombinedModelPack;


#endif /* CONVERTER_H */

CombinedModelPack file_to_combinedmodelpack(const char* file);

GenModelPack * files_to_genmodels(
unsigned int num_files,
const char* * files );

AnimReturn files_to_animation(
unsigned int num_files,
const char* * files );

void write_model_ressource_file(
GenModelPack * gmr,
const char * filename);

void write_animations_file(
AnimReturn ar,
const char * filename);


#ifdef CONVERTER_IMPLEMENTATION


void print_nodestack( NodeStack * ns )
{
	std::cout << "printing nodestack:" << '\n';
	for (int i = 0; i < ns->next_n; ++i)
	{
		std::cout <<
			i << " : "
			<< " mesh: " << ns->nodes[i].mesh
			<< " parent: " << ns->parent[i]
			<< " skin: " << ns->nodes[i].skin
			<< '\n';
        
	}
}

void print_skinstack( const SkinStack& ss )
{
	for (int i = 0; i < ss.next_skin; ++i)
	{
		Skin sk = ss.skins[i];
        
		std::cout << "skin: " << sk.idx << " " << sk.num << '\n';
		for (int j = 0; j < sk.num; ++j)
		{
			std::cout << ss.skin_vals[sk.idx+j].node << '\n';
		}  	
	}
}

void print_genmeshstack( const GenMeshStack& ms)
{
    
	std::cout << "printing genmeshstack: " << '\n';
    
	for (int i = 0; i < ms.next_m; ++i)
	{
        
		std::cout <<
			i << " : \n" <<
			ms.meshes[i].num_v << " " << ms.meshes[i].ndx_v << '\n' <<
			ms.meshes[i].num_i << " " << ms.meshes[i].ndx_i << '\n' <<
			ms.meshes[i].skinned << '\n';
		/*
  for (int j = 0; j < ms.meshes[i].num_v; ++j)
  {
   S_Vertex sv = ms.vertices[ms.meshes[i].ndx_v+j];
   std::cout << "j: " << j << '\n' <<
    " x: " << sv.x << " y: " << sv.y << " z: " << sv.z << '\n' <<
    " b: " << sv.b_index[0] << " " << sv.b_index[1] << " " << sv.b_index[2] << " " << sv.b_index[3] << '\n' <<
    " b: " << sv.b_weight[0] << " " << sv.b_weight[1] << " " <<  sv.b_weight[2] << " " << sv.b_weight[3] << '\n';
  }*/
	}
}

void print_animstack( AnimStack * as )
{
	std::cout << "Animations:\n";
	for (int i = 0; i < as->next_anim; ++i)
	{
		std::cout << "anim " << i << ":\n"
			<< "num_tranch: " << as->anims[i].num_tranch << "\n"
			<< "num_rotch: " << as->anims[i].num_rotch << "\n"
			<< "tranch: " << as->anims[i].tranch << "\n"
			<< "rotch: " << as->anims[i].rotch << "\n";
	}
    
	std::cout << "TranChannels:\n";
	for (int i = 0; i < as->next_tranch; ++i)
	{
		std::cout << "tranch " << i << ":\n"
			<< "target: " << as->tranchs[i].target << "\n"
			<< "len: " << as->tranchs[i].len << "\n"
			<< "time_idx: " << as->tranchs[i].time_idx << "\n"
			<< "idx: " << as->tranchs[i].idx << "\n";
        
	}
    
	std::cout << "RotChannels:\n";
	for (int i = 0; i < as->next_rotch; ++i)
	{
		std::cout << "rotch " << i << ":\n"
			<< "target: " << as->rotchs[i].target << "\n"
			<< "len: " << as->rotchs[i].len << "\n"
			<< "time_idx: " << as->rotchs[i].time_idx << "\n"
			<< "idx: " << as->rotchs[i].idx << "\n";
	}
}


NodeStack make_nodestack(
unsigned int max_n
)
{
	NodeStack ns;
	ns.max_n = max_n;
	ns.next_n = 0;
	ns.names = new Resource_Name[max_n];
    ns.name_indices = new int[max_n];
    ns.nodes = new Node[max_n];
	ns.parent = new int[max_n];
    
	return ns;
}


Resource_Name string_to_resource_name(std::string str) {
    
    Resource_Name name;
    
    int max_len = 100;
    if (str.length() < max_len) {
        max_len = str.length();
    }
    for (int i = 0; i < max_len; i++) {
        name.chars[i] = str[i];
    }
    name.chars[max_len] ='\0';
    return name;
}


bool add_node(
NodeStack * ns,
Node n,
int parent,
std::string name
)
{
    
	unsigned int new_next_n = ns->next_n + 1;
    
	if ( new_next_n > ns->max_n ) {
		return 0;
	}
    
    ns->names[ns->next_n] = string_to_resource_name(name);
    ns->name_indices[ns->next_n] = ns->next_n; 
    // NOTE: this is kind of dumb obviously, but saves us the trouble later of finding out the indices,
    // and saves us some work, since we can just memcpy to instantiate
    ns->nodes[ns->next_n] = n;
	ns->parent[ns->next_n] = parent;
    
	ns->next_n = new_next_n;
    
	return 1;
}

GenMeshStack make_genmeshstack(
unsigned int max_m,
unsigned int max_v,
unsigned int max_sv,
unsigned int max_i,
unsigned int max_si
)
{
    
	GenMeshStack ms;
	ms.max_m = max_m;
	ms.max_v = max_v;
	ms.max_i = max_i;
	ms.max_sv = max_sv;
	ms.max_si = max_si;
    
	ms.next_m = 0;
	ms.next_v = 0;
	ms.next_i = 0;
	ms.next_sv = 0;
	ms.next_si = 0;
    
    
	ms.names = new Resource_Name[max_m];
	ms.meshes = new GenMesh[max_m];
	ms.vertices = new Vertex[max_v];
	ms.indices = new unsigned int[max_i];
	ms.s_vertices = new S_Vertex[max_sv];
	ms.s_indices = new unsigned int[max_si];
	return ms;
}


SkinStack make_skinstack(
unsigned int max_skin,
unsigned int max_val
)
{
	SkinStack ss;
    
	ss.max_skin = max_skin;
	ss.max_val = max_val;
	ss.next_skin = 0;
	ss.next_val = 0;
    
	ss.skins = new Skin[max_skin];
	ss.skin_vals = new SkinVal[max_val];
    
	return ss;
}

static std::string GetFilePathExtension(const std::string &FileName)
{
	if (FileName.find_last_of(".") != std::string::npos)
		return FileName.substr(FileName.find_last_of(".") + 1);
	return "";
}

bool add_mesh(
GenMeshStack * ms,
unsigned int num_v,
unsigned int num_i,
Vertex * verts,
unsigned int * inds,
std::string name
)
{
	unsigned int next_m = ms->next_m;
	unsigned int next_v = ms->next_v;
	unsigned int next_i = ms->next_i;
    
	unsigned int new_next_m = next_m + 1;
	unsigned int new_next_v = next_v + num_v;
	unsigned int new_next_i = next_i + num_i;
    
	if ( ( new_next_v > ms->max_v ) || ( new_next_i > ms->max_i ) || ( new_next_m > ms->max_m ) ) {
		return 0;
	}
    
	ms->names[next_m] = string_to_resource_name(name);
    
	ms->meshes[next_m].num_v = num_v;
	ms->meshes[next_m].ndx_v = next_v;
    
	ms->meshes[next_m].num_i = num_i;
	ms->meshes[next_m].ndx_i = next_i;
    
	ms->meshes[next_m].skinned = false;
    
	memcpy(
		&(ms->vertices[next_v]),
		verts,
		(sizeof(Vertex)*(num_v))
        );
    
	memcpy(
		&(ms->indices[next_i]),
		inds,
		(sizeof(unsigned int)*(num_i))
        );
    
	ms->next_m = new_next_m;
	ms->next_v = new_next_v;
	ms->next_i = new_next_i;
    
	return 1;
}

bool add_s_mesh(
GenMeshStack * ms,
unsigned int num_sv,
unsigned int num_si,
S_Vertex * verts,
unsigned int * inds,
std::string name
)
{
	unsigned int next_m = ms->next_m;
	unsigned int next_sv = ms->next_sv;
	unsigned int next_si = ms->next_si;
    
	unsigned int new_next_m = next_m + 1;
	unsigned int new_next_sv = next_sv + num_sv;
	unsigned int new_next_si = next_si + num_si;
    
	if ( ( new_next_sv > ms->max_sv ) || ( new_next_si > ms->max_si ) || ( new_next_m > ms->max_m ) ) {
		std::cout << "( new_next_sv > ms->max_sv ) || ( new_next_si > ms->max_si ) || ( new_next_m > ms->max_m )\n";
		// TODO: Add assert()
		return 0;
	}
    
	ms->names[next_m] = string_to_resource_name(name);
    
	ms->meshes[next_m].num_v = num_sv;
	ms->meshes[next_m].ndx_v = next_sv;
    
	ms->meshes[next_m].num_i = num_si;
	ms->meshes[next_m].ndx_i = next_si;
    
	ms->meshes[next_m].skinned = true;
    
    
	memcpy(
		&(ms->s_vertices[next_sv]),
		verts,
		(sizeof(S_Vertex)*(num_sv))
        );
    
	memcpy(
		&(ms->s_indices[next_si]),
		inds,
		(sizeof(unsigned int)*(num_si))
        );
    
	ms->next_m = new_next_m;
	ms->next_sv = new_next_sv;
	ms->next_si = new_next_si;
    
	return 1;
}

bool load_smesh_from_model(tinygltf::Model model,  GenMeshStack * gms, int i)
{
    
	// find the accessor for the positions and normals
	int pos_acc, nor_acc, uv_acc,  ind_acc, joi_acc, wei_acc  = -1;
    
	if ( model.meshes[i].primitives.empty() ) {
		std::cout << "mesh " << i << " has no primitives" << '\n';
		return false;
	}
	
	auto attributes = model.meshes[i].primitives[0].attributes;
	
	auto search = attributes.find("POSITION");
	if(search == attributes.end()) {
		std::cout << "mesh " << i << " has no POSITION" << '\n';
		return false;
	}
	pos_acc = search->second;
    
	search = attributes.find("NORMAL");
	if(search == attributes.end()) {
		std::cout << "mesh " << i << " has no NORMAL" << '\n';
		return false;
	}
	nor_acc = search->second;
	
	search = attributes.find("TEXCOORD_0");
	if(search == attributes.end()) {
		std::cout << "mesh " << i << " has no TEXCOORD_0" << '\n';
		return false;
	}
    uv_acc = search->second;
	
	search = attributes.find("JOINTS_0");
	if(search != attributes.end()) {
		joi_acc = search->second;
	}
    
	search = attributes.find("WEIGHTS_0");
	if(search != attributes.end()) {
		wei_acc = search->second;
	}
    
	ind_acc = model.meshes[i].primitives[0].indices;
    
	// assume that each mesh refers to distinct accessors
	// if this isn't the case we end up duplicating our data
    
	tinygltf::Accessor pos_access = model.accessors[pos_acc];
	tinygltf::Accessor nor_access = model.accessors[nor_acc];
	tinygltf::Accessor uv_access = model.accessors[uv_acc];
	tinygltf::Accessor ind_access = model.accessors[ind_acc];
    
	tinygltf::BufferView pos_view = model.bufferViews[pos_access.bufferView];
	tinygltf::BufferView nor_view = model.bufferViews[nor_access.bufferView];
	tinygltf::BufferView uv_view = model.bufferViews[uv_access.bufferView];
	tinygltf::BufferView ind_view = model.bufferViews[ind_access.bufferView];
    
	tinygltf::Buffer pos_buf = model.buffers[pos_view.buffer];
	tinygltf::Buffer nor_buf = model.buffers[nor_view.buffer];
	tinygltf::Buffer uv_buf = model.buffers[uv_view.buffer];
	tinygltf::Buffer ind_buf = model.buffers[ind_view.buffer];
    
    
	//check if this model fits vec3 of floats for pos and nor, vec2 for uv, and scalar for indices
	if ( pos_access.type != 3 || nor_access.type != 3 || uv_access.type != 2 || ind_access.type != 64 + 1) {
		std::cout
			<< "type isn't vec3 of pos: " << pos_access.type
			<< " or nor: " << nor_access.type
			<< " or uv:  " << uv_access.type
			<< " or scalar of ind: " << ind_access.type << '\n';
		return false;
	}
    
	if ( pos_access.componentType != 5126 || nor_access.componentType != 5126 || uv_access.componentType != 5126) {
		std::cout
			<< "componentType isn't float of pos: " << pos_access.componentType
			<< " or nor: " << nor_access.componentType << " or uv: " << uv_access.componentType
			<< '\n';
		return false;
	}
    
	if ( ind_access.componentType != 5121 && ind_access.componentType != 5123 && ind_access.componentType != 5125 ) {
		std::cout
			<< "componentType of indice isn't 5123  or 5125, but: " << ind_access.componentType << '\n';
		return false;
	}
    
	// copy data from accessors to vertices
    
	int pos_count = pos_access.count;
	int nor_count = nor_access.count;
	int uv_count = uv_access.count;
	int ind_count = ind_access.count;
    
	//v_len += pos_count;
	//i_len += ind_count;
    
	if ( pos_count != nor_count || pos_count != uv_count ) {
		std::cout << "pos and nor/uv count are different of mesh " << i << '\n';
		return false;
	}
    
	int pos_stride = pos_access.ByteStride( pos_view );
	int nor_stride = nor_access.ByteStride( nor_view );
	int uv_stride = uv_access.ByteStride( uv_view );
	int ind_stride = ind_access.ByteStride( ind_view );
	
	int pos_offset = pos_view.byteOffset + pos_access.byteOffset;
	int nor_offset = nor_view.byteOffset + nor_access.byteOffset;
	int uv_offset = uv_view.byteOffset + uv_access.byteOffset;
	int ind_offset = ind_view.byteOffset + ind_access.byteOffset;
    
	// length in char
	int pos_length = pos_view.byteLength;
	int nor_length = nor_view.byteLength;
	int uv_length = uv_view.byteLength;
	int ind_length = ind_view.byteLength;
    
	S_Vertex * vertices = new S_Vertex[pos_count];
	unsigned int * indices = new unsigned int[ind_count];
    
	for (int j = 0; j < pos_count; ++j)
	{
		int pbi = pos_offset + pos_stride*j;
		float * x_addr = (float *)(&pos_buf.data[pbi]);
		float * y_addr = (float *)(&pos_buf.data[pbi+4]);
		float * z_addr = (float *)(&pos_buf.data[pbi+8]);
        
		int nbi = nor_offset + nor_stride*j;
		float * nx_addr = (float *)(&pos_buf.data[nbi]);
		float * ny_addr = (float *)(&pos_buf.data[nbi+4]);
		float * nz_addr = (float *)(&pos_buf.data[nbi+8]);
        
		int uvbi = uv_offset + uv_stride*j;
		float * uvx_addr = (float *)(&uv_buf.data[uvbi]);
		float * uvy_addr = (float *)(&uv_buf.data[uvbi+4]);
		
		vertices[j].x = *x_addr;
		vertices[j].y = *y_addr;
		vertices[j].z = *z_addr;
        
		vertices[j].nx = *nx_addr;
		vertices[j].ny = *ny_addr;
		vertices[j].nz = *nz_addr;
        
		vertices[j].s0 = *uvx_addr;
		vertices[j].t0 = *uvy_addr;
        
		//std::cout << "j: " << j << " bi: " << pbi << " x y z: " << x << " " << y << " " << z << '\n';
		//std::cout << "j: " << j << " bi: " << nbi << " nx ny nz: " << nx << " " << ny << " " << nz << '\n';
        
		//printVertex(vertices[j], j);
	}
    
    
	// if there is joint and weight data, add them to the vertices
	if ( joi_acc != -1 && wei_acc != -1 ) {
        
		tinygltf::Accessor joi_access = model.accessors[joi_acc];
		tinygltf::Accessor wei_access = model.accessors[wei_acc];
        
		tinygltf::BufferView joi_view = model.bufferViews[joi_access.bufferView];
		tinygltf::BufferView wei_view = model.bufferViews[wei_access.bufferView];
        
		tinygltf::Buffer joi_buf = model.buffers[joi_view.buffer];
		tinygltf::Buffer wei_buf = model.buffers[wei_view.buffer];
        
		if ( joi_access.type != 4 || wei_access.type != 4 ) {
			std::cout
				<< "type isn't vec4 of joi: " << joi_access.type
				<< " or wei: " << wei_access.type << '\n';
			return false;
		}
        
		if ( wei_access.componentType != 5126) {
			std::cout
				<< "componentType isn't float of wei: " << wei_access.componentType
				<< '\n';
			return false;
		}
        
		if ( joi_access.componentType != 5123 && joi_access.componentType != 5125 ) {
			std::cout
				<< "componentType of joints isn't 5123  or 5125, but: " << joi_access.componentType << '\n';
			return false;
		}
        
		int joi_count = joi_access.count;
		int wei_count = wei_access.count;
        
		if ( joi_count != pos_count || wei_count != pos_count ) {
			std::cout
				<< "the length of joi or wei is not the same as pos \n";
			return false;
		}
        
		int joi_stride = joi_access.ByteStride( joi_view );
		int wei_stride = wei_access.ByteStride( wei_view );
        
		int joi_offset = joi_view.byteOffset + joi_access.byteOffset;
		int wei_offset = wei_view.byteOffset + wei_access.byteOffset;
        
		// length in char
		int joi_length = joi_view.byteLength;
		int wei_length = wei_view.byteLength;
        
		for (int j = 0; j < pos_count; ++j)
		{
			int jbi = joi_offset + joi_stride*j;
            
			unsigned short * j0_addr = (unsigned short *)(&joi_buf.data[jbi]);
			unsigned short * j1_addr = (unsigned short *)(&joi_buf.data[jbi + 2]);
			unsigned short * j2_addr = (unsigned short *)(&joi_buf.data[jbi + 4]);
			unsigned short * j3_addr = (unsigned short *)(&joi_buf.data[jbi + 6]);
            
			int wbi = wei_offset + wei_stride*j;
            
			float * w0_addr = (float *)(&wei_buf.data[wbi]);
			float * w1_addr = (float *)(&wei_buf.data[wbi+4]);
			float * w2_addr = (float *)(&wei_buf.data[wbi+8]);
			float * w3_addr = (float *)(&wei_buf.data[wbi+12]);
            
			vertices[j].b_index[0] = *j0_addr;
			vertices[j].b_index[1] = *j1_addr;
			vertices[j].b_index[2] = *j2_addr;
			vertices[j].b_index[3] = *j3_addr;
            
			vertices[j].b_weight[0] = *w0_addr;
			vertices[j].b_weight[1] = *w1_addr;
			vertices[j].b_weight[2] = *w2_addr;
			vertices[j].b_weight[3] = *w3_addr;
            
		}
        
	}
    
    
	for (int j = 0; j < ind_count; ++j)
	{
		int ibi = ind_offset + ind_stride*j;
        
		unsigned int index;
		// if byte
		if ( ind_access.componentType == 5121 ) {
			unsigned char * ix = (unsigned char *)(&ind_buf.data[ibi]);
			index = (unsigned int)*ix;
		}
		// if short
		if ( ind_access.componentType == 5123 ) {
			unsigned short * ix = (unsigned short *)(&ind_buf.data[ibi]);
			index = (unsigned int)*ix;
		}
		if ( ind_access.componentType == 5125 ) {
			unsigned int * ix = (unsigned int *)(&ind_buf.data[ibi]);
			index = *ix;
		}
		indices[j] = index;
		//std::cout << "index " << j << " " << index << '\n';
	}
    
	bool success = add_s_mesh(
		gms,
		pos_count,
		ind_count,
		vertices,
		indices,
		model.meshes[i].name
		);
    
	if ( !success ) {
		std::cout << "couldn't load s model: !success\n";
		return false;
	}
	return true;
}

bool load_mesh_from_model(tinygltf::Model model,  GenMeshStack * gms, int i)
{
	// find the accessor for the positions and normals
	int pos_acc, nor_acc, uv_acc, ind_acc = -1;
    
	if ( model.meshes[i].primitives.empty() ) {
		std::cout << "mesh " << i << " has no primitives" << '\n';
		return false;
	}
	auto attributes = model.meshes[i].primitives[0].attributes;
	auto search = attributes.find("POSITION");
	if(search == attributes.end()) {
		std::cout << "mesh " << i << " has no POSITION" << '\n';
		return false;
	}
	pos_acc = search->second;
    
	search = attributes.find("NORMAL");
	if(search == attributes.end()) {
		std::cout << "mesh " << i << " has no NORMAL" << '\n';
		return false;
	}
	nor_acc = search->second;
    
	search = attributes.find("TEXCOORD_0");
	if(search == attributes.end()) {
		std::cout << "mesh " << i << " has no TEXCOORD_0" << '\n';
		return false;
	}
    uv_acc = search->second;
	
	
	ind_acc = model.meshes[i].primitives[0].indices;
    
	// assume that each mesh refers to distinct accessors
	// if this isn't the case we end up duplicating our data
    
	tinygltf::Accessor pos_access = model.accessors[pos_acc];
	tinygltf::Accessor nor_access = model.accessors[nor_acc];
	tinygltf::Accessor uv_access = model.accessors[uv_acc];
	tinygltf::Accessor ind_access = model.accessors[ind_acc];
    
	tinygltf::BufferView pos_view = model.bufferViews[pos_access.bufferView];
	tinygltf::BufferView nor_view = model.bufferViews[nor_access.bufferView];
	tinygltf::BufferView uv_view = model.bufferViews[uv_access.bufferView];
	tinygltf::BufferView ind_view = model.bufferViews[ind_access.bufferView];
    
	tinygltf::Buffer pos_buf = model.buffers[pos_view.buffer];
	tinygltf::Buffer nor_buf = model.buffers[nor_view.buffer];
	tinygltf::Buffer uv_buf = model.buffers[uv_view.buffer];
	tinygltf::Buffer ind_buf = model.buffers[ind_view.buffer];
    
	//check if this model fits vec3 of floats for pos and nor, vec2 for uv, and scalar for indices
	if ( pos_access.type != 3 || nor_access.type != 3 || uv_access.type != 2 || ind_access.type != 64 + 1) {
		std::cout
			<< "type isn't vec3 of pos: " << pos_access.type
			<< " or nor: " << nor_access.type
			<< " or uv:  " << uv_access.type
			<< " or scalar of ind: " << ind_access.type << '\n';
		return false;
	}
    
	if ( pos_access.componentType != 5126 || nor_access.componentType != 5126 || uv_access.componentType != 5126) {
		std::cout
			<< "componentType isn't float of pos: " << pos_access.componentType
			<< " or nor: " << nor_access.componentType << " or uv: " << uv_access.componentType
			<< '\n';
		return false;
	}
    
	if ( ind_access.componentType != 5121 && ind_access.componentType != 5123 && ind_access.componentType != 5125 ) {
		std::cout
			<< "componentType of indice isn't 5123  or 5125, but: " << ind_access.componentType << '\n';
		return false;
	}
    
	// copy data from accessors to vertices
    
	int pos_count = pos_access.count;
	int nor_count = nor_access.count;
	int uv_count = uv_access.count;
	int ind_count = ind_access.count;
    
	if ( pos_count != nor_count || pos_count != uv_count ) {
		std::cout << "pos and nor/uv count are different of mesh " << i << '\n';
		return false;
	}
    
	int pos_stride = pos_access.ByteStride( pos_view );
	int nor_stride = nor_access.ByteStride( nor_view );
	int uv_stride = uv_access.ByteStride( uv_view );
	int ind_stride = ind_access.ByteStride( ind_view );
	
	int pos_offset = pos_view.byteOffset + pos_access.byteOffset;
	int nor_offset = nor_view.byteOffset + nor_access.byteOffset;
	int uv_offset = uv_view.byteOffset + uv_access.byteOffset;
	int ind_offset = ind_view.byteOffset + ind_access.byteOffset;
    
	// length in char
	int pos_length = pos_view.byteLength;
	int nor_length = nor_view.byteLength;
	int uv_length = uv_view.byteLength;
	int ind_length = ind_view.byteLength;
    
	Vertex * vertices = new Vertex[pos_count];
	unsigned int * indices = new unsigned int[ind_count];
    
	for (int j = 0; j < pos_count; ++j)
	{
		int pbi = pos_offset + pos_stride*j;
		float * x_addr = (float *)(&pos_buf.data[pbi]);
		float * y_addr = (float *)(&pos_buf.data[pbi+4]);
		float * z_addr = (float *)(&pos_buf.data[pbi+8]);
        
		int nbi = nor_offset + nor_stride*j;
		float * nx_addr = (float *)(&pos_buf.data[nbi]);
		float * ny_addr = (float *)(&pos_buf.data[nbi+4]);
		float * nz_addr = (float *)(&pos_buf.data[nbi+8]);
        
		int uvbi = uv_offset + uv_stride*j;
		float * uvx_addr = (float *)(&uv_buf.data[uvbi]);
		float * uvy_addr = (float *)(&uv_buf.data[uvbi+4]);
		
		vertices[j].x = *x_addr;
		vertices[j].y = *y_addr;
		vertices[j].z = *z_addr;
        
		vertices[j].nx = *nx_addr;
		vertices[j].ny = *ny_addr;
		vertices[j].nz = *nz_addr;
        
		vertices[j].s0 = *uvx_addr;
		vertices[j].t0 = *uvy_addr;
        
		//std::cout << "j: " << j << " bi: " << pbi << " x y z: " << x << " " << y << " " << z << '\n';
		//std::cout << "j: " << j << " bi: " << nbi << " nx ny nz: " << nx << " " << ny << " " << nz << '\n';
        
		//printVertex(vertices[j], j);
	}
    
	for (int j = 0; j < ind_count; ++j)
	{
		int ibi = ind_offset + ind_stride*j;
        
		unsigned int index;
		// if byte
		if ( ind_access.componentType == 5121 ) {
			unsigned char * ix = (unsigned char *)(&ind_buf.data[ibi]);
			index = (unsigned int)*ix;
		}
		// if short
		if ( ind_access.componentType == 5123 ) {
			unsigned short * ix = (unsigned short *)(&ind_buf.data[ibi]);
			index = (unsigned int)*ix;
		}
		if ( ind_access.componentType == 5125 ) {
			unsigned int * ix = (unsigned int *)(&ind_buf.data[ibi]);
			index = *ix;
		}
		indices[j] = index;
		//std::cout << "index " << j << " " << index << '\n';
	}
    
	bool success = add_mesh(
		gms,
		pos_count,
		ind_count,
		vertices,
		indices,
		model.meshes[i].name
		);
    
	if ( !success ) {
		return false;
	}
	return true;
}



bool blender_build_skin_new(
NodeStack * ns,
std::vector<tinygltf::Node> nodes,
int mesh_offset,
int skin_offset
)
{
	//printf("we're in blender build skin now!\n");
	//printf("nodes.size %li\n", nodes.size());
	int temp_parents[nodes.size()];
	for (int i = 0; i < nodes.size(); ++i)
	{
		temp_parents[i] = -1;
	}
    
    
	for (int i = 0; i < nodes.size(); i++)
	{
        
        
		tinygltf::Node c = nodes[i];
		Node n;
        
		//std::cout << "node " << i << " mesh " << c.mesh << " name " << c.name << " children " << c.children.size() << std::endl;
        
		for (int chi = 0; chi < c.children.size(); ++chi)
		{
			temp_parents[c.children[chi]] = i; 
		}
        
		int parent = temp_parents[i];
        
		n.mesh = -1;
		if ( c.mesh != -1 ) {
			n.mesh = c.mesh + mesh_offset;
		}
        
        
		hmm_quaternion rot;
		hmm_vec3 trans;
        
		if ( c.translation.size() == 3 ) {
			trans = HMM_Vec3(
				(float)c.translation[0],
				(float)c.translation[1],
				(float)c.translation[2]);
		} else {
			trans = HMM_Vec3(0.0f, 0.0f, 0.0f);
		}
        
		if ( c.rotation.size() == 4 ) {
			rot.X = (float)c.rotation[0];
			rot.Y = (float)c.rotation[1];
			rot.Z = (float)c.rotation[2];
			rot.W = (float)c.rotation[3];
		} else {
			rot.X = 0.0f;
			rot.Y = 0.0f;
			rot.Z = 0.0f;
			rot.W = 1.0f;
		}
        
        
		// TODO: check if maybe there is a matrix instead
		// but maybe just ignore that shit, since animation doesn't support matrices anyway
        
		n.loc_transform.rotation = rot;
		n.loc_transform.translation = trans;
        
		n.skin = -1;
		if (c.skin != -1) {
			n.skin = c.skin + skin_offset;
		}
        
		bool success = add_node(ns, n, parent, c.name);
		if (!success) {
			return 0;
		}
	}
    
	return 1;
}

struct IntArray{
	int *data;
	int next_int;
};

// linearizes the node hierachy, saves the original gltf-index into node_raname_array, so that we can later rename the node-indices in the skinstack 
bool recursive_build_nodes(
NodeStack * ns,
std::vector<tinygltf::Node> nodes,
int mesh_offset,
int skin_offset,
int current_node_index,
int parent_node_index,
struct IntArray *node_rename_array)
{
	
	tinygltf::Node c = nodes[current_node_index];
	Node n;
	
	std::cout << "node " << current_node_index << " mesh " << c.mesh << " name " << c.name << " children " << c.children.size() << std::endl;
	std::cout << "node_rename_array->data[parent_node_index] " << node_rename_array->data[parent_node_index]  << "\n";
	
	n.mesh = -1;
	if ( c.mesh != -1 ) {
		n.mesh = c.mesh + mesh_offset;
	}
	
	hmm_quaternion rot;
	hmm_vec3 trans;
	
	if ( c.translation.size() == 3 ) {
		trans = HMM_Vec3(
			(float)c.translation[0],
			(float)c.translation[1],
			(float)c.translation[2]);
	} else {
		trans = HMM_Vec3(0.0f, 0.0f, 0.0f);
	}
	
	if ( c.rotation.size() == 4 ) {
		rot.X = (float)c.rotation[0];
		rot.Y = (float)c.rotation[1];
		rot.Z = (float)c.rotation[2];
		rot.W = (float)c.rotation[3];
	} else {
		rot.X = 0.0f;
		rot.Y = 0.0f;
		rot.Z = 0.0f;
		rot.W = 1.0f;
	}
	
	n.loc_transform.rotation = rot;
	n.loc_transform.translation = trans;
	
	n.skin = -1;
	if (c.skin != -1) {
		n.skin = c.skin + skin_offset;
	}
	
	bool success = add_node(ns, n, parent_node_index, c.name);
	if (!success) {
		return 0;
	}
	
	// the next int of the node rename array is used to see what our relative place in the nodestack is
	int new_node_index = node_rename_array->next_int;
	// now insert our gltf-index into the rename array
	node_rename_array->data[current_node_index] = new_node_index;
	node_rename_array->next_int++;
	
	for (int chi = 0; chi < c.children.size(); chi++)
	{
		success = recursive_build_nodes(
			ns,
			nodes,
			mesh_offset,
			skin_offset,
			c.children[chi],
			new_node_index,
			node_rename_array);
		if (!success) return 0;
	}
	
	return 1;
}

GenModel * gltf_to_generalmodel(
tinygltf::Model model,
GenMeshStack * gms,
NodeStack * ns,
SkinStack * ss,
struct IntArray *node_rename_array)
{
    
	unsigned int next_m = gms->next_m;
	unsigned int next_n = ns->next_n;
	unsigned int next_skin = ss->next_skin;
    
	if ( model.scenes.size() == 0 ) {
		std::cout << "no scenes \n";
		return NULL;
	}
    
	if ( model.defaultScene == -1 ) {
		std::cout << "no def scene, loading scene 0\n";
		model.defaultScene = 0;
	}
    
	if ( model.scenes[model.defaultScene].nodes.size() == 0 ) {
		return NULL;
	}
    
	// just load model 0 for now
	// will program of loading more if there is a justification for that
	int root = model.scenes[model.defaultScene].nodes[0];
	
	bool success = recursive_build_nodes(
		ns,
		model.nodes,
		next_m,
		next_skin,
		0,
		-1,
		node_rename_array);
	
	/*
bool success = blender_build_skin_new(
ns,
model.nodes,
next_m,
next_skin
);
*/
	//std::cout << "next_m: " << next_m << '\n';
    
	if ( !success ) {
		std::cout << "fucking failed!\n" << std::endl;
		return NULL;
	}
    
	
	// load the skin
	if ( model.skins.size() == 0 ) {
		std::cout << "no skins \n";
	} else {
        
		// get all the skins
		for (int i = 0; i < model.skins.size(); ++i)
		{
			tinygltf::Skin skin = model.skins[i];
            
			if ( skin.inverseBindMatrices == -1 ) {
				std::cout << "there are no invbinds we can load for this models skin \n";
				continue;
			}
            
			tinygltf::Accessor ski_access = model.accessors[skin.inverseBindMatrices];
            
			tinygltf::BufferView ski_view = model.bufferViews[ski_access.bufferView];
            
			tinygltf::Buffer ski_buf = model.buffers[ski_view.buffer];
            
			//check if binds are mat4
			if ( ski_access.type != 36 ) {
				std::cout
					<< " binds aren't mat4: " << ski_access.type << '\n';
				return NULL;
			}
            
			if ( ski_access.componentType != 5126 ) {
				std::cout
					<< "componentType isn't float of ski: " << ski_access.componentType << '\n';
				return NULL;
			}
            
			int ski_count = ski_access.count;
            
			if ( ski_count < skin.joints.size() ) {
				std::cout << "ski is smaller than joints size" << '\n';
				return NULL;
			}
            
			int ski_stride = ski_access.ByteStride( ski_view );
			int ski_offset = ski_view.byteOffset + ski_access.byteOffset;
			int ski_length = ski_view.byteLength;
            
			Skin sk;
			sk.idx = ss->next_val;
			sk.num = skin.joints.size();
			ss->skins[ss->next_skin] = sk;
			ss->next_skin += 1;
            
			for (int j = 0; j < skin.joints.size(); ++j)
			{
				int sbi = ski_offset + ski_stride*j;
                
				hmm_mat4 * m_addr = (hmm_mat4 *)(&ski_buf.data[sbi]);
				
				ss->skin_vals[ss->next_val].node = node_rename_array->data[skin.joints[j]]; //TEST
				std::cout << "skins " << ss->skin_vals[ss->next_val].node << std::endl;
				ss->skin_vals[ss->next_val].inv_bind = *m_addr;
				ss->next_val += 1;
				
                
                
			}
		}		
	}
	
	
	
	// get all meshes in temporary meshes
    
	for (int i = 0; i < model.meshes.size(); ++i)
	{
        
		// find the accessor for the positions and normals
		int pos_acc, nor_acc, ind_acc, joi_acc, wei_acc  = -1;
        
		if ( model.meshes[i].primitives.empty() ) {
			std::cout << "mesh " << i << " has no primitives" << '\n';
			return NULL;
		}
		auto attributes = model.meshes[i].primitives[0].attributes;
		auto search = attributes.find("POSITION");
		if(search == attributes.end()) {
			std::cout << "mesh " << i << " has no POSITION" << '\n';
			return NULL;
		}
		pos_acc = search->second;
        
		search = attributes.find("NORMAL");
		if(search == attributes.end()) {
			std::cout << "mesh " << i << " has no NORMAL" << '\n';
			return NULL;
		}
		nor_acc = search->second;
        
		search = attributes.find("JOINTS_0");
		if(search != attributes.end()) {
			joi_acc = search->second;
		}
        
		search = attributes.find("WEIGHTS_0");
		if(search != attributes.end()) {
			wei_acc = search->second;
		}
        
		if ( joi_acc != -1 && wei_acc != -1 ) {
			load_smesh_from_model(model, gms, i);
		} else {
			load_mesh_from_model(model, gms, i);
		}
        
	}
    
	//print_skinnedmeshstack(*sms);
    
	// TODO: for now dynamically allocated
	// will stop doing this when I am sure how I will allocate this
	// maybe just a functional approach is the best
	// not like skinnedmodels are large
	GenModel * ret_dm = new GenModel();
	ret_dm->num_meshes = gms->next_m - next_m;
	ret_dm->num_nodes = ns->next_n - next_n;
	ret_dm->mesh_idx = next_m;
	ret_dm->node_idx = next_n;
    
    
	return ret_dm;
}
/*
GenModelPack * files_to_genmodels(
unsigned int num_files,
const char* * files )
{
 GenModel * gma = new GenModel[num_files];
 
 tinygltf::TinyGLTF loader;
 tinygltf::Model model[num_files];
 
 bool ret = false;
 
 unsigned int num_nodes = 0;
 unsigned int num_meshes = 0;
 unsigned int num_vertices = 0;
 unsigned int num_indices = 0;
 
 std::string err;
 
 for (int i = 0; i < num_files; ++i)
 {
  const std::string input_filename(files[i]);
  std::string ext = GetFilePathExtension(input_filename);
  
  if (ext.compare("glb") == 0) {
   // assume binary glTF.
   ret = loader.LoadBinaryFromFile(&model[i], &err, &err, input_filename);
  } else {
   // assume ascii glTF.
   ret = loader.LoadASCIIFromFile(&model[i], &err, &err, input_filename);
  }
  if (!err.empty()) {
   printf("ERR: %s\n", err.c_str());
   return NULL;
  }
  if (!ret) {
   std::cout << "Failed to load .glTF : " << files[i] << '\n';
   return NULL;
  }
  
  num_nodes += model[i].nodes.size();
  num_meshes += model[i].meshes.size();
 }
 
 
 // just use seomthing for now
 // TODO: do the real thing after testing
 num_vertices = 1024 << 8;
 num_indices = 1024 << 8;
 int num_skins = 1024 << 8;
 int num_vals = 1024 << 8;
 
 
 GenMeshStack ms = make_genmeshstack(num_meshes, num_vertices, num_vertices, num_indices, num_indices);
 NodeStack ns = make_nodestack(num_nodes);
 SkinStack ss = make_skinstack(num_skins,num_vals);
 
 
 for (int i = 0; i < num_files; ++i)
 {
  GenModel * ret = gltf_to_generalmodel(model[i], &ms, &ns, &ss);
  if ( !ret ) {
   return NULL;
  }
  gma[i] = *ret;
  
 }
 
 GenModelPack * mret = new GenModelPack();
 mret->num_models = num_files;
 mret->gma = gma;
 mret->gms =  ms;
 mret->ns = ns;
 mret->ss = ss;
 
 return mret;
}
*/
AnimStack make_animstack(
unsigned int max_anims,
unsigned int max_times,
unsigned int max_tranch,
unsigned int max_rotch,
unsigned int max_tran,
unsigned int max_rot
)
{
	AnimStack as;
    
	as.max_anims = max_anims;
	as.max_times = max_times;
	as.max_tranch = max_tranch;
	as.max_rotch = max_rotch;
	as.max_tran = max_tran;
	as.max_rot = max_rot;
    
	as.next_anim = 0;
	as.next_time = 0;
	as.next_tranch = 0;
	as.next_rotch = 0;
	as.next_trans = 0;
	as.next_rot = 0;
    
	as.anims = new Anim[max_anims];
	as.times = new float[max_times];
	as.tranchs = new TranChannel[max_tranch];
	as.rotchs = new RotChannel[max_rotch];
	as.trans = new hmm_vec3[max_tran];
	as.rots = new hmm_quaternion[max_rot];
    
	return as;
}


struct SamplerIdx {
	uint sampler;
	uint idx;
	uint len; 
};


void load_animationsampler_data(
tinygltf::Model model,
tinygltf::AnimationSampler sampler, // the animationsampler we want to load into memory
AnimStack *as,
SamplerIdx *sidx,
uint * next_sidx )
{
    
    
	// figure out if the sampler has already been loaded
	// if yes, then do nothing
	// if not append the new data to the end
    
	// first for the time/input
	int i = 0;
	for (; i < *next_sidx; ++i)
	{
		if ( sidx[i].sampler == sampler.input ) {
			break;
		}
	}
	if ( i == *next_sidx ) {
		// get the time data onto the stack
		SamplerIdx sx;
		sx.sampler = sampler.input;
		sx.idx = as->next_time;
        
		int time_acc = sampler.input;
		tinygltf::Accessor time_access = model.accessors[time_acc];
        
		tinygltf::BufferView time_view = model.bufferViews[time_access.bufferView];
        
		tinygltf::Buffer time_buf = model.buffers[time_view.buffer];
        
		// check if it fits scala for times
		if ( time_access.type != 65 ) {
			std::cout << "time_access.type isn't scalar \n";
		}
        
		if ( time_access.componentType != 5126 ) {
			std::cout << "time_access.componentType isn't float \n";
		}
        
		int time_count = time_access.count;
		sx.len = time_count;
        
		int time_stride = time_access.ByteStride(time_view);
        
		int time_offset = time_view.byteOffset + time_access.byteOffset;
        
		// length in char
		int time_length = time_view.byteLength;
        
		// push data onto stack
		for (int j = 0; j < time_count; ++j)
		{
			int tbi = time_offset + time_stride*j;
			float * t_addr = (float *)(&time_buf.data[tbi]);
			as->times[as->next_time] = *t_addr;
			as->next_time = as->next_time + 1;
		}
        
		// add the sampleridx to the stack
		sidx[*next_sidx] = sx;
		*next_sidx = *next_sidx + 1;
	}
    
	// now for the output
	for (i = 0; i < *next_sidx; ++i)
	{
		if ( sidx[i].sampler == sampler.output ) {
			break;
		}
	}
	if ( i == *next_sidx ) {
		// get the data onto the right (tran or rot) stack
        
		SamplerIdx sx;
		sx.sampler = sampler.output;
        
		tinygltf::Accessor access = model.accessors[sampler.output];
        
		tinygltf::BufferView view = model.bufferViews[access.bufferView];
        
		tinygltf::Buffer buf = model.buffers[view.buffer];
        
		if ( access.componentType != 5126 ) {
			std::cout << " componentType of accessor is not flaot \n";
		}
        
		int count = access.count;
		sx.len = count;
        
		int stride = access.ByteStride(view);
		int offset = view.byteOffset + access.byteOffset;
        
		// length in char
		int length = view.byteLength;
        
		// check of its either tran data (vec3) or rot data (vec4)
		if ( access.type == 3 ) {
            
			sx.idx = as->next_trans;
            
			for (int j = 0; j < count; ++j)
			{
				int bi = offset + stride*j;
				float * x_addr = (float *)(&buf.data[bi]);
				float * y_addr = (float *)(&buf.data[bi+4]);
				float * z_addr = (float *)(&buf.data[bi+8]);
                
				as->trans[as->next_trans] = HMM_Vec3(*x_addr,*y_addr,*z_addr);
				as->next_trans = as->next_trans + 1;
			}
		}
        
		if ( access.type == 4 ) {
            
			sx.idx = as->next_rot;
            
			for (int j = 0; j < count; ++j)
			{
				int bi = offset + stride*j;
				float * x_addr = (float *)(&buf.data[bi]);
				float * y_addr = (float *)(&buf.data[bi+4]);
				float * z_addr = (float *)(&buf.data[bi+8]);
				float * w_addr = (float *)(&buf.data[bi+12]);
                
                
				as->rots[as->next_rot].X = *x_addr;
				as->rots[as->next_rot].Y = *y_addr;
				as->rots[as->next_rot].Z = *z_addr;
				as->rots[as->next_rot].W = *w_addr;
				as->next_rot = as->next_rot + 1;
			}
		}
		// add the sampleridx to the stack
		sidx[*next_sidx] = sx;
		*next_sidx = *next_sidx + 1;		
	}
}


void load_anim(
int ani,
tinygltf::Model model,
AnimStack *as,
struct IntArray *node_rename_array)
{
	Anim an;
    
	an.num_tranch = 0;
	an.num_rotch = 0;
	an.tranch = as->next_tranch;
	an.rotch = as->next_rotch;
    
	// remember the time index before loading anything, so that we can afterwards loop through all the times
	// and get the time of our last frame this way
	int time_index_at_start = as->next_time;
    
	tinygltf::Animation gan = model.animations[ani];
    
	an.name = string_to_resource_name(gan.name);
    
	// SamplerIdx is the place where we will store the corresponding index and len of a specific sampler
	SamplerIdx sidx[gan.channels.size()*2];
	uint next_sidx = 0;
    
    
	// gather which samplers we will need/consider
	// for now we will save that in the channels idx, which usually stores the id into the stack
	// but this way we won't have to allocate anything new
    
	//std::cout << "gan.channels.size(): " << gan.channels.size() << "\n";
    
	for (int i = 0; i < gan.channels.size(); ++i)
	{
        
		if ( gan.channels[i].target_path != "translation" && gan.channels[i].target_path != "rotation" ) {
			continue;
		}
        
		load_animationsampler_data(
			model,
			gan.samplers[gan.channels[i].sampler],
			as,
			sidx,
			&next_sidx );
        
		// now look for the fitting channel values inside of sidx
        
		int input = gan.samplers[gan.channels[i].sampler].input;
		int output = gan.samplers[gan.channels[i].sampler].output;
        
		if (gan.channels[i].target_node >= node_rename_array->next_int) {
			printf("gan.channels[i].target_node >= node_rename_array->next_int");
		};
		
		uint chtarget = node_rename_array->data[gan.channels[i].target_node];
		uint chlen = -1;
		uint chtime_idx;
		uint chidx;
        
		for (int j = 0; j < next_sidx; j++) {
			if (sidx[j].sampler == input) {
				chlen = sidx[j].len;
				//std::cout << "sidx[" << j << "].sampler: " << sidx[j].sampler << '\n'; 
				//std::cout << "sidx[" << j << "].len: " << sidx[j].len << '\n'; 
				chtime_idx = sidx[j].idx;
			}
		}
        
		for (int j = 0; j < next_sidx; j++) {
			if (sidx[j].sampler == output) {
				if ( chlen != sidx[j].len ) {
					std::cout << "error: chlen != sidx[j].len\n";
				}
				chidx = sidx[j].idx;
			}
		}
        
		if ( gan.channels[i].target_path == "translation" ) {
			TranChannel tch;
			tch.target = chtarget;
			tch.len = chlen;
			tch.time_idx = chtime_idx;
			tch.idx = chidx;
            
			as->tranchs[as->next_tranch] = tch;
			as->next_tranch = as->next_tranch + 1;
			an.num_tranch = an.num_tranch + 1;
            
			//std::cout << "num_tranch " << an.num_tranch << std::endl;
            
            
		}
		if ( gan.channels[i].target_path == "rotation" ) {
			RotChannel rch;
			rch.target = chtarget;
			rch.len = chlen;
			rch.time_idx = chtime_idx;
			rch.idx = chidx;
            
			as->rotchs[as->next_rotch] = rch;
			as->next_rotch = as->next_rotch + 1;
			an.num_rotch = an.num_rotch + 1;
		}
        
	}
    
	an.last_time = 0.0;
    
	for (int i = time_index_at_start; i < as->next_time; i++) {
		if ( as->times[i] > an.last_time ) {
			an.last_time = as->times[i];
		}
	}
	
    
	as->anims[as->next_anim] = an;
	as->next_anim = as->next_anim + 1;
}


AnimDescr load_animation(
tinygltf::Model model,
AnimStack * as,
IntArray *node_rename_array)
{
	AnimDescr ad;
    
	if ( model.scenes.size() == 0 ) {
		std::cout << "no scenes \n";
		return ad;
	}
	if ( model.defaultScene == -1 ) {
		std::cout << "no def scene, loading scene 0\n";
		model.defaultScene = 0;
	}
    
	uint num_anims = model.animations.size();
	ad.num_anims = num_anims;
	ad.anims = as->next_anim;
    
	//std::cout << "ad.anims: " << ad.anims << "\n";
    
	for (int i = 0; i < num_anims; ++i)
	{
		//std::cout << "loading anim: " << i << '\n';
		load_anim(i, model, as, node_rename_array);
	}
    
    
	//print_animstack(as); 
    
	return ad;
}


CombinedModelPack file_to_combinedmodelpack(const char* file) {
	
	CombinedModelPack ret_modelpack;
	
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
    
	unsigned int num_nodes = 0;
	unsigned int num_meshes = 0;
	unsigned int num_vertices = 0;
	unsigned int num_indices = 0;
    
	bool ret = false;
	std::string err;
	const std::string input_filename(file);
	std::string ext = GetFilePathExtension(input_filename);
	
	if (ext.compare("glb") == 0) {
		// assume binary glTF.
		ret = loader.LoadBinaryFromFile(&model, &err, &err, input_filename);
	} else {
		// assume ascii glTF.
		ret = loader.LoadASCIIFromFile(&model, &err, &err, input_filename);
	}
	if (!err.empty()) {
		printf("ERR: %s\n", err.c_str());
		return ret_modelpack;
	}
	if (!ret) {
		std::cout << "Failed to load .glTF : " << file << '\n';
		return ret_modelpack;
	}
	
	num_nodes = model.nodes.size();
	num_meshes = model.meshes.size();
    
    
	// just use seomthing for now
	// TODO: do the real thing after testing
	num_vertices = 1024 << 8;
	num_indices = 1024 << 8;
	int num_skins = 1024 << 8;
	int num_vals = 1024 << 8;
    
	GenMeshStack ms = make_genmeshstack(num_meshes, num_vertices, num_vertices, num_indices, num_indices);
	NodeStack ns = make_nodestack(num_nodes);
	SkinStack ss = make_skinstack(num_skins,num_vals);
    
    int rename_data[model.nodes.size()];
	struct IntArray node_rename_array;
	node_rename_array.data = rename_data;
    node_rename_array.next_int = 0;
	
	
	// HACK TODO All of this shit we are doing are just dumb hacks
	// make it more sensible 
	// allthough it doesn't matter much, because who cares about this shitty converter honestly
	GenModel * ret_genmodel = gltf_to_generalmodel(model, &ms, &ns, &ss, &node_rename_array);
	if ( !ret ) {
		fprintf( stderr,"failed to load the model in gltf_to_generalmodel(model, &ms, &ns, &ss)" );
		return ret_modelpack;
	}
	ret_modelpack.genmodelpack.gm = *ret_genmodel;
	ret_modelpack.genmodelpack.gms =  ms;
	ret_modelpack.genmodelpack.ns = ns;
	ret_modelpack.genmodelpack.ss = ss;
    
	
	
	
	// ----  animation stuff
	
	
	AnimStack as = make_animstack(1024 << 8, 1024 << 8, 1024 << 8, 1024 << 8, 1024 << 8, 1024 << 8);
	
	ret_modelpack.animreturn.descr = load_animation(model, &as, &node_rename_array );
	
	ret_modelpack.animreturn.as = as;
	
	
	return ret_modelpack;
};


void write_model_ressource_file(
GenModelPack * gmr,
const char * filename)
{
	
	FILE * mfile = fopen( filename, "wb" );
	
	int number_of_meshes = gmr->gms.next_m;
	int number_of_vertices_static = gmr->gms.next_v;
	int number_of_indices_static = gmr->gms.next_i;
	int number_of_vertices_skinned = gmr->gms.next_sv;
	int number_of_indices_skinned = gmr->gms.next_si;
	
	int number_of_nodes = gmr->ns.next_n;
	
	int number_of_skins = gmr->ss.next_skin;
	int number_of_skin_values = gmr->ss.next_val;
	
	fwrite(&number_of_meshes, sizeof(int), 1, mfile);
	fwrite(&number_of_vertices_static, sizeof(int), 1, mfile);
	fwrite(&number_of_indices_static, sizeof(int), 1, mfile);
	fwrite(&number_of_vertices_skinned, sizeof(int), 1, mfile);
	fwrite(&number_of_indices_skinned, sizeof(int), 1, mfile);
	
	fwrite(&number_of_nodes, sizeof(int), 1, mfile);
	
	fwrite(&number_of_skins, sizeof(int), 1, mfile);
	fwrite(&number_of_skin_values, sizeof(int), 1, mfile);
	
    fwrite(gmr->gms.names, sizeof(Resource_Name), number_of_meshes, mfile);
    fwrite(gmr->gms.meshes, sizeof(GenMesh), number_of_meshes, mfile);
    
	fwrite(gmr->gms.vertices, sizeof(Vertex), number_of_vertices_static, mfile);
	fwrite(gmr->gms.indices, sizeof(uint), number_of_indices_static, mfile);
	fwrite(gmr->gms.s_vertices, sizeof(S_Vertex), number_of_vertices_skinned, mfile);
	fwrite(gmr->gms.s_indices, sizeof(uint), number_of_indices_skinned, mfile);
	
    fwrite(gmr->ns.names, sizeof(Resource_Name), number_of_nodes, mfile);
    fwrite(gmr->ns.name_indices, sizeof(int), number_of_nodes, mfile);
	fwrite(gmr->ns.parent, sizeof(int), number_of_nodes, mfile);
	fwrite(gmr->ns.nodes, sizeof(Node), number_of_nodes, mfile);
	
	fwrite(gmr->ss.skins, sizeof(Skin), number_of_skins, mfile);
	fwrite(gmr->ss.skin_vals, sizeof(SkinVal), number_of_skin_values, mfile);
	
	fclose( mfile );
	printf("saving of model successful\n");
}

void write_animations_file(
AnimReturn ar,
const char * filename)
{
	FILE * afile = fopen( filename, "wb" );
	
	int number_of_animations = ar.as.next_anim;
	
	int number_of_translation_channels = ar.as.next_tranch;
	int number_of_rotation_channels = ar.as.next_rotch;
	
	int number_of_times = ar.as.next_time;
	int number_of_translations = ar.as.next_trans;
	int number_of_rotations = ar.as.next_rot;
	
	fwrite(&number_of_animations, sizeof(int), 1, afile);
	
	fwrite(&number_of_translation_channels, sizeof(int), 1, afile);
	fwrite(&number_of_rotation_channels, sizeof(int), 1, afile);
	
	fwrite(&number_of_times, sizeof(int), 1, afile);
	fwrite(&number_of_translations, sizeof(int), 1, afile);
	fwrite(&number_of_rotations, sizeof(int), 1, afile);
	
	fwrite(ar.as.anims, sizeof(Anim), number_of_animations, afile );
	
	fwrite(ar.as.tranchs, sizeof(TranChannel), number_of_translation_channels, afile );
	fwrite(ar.as.rotchs, sizeof(RotChannel), number_of_rotation_channels, afile );
	
	fwrite(ar.as.times, sizeof(float), number_of_times, afile );
	fwrite(ar.as.trans, sizeof(hmm_vec3), number_of_translations, afile );
	fwrite(ar.as.rots, sizeof(hmm_quaternion), number_of_rotations, afile );
	
	fclose( afile );
	printf("saving of animation successful\n");
}


#endif /* CONVERTER_IMPLEMENTATION */