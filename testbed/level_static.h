
#define MAX_TRIS 1000000

typedef struct PhysicsMesh {
	uint tri_index;
	uint tri_num;
} PhysicsMesh;

typedef struct PhysicsMeshStack {
	baka_triangle *triangles;
	uint max_triangles;
	uint num_triangles;
	
	PhysicsMesh *meshes;
	uint max_meshes;
	uint num_meshes;
	
	Map mesh_map;
} PhysicsMeshStack;


typedef struct EntityPhysics {
	uint object_index;
	uint num_objects;
	
	baka_aabb_binary_tree tree;
} EntityPhysics;


typedef struct StaticEntityPrototype {
	char name[512]; // NOTE: the name is for levelbuilding purposes and could be removed from the actual game
	GenModel model;
	
	bool scale_single;
	
	EntityPhysics entity_phys;
} StaticEntityPrototype;

typedef struct PrototypeStack {
	StaticEntityPrototype *static_prototypes;
	uint next_prototype;
	uint max_prototype;
	Map prototype_map;
	
	// store the nodestack here for convenience sake 
	NodeStack *nodestack;
} PrototypeStack;


typedef struct GameAssets {
	PrototypeStack prototypes;
	baka_StaticObjects prototype_static_objects;
	ModelMemory model_memory;
	Texture_Asset_Manager tex_ass_man;
	//NodeStack ns_instances;
} GameAssets;




typedef struct StaticLevelEntity {
	GenModel model;
	uint prototype_index;
	Transform transform;
	v3 scale;
	int aabb_node_index;
	
	// can't scale spheres currently, so objects that contain them are restricted to be scaled in one dimension
	// NOTE: I wonder if, in the final game, we even need scaling in multiple dimensions, but for now with simple boxes, its ok
	bool scale_single;
	
} StaticLevelEntity;

typedef struct LevelStatic {
	StaticLevelEntity *level_entities;
	uint max_level_entity;
	uint next_level_entity;
	
	GenModel *genmodels;
	
	uint *prototype_index;
	
	Transform *transforms;
	v3 *scales;
	
	baka_aabb_binary_tree entity_tree;
	
	baka_triangle * model_tris;
	uint num_tris;
	baka_aabb_binary_tree tri_tree;
} LevelStatic;


