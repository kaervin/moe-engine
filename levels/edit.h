
bool mouse_within_nk_rect(struct nk_rect rect, Key_Struct keys) {
	return NK_INBOX(keys.xpos, keys.ypos, rect.x, rect.y, rect.w, rect.h);
}

bool ui_edit_topbar(struct nk_context *ctx, EditorState *es, Key_Struct keys) {
	bool ret = 0;
	
	if (nk_begin(ctx, "editor topbar", nk_rect(5, 5, WINDOW_WIDTH-10, 35), NK_WINDOW_NO_SCROLLBAR ))
	{
		struct nk_rect panel_rect = nk_window_get_bounds(ctx);
		ret =  mouse_within_nk_rect(panel_rect, keys);
		
		nk_layout_row_dynamic(ctx, 20, 8);
		
		if (nk_button_label(ctx, "SETTINGS")) {
			es->editor_mode = SETTINGS;
			es->edit_object_index = -1;
		};
		
		if (nk_button_label(ctx, "SELECTION")) {
			es->editor_mode = SELECTION;
			es->edit_object_index = -1;
		};
		
		if (nk_button_label(ctx, "NEW_OBJECT")) {
			es->editor_mode = NEW_OBJECT;
			es->edit_object_index = -1;
		};
		
	}
	nk_end(ctx);
	
	return ret;
} 

void draw_ruler_plane(float ruler_distance, v3 plane_point, v3 plane_normal, Quat rot) {
	
	Quat n_q = quaternion_rotation_from_vectors(vec3(0.0, 0.0, 1.0), plane_normal);
	
	v3 origin_line_forward = rotate_vec3_quat(vec3(1.0, 0.0, 0.0), n_q);
	v3 origin_line_up = rotate_vec3_quat(vec3(0.0, 1.0, 0.0), n_q);
	
	origin_line_forward = rotate_vec3_quat(origin_line_forward, rot);
	origin_line_up = rotate_vec3_quat(origin_line_up, rot);
	
	//v3 origin_line_forward = rotate_vec3_quat(vec3(plane_normal.Z, plane_normal.X, plane_normal.Y), rot);
	//v3 origin_line_up = rotate_vec3_quat(vec3(plane_normal.Y, plane_normal.Z, plane_normal.X), rot);
	
	mush_draw_segment(dbg_list, plane_point, add_v3(plane_point, origin_line_forward), Red4, Yellow4);
	mush_draw_segment(dbg_list, plane_point, add_v3(plane_point, origin_line_up), Red4, Green4);
	
	
	v4 line_color = vec4(1.0f, 1.0f, 1.0f, 0.03f);
	v4 line_color_non_cleared = vec4(1.0f, 1.0f, 1.0f, 0.3f);
	float line_distance = ruler_distance * 50.0f;
	
	
	for (int i = -50; i < 51; i++) {
		v3 new_plane_point = add_v3(plane_point, mul_v3f(origin_line_up, ruler_distance*i));
		
		v3 line_start = add_v3(new_plane_point, mul_v3f(origin_line_forward, -line_distance));
		
		v3 line_end = add_v3(new_plane_point, mul_v3f(origin_line_forward, line_distance));
		
		mush_draw_segment(dbg_list, line_start, line_end, line_color, line_color);
		
	}
	
	for (int i = -50; i < 51; i++) {
		v3 new_plane_point = add_v3(plane_point, mul_v3f(origin_line_forward, ruler_distance*i));
		
		v3 line_start = add_v3(new_plane_point, mul_v3f(origin_line_up, -line_distance));
		
		v3 line_end = add_v3(new_plane_point, mul_v3f(origin_line_up, line_distance));
		
		mush_draw_segment(dbg_list, line_start, line_end, line_color, line_color);
		mush_draw_segment(dbg_list_non_cleared, line_start, line_end, line_color_non_cleared, line_color_non_cleared);
		
	}
	
}

