
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

typedef struct Entity_Prototype {
	char name[512]; // TODO: NOTE: the name is for levelbuilding purposes and could be removed from the actual game
	GenModel model;
	baka_Body body;
	bool scale_single;
	
} Entity_Prototype;

typedef struct PrototypeStack {
	Entity_Prototype *prototypes;
	uint next_prototype;
	uint max_prototype;
	Map prototype_map;
	
	// store the nodestack here for convenience sake 
	NodeStack *nodestack;
} PrototypeStack;


typedef struct GameAssets {
	PrototypeStack prototypes;
	baka_Shape_Stack prototype_static_objects;
	ModelMemory model_memory;
	Texture_Asset_Manager tex_ass_man;
	float pad[1];
	//NodeStack ns_instances;
} GameAssets;


typedef struct Base_Entity {
	GenModel model;
	uint prototype_index;
	int aabb_node_index;
	
	Transform transform;
	v3 scale;
	// can't scale spheres currently, so objects that contain them are restricted to be scaled in one dimension
	bool scale_single;
} Base_Entity; 

typedef struct Entity_Collection {
	Base_Entity *els;
	uint max;
	uint num;
	
	baka_aabb_binary_tree tree;
} Entity_Collection;

Entity_Collection init_entity_collection(uint num_els) {
	Entity_Collection col;
	col.max = num_els;
	col.num = 0;
	col.els = malloc(sizeof(Base_Entity)*num_els);
	
	col.tree.nodes = malloc(sizeof(aabb_tree_node)*num_els*2);
	col.tree.root_node_index = -1;
	col.tree.max_object = num_els*2;
	col.tree.next_object = 0;
	col.tree.free_indices = malloc(sizeof(int)*num_els*2);
	col.tree.next_free_index = 0;
	
	return col;
}

void reset_entity_collection(Entity_Collection *col) {
	col->num = 0;
	col->tree.root_node_index = -1;
	col->tree.next_object = 0;
	col->tree.next_free_index = 0;
}