StaticEntityPrototype parse_static_entity_prototype(ModelMemory *model_memory, Texture_Asset_Manager *tex_ass_man,
													PhysicsMeshStack * physics_meshstack,baka_StaticObjects * static_objects, NodeStack *prototype_ns, const char* filename, const char* entity_name) {
	StaticEntityPrototype prototype;
	
	prototype.scale_single = 0;
	
	prototype.entity_phys.object_index = static_objects->num_objects;
	prototype.entity_phys.num_objects = 0;
	
	strncpy(prototype.name, entity_name, 512);
	
	FILE * entity_file = fopen( filename, "rb" );
	if (entity_file == NULL) {
		printf("error opening entity file %s\n", filename);
		return prototype;
	}
	char line_buffer[1024];
	
	int linecount = -1;
	
	while(1) {
		linecount++;
		if (fgets(line_buffer, 1024, entity_file) == NULL) {
			goto END;
		}
		
		if (!strncmp(line_buffer, "modl", 4)) {
			linecount++;
			if (fgets(line_buffer, 1024, entity_file) == NULL) {
				goto END;
			}
			
			if(line_buffer[0] != '\t') {
				printf("no model name at line %i\nline: %s\n", linecount, line_buffer);
				goto END;
			}
			
			char * model_file_name = &line_buffer[1]; 
			clean_word(model_file_name);
			printf("\tloading this modelfile: %s\n", model_file_name);
			
			bool ok;
			GenModel modl = get_model_from_crc32(model_memory, &ok, crc32_cstring(model_file_name));
			
			if (!ok) {
				printf("couldn't load model\n");
				continue;
			}
			ok = instantiate_genmodel(
				model_memory->node_stack,
				modl,
				prototype_ns,
				&prototype.model);
			
			if (!ok) {
				printf("couldn't instantiate model\n");
				continue;
			}
			
			continue;
		}
		
		if (!strncmp(line_buffer, "text", 4)) {
			linecount++;
			if (fgets(line_buffer, 1024, entity_file) == NULL) {
				goto END;
			}
			
			if(line_buffer[0] != '\t') {
				printf("no texture pair at line %i\nline: %s\n", linecount, line_buffer);
				goto END;
			}
			
			char *node_identifier = NULL;
			char *texture_identifier = NULL;
			
			char * words[2];
			
			int num_words = split_line_into_words(line_buffer, words, 2);
			
			node_identifier = words[0];
			texture_identifier = words[1];
			clean_word(node_identifier);
			clean_word(texture_identifier);
			printf("\tsetting the texture of node %s, to %s\n", node_identifier, texture_identifier);
			
			bool ok;
			unsigned int texture_ident;
			texture_ident = map_lookup_key(&tex_ass_man->map, &ok, crc32_cstring(texture_identifier));
			if (!ok) {
				printf("couldn't look up texture asset\n");
			}
			struct texture_asset ta = to_texture_asset(texture_ident);
			printf("\ttexture asset %u %u %u\n", texture_ident, ta.type, ta.nr);
			
			set_node_texture_asset_by_name(prototype_ns, &prototype.model, node_identifier, ta);
			
			continue;
		}
		
		
		if (!strncmp(line_buffer, "phys", 4)) {
			
			
			while(1) {
				linecount++;
				
				if (fgets(line_buffer, 1024, entity_file) == NULL) {
					goto END;
				}
				
				if(line_buffer[0] != '\t') {
					printf("no physical object at line %i\nline: %s\n", linecount, line_buffer);
					goto END;
				}
				
				
				if(!strncmp(&line_buffer[1], "obb", 3)) {
					
					if (fgets(line_buffer, 1024, entity_file) == NULL) {
						goto END;
					}
					v3 obb_mid = parse_vec3(line_buffer);
					
					if (fgets(line_buffer, 1024, entity_file) == NULL) {
						goto END;
					}
					v3 obb_ex = parse_vec3(line_buffer);
					
					if (fgets(line_buffer, 1024, entity_file) == NULL) {
						goto END;
					}
					Quat obb_rot = parse_quaternion(line_buffer);
					
					printf("\tadding obb with\n\tmid %f %f %f\n\tex %f %f %f\n\trot %f %f %f %f\n", obb_mid.X,obb_mid.Y,obb_mid.Z,obb_ex.X,obb_ex.Y,obb_ex.Z, obb_rot.X,obb_rot.Y,obb_rot.Z,obb_rot.W);
					
					baka_OBB obb = baka_make_OBB(obb_mid, obb_ex, obb_rot);
					
					baka_static_object new_object;
					new_object.type_id = PHYS_OBB;
					new_object.obb = obb;
					baka_push_static_object(static_objects, new_object);
					prototype.entity_phys.num_objects++;
				}
				
				if(!strncmp(&line_buffer[1], "sphere", 6)) {
					
					if (fgets(line_buffer, 1024, entity_file) == NULL) {
						goto END;
					}
					v3 sphere_mid = parse_vec3(line_buffer);
					
					if (fgets(line_buffer, 1024, entity_file) == NULL) {
						goto END;
					}
					float sphere_ex = parse_float(line_buffer);
					
					printf("\tadding sphere with\n\tmid %f %f %f\n\tex %f\n", sphere_mid.X,sphere_mid.Y,sphere_mid.Z,sphere_ex);
					
					baka_sphere sphere = baka_make_sphere(sphere_mid, sphere_ex);
					
					baka_static_object new_object;
					new_object.type_id = PHYS_SPHERE;
					new_object.sphere = sphere;
					baka_push_static_object(static_objects, new_object);
					prototype.entity_phys.num_objects++;
					
					prototype.scale_single = true;
				}
				
				
				if(!strncmp(&line_buffer[1], "mesh", 4)) {
					
					clean_word(&line_buffer[6]);
					
					char * mesh_name = &line_buffer[6];
					
					bool ok;
					
					int mesh_ident = map_lookup_key(&physics_meshstack->mesh_map, &ok, crc32_cstring(mesh_name));
					
					PhysicsMesh pmesh = physics_meshstack->meshes[mesh_ident];
					
					if (!ok) {
						printf("couldn't look up physical mesh %s %i\n", mesh_name, crc32_cstring(mesh_name));
					}
					
					if (fgets(line_buffer, 1024, entity_file) == NULL) {
						goto END;
					}
					v3 mesh_mid = parse_vec3(line_buffer);
					
					if (fgets(line_buffer, 1024, entity_file) == NULL) {
						goto END;
					}
					v3 mesh_ex = parse_vec3(line_buffer);
					
					if (fgets(line_buffer, 1024, entity_file) == NULL) {
						goto END;
					}
					Quat mesh_rot = parse_quaternion(line_buffer);
					
					printf("\tadding mesh with\n\tmid %f %f %f\n\tex %f %f %f\n\trot %f %f %f %f\n\tnum_tris: %i", mesh_mid.X,mesh_mid.Y,mesh_mid.Z,mesh_ex.X,mesh_ex.Y,mesh_ex.Z, mesh_rot.X,mesh_rot.Y,mesh_rot.Z,mesh_rot.W, pmesh.tri_num);
					
					for (int i = 0; i < pmesh.tri_num; i++) {
						int tri_index = pmesh.tri_index + i;
						
						baka_triangle next_tri = physics_meshstack->triangles[tri_index];
						
						baka_static_object new_object;
						new_object.type_id = PHYS_TRIANGLE;
						new_object.tri = next_tri;
						baka_push_static_object(static_objects, new_object);
						prototype.entity_phys.num_objects++;
					}
				}
				
				
			}
			continue;
		}
	}
	END:
	
	fclose(entity_file);
	return prototype;
}