bool ui_edit_settings(struct nk_context *ctx, EditorState *es, Key_Struct keys) {
	
	bool ret = 0;
	if (nk_begin(ctx, "ui_edit_settings", nk_rect(5, 45, 250, 300), 0))
	{
		struct nk_rect panel_rect = nk_window_get_bounds(ctx);
		ret = mouse_within_nk_rect(panel_rect, keys);
		
		if (!ret) {
			nk_edit_unfocus(ctx);
		}
		
		nk_layout_row_dynamic(ctx, 20, 1);
		
		if (nk_button_label(ctx, "save level")) {
			es->level_needs_saving = true;
		};
		
		nk_label(ctx, "level file:", NK_TEXT_LEFT);
		nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, &es->level_load_name[0], 128, nk_filter_ascii);
		
		if (nk_button_label(ctx, "load level")) {
			es->level_needs_loading = true;
		};
		
		es->show_dbg = nk_check_label(ctx, "show dbg", es->show_dbg);
		es->show_bvh = nk_check_label(ctx, "show bhv", es->show_bvh);
		es->show_all_physics_meshes = nk_check_label(ctx, "show all phys", es->show_all_physics_meshes);
	}
	nk_end(ctx);
	
	return ret;
}

bool ui_edit_static_level_entity(struct nk_context *ctx, LevelStatic *level_static, NodeStack *ns_instances, PrototypeStack *prototypes, baka_StaticObjects *prototype_static_objects, StringStack *ss, int static_level_entity_index, Key_Struct keys) {
	
	bool ret = 0;
	
	StaticLevelEntity *static_level_entity = &level_static->level_entities[static_level_entity_index];
	
	StaticEntityPrototype prot = prototypes->static_prototypes[static_level_entity->prototype_index];
	
	if (nk_begin(ctx, "ui_edit_static_level_entity", nk_rect(5, 45, 250, 300), 0))
	{
		struct nk_rect panel_rect = nk_window_get_bounds(ctx);
		ret =  mouse_within_nk_rect(panel_rect, keys);
		
		nk_layout_row_dynamic(ctx, 20, 1);
		const char* node_name = (const char*)prot.name;
		
		const char* prot_index_string  = &ss->chars[ss->next_string];ss->next_string += 1+sprintf(&ss->chars[ss->next_string], "prot_index: %i", static_level_entity->prototype_index);
		
		nk_label(ctx, node_name, NK_TEXT_LEFT);
		nk_label(ctx, prot_index_string, NK_TEXT_LEFT);
		
		float new_X = nk_propertyf(ctx, "X: ", -10000.0, static_level_entity->transform.translation.X, 10000.0, 0.5f, 0.5f);
		float new_Y = nk_propertyf(ctx, "Y: ", -10000.0, static_level_entity->transform.translation.Y, 10000.0, 0.5f, 0.5f);
		float new_Z = nk_propertyf(ctx, "Z: ", -10000.0, static_level_entity->transform.translation.Z, 10000.0, 0.5f, 0.5f);
		
		float x_rot, y_rot, z_rot;
		
		quaternion_to_euler_angle(static_level_entity->transform.rotation, &x_rot, &y_rot, &z_rot);
		
		x_rot = x_rot * (180.0f/HMM_PI);
		y_rot = y_rot * (180.0f/HMM_PI);
		z_rot = z_rot * (180.0f/HMM_PI);
		
		float new_rot_x_angle = nk_propertyf(ctx, "X a: ", -180.0, x_rot, 180.0, 1.0f, 1.0f);
		float new_rot_y_angle = nk_propertyf(ctx, "Y a: ", -180.0, y_rot, 180.0, 1.0f, 1.0f);
		float new_rot_z_angle = nk_propertyf(ctx, "Z a: ", -180.0, z_rot, 180.0, 1.0f, 1.0f);
		
		float x_rot_new = new_rot_x_angle * (HMM_PI/180.0f);
		float y_rot_new = new_rot_y_angle * (HMM_PI/180.0f);
		float z_rot_new = new_rot_z_angle * (HMM_PI/180.0f);
		
		Quat new_rot = euler_to_quaternion(x_rot_new, y_rot_new, z_rot_new);
		
		// -- scale
		float new_x_scale = nk_propertyf(ctx, "X s: ", 0.1, static_level_entity->scale.X, 100.0, 0.5f, 0.5f);
		
		static_level_entity->scale.X = new_x_scale;
		level_static->scales[static_level_entity_index].X = new_x_scale;
		
		if (!static_level_entity->scale_single) {
			float new_y_scale = nk_propertyf(ctx, "Y s: ", 0.1, static_level_entity->scale.Y, 100.0, 0.5f, 0.5f);
			
			float new_z_scale = nk_propertyf(ctx, "Z s: ", 0.1, static_level_entity->scale.Z, 100.0, 0.5f, 0.5f);
			static_level_entity->scale.Y = new_y_scale;
			static_level_entity->scale.Z = new_z_scale;
			
			level_static->scales[static_level_entity_index].Y = new_y_scale;
			level_static->scales[static_level_entity_index].Z = new_z_scale;
		} else {
			static_level_entity->scale.Y = new_x_scale;
			static_level_entity->scale.Z = new_x_scale;
			
			level_static->scales[static_level_entity_index].Y = new_x_scale;
			level_static->scales[static_level_entity_index].Z = new_x_scale;
		}
		
		
		static_level_entity->transform.rotation = new_rot;
		level_static->transforms[static_level_entity_index].rotation = new_rot;
		
		static_level_entity->transform.translation.X = new_X;
		level_static->transforms[static_level_entity_index].translation.X = new_X;
		
		static_level_entity->transform.translation.Y = new_Y;
		level_static->transforms[static_level_entity_index].translation.Y = new_Y;
		
		static_level_entity->transform.translation.Z = new_Z;
		level_static->transforms[static_level_entity_index].translation.Z = new_Z;
		
		baka_AABB merged_aabb = calculate_static_level_entity_aabb(level_static, prototypes, prototype_static_objects, static_level_entity_index);
		
		//printf("updating static_level_entity->aabb_node_index %i\n", static_level_entity->aabb_node_index);
		
		static_level_entity->aabb_node_index = tree_update_aabb_node(&level_static->entity_tree, static_level_entity->aabb_node_index, merged_aabb);
		
		calculate_object_transforms(
			*ns_instances,
			&static_level_entity->model,
			&level_static->transforms[static_level_entity_index],
			1);
		
		//float nk_propertyf(struct nk_context *ctx, const char *name, float min, float val, float max, float step, float inc_per_pixel);
		//float nk_propertyf(struct nk_context *ctx, const char *name, float min, float val, float max, float step, float inc_per_pixel);
	}
	nk_end(ctx);
	return ret;
}