baka_AABB baka_aabb_from_body(GameAssets *assets, baka_Body body, v3 mid, Quat rot, v3 scale) {
	
	baka_AABB merged_aabb = baka_make_AABB(mid, vec3(0.0f, 0.0f, 0.0f));
	
	for (int j = 0; j < body.num_shapes; j++) {
		baka_Shape next_object = assets->prototype_static_objects.els[body.shape_index + j];
		
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
	return merged_aabb;
}

bool push_entity(GameAssets *assets, Entity_Collection *col, Base_Entity b, uint64_t entity_index) {
	if (col->num >= col->max) {
		return 0;
	}
	
	// insert into tree
	baka_Body body = assets->prototypes.prototypes[b.prototype_index].body;
	
	v3 mid = b.transform.translation;
	Quat rot = b.transform.rotation;
	v3 scale = b.scale;
	
	baka_AABB merged_aabb = baka_aabb_from_body(assets, body, mid, rot, scale);
	
	b.aabb_node_index = tree_insert_aabb(&col->tree, merged_aabb, entity_index);
	
	col->els[col->num] = b;
	col->num++;
	return 1;
}

// TODO: shouldn't NodeStack be a pointer for consistencies sake?
void calculate_entity_collection_transforms(NodeStack ns, Entity_Collection *col) {
	START_TIME;
	for (int i = 0; i < col->num; ++i)
	{
		calc_glo(ns, col->els[i].model, col->els[i].transform);
	}
	END_TIME;
}

typedef struct LevelStatic {
	//StaticLevelEntity *level_entities;
	Entity_Collection entities;
	
	baka_triangle * model_tris;
	uint num_tris;
	baka_aabb_binary_tree tri_tree;
} LevelStatic;

typedef struct String_Stack {
	char* chars;
	uint next_string;
	uint max_char;
} 
String_Stack;

typedef struct Game_Point {
	char *name;
	v3 point;
} Game_Point;

typedef struct Game_Points {
	String_Stack point_names;
	uint next_point;
	uint max_point;
	Game_Point *points;
	Map map;
} Game_Points;


char * push_string(String_Stack *ss, const char *cstr) {
	char *ret = &ss->chars[ss->next_string];
	int len = strlen(cstr);
	if ((1 + len + ss->next_string) >= ss->max_char) {
		return NULL;
	}
	strcpy(&ss->chars[ss->next_string], cstr);
	ss->chars[ss->next_string+len] = 0;
	ss->next_string += len+1;
	return ret;
}

void update_game_point_name(Game_Points * game_points, uint idx, const char *cstr) {
	char * name = push_string(&game_points->point_names, cstr);
	assert(name != NULL);
	char * old_name = game_points->points[idx].name;
	map_remove_key(&game_points->map, crc32_cstring(old_name));
	map_insert_kv(&game_points->map, crc32_cstring(name), idx);
	game_points->points[idx].name = name;
	return;
}

int create_base_entity_from_prototype_name(GameAssets *assets, NodeStack *dest, Entity_Collection *col, const char *cstr) {
	Base_Entity b;
	bool ok = false;
	
	PrototypeStack *protstack = &assets->prototypes;
	uint name_prot_index = map_lookup_key(&protstack->prototype_map, &ok, crc32_cstring(cstr));
	
	Entity_Prototype name_prot = protstack->prototypes[name_prot_index];
	
	if (!ok) {
		printf("couldn't find Entity_Prototype %s\n", cstr);
		return -1;
	}
	
	ok = instantiate_genmodel(
		*protstack->nodestack,
		name_prot.model,
		dest,
		&b.model);
	
	if (!ok) {
		printf("couldn't instantiate %s\n", cstr);
		return -1;
	}
	
	b.prototype_index = name_prot_index;
	
	int ret = col->num;
	push_entity(assets, col, b, col->num);
	return ret;
}

void update_base_entity(GameAssets *assets, Entity_Collection *col, uint e_idx) {
	
	Base_Entity *e = &col->els[e_idx];
	baka_Body body = assets->prototypes.prototypes[e->prototype_index].body;
	
	v3 mid = e->transform.translation;
	Quat rot = e->transform.rotation;
	v3 scale = e->scale;
	
	baka_AABB aabb = baka_aabb_from_body(assets, body, mid, rot, scale);
	
	e->aabb_node_index = tree_update_aabb_node(&col->tree, e->aabb_node_index, aabb);
}

Entity_Prototype parse_static_entity_prototype(ModelMemory *model_memory, Texture_Asset_Manager *tex_ass_man,
											   PhysicsMeshStack * physics_meshstack,baka_Shape_Stack * static_objects, NodeStack *prototype_ns, const char* filename, const char* entity_name) {
	Entity_Prototype prototype;
	
	prototype.scale_single = 0;
	
	prototype.body.shape_index = static_objects->num;
	prototype.body.num_shapes = 0;
	
	strncpy(prototype.name, entity_name, 512);
	
	char * file_contents = read_file_into_memory_null_terminate(filename);
	bool parsing = true;
	Tokenizer tknzr;
	tknzr.at = file_contents;
	
	while(parsing) {
		START_PARSE:;
		Token token = get_token(&tknzr);
		
		switch (token.type) {
			case TOKEN_EOF: {
				parsing = false;
			} break;
			
			case TOKEN_IDENTIFIER: {
				
				if (does_identifier_match(token, "modl")) {
					bool ok;
					
					Token modl_name_token = get_token(&tknzr);
					char model_file_name[1024];
					ok = token_to_cstr(model_file_name, 1024, modl_name_token);
					if (!ok) {
						printf("not enough space in modl_file_name_buf for token\n");
						continue;
					}
					printf("\tloading this modelfile: %s\n", model_file_name);
					GenModel modl = get_model_from_crc32(model_memory, &ok, crc32_cstring(model_file_name));
					
					if (!ok) {
						printf("couldn't load model %s\n", model_file_name);
						continue;
					}
					ok = instantiate_genmodel(
						model_memory->node_stack,
						modl,
						prototype_ns,
						&prototype.model);
					
					if (!ok) {
						printf("couldn't instantiate model %s\n", model_file_name);
						continue;
					}
					continue;
				}
				
				if (does_identifier_match(token, "text")) {
					bool ok;
					
					Token node_id_token = get_token(&tknzr);
					Token tex_id_token = get_token(&tknzr);
					
					char node_id_buf[1024];
					char tex_id_buf[1024];
					
					ok = token_to_cstr(node_id_buf, 1024, node_id_token);
					if (!ok) {
						printf("not enough space in node_id_buf for token\n");
						continue;
					}
					ok = token_to_cstr(tex_id_buf, 1024, tex_id_token);
					if (!ok) {
						printf("not enough space in tex_id_buf for token\n");
						continue;
					}
					printf("\tsetting the texture of node %s, to %s\n", node_id_buf, tex_id_buf);
					
					unsigned int texture_ident = map_lookup_key(&tex_ass_man->map, &ok, crc32_cstring(tex_id_buf));
					if (!ok) {
						printf("couldn't look up texture asset\n");
					}
					struct texture_asset ta = to_texture_asset(texture_ident);
					printf("\ttexture asset %u %u %u\n", texture_ident, ta.type, ta.nr);
					
					set_node_texture_asset_by_name(prototype_ns, &prototype.model, node_id_buf, ta);
					continue;
				}
				
				if (does_identifier_match(token, "phys")) {
					while(1) {
						Token phys_token = get_token(&tknzr);
						if (phys_token.type == TOKEN_EOF) {
							parsing = false;
							goto START_PARSE;
						}
						bool ok;
						
						if (does_identifier_match(phys_token, "obb")) {
							v3 mid = parse_vec3_token(&tknzr);
							v3 ex = parse_vec3_token(&tknzr);
							Quat rot = parse_quaternion_token(&tknzr);
							
							printf("\tadding obb with\n\tmid %f %f %f\n\tex %f %f %f\n\trot %f %f %f %f\n", mid.X,mid.Y,mid.Z,ex.X,ex.Y,ex.Z, rot.X,rot.Y,rot.Z,rot.W);
							
							baka_OBB obb = baka_make_OBB(mid, ex, rot);
							
							baka_Shape new_object;
							new_object.type_id = PHYS_OBB;
							new_object.obb = obb;
							baka_push_shape_stack(static_objects, new_object);
							prototype.body.num_shapes++;
						}
						
						if (does_identifier_match(phys_token, "sphere")) {
							v3 mid = parse_vec3_token(&tknzr);
							float ex = parse_float_token(&tknzr);
							
							
							printf("\tadding sphere with\n\tmid %f %f %f\n\tex %f\n", mid.X,mid.Y,mid.Z,ex);
							
							baka_sphere sphere = baka_make_sphere(mid, ex);
							
							baka_Shape new_object;
							new_object.type_id = PHYS_SPHERE;
							new_object.sphere = sphere;
							baka_push_shape_stack(static_objects, new_object);
							prototype.body.num_shapes++;
							
							prototype.scale_single = true;
						}
						if (does_identifier_match(phys_token, "mesh")) {
							
							Token mesh_name_token = get_token(&tknzr);
							char mesh_name[1024];
							ok = token_to_cstr(mesh_name, 1024, mesh_name_token);
							if (!ok) {
								printf("not enough space in mesh_name\n");
							}
							int mesh_ident = map_lookup_key(&physics_meshstack->mesh_map, &ok, crc32_cstring(mesh_name));
							if (!ok) {
								printf("couldn't look up physical mesh %s %i\n", mesh_name, crc32_cstring(mesh_name));
							}
							PhysicsMesh pmesh = physics_meshstack->meshes[mesh_ident];
							
							v3 mid = parse_vec3_token(&tknzr);
							v3 ex = parse_vec3_token(&tknzr);
							Quat rot = parse_quaternion_token(&tknzr);
							
							printf("\tadding mesh %s with\n\tmid %f %f %f\n\tex %f %f %f\n\trot %f %f %f %f\n", mesh_name,  mid.X,mid.Y,mid.Z,ex.X,ex.Y,ex.Z, rot.X,rot.Y,rot.Z,rot.W);
							
							
							for (int i = 0; i < pmesh.tri_num; i++) {
								int tri_index = pmesh.tri_index + i;
								
								baka_triangle next_tri = physics_meshstack->triangles[tri_index];
								
								baka_Shape new_object;
								new_object.type_id = PHYS_TRIANGLE;
								new_object.tri = next_tri;
								baka_push_shape_stack(static_objects, new_object);
								prototype.body.num_shapes++;
							}
						}
					}
				}
				
				
			} break;
			
			case TOKEN_ERROR: {
				//printf("%.*s", token.length, token.text);
			} break;
			
			default: {} break;
		}
	}
	return prototype;
	/*
	
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
  
  baka_Shape new_object;
  new_object.type_id = PHYS_OBB;
  new_object.obb = obb;
  baka_push_shape_stack(static_objects, new_object);
  prototype.body.num_shapes++;
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
  
  baka_Shape new_object;
  new_object.type_id = PHYS_SPHERE;
  new_object.sphere = sphere;
  baka_push_shape_stack(static_objects, new_object);
  prototype.body.num_shapes++;
  
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
   
   baka_Shape new_object;
   new_object.type_id = PHYS_TRIANGLE;
   new_object.tri = next_tri;
   baka_push_shape_stack(static_objects, new_object);
   prototype.body.num_shapes++;
  }
 }
 
 
   }
   continue;
  }
 }
 END:
 
 fclose(entity_file);
 return prototype;*/
}


PrototypeStack gather_static_entity_prototypes(ModelMemory *model_memory,
											   PhysicsMeshStack * physics_meshstack,
											   baka_Shape_Stack * static_objects,Texture_Asset_Manager *tex_ass_man,NodeStack *prototype_ns) {
	PrototypeStack ps;
	ps.next_prototype = 0;
	
	printf("\ngathering static prototype entities\n");
	
	Entity_Prototype *prototypes = malloc(sizeof(Entity_Prototype) * 1024);
	uint next_prototype = 0;
	uint max_prototype = 1024;
	Map prototype_map = init_map();
	
	DIR *entity_directory;
	struct dirent *directory_entry;
	entity_directory = opendir("../entities");
	
	
	while(NULL != (directory_entry = readdir(entity_directory))) {
		printf("d_name: %s\n", directory_entry->d_name);
		
		if (directory_entry->d_type != DT_REG) {
			continue;
		}
		
		if (!compare_tail(directory_entry->d_name, ".entity")) {
			continue;
		}
		
		char dirnamebuffer[strlen(directory_entry->d_name) + sizeof("../entities/")];
		sprintf(dirnamebuffer, "../entities/%s", directory_entry->d_name);
		printf("\t%s\n", dirnamebuffer);
		
		Entity_Prototype prot = parse_static_entity_prototype(model_memory, tex_ass_man, physics_meshstack, static_objects,prototype_ns, dirnamebuffer, directory_entry->d_name);
		// TODO: build in a failure-case
		
		printf("\tscale single %i\n", prot.scale_single);
		
		if (next_prototype >= max_prototype) {
			printf("error when gathering static prototypes: next_prototype >= max_prototype\n");
			return ps;
		}
		prototypes[next_prototype] = prot;
		
		unsigned int value = next_prototype;
		unsigned int key = crc32_cstring(directory_entry->d_name);
		next_prototype++;
		
		printf("\tcrc32 %s %u %u\n", directory_entry->d_name, key, value);
		
		unsigned int ok = map_insert_kv(&prototype_map, key, value);
		printf("\tinsert ok %u\n\n", ok);
	}
	ps.prototypes = prototypes;
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
	
	Entity_Prototype prot = protstack->prototypes[prot_index];
	
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
	
	Entity_Prototype prot = protstack->prototypes[prot_index];
	
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
	
	char * file_contents = read_file_into_memory_null_terminate(filename);
	bool parsing = true;
	Tokenizer tknzr;
	tknzr.at = file_contents;
	//bool ok;
	
	while(parsing) {
		Token token = get_token(&tknzr);
		if (token.type == TOKEN_EOF) {
			parsing = false;
			break;
		}
		
		if (does_identifier_match(token, "v")) {
			vertex_num++;
		}
	}
	
	v3 vertices[vertex_num];
	int next_vertex = 0;
	
	tknzr.at = file_contents;
	parsing = true;
	
	while(parsing) {
		Token token = get_token(&tknzr);
		if (token.type == TOKEN_EOF) {
			parsing = false;
			break;
		}
		
		if (does_identifier_match(token, "v")) {
			vertices[next_vertex] = parse_vec3_token(&tknzr);
			next_vertex++;
			continue;
		}
		
		if (does_identifier_match(token, "f")) {
			
			baka_triangle new_tri;
			
			int i1 = parse_int_token(&tknzr)-1;
			int i2 = parse_int_token(&tknzr)-1;
			int i3 = parse_int_token(&tknzr)-1;
			
			new_tri.a = vertices[i1];
			new_tri.b = vertices[i2];
			new_tri.c = vertices[i3];
			
			int next_tri_index = pmesh.tri_index + pmesh.tri_num;
			pms->triangles[next_tri_index] = new_tri;
			pmesh.tri_num++;
			continue;
		}
		
	}
	
	printf("\tparsed pmesh, tri_num: %i\n", pmesh.tri_num);
	
	pms->meshes[pms->num_meshes] = pmesh;
	
	unsigned int value = pms->num_meshes;
	unsigned int key = crc32_cstring(physics_name);
	
	pms->num_meshes++;
	
	printf("\tcrc32 %s %u %u\n", physics_name, key, value);
	
	unsigned int ok = map_insert_kv(&pms->mesh_map, key, value);
	printf("\tinsert ok %u\n\n", ok);
	return;
	
	/*
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
 */
	
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

void debug_draw_base_entity(mush_draw_list *dbg_list, GameAssets *assets, Base_Entity b, v4 color) {
	
	baka_Body body = assets->prototypes.prototypes[b.prototype_index].body;
	
	v3 mid = b.transform.translation;
	Quat rot = b.transform.rotation;
	v3 scale = b.scale;
	
	for (int j = 0; j < body.num_shapes; j++) {
		baka_Shape draw_object = assets->prototype_static_objects.els[body.shape_index + j];
		
		if(draw_object.type_id == PHYS_OBB) {
			baka_OBB draw_obb = baka_make_OBB(add_v3(rotate_vec3_quat(mul_v3(draw_object.obb.mid, scale), rot), mid), mul_v3(draw_object.obb.ex, scale), mul_quat(draw_object.obb.rot, rot));
			
			mush_draw_cube(dbg_list, draw_obb.mid, draw_obb.ex, draw_obb.rot, color);
		}
		
		if(draw_object.type_id == PHYS_SPHERE) {
			baka_sphere draw_sphere = baka_make_sphere(add_v3(rotate_vec3_quat( mul_v3f(draw_object.sphere.center, scale.X), rot), mid), draw_object.sphere.radius * scale.X);
			
			mush_draw_sphere(dbg_list, draw_sphere.center, draw_sphere.radius, color);
		}
		
		if(draw_object.type_id == PHYS_TRIANGLE) {
			baka_triangle draw_tri;
			draw_tri.a = add_v3(rotate_vec3_quat( mul_v3(draw_object.tri.a, scale), rot), mid);
			draw_tri.b = add_v3(rotate_vec3_quat( mul_v3(draw_object.tri.b, scale), rot), mid);
			draw_tri.c = add_v3(rotate_vec3_quat( mul_v3(draw_object.tri.c, scale), rot), mid);
			
			mush_draw_triangle(dbg_list, draw_tri.a, draw_tri.b, draw_tri.c, color);
		}
	}
}


/*
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
*/

LevelStatic init_level_static() {
	LevelStatic level_static;
	// TODO: think of a good number to pass
	level_static.entities = init_entity_collection(10000);
	
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
	reset_entity_collection(&level_static->entities);
	
	level_static->num_tris = 0;
	
	level_static->tri_tree.root_node_index = -1;
	level_static->tri_tree.next_object = 0;
	level_static->tri_tree.next_free_index = 0;
}


void rebuild_entity_collection_tree(GameAssets *assets, Entity_Collection *col) {
	col->tree.root_node_index = -1;
	col->tree.next_object = 0;
	col->tree.next_free_index = 0;
	
	for (int i = 0; i < col->num; i++) {
		Base_Entity *b = &col->els[i];
		
		// insert into tree
		baka_Body body = assets->prototypes.prototypes[b->prototype_index].body;
		
		v3 mid = b->transform.translation;
		Quat rot = b->transform.rotation;
		v3 scale = b->scale;
		
		baka_AABB merged_aabb = baka_aabb_from_body(assets, body, mid, rot, scale);
		
		b->aabb_node_index = tree_insert_aabb(&col->tree, merged_aabb, i);
	}
}


void load_level_static(LevelStatic *level_static, GameAssets *ga, NodeStack *ns_instances, Game_Points *game_points, const char* filename) {
	
	printf("loading level %s\n", filename);
	
	char * file_contents = read_file_into_memory_null_terminate(filename);
	bool parsing = true;
	Tokenizer tknzr;
	tknzr.at = file_contents;
	bool ok;
	
	while(parsing) {
		//START_PARSE_LEVEL:;
		Token token = get_token(&tknzr);
		
		if (does_identifier_match(token, "points")) {
			PARSE_POINTS:;
			Token point_name_token = get_token(&tknzr);
			if (point_name_token.type == TOKEN_EOF) {
				parsing = false;
				break;
			}
			if (does_identifier_match(point_name_token, "static")) {
				goto PARSE_STATIC;
			}
			
			char point_name[1024];
			ok = token_to_cstr(point_name, 1024, point_name_token);
			if (!ok) {
				printf("not enough space in point_name\n");
			}
			printf("point_name: %s\n", point_name);
			
			Game_Point gp;
			gp.point = parse_vec3_token(&tknzr);
			gp.name = push_string(&game_points->point_names, point_name);
			uint key = crc32_cstring(point_name);
			game_points->points[game_points->next_point] = gp;
			
			map_insert_kv(&game_points->map, key, game_points->next_point);
			game_points->next_point++;
			goto PARSE_POINTS;
		}
		
		if (does_identifier_match(token, "static")) {
			PARSE_STATIC:;
			Token entity_name_token = get_token(&tknzr);
			
			NEXT_ENTITY_TYPE:;
			if (entity_name_token.type == TOKEN_EOF) {
				parsing = false;
				break;
			}
			if (does_identifier_match(entity_name_token, "points")) {
				goto PARSE_POINTS;
			}
			char entity_name[1024];
			ok = token_to_cstr(entity_name, 1024, entity_name_token);
			if (!ok) {
				printf("not enough space in entity_name\n");
			}
			
			// instantiate the entities of that name
			while(1) {
				Token next_entity_id_token = get_token(&tknzr);
				if (next_entity_id_token.type == TOKEN_EOF) {
					parsing = false;
					break;
				}
				if (!does_identifier_match(next_entity_id_token, "-")) {
					entity_name_token = next_entity_id_token;
					goto NEXT_ENTITY_TYPE;
				}
				v3 mid = parse_vec3_token(&tknzr);
				v3 ex = parse_vec3_token(&tknzr);
				Quat rot = parse_quaternion_token(&tknzr);
				
				uint prot_index = map_lookup_key(&ga->prototypes.prototype_map, &ok, crc32_cstring(entity_name));
				if (!ok) {
					printf("couldn't find prot_index for %s\n", entity_name);
					continue;
				}
				
				GenModel instanced_model = instantiate_static_entity_prototype(ns_instances, &ga->prototypes, prot_index, &ok);
				if (!ok) {
					printf("couldn't instantiate %s\n", entity_name);
					continue;
				}
				
				Base_Entity b;
				b.model = instanced_model;
				b.prototype_index = prot_index;
				b.transform.translation = mid;
				b.transform.rotation = rot;
				b.scale = ex;
				b.scale_single = ga->prototypes.prototypes[prot_index].scale_single;
				
				push_entity(ga, &level_static->entities, b, level_static->entities.num);
				
			}
		}
	}
	
	calculate_entity_collection_transforms(*ns_instances, &level_static->entities);
	rebuild_entity_collection_tree(ga, &level_static->entities);
	//construct_all_static_level_triangles(level_static, ga, ns_instances);
	return;
	
	
	/*
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
  if (!strncmp(line_buffer, "points", 6)) {
   while(1) {
 linecount++;
 if (fgets(line_buffer, 1024, file) == NULL) {
  goto END;
 }
 
 //NEXT_POINT:
 if (line_buffer[0] != '\t') {
  goto START;
 }
 
 clean_word(&line_buffer[1]);
 char point_name[1024];
 strcpy(point_name, &line_buffer[1]);
 
 linecount++;
 if (fgets(line_buffer, 1024, file) == NULL) {
  goto END;
 }
 
 if (line_buffer[0] != '\t' && line_buffer[1] != '\t') {
  goto START;
 }
 if (game_points->next_point >= game_points->max_point) {
  goto START;
 }
 
 Game_Point gp;
 gp.point = parse_vec3(line_buffer);
 gp.name = push_string(&game_points->point_names, point_name);
 uint key = crc32_cstring(point_name);
 game_points->points[game_points->next_point] = gp;
 
 map_insert_kv(&game_points->map, key, game_points->next_point);
 game_points->next_point++;
   }
  }
  
  
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
  
  Base_Entity b;
  b.model = instanced_model;
  b.prototype_index = prot_index;
  b.transform.translation = entity_mid;
  b.transform.rotation = entity_rot;
  b.scale = entity_scale;
  b.scale_single = ga->prototypes.prototypes[prot_index].scale_single;
  
  push_entity(ga, &level_static->entities, b, level_static->entities.num);
 }
   }
  }
 }
 
 END:
 fclose(file);
 
 calculate_entity_collection_transforms(*ns_instances, &level_static->entities);
 rebuild_entity_collection_tree(ga, &level_static->entities);
 //construct_all_static_level_triangles(level_static, ga, ns_instances);
 return;*/
}


void save_level_static(LevelStatic *level_static, const char* levelname, PrototypeStack *prototypes, Game_Points *game_points) {
	
	printf("saving level to: %s\n", levelname);
	int buffer_len = sizeof(LEVEL_DIR) + MAX_NAME_LENGTH + sizeof(LEVEL_END) + 1;
	
	char level_path[buffer_len];
	
	snprintf(level_path, buffer_len,"%s%s%s", LEVEL_DIR, levelname, LEVEL_END);
	
	FILE * file = fopen( level_path, "wb" );
	if (file == NULL) {
		printf("error opening level file for saving %s\n", level_path);
		return;
	}
	char line_buffer[1024];
	
	fprintf(file, "points\n");
	for (int i = 0; i < game_points->next_point; i++) {
		Game_Point gp = game_points->points[i];
		fprintf(file, "\t%s\n", gp.name);
		fprintf(file, "\t\t%f %f %f\n", gp.point.X, gp.point.Y, gp.point.Z);
	}
	
	fprintf(file, "static\n");
	for(int i = 0; i < prototypes->next_prototype; i++) {
		int first_instantiation = 1;
		
		for (int j = 0; j < level_static->entities.num; j++) {
			Base_Entity *b = &level_static->entities.els[j];
			
			if (b->prototype_index != i) {
				continue;
			}
			
			if (first_instantiation) {
				fprintf(file, "\t%s\n", prototypes->prototypes[i].name);
				first_instantiation = 0;
			}
			fprintf(file, "\t\t-\n");
			
			fprintf(file, "\t\t%f %f %f\n", b->transform.translation.X, b->transform.translation.Y, b->transform.translation.Z);
			
			fprintf(file, "\t\t%f %f %f\n", b->scale.X, b->scale.Y, b->scale.Z);
			
			fprintf(file, "\t\t%f %f %f %f\n", b->transform.rotation.X, b->transform.rotation.Y, b->transform.rotation.Z, b->transform.rotation.W);
		}
	}
	
	fclose(file);
}