PrototypeStack gather_static_entity_prototypes(ModelMemory *model_memory,
											   PhysicsMeshStack * physics_meshstack,
											   baka_StaticObjects * static_objects,Texture_Asset_Manager *tex_ass_man,NodeStack *prototype_ns) {
	PrototypeStack ps;
	ps.next_prototype = 0;
	
	printf("\ngathering static prototype entities\n");
	
	StaticEntityPrototype *static_prototypes = malloc(sizeof(StaticEntityPrototype) * 1024);
	uint next_prototype = 0;
	uint max_prototype = 1024;
	Map prototype_map = init_map();
	
	DIR *entity_directory;
	struct dirent *directory_entry;
	entity_directory = opendir("../entities");
	
	
	while(NULL != (directory_entry = readdir(entity_directory))) {
		if (directory_entry->d_type != DT_REG) {
			continue;
		}
		
		if (!compare_tail(directory_entry->d_name, ".entity")) {
			continue;
		}
		
		char dirnamebuffer[strlen(directory_entry->d_name) + sizeof("../entities/")];
		sprintf(dirnamebuffer, "../entities/%s", directory_entry->d_name);
		printf("\t%s\n", dirnamebuffer);
		
		StaticEntityPrototype prot = parse_static_entity_prototype(model_memory, tex_ass_man, physics_meshstack, static_objects,prototype_ns, dirnamebuffer, directory_entry->d_name);
		// TODO: build in a failure-case
		
		printf("\tscale single %i\n", prot.scale_single);
		
		if (next_prototype >= max_prototype) {
			printf("error when gathering static prototypes: next_prototype >= max_prototype\n");
			return ps;
		}
		static_prototypes[next_prototype] = prot;
		
		unsigned int value = next_prototype;
		unsigned int key = crc32_cstring(directory_entry->d_name);
		next_prototype++;
		
		printf("\tcrc32 %s %u %u\n", directory_entry->d_name, key, value);
		
		unsigned int ok = map_insert_kv(&prototype_map, key, value);
		printf("\tinsert ok %u\n\n", ok);
	}
	ps.static_prototypes = static_prototypes;
	ps.next_prototype = next_prototype;
	ps.max_prototype = max_prototype;
	ps.prototype_map = prototype_map;
	ps.nodestack = prototype_ns;
	
	return ps;
}

GenModel instantiate_static_entity_prototype_by_name(NodeStack *instances_ns, PrototypeStack *protstack, char * prot_name, bool *ok) {
	GenModel instanced_entity;
	instanced_entity.num_meshes = 0;
	
	uint prot_index = map_lookup_key(&protstack->prototype_map, ok, crc32_cstring(prot_name));
	
	StaticEntityPrototype prot = protstack->static_prototypes[prot_index];
	
	if (!*ok) {
		printf("couldn't find the entity %s crc32 %u\n", prot_name, crc32_cstring(prot_name));
		return instanced_entity;
	}
	
	*ok = instantiate_genmodel(
		*(protstack->nodestack),
		prot.model,
		instances_ns,
		&instanced_entity);
	
	if (!*ok) {
		printf("couldn't instantiate the entity %s\n", prot_name);
		return instanced_entity;
	}
	
	return instanced_entity;
}