bool ui_create_static_level_entity(struct nk_context *ctx, LevelStatic *level_static, NodeStack *ns_instances, PrototypeStack *prototypes, baka_StaticObjects *prototype_static_objects, StringStack *ss, int static_level_entity_index, EditorState *es,  Key_Struct keys) {
	bool ret = 0;
	
	if (nk_begin(ctx, "ui_create_static_level_entity", nk_rect(5, 45, 250, 300), 0))
	{
		struct nk_rect panel_rect = nk_window_get_bounds(ctx);
		ret =  mouse_within_nk_rect(panel_rect, keys);
		
		nk_layout_row_dynamic(ctx, 20, 1);
		
		if (es->new_object_prototype_index != -1) {
			StaticEntityPrototype *prot = &prototypes->static_prototypes[es->new_object_prototype_index];
			const char* node_name = (const char*)prot->name;
			nk_label(ctx, node_name, NK_TEXT_LEFT);
		}
		
		for (int i = 0; i < prototypes->next_prototype; i++) {
			StaticEntityPrototype *prot = &prototypes->static_prototypes[i];
			const char* node_name = (const char*)prot->name;
			if (nk_button_label(ctx, node_name)) {
				es->new_object_prototype_index = i;
			};
			
		}
	}
	nk_end(ctx);
	return ret;
}