GenModel instantiate_static_entity_prototype(NodeStack *instances_ns, PrototypeStack *protstack, uint prot_index, bool *ok) {
	GenModel instanced_entity;
	instanced_entity.num_meshes = 0;
	
	StaticEntityPrototype prot = protstack->static_prototypes[prot_index];
	
	char * prot_name = prot.name;
	
	*ok = instantiate_genmodel(
		*(protstack->nodestack),
		prot.model,
		instances_ns,
		&instanced_entity);
	
	if (!*ok) {
		printf("couldn't instantiate the entity %s\n", prot_name);
		return instanced_entity;
	}
	
	return instanced_entity;
}


void parse_physics_obj(PhysicsMeshStack *pms, char *filename, char *physics_name) {
	
	PhysicsMesh pmesh;
	
	pmesh.tri_index = pms->num_triangles;
	pmesh.tri_num = 0;
	
	int vertex_num = 0;
	
	FILE * file = fopen( filename, "rb" );
	if (file == NULL) {
		printf("error opening physics_mesh file %s\n", filename);
		return;
	}
	
	char line_buffer[1024];
	
	int linecount = -1;
	
	while(1) {
		linecount++;
		if (fgets(line_buffer, 1024, file) == NULL) {
			break;
		}
		
		if (line_buffer[0] == 'v') {
			vertex_num++;
		}
	}
	
	printf("\tphysics_obj %s has %i vertices\n", physics_name, vertex_num);
	
	rewind(file);
	
	v3 vertices[vertex_num];
	int next_vertex = 0;
	
	linecount = -1;
	
	while(1) {
		linecount++;
		if (fgets(line_buffer, 1024, file) == NULL) {
			goto END;
		}
		
		if (line_buffer[0] == 'v') {
			vertices[next_vertex] = parse_vec3(&line_buffer[2]);
			next_vertex++;
			continue;
		}
		
		if (line_buffer[0] == 'f') {
			char * words[3];
			
			int num_words = split_line_into_words(&line_buffer[2], words, 3);
			if (num_words != 3) {
				printf("number of words isn't 3 for f  %s\n", line_buffer);
			}
			clean_word(words[2]);
			
			baka_triangle new_tri;
			
			int i1 = atoi(words[0])-1;
			int i2 = atoi(words[1])-1;
			int i3 = atoi(words[2])-1;
			
			new_tri.a = vertices[i1];
			new_tri.b = vertices[i2];
			new_tri.c = vertices[i3];
			
			int next_tri_index = pmesh.tri_index + pmesh.tri_num;
			pms->triangles[next_tri_index] = new_tri;
			pmesh.tri_num++;
			continue;
		}
		
	}
	
	END:
	
	printf("\tparsed pmesh, tri_num: %i\n", pmesh.tri_num);
	
	pms->meshes[pms->num_meshes] = pmesh;
	
	unsigned int value = pms->num_meshes;
	unsigned int key = crc32_cstring(physics_name);
	
	pms->num_meshes++;
	
	printf("\tcrc32 %s %u %u\n", physics_name, key, value);
	
	unsigned int ok = map_insert_kv(&pms->mesh_map, key, value);
	printf("\tinsert ok %u\n\n", ok);
	
	fclose(file);
	return;
}

PhysicsMeshStack gather_physics_meshes() {
	
	printf("\ngathering physics meshes\n");
	
	PhysicsMeshStack pms;
	
	// TODO: better select numbers
	pms.triangles = malloc(sizeof(baka_triangle) * 5124);
	pms.num_triangles = 0;
	pms.max_triangles = 5124;
	pms.meshes = malloc(sizeof(PhysicsMesh) * 1024);
	pms.num_meshes = 0;
	pms.max_meshes = 1024;
	pms.mesh_map = init_map();
	
	DIR *phys_directory;
	struct dirent *directory_entry;
	phys_directory = opendir("../physic_meshes");
	
	
	while(NULL != (directory_entry = readdir(phys_directory))) {
		if (directory_entry->d_type != DT_REG) {
			continue;
		}
		
		if (!compare_tail(directory_entry->d_name, ".obj")) {
			continue;
		}
		
		char dirnamebuffer[strlen(directory_entry->d_name) + sizeof("../physic_meshes/")];
		sprintf(dirnamebuffer, "../physic_meshes/%s", directory_entry->d_name);
		printf("\t%s\n", dirnamebuffer);
		
		parse_physics_obj(&pms, dirnamebuffer, directory_entry->d_name);
		
	}
	return pms;
}


void debug_draw_static_level_entity(mush_draw_list *dbg_list, StaticLevelEntity level_entity, PrototypeStack *prototypes,baka_StaticObjects *prototype_static_objects) {
	
	EntityPhysics entity_phys = prototypes->static_prototypes[level_entity.prototype_index].entity_phys;
	
	v3 mid = level_entity.transform.translation;
	Quat rot = level_entity.transform.rotation;
	v3 scale = level_entity.scale;
	
	for (int j = 0; j < entity_phys.num_objects; j++) {
		baka_static_object draw_object = prototype_static_objects->objects[entity_phys.object_index + j];
		
		if(draw_object.type_id == PHYS_OBB) {
			baka_OBB draw_obb = baka_make_OBB(add_v3(rotate_vec3_quat(mul_v3(draw_object.obb.mid, scale), rot), mid), mul_v3(draw_object.obb.ex, scale), mul_quat(draw_object.obb.rot, rot));
			
			mush_draw_cube(dbg_list, draw_obb.mid, draw_obb.ex, draw_obb.rot, vec4(0.0, 1.0, 1.0, 1.0));
		}
		
		if(draw_object.type_id == PHYS_SPHERE) {
			baka_sphere draw_sphere = baka_make_sphere(add_v3(rotate_vec3_quat( mul_v3f(draw_object.sphere.center, scale.X), rot), mid), draw_object.sphere.radius * scale.X);
			
			mush_draw_sphere(dbg_list, draw_sphere.center, draw_sphere.radius, vec4(0.0, 0.5, 0.5, 0.1));
		}
		
		if(draw_object.type_id == PHYS_TRIANGLE) {
			baka_triangle draw_tri;
			draw_tri.a = add_v3(rotate_vec3_quat( mul_v3(draw_object.tri.a, scale), rot), mid);
			draw_tri.b = add_v3(rotate_vec3_quat( mul_v3(draw_object.tri.b, scale), rot), mid);
			draw_tri.c = add_v3(rotate_vec3_quat( mul_v3(draw_object.tri.c, scale), rot), mid);
			
			mush_draw_triangle(dbg_list, draw_tri.a, draw_tri.b, draw_tri.c, vec4(0.0, 0.7, 0.9, 1.0));
		}
	}
}



void rebuild_level_static_tree(LevelStatic *level_static, PrototypeStack *prototypes, baka_StaticObjects *prototype_static_objects) {
	level_static->entity_tree.root_node_index = -1;
	level_static->entity_tree.next_object = 0;
	
	for (int i = 0; i < level_static->next_level_entity; i++) {
		StaticLevelEntity *to_insert = &level_static->level_entities[i];
		EntityPhysics entity_phys = prototypes->static_prototypes[to_insert->prototype_index].entity_phys;
		
		v3 mid = to_insert->transform.translation;
		Quat rot = to_insert->transform.rotation;
		v3 scale = to_insert->scale;
		
		baka_AABB merged_aabb = baka_make_AABB(mid, vec3(0.0f, 0.0f, 0.0f));
		
		for (int j = 0; j < entity_phys.num_objects; j++) {
			baka_static_object next_object = prototype_static_objects->objects[entity_phys.object_index + j];
			
			if(next_object.type_id == PHYS_OBB) {
				baka_OBB new_obb = baka_make_OBB(add_v3(rotate_vec3_quat(mul_v3(next_object.obb.mid, scale), rot), mid), mul_v3(next_object.obb.ex, scale), mul_quat(next_object.obb.rot, rot));
				merged_aabb = merge_aabb(aabb_from_obb(new_obb), merged_aabb);
			}
			
			if(next_object.type_id == PHYS_SPHERE) {
				baka_sphere new_sphere = baka_make_sphere(add_v3(rotate_vec3_quat( mul_v3f(next_object.sphere.center, scale.X), rot), mid), next_object.sphere.radius * scale.X);
				merged_aabb = merge_aabb(aabb_from_sphere(new_sphere), merged_aabb);
			}
			
			if(next_object.type_id == PHYS_TRIANGLE) {
				baka_triangle new_tri;
				new_tri.a = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.a, scale), rot), mid);
				new_tri.b = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.b, scale), rot), mid);
				new_tri.c = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.c, scale), rot), mid);
				merged_aabb = merge_aabb(aabb_from_triangle(new_tri), merged_aabb);
			}
		}
		to_insert->aabb_node_index = tree_insert_aabb(&level_static->entity_tree, merged_aabb, i);
	}
}