void editor_step(Game_Struct * game, float dt){//LevelStatic *level_static, Terrain terra, GameAssets *ga, EditorState *es, CameraState *cs, float dt, Key_Struct keys) {
	
	EditorState *es = &game->es;
	Key_Struct keys = game->keys;
	LevelStatic *level_static = &game->level_static;
	GameAssets *ga = &game->assets;
	CameraState *cs = &game->cs;
	NodeStack *ns_instances = &game->ns_instances;
	
	
	bool dont_move_cam = false;
	
	if (es->editor_mode == SETTINGS) {
		if (ui_edit_settings(game->ctx, es, keys)) {
			dont_move_cam = true;
		}
	}
	
	if (es->level_needs_saving) {
		char save_path[PATH_MAX];
		strcpy(save_path, LEVEL_DIR);
		strcat(save_path, game->level_to_be_loaded);
		strcat(save_path, "/");
		strcat(save_path, game->static_level_file_name);
		
		save_level_static(level_static, save_path, &ga->prototypes);
		
		es->level_needs_saving = false;
	}
	
	if (es->level_needs_loading) {
		strcpy(game->level_to_be_loaded, es->level_load_name);
		game->should_reload_lib = true;
		es->level_needs_loading = false;
	}
	
	v3 editor_right;
	v3 editor_forward_XZ = normalize_v3(vec3(es->editor_forward.X, 0.0f, es->editor_forward.Z));
	Quat movement_quaternion = quaternion_rotation_from_vectors(vec3(1.0f, 0.0f, 0.0f), editor_forward_XZ);
	
	if (!dont_move_cam) {
		
		Quat movement_quaternion = quaternion_rotation_from_vectors(vec3(1.0f, 0.0f, 0.0f), editor_forward_XZ);
		
		v3 movement_direction = vec3(keys.l.Y, 0.0f, keys.l.X);
		
		movement_direction = rotate_vec3_quat(movement_direction, movement_quaternion);
		
		movement_direction.Y = keys.l.z;
		
		float move_speed = 100.0f;
		
		es->editor_point = add_v3(es->editor_point, mul_v3f(movement_direction, dt*move_speed));
		
		float rotate_speed = 1.0f;
		
		
		Quat rot_around_up = quaternionFromAxisAngle(vec3(0.0f, 1.0f, 0.0f), -dt*rotate_speed*keys.r.X);
		
		es->editor_forward = rotate_vec3_quat(es->editor_forward, rot_around_up);
		
		editor_right = normalize_v3(cross(es->editor_forward, vec3(0.0f, 1.0f, 0.0f)));
		
		Quat rot_around_right = quaternionFromAxisAngle(editor_right, dt*rotate_speed*keys.r.Y);
		
		es->editor_forward = rotate_vec3_quat(es->editor_forward, rot_around_right);
		
	}
	
	editor_right = normalize_v3(cross(es->editor_forward, vec3(0.0f, 1.0f, 0.0f)));
	
	Quat camera_rot_quat = quaternion_rotation_from_vectors(vec3(1.0f, 0.0f, 0.0f), editor_right);
	
	
	editor_forward_XZ = normalize_v3(vec3(es->editor_forward.X, 0.0f, es->editor_forward.Z));
	
	camera_rot_quat = mul_quat(quaternion_rotation_from_vectors(editor_forward_XZ, es->editor_forward), camera_rot_quat);
	
	// cast a ray from the screen mouse coordinates
	double clip_space_xpos = (keys.xpos / (double)keys.width)*2.0 - 1.0f;
	double clip_space_ypos = -((keys.ypos / (double)keys.height)*2.0 - 1.0f);
	
	v4 raypoint_clipspace = vec4(clip_space_xpos, clip_space_ypos, 1.0, 1.0);
	
    v4 raypoint_cameraspace4 = HMM_MultiplyMat4ByVec4(cs->inv_projection, raypoint_clipspace);
    v3 raypoint_cameraspace = vec3(raypoint_cameraspace4.X, raypoint_cameraspace4.Y, raypoint_cameraspace4.Z);
	v3 raypoint = add_v3(rotate_vec3_quat(raypoint_cameraspace, camera_rot_quat), es->editor_point);
	
	v3 ray_direction = sub_v3(raypoint, es->editor_point);
	
	mush_draw_segment(dbg_list, es->editor_point, add_v3(es->editor_point, mul_v3f(ray_direction, 30.0f)), Blue4, Yellow4);
	
	//mush_draw_sphere(&dbg_list, add_v3(es->editor_point, mul_v3f(ray_direction, 20.0f)), 0.12f, Red4);
	
	
	uint64_t hit_id;
	float tmin_ret;
	v3 tempnormal;
	
	bool didhit = raycast_return_normal(*level_static, ga->prototypes, ga->prototype_static_objects,
										&level_static->entity_tree, es->editor_point, ray_direction, &tmin_ret, &tempnormal, &hit_id);
	/*
 float tri_tmin_ret;
 v3 tri_normal;
 uint64_t tri_col_ids[50];
 int col_id_ret_count;
 
 bool didhit_tri = baka_raycast_tree_return_normal(&level_static->tri_tree, es->editor_point, ray_direction, &tri_tmin_ret, &tri_normal, tri_col_ids, &col_id_ret_count, 50);
 
 if (didhit_tri) {
  for (int i = 0; i < col_id_ret_count; i++) {
   baka_triangle draw_tri = level_static->model_tris[tri_col_ids[i]];
   mush_draw_triangle(&dbg_list, draw_tri.a, draw_tri.b, draw_tri.c, Green4);
  }
 }
 */
	baka_OBB test_obb = baka_make_OBB(vec3(0.0f, 10.0f, 0.0f), vec3(3.0f, 3.0f, 3.0f), quaternion(0.0f, 0.0f, 0.0f, 1.0f));
	
	mush_draw_cube(dbg_list, test_obb.mid, test_obb.ex, test_obb.rot, Blue4);
	
	create_clipped_decal_mesh(level_static, test_obb);
	
	bool mouse_over_ui = 0;
	
	mouse_over_ui = ui_edit_topbar(game->ctx, es, keys);
	
	if (es->editor_mode == SELECTION) {
		
		if (es->edit_object_index != -1) {
			StaticLevelEntity edit_entity = level_static->level_entities[es->edit_object_index];
			
			//mush_draw_sphere(&dbg_list, edit_entity.transform.translation, 0.4f, Yellow4);
			
			mouse_over_ui = ui_edit_static_level_entity(game->ctx, level_static, ns_instances, &ga->prototypes, &ga->prototype_static_objects, &game->string_stack, es->edit_object_index, keys);
			
			if (keys.m_key.down) {
				draw_ruler_plane(5.0f, vec3(es->editor_point.X, edit_entity.transform.translation.Y, es->editor_point.Z), vec3(0.0f, 1.0f, 0.0f), movement_quaternion);
				
				float plane_t;
				
				bool did_hit_plane = baka_raycast_plane(es->editor_point, ray_direction, edit_entity.transform.translation.Y, vec3(0.0f, 1.0f, 0.0f), &plane_t);
				
				if (did_hit_plane) {
					v3 point_of_impact = add_v3(es->editor_point,mul_v3f(ray_direction, plane_t));
					
					level_static->level_entities[es->edit_object_index].transform.translation = point_of_impact;
				}
				
			}
			
			else if (keys.n_key.down) {
				
				
				mush_draw_segment(dbg_list, edit_entity.transform.translation, add_v3(edit_entity.transform.translation, editor_forward_XZ), Red4, Red4);
				
				draw_ruler_plane(5.0f, edit_entity.transform.translation, editor_forward_XZ, quaternion(0.0,0.0,0.0,1.0));
				
				
				float plane_t;
				
				
				
				bool did_hit_plane = baka_raycast_plane( sub_v3(es->editor_point, edit_entity.transform.translation), ray_direction, 0.0f, editor_forward_XZ, &plane_t);
				
				if (did_hit_plane) {
					v3 point_of_impact = add_v3(es->editor_point,mul_v3f(ray_direction, plane_t));
					
					level_static->level_entities[es->edit_object_index].transform.translation = point_of_impact;
				}
			}
		}
		
		if (didhit && !mouse_over_ui) {
			StaticLevelEntity show_entity = level_static->level_entities[hit_id];
			debug_draw_static_level_entity(dbg_list, show_entity, &ga->prototypes, &ga->prototype_static_objects);
		}
		
		
		if (didhit && keys.left_mouse_button.initial && !mouse_over_ui && !keys.m_key.down) {
			es->edit_object_index = hit_id;
			
		}
		
	}
	
	if (es->editor_mode == NEW_OBJECT) {
		
		mouse_over_ui = ui_create_static_level_entity(game->ctx, level_static, ns_instances, &ga->prototypes, &ga->prototype_static_objects, &game->string_stack, es->edit_object_index, es, keys);
		
		draw_ruler_plane(5.0f, vec3(es->editor_point.x, es->plane_height, es->editor_point.z), vec3(0.0, 1.0, 0.0), movement_quaternion);
		
		
		if (!mouse_over_ui) {
			float plane_t;
			
			bool did_hit_plane = baka_raycast_plane(es->editor_point, ray_direction, es->plane_height, vec3(0.0f, 1.0f, 0.0f), &plane_t);
			
			if (did_hit_plane) {
				v3 point_of_impact = add_v3(es->editor_point,mul_v3f(ray_direction, plane_t));
				
				mush_draw_sphere(dbg_list, point_of_impact, 0.2f, Green4);
				
				if (keys.left_mouse_button.initial && es->new_object_prototype_index != -1) {
					
					bool ok = 0;
					
					StaticEntityPrototype prot = ga->prototypes.static_prototypes[es->new_object_prototype_index];
					
					GenModel instanced_model = instantiate_static_entity_prototype(ns_instances, &ga->prototypes, es->new_object_prototype_index, &ok);
					
					printf("instanced_model.node_idx %i, instanced_model.mesh_idx %i\n",instanced_model.node_idx, instanced_model.mesh_idx);
					
					if (!ok) {
						printf("couldn't instantiate static entity prototype in editor\n");
						return;
					}
					
					es->edit_object_index = level_static->next_level_entity;
					es->editor_mode = SELECTION;
					
					StaticLevelEntity sel;
					sel.prototype_index = es->new_object_prototype_index;
					sel.model = instanced_model;
					sel.transform.translation = point_of_impact;
					sel.transform.rotation = quaternion(0.0f, 0.0f, 0.0f, 1.0f);
					sel.scale = vec3(1.0f, 1.0f, 1.0f);
					sel.scale_single = prot.scale_single;
					
					add_static_level_entity_to_level(level_static, ns_instances, sel);
					
					rebuild_level_static_tree(level_static, &ga->prototypes, &ga->prototype_static_objects);
				}
			}
		}
	}
	
	
	if (es->show_bvh) {
		for (int i = 0; i < level_static->entity_tree.next_object; i++) {
			baka_AABB aabb = level_static->entity_tree.nodes[i].aabb;
			mush_draw_cube(dbg_list, aabb.mid, aabb.ex, quaternion(0.0, 0.0, 0.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0));
		}
		baka_AABB root_node_aabb = level_static->entity_tree.nodes[level_static->entity_tree.root_node_index].aabb;
		mush_draw_cube(dbg_list, root_node_aabb.mid, root_node_aabb.ex, quaternion(0.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 1.0, 1.0));
		
	}
	
	if (es->show_all_physics_meshes) {
		for (int i = 0; i < level_static->next_level_entity; i++) {
			StaticLevelEntity statent = level_static->level_entities[i];
			
			debug_draw_static_level_entity(dbg_list, statent, &ga->prototypes, &ga->prototype_static_objects);
		}
	}
	if (es->edit_object_index != -1) {
		StaticLevelEntity edit_entity = level_static->level_entities[es->edit_object_index];
		debug_draw_static_level_entity(dbg_list, edit_entity, &ga->prototypes, &ga->prototype_static_objects);
	}
	
}