void add_static_level_entity_to_level(LevelStatic *level_static, NodeStack *instances_ns, StaticLevelEntity sel) {
	if (level_static->next_level_entity >= level_static->max_level_entity) {
		printf("can't add anymore static level entities\n");
		return;
	}
	
	// TODO: maybe all of those SOA-style arrays can be removed, since all the information is already in the StaticLevelEntity
	// performance could suffer a little though, no idea how much it would
	// also all the apis that use them would have to be changed in ways that aren't obvious to me how to solve without tight coupling of the codebases 
	level_static->transforms[level_static->next_level_entity] = sel.transform;
	level_static->scales[level_static->next_level_entity] = sel.scale;
	level_static->genmodels[level_static->next_level_entity] = sel.model;
	level_static->prototype_index[level_static->next_level_entity] = sel.prototype_index;
	
	level_static->level_entities[level_static->next_level_entity] = sel;
	
	// TODO: recalculating every single one of the object transforms isn't really necessary...
	calculate_object_transforms(
		*instances_ns,
		&sel.model,
		&level_static->transforms[level_static->next_level_entity],
		1);
	
	level_static->next_level_entity++;
}


void construct_all_static_level_triangles(LevelStatic *level_static, GameAssets *ga, NodeStack *ns_instances) {
	
	GenModel *gma = level_static->genmodels;
	
	for (int i = 0; i < level_static->next_level_entity; i++) {
		StaticLevelEntity lvl_ent = level_static->level_entities[i];
		for (int j = 0; j < lvl_ent.model.num_nodes; j++) {
			
			Node n = ns_instances->nodes[lvl_ent.model.node_idx];
			int relative_mesh_id = n.mesh;
			int mesh_id = relative_mesh_id + lvl_ent.model.mesh_idx;
			
			if ( relative_mesh_id == -1 ) {
				continue;
			}
			GenMeshStack gms = ga->model_memory.mesh_stack;
			GenMesh mesh = gms.meshes[mesh_id];
			for (int k = 0; k < mesh.num_i; ) {
				int tri_index1 = mesh.ndx_v + gms.indices[k + mesh.ndx_i];
				k++;
				int tri_index2 = mesh.ndx_v + gms.indices[k + mesh.ndx_i];
				k++;
				int tri_index3 = mesh.ndx_v + gms.indices[k + mesh.ndx_i];
				k++;
				
				if (mesh.skinned) {
					// TODO: decide if skinned meshes get in here too
				}
				else {
					Vertex tri_v1 = gms.vertices[tri_index1];
					Vertex tri_v2 = gms.vertices[tri_index2];
					Vertex tri_v3 = gms.vertices[tri_index3];
					
					baka_triangle tri;
					tri.a = vec3(tri_v1.x, tri_v1.y, tri_v1.z);
					tri.b = vec3(tri_v2.x, tri_v2.y, tri_v2.z);
					tri.c = vec3(tri_v3.x, tri_v3.y, tri_v3.z);
					
					baka_triangle new_tri;
					new_tri.a = add_v3(rotate_vec3_quat( mul_v3(tri.a, lvl_ent.scale), lvl_ent.transform.rotation), lvl_ent.transform.translation);
					new_tri.b = add_v3(rotate_vec3_quat( mul_v3(tri.b, lvl_ent.scale), lvl_ent.transform.rotation), lvl_ent.transform.translation);
					new_tri.c = add_v3(rotate_vec3_quat( mul_v3(tri.c, lvl_ent.scale), lvl_ent.transform.rotation), lvl_ent.transform.translation);
					
					
					if (level_static->num_tris >= MAX_TRIS) {
						printf("construct_all_static_level_triangles: we have reached the maximum umber of tris\n");
						return;
					}
					
					level_static->model_tris[level_static->num_tris] = new_tri;
					level_static->num_tris++;
				}
			}
		}
	}
	
	for (int i = 0; i < level_static->num_tris; i++) {
		baka_triangle tri = level_static->model_tris[i];
		
		tree_insert_triangle(&level_static->tri_tree, tri, i);
	}
}

LevelStatic init_level_static() {
	LevelStatic level_static;
	level_static.max_level_entity = 1024;
	level_static.next_level_entity = 0;
	
	level_static.genmodels = malloc(sizeof(GenModel)*level_static.max_level_entity);
	
	level_static.prototype_index = malloc(sizeof(uint)*level_static.max_level_entity); 
	level_static.transforms = malloc(sizeof(Transform)*level_static.max_level_entity);
	level_static.scales = malloc(sizeof(v3)*level_static.max_level_entity);
	
	// TODO: some magic number for now, change it later
	level_static.level_entities = malloc(sizeof(StaticLevelEntity)*level_static.max_level_entity);
	
	level_static.entity_tree.nodes = malloc(sizeof(aabb_tree_node)*10000);
	level_static.entity_tree.root_node_index = -1;
	level_static.entity_tree.max_object = 10000;
	level_static.entity_tree.next_object = 0;
	level_static.entity_tree.free_indices = malloc(sizeof(int)*10000);
	level_static.entity_tree.next_free_index = 0;
	
	level_static.model_tris = malloc(sizeof(baka_triangle)*MAX_TRIS);
	level_static.num_tris = 0;
	
	level_static.tri_tree.nodes = malloc(sizeof(aabb_tree_node)*MAX_TRIS);
	level_static.tri_tree.root_node_index = -1;
	level_static.tri_tree.max_object = MAX_TRIS;
	level_static.tri_tree.next_object = 0;
	level_static.tri_tree.free_indices = malloc(sizeof(int)*MAX_TRIS);
	level_static.tri_tree.next_free_index = 0;
	
	return level_static;
}

void reset_level_static(LevelStatic *level_static) {
	level_static->next_level_entity = 0;
	
	level_static->entity_tree.root_node_index = -1;
	level_static->entity_tree.next_object = 0;
	level_static->entity_tree.next_free_index = 0;
	
	level_static->num_tris = 0;
	
	level_static->tri_tree.root_node_index = -1;
	level_static->tri_tree.next_object = 0;
	level_static->tri_tree.next_free_index = 0;
}

void load_level_static(LevelStatic * level_static, const char* filename, GameAssets *ga, NodeStack *ns_instances) {
	
	FILE * file = fopen( filename, "rb" );
	if (file == NULL) {
		printf("error opening level file %s\n", filename);
		return;
	}
	char line_buffer[1024];
	
	int linecount = -1;
	
	while(1) {
		linecount++;
		if (fgets(line_buffer, 1024, file) == NULL) {
			goto END;
		}
		
		START:
		
		if (!strncmp(line_buffer, "static", 6)) {
			// instantiate the list of static objects
			while(1) {
				linecount++;
				if (fgets(line_buffer, 1024, file) == NULL) {
					goto END;
				}
				
				NEXT_ENTITY_TYPE:
				
				if (line_buffer[0] != '\t') {
					goto START;
				}
				
				clean_word(&line_buffer[1]);
				char entity_type[1024];
				strcpy(entity_type, &line_buffer[1]);
				
				// instantiate the entities of this particular type
				while(1) {
					linecount++;
					if (fgets(line_buffer, 1024, file) == NULL) {
						goto END;
					}
					
					// if it isn't a next entity of the same type, then parse a new entity
					if (strncmp(line_buffer, "\t\t-", 3)) {
						goto NEXT_ENTITY_TYPE;
					}
					// otherwise just get our transform
					v3 entity_mid;
					v3 entity_scale;
					Quat entity_rot;
					
					linecount++;
					if (fgets(line_buffer, 1024, file) == NULL) {
						goto END;
					}
					entity_mid = parse_vec3(line_buffer);
					
					linecount++;
					if (fgets(line_buffer, 1024, file) == NULL) {
						goto END;
					}
					entity_scale = parse_vec3(line_buffer);
					linecount++;
					if (fgets(line_buffer, 1024, file) == NULL) {
						goto END;
					}
					entity_rot = parse_quaternion(line_buffer);
					
					bool ok;
					
					uint prot_index = map_lookup_key(&ga->prototypes.prototype_map, &ok, crc32_cstring(entity_type));
					if (!ok) {
						continue;
					}
					
					GenModel instanced_model = instantiate_static_entity_prototype(ns_instances, &ga->prototypes, prot_index, &ok);
					if (!ok) {
						continue;
					}
					
					// prepare the static level entity
					
					StaticLevelEntity sel;
					sel.scale_single = ga->prototypes.static_prototypes[prot_index].scale_single;
					sel.prototype_index = prot_index;
					sel.model = instanced_model;
					sel.transform.translation = entity_mid;
					sel.transform.rotation = entity_rot;
					sel.scale = entity_scale;
					
					add_static_level_entity_to_level(level_static, ns_instances, sel);
				}
			}
		}
	}
	
	END:
	fclose(file);
	
	calculate_object_transforms(
		*ns_instances,
		level_static->genmodels,
		level_static->transforms,
		level_static->next_level_entity);
	
	rebuild_level_static_tree(level_static, &ga->prototypes, &ga->prototype_static_objects);
	construct_all_static_level_triangles(level_static, ga, ns_instances);
	return;
}


void save_level_static(LevelStatic *level_static, const char* levelname, PrototypeStack *prototypes) {
	
	int buffer_len = sizeof(LEVEL_DIR) + MAX_NAME_LENGTH + sizeof(LEVEL_END) + 1;
	
	char level_path[buffer_len];
	
	snprintf(level_path, buffer_len,"%s%s%s", LEVEL_DIR, levelname, LEVEL_END);
	
	FILE * file = fopen( level_path, "wb" );
	if (file == NULL) {
		printf("error opening level file for saving %s\n", level_path);
		return;
	}
	char line_buffer[1024];
	
	fprintf(file, "static\n");
	
	for(int i = 0; i < prototypes->next_prototype; i++) {
		int first_instantiation = 1;
		
		for (int j = 0; j < level_static->next_level_entity; j++) {
			
			if (level_static->prototype_index[j] != i) {
				continue;
			}
			
			if (first_instantiation) {
				fprintf(file, "\t%s\n", prototypes->static_prototypes[i].name);
				first_instantiation = 0;
			}
			fprintf(file, "\t\t-\n");
			
			fprintf(file, "\t\t%f %f %f\n", level_static->transforms[j].translation.X, level_static->transforms[j].translation.Y, level_static->transforms[j].translation.Z);
			
			fprintf(file, "\t\t%f %f %f\n", level_static->scales[j].X, level_static->scales[j].Y, level_static->scales[j].Z);
			
			fprintf(file, "\t\t%f %f %f %f\n", level_static->transforms[j].rotation.X, level_static->transforms[j].rotation.Y, level_static->transforms[j].rotation.Z,level_static->transforms[j].rotation.W);
		}
	}
	
	fclose(file);
}


baka_AABB calculate_static_level_entity_aabb(LevelStatic *level_static, PrototypeStack *prototypes, baka_StaticObjects *prototype_static_objects, int static_level_entity_index) {
	
	StaticLevelEntity *to_insert = &level_static->level_entities[static_level_entity_index];
	EntityPhysics entity_phys = prototypes->static_prototypes[to_insert->prototype_index].entity_phys;
	
	v3 mid = to_insert->transform.translation;
	Quat rot = to_insert->transform.rotation;
	v3 scale = to_insert->scale;
	
	baka_AABB merged_aabb = baka_make_AABB(mid, vec3(0.0f, 0.0f, 0.0f));
	
	for (int j = 0; j < entity_phys.num_objects; j++) {
		baka_static_object next_object = prototype_static_objects->objects[entity_phys.object_index + j];
		
		if(next_object.type_id == PHYS_OBB) {
			baka_OBB new_obb = baka_make_OBB(add_v3(rotate_vec3_quat(mul_v3(next_object.obb.mid, scale), rot), mid), mul_v3(next_object.obb.ex, scale), mul_quat(next_object.obb.rot, rot));
			merged_aabb = merge_aabb(aabb_from_obb(new_obb), merged_aabb);
		}
		
		if(next_object.type_id == PHYS_SPHERE) {
			baka_sphere new_sphere = baka_make_sphere(add_v3(rotate_vec3_quat(mul_v3f(next_object.sphere.center, scale.X), rot), mid), next_object.sphere.radius * scale.X);
			merged_aabb = merge_aabb(aabb_from_sphere(new_sphere), merged_aabb);
		}
		
		if(next_object.type_id == PHYS_TRIANGLE) {
			baka_triangle new_tri;
			new_tri.a = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.a, scale), rot), mid);
			new_tri.b = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.b, scale), rot), mid);
			new_tri.c = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.c, scale), rot), mid);
			merged_aabb = merge_aabb(aabb_from_triangle(new_tri), merged_aabb);
		}
	}
	return merged_aabb;
	
}
