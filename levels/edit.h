
#define NK_UNUSED(x) ((void)(x))
#define NK_SATURATE(x) (NK_MAX(0, NK_MIN(1.0f, x)))
#define NK_LEN(a) (sizeof(a)/sizeof(a)[0])
#define NK_ABS(a) (((a) < 0) ? -(a) : (a))
#define NK_BETWEEN(x, a, b) ((a) <= (x) && (x) < (b))
#define NK_INBOX(px, py, x, y, w, h)\
(NK_BETWEEN(px,x,x+w) && NK_BETWEEN(py,y,y+h))
#define NK_INTERSECT(x0, y0, w0, h0, x1, y1, w1, h1) \
(!(((x1 > (x0 + w0)) || ((x1 + w1) < x0) || (y1 > (y0 + h0)) || (y1 + h1) < y0)))
#define NK_CONTAINS(x, y, w, h, bx, by, bw, bh)\
(NK_INBOX(x,y, bx, by, bw, bh) && NK_INBOX(x+w,y+h, bx, by, bw, bh))

bool mouse_within_ui_rect(mu_Rect rect, Key_Struct keys) {
	return NK_INBOX(keys.xpos, keys.ypos, rect.x, rect.y, rect.w, rect.h);
}

bool new_ui_edit_topbar(struct mu_Context *ctx, EditorState *es, Key_Struct keys) {
	
	static mu_Container window;
	
	/* init window manually so we can set its position and size */
	if (!window.inited) {
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(5, 5, WINDOW_WIDTH-10, 35);
	}
	
	bool ret = mouse_within_ui_rect(window.rect, keys);
	
	if (mu_begin_window_ex(ctx, &window, "Panel", MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_NORESIZE)) {
		
		mu_layout_row(ctx, -1, NULL, 0);
		
		if (mu_button(ctx, "SETTINGS")) {
			es->editor_mode = SETTINGS;
			es->edit_object_index = -1;
		};
		
		if (mu_button(ctx, "SELECTION")) {
			es->editor_mode = SELECTION;
			es->edit_object_index = -1;
		};
		
		if (mu_button(ctx, "NEW_OBJECT")) {
			es->editor_mode = NEW_OBJECT;
			es->edit_object_index = -1;
		};
		
		
		mu_end_window(ctx);
	}
	
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
#define MAX_NAME_LIST_CHARS 1024<<4

char level_name_list[MAX_NAME_LIST_CHARS];

void get_level_name_list() {
	int next_char = 0;
	
	DIR *d;
	struct dirent *dir;
	
	d = opendir(LEVEL_DIR);
	if (d) {
		while ((dir = readdir(d)) != NULL)
		{
			char namebuf[2048];
			sprintf(namebuf, "%s/%s/gamestep.c", LEVEL_DIR, dir->d_name);
			
			FILE * file;
			file = fopen(namebuf, "r");
			if (file) {
				if ( (next_char + strlen(dir->d_name)) >= MAX_NAME_LIST_CHARS ) {
					return;
				}
				strcpy(&level_name_list[next_char], dir->d_name);
				next_char += strlen(dir->d_name) + 1;
				fclose(file);
			}
		}
		closedir(d);
	}
}

bool new_ui_edit_settings(mu_Context *ctx, EditorState *es, Key_Struct keys) {
	
	static mu_Container window;
	
	/* init window manually so we can set its position and size */
	if (!window.inited) {
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(5, 45, 250, 300);
		get_level_name_list();
	}
	
	bool ret = mouse_within_ui_rect(window.rect, keys);
	if (mu_begin_window_ex(ctx, &window, "settings", MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_NORESIZE)) {
		mu_layout_row(ctx, 1, (int[]) { 237 }, 0);
		
		if (mu_button(ctx, "save level")) {
			es->level_needs_saving = true;
		};
		mu_checkbox(ctx, &es->show_dbg, "show dbg");
		mu_checkbox(ctx, &es->show_bvh, "show bvh");
		mu_checkbox(ctx, &es->show_all_physics_meshes, "show phys");
		mu_checkbox(ctx, &es->show_perf_times, "show times");
		
		mu_label(ctx, "load level:");
		
		int next_char = 0;
		while(true) {
			char namebuf[2048];
			strcpy(namebuf, &level_name_list[next_char]);
			int len = strlen(namebuf);
			if (len == 0) {
				break;
			}
			next_char += len+1;
			if (mu_button(ctx, namebuf)) {
				strcpy(es->level_load_name, namebuf);
				es->level_needs_loading = true;
			}
		}
		
		mu_end_window(ctx);
	}
	return ret;
}

float within_360(float x) {
	if (x > 180) {
		return (x - 360.0f);
	}
	if (x <= 180) {
		return (x + 360.0f);
	}
	return x;
}

bool edit_point_string_needs_update;

bool new_ui_edit_point(Game_Struct *game, int point_index) {
	Game_Point *gp = &game->game_points.points[point_index];
	mu_Context *ctx = game->mu_ctx;
	Key_Struct keys = game->keys;
	EditorState *es = &game->es;
	
	bool ret = 0;
	
	static mu_Container window;
	
	/* init window manually so we can set its position and size */
	if (!window.inited) {
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(5, 45, 250, 500);
	}
	
	if (edit_point_string_needs_update) {
		strcpy(es->edit_point_string, gp->name);
		edit_point_string_needs_update = false;
	}
	
	ret = mouse_within_ui_rect(window.rect, keys);
	if (mu_begin_window_ex(ctx, &window, "Edit Point", MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_NORESIZE)) {
		
		mu_layout_row(ctx, 1, (int[]) { 237 }, 0);
		
		mu_label(ctx, (const char*)gp->name);
		
		if (mu_textbox(ctx, es->edit_point_string, MAX_NAME_LENGTH) & MU_RES_SUBMIT) {
			mu_set_focus(ctx, ctx->last_id);
			update_game_point_name(&game->game_points, point_index, es->edit_point_string);
		}
		
		mu_label(ctx, "pos:");
		// translation
		mu_number(ctx, &(gp->point.X), 0.5f);
		mu_number(ctx, &(gp->point.Y), 0.5f);
		mu_number(ctx, &(gp->point.Z), 0.5f);
		
		mu_end_window(ctx);
	}
	return ret;
}

bool new_ui_edit_static_level_entity(Game_Struct *game, int entity_index) {
	
	mu_Context *ctx = game->mu_ctx;
	LevelStatic *level_static = &game->level_static;
	EditorState *es = &game->es;
	Key_Struct keys = game->keys;
	GameAssets *ga = &game->assets;
	CameraState *cs = &game->cs;
	NodeStack *ns_instances = &game->ns_instances;
	//StringStack *ss = &game->string_stack; 
	PrototypeStack *prototypes = &ga->prototypes;
	baka_Shape_Stack *prototype_static_objects = &ga->prototype_static_objects;
	
	bool ret = 0;
	
	static mu_Container window;
	
	// init window manually so we can set its position and size
	if (!window.inited) {
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(5, 45, 250, 500);
	}
	
	Base_Entity *entity = &level_static->entities.els[entity_index];
	
	Entity_Prototype prot = prototypes->prototypes[entity->prototype_index];
	
	ret = mouse_within_ui_rect(window.rect, keys);
	if (mu_begin_window_ex(ctx, &window, "Edit Entity", MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_NORESIZE)) {
		char buf[1024];
		
		mu_layout_row(ctx, 1, (int[]) { 237 }, 0);
		
		mu_label(ctx, (const char*)prot.name);
		sprintf(buf, "prot_index: %i", entity->prototype_index);
		mu_label(ctx, buf);
		
		mu_label(ctx, "pos:");
		// translation
		mu_number(ctx, &(entity->transform.translation.X), 0.5f);
		mu_number(ctx, &(entity->transform.translation.Y), 0.5f);
		mu_number(ctx, &(entity->transform.translation.Z), 0.5f);
		
		mu_label(ctx, "rot:");
		
		// TODO: rotation doesn't work correctly, figure out why the hell
		// TODO2: its because of gimbal lock, fuck that shit, maybe just edit quats directly?
		// rotation
		float x_rot, y_rot, z_rot;
		quaternion_to_euler_angle(entity->transform.rotation, &x_rot, &y_rot, &z_rot);
		
		x_rot = x_rot * (180.0f/(HMM_PI));
		y_rot = y_rot * (180.0f/(HMM_PI));
		z_rot = z_rot * (180.0f/(HMM_PI));
		
		mu_number(ctx, &(x_rot), 1.0f);
		mu_number(ctx, &(y_rot), 1.0f);
		mu_number(ctx, &(z_rot), 1.0f);
		
		x_rot = within_360(x_rot);
		y_rot = within_360(y_rot);
		z_rot = within_360(z_rot);
		
		float x_rot_new = x_rot * ((HMM_PI)/180.0f);
		float y_rot_new = y_rot * ((HMM_PI)/180.0f);
		float z_rot_new = z_rot * ((HMM_PI)/180.0f);
		
		Quat new_rot = euler_to_quaternion(x_rot_new, y_rot_new, z_rot_new);
		
		entity->transform.rotation = new_rot;
		
		// scale
		mu_number(ctx, &entity->scale.X, 0.5f);
		
		if (!entity->scale_single) {
			mu_number(ctx, &entity->scale.Y, 0.5f);
			mu_number(ctx, &entity->scale.Z, 0.5f);
		} else {
			entity->scale.Y = entity->scale.X;
			entity->scale.Z = entity->scale.X;
		}
		
		update_base_entity(ga, &level_static->entities, entity_index);
		
		mu_end_window(ctx);
	}
	
	return ret;
}

bool new_ui_create_static_level_entity(Game_Struct *game, int static_level_entity_index) {
	
	mu_Context *ctx = game->mu_ctx;
	LevelStatic *level_static = &game->level_static;
	EditorState *es = &game->es;
	Key_Struct keys = game->keys;
	GameAssets *ga = &game->assets;
	CameraState *cs = &game->cs;
	NodeStack *ns_instances = &game->ns_instances;
	PrototypeStack *prototypes = &ga->prototypes;
	baka_Shape_Stack *prototype_static_objects = &ga->prototype_static_objects;
	
	bool ret = 0;
	
	static mu_Container window;
	
	if (!window.inited) {
		mu_init_window(ctx, &window, 0);
		window.rect = mu_rect(5, 45, 250, 300);
	}
	if (mu_begin_window_ex(ctx, &window, "Create", MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_NORESIZE)) {
		
		mu_layout_row(ctx, 1, (int[]) { 237 }, 0);
		
		if (es->new_object_prototype_index != -1) {
			Entity_Prototype *prot = &prototypes->prototypes[es->new_object_prototype_index];
			const char* node_name = (const char*)prot->name;
			mu_label(ctx,node_name);
		}
		
		if (mu_button(ctx, "add point")) {
			es->new_object_prototype_index = -1;
			es->create_new_point = true;
		}
		
		for (int i = 0; i < prototypes->next_prototype; i++) {
			Entity_Prototype *prot = &prototypes->prototypes[i];
			const char* node_name = (const char*)prot->name;
			if (mu_button(ctx, node_name)) {
				es->new_object_prototype_index = i;
				es->create_new_point = false;
			}
		}
		
		ret = mouse_within_ui_rect(window.rect, keys);
		
		mu_end_window(ctx);
	}
	return ret;
}

void editor_step(Game_Struct * game, float dt){
	
	EditorState *es = &game->es;
	Key_Struct keys = game->keys;
	LevelStatic *level_static = &game->level_static;
	GameAssets *ga = &game->assets;
	CameraState *cs = &game->cs;
	NodeStack *ns_instances = &game->ns_instances;
	
	
	bool dont_move_cam = false;
	
	if (game->mu_ctx->focus != 0) {
		dont_move_cam = true;
	}
	
	if (es->editor_mode == SETTINGS) {
		if (new_ui_edit_settings(game->mu_ctx, es, keys)) {
			dont_move_cam = true;
		}
	}
	
	if (es->level_needs_saving) {
		char save_path[PATH_MAX];
		strcpy(save_path, LEVEL_DIR);
		strcat(save_path, game->level_to_be_loaded);
		strcat(save_path, "/");
		strcat(save_path, game->static_level_file_name);
		
		save_level_static(level_static, save_path, &ga->prototypes, &game->game_points);
		
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
	
	bool didhit = raycast_entity_collection_return_normal(ga, &level_static->entities, es->editor_point, ray_direction, &tmin_ret, &tempnormal, &hit_id);
	
	bool hit_a_point = false;
	uint point_idx = 0;
	
	for (int i = 0; i < game->game_points.next_point; i++) {
		v3 point = game->game_points.points[i].point;
		
		mush_draw_sphere(
			dbg_list,
			point,
			1.0f,
			vec4(1.0f,1.0,0.0f,0.3f));
		
		baka_sphere p_sphere;
		p_sphere.center = point;
		p_sphere.radius = 1.0f;
		
		float p_t;
		v3 p_nrm;
		
		bool didhit_p = baka_raycast_sphere_get_normal(es->editor_point, ray_direction, p_sphere, &p_t, &p_nrm);
		if (didhit_p && (p_t < tmin_ret)) {
			tmin_ret = p_t;
			hit_a_point = true;
			point_idx = i;
			edit_point_string_needs_update = true;
		}
	}
	
	bool mouse_over_ui = 0;
	
	mouse_over_ui = new_ui_edit_topbar(game->mu_ctx, es, keys);
	
	if (es->editor_mode == SELECTION) {
		
		if (es->edit_object_index != -1) {
			Base_Entity edit_entity = level_static->entities.els[es->edit_object_index];
			
			mouse_over_ui = new_ui_edit_static_level_entity(game, es->edit_object_index);
			
			if (!dont_move_cam) {
				if (keys.m_key.down) {
					draw_ruler_plane(5.0f, vec3(es->editor_point.X, edit_entity.transform.translation.Y, es->editor_point.Z), vec3(0.0f, 1.0f, 0.0f), movement_quaternion);
					
					float plane_t;
					
					bool did_hit_plane = baka_raycast_plane(es->editor_point, ray_direction, edit_entity.transform.translation.Y, vec3(0.0f, 1.0f, 0.0f), &plane_t);
					
					if (did_hit_plane) {
						v3 point_of_impact = add_v3(es->editor_point,mul_v3f(ray_direction, plane_t));
						
						level_static->entities.els[es->edit_object_index].transform.translation = point_of_impact;
					}
					
				}
				else if (keys.n_key.down) {
					mush_draw_segment(dbg_list, edit_entity.transform.translation, add_v3(edit_entity.transform.translation, editor_forward_XZ), Red4, Red4);
					
					draw_ruler_plane(5.0f, edit_entity.transform.translation, editor_forward_XZ, quaternion(0.0,0.0,0.0,1.0));
					
					float plane_t;
					bool did_hit_plane = baka_raycast_plane( sub_v3(es->editor_point, edit_entity.transform.translation), ray_direction, 0.0f, editor_forward_XZ, &plane_t);
					
					if (did_hit_plane) {
						v3 point_of_impact = add_v3(es->editor_point,mul_v3f(ray_direction, plane_t));
						
						level_static->entities.els[es->edit_object_index].transform.translation = point_of_impact;
					}
				}
			}
		}
		
		if (es->edit_point_index != -1) {
			Game_Point *edit_point = &game->game_points.points[es->edit_point_index];
			
			mouse_over_ui = new_ui_edit_point(game, es->edit_point_index);
			if (!dont_move_cam) {
				if (keys.m_key.down) {
					draw_ruler_plane(5.0f, vec3(es->editor_point.X, edit_point->point.Y, es->editor_point.Z), vec3(0.0f, 1.0f, 0.0f), movement_quaternion);
					float plane_t;
					
					bool did_hit_plane = baka_raycast_plane(es->editor_point, ray_direction, edit_point->point.Y, vec3(0.0f, 1.0f, 0.0f), &plane_t);
					
					if (did_hit_plane) {
						v3 point_of_impact = add_v3(es->editor_point,mul_v3f(ray_direction, plane_t));
						
						edit_point->point = point_of_impact;
					}
					
				}
				
				else if (keys.n_key.down) {
					mush_draw_segment(dbg_list, edit_point->point, add_v3(edit_point->point, editor_forward_XZ), Red4, Red4);
					draw_ruler_plane(5.0f, edit_point->point, editor_forward_XZ, quaternion(0.0,0.0,0.0,1.0));
					
					float plane_t;
					bool did_hit_plane = baka_raycast_plane( sub_v3(es->editor_point, edit_point->point), ray_direction, 0.0f, editor_forward_XZ, &plane_t);
					
					if (did_hit_plane) {
						v3 point_of_impact = add_v3(es->editor_point,mul_v3f(ray_direction, plane_t));
						
						edit_point->point = point_of_impact;
					}
				}
			}
		}
		
		if (didhit && !mouse_over_ui) {
			Base_Entity show_entity = level_static->entities.els[hit_id];
			debug_draw_base_entity(dbg_list, ga, show_entity, vec4(0.0f, 1.0f, 1.0f, 1.0f));
		}
		
		
		if (didhit && !hit_a_point && keys.left_mouse_button.initial && !mouse_over_ui && !keys.m_key.down) {
			es->edit_object_index = hit_id;
			es->edit_point_index = -1;
		}
		
		if (hit_a_point && keys.left_mouse_button.initial && !mouse_over_ui && !keys.m_key.down) {
			es->edit_object_index = -1;
			es->edit_point_index = point_idx;
		}
	}
	
	if (es->editor_mode == NEW_OBJECT) {
		mouse_over_ui = new_ui_create_static_level_entity(game, es->edit_object_index);
		
		draw_ruler_plane(5.0f, vec3(es->editor_point.x, es->plane_height, es->editor_point.z), vec3(0.0, 1.0, 0.0), movement_quaternion);
		
		if (!mouse_over_ui && !dont_move_cam) {
			float plane_t;
			
			bool did_hit_plane = baka_raycast_plane(es->editor_point, ray_direction, es->plane_height, vec3(0.0f, 1.0f, 0.0f), &plane_t);
			
			if (did_hit_plane) {
				v3 point_of_impact = add_v3(es->editor_point,mul_v3f(ray_direction, plane_t));
				
				mush_draw_sphere(dbg_list, point_of_impact, 0.2f, Green4);
				
				if (keys.left_mouse_button.initial && es->create_new_point== true) {
					Game_Points *game_points = &game->game_points;
					if (game_points->next_point >= game_points->max_point) {
						printf("can't add any more game points\n");
					} else {
						Game_Point gp;
						gp.point = point_of_impact;
						char temp_name[MAX_NAME_LENGTH];
						sprintf(temp_name, "point%i", game_points->next_point);
						
						gp.name = push_string(&game_points->point_names, temp_name);
						uint key = crc32_cstring(temp_name);
						game_points->points[game_points->next_point] = gp;
						
						map_insert_kv(&game_points->map, key, game_points->next_point);
						es->edit_point_index = game_points->next_point;
						es->edit_object_index = -1;
						game_points->next_point++;
						es->editor_mode = SELECTION;
					}
				}
				
				if (keys.left_mouse_button.initial && es->new_object_prototype_index != -1) {
					
					bool ok = 0;
					
					Entity_Prototype prot = ga->prototypes.prototypes[es->new_object_prototype_index];
					
					GenModel instanced_model = instantiate_static_entity_prototype(ns_instances, &ga->prototypes, es->new_object_prototype_index, &ok);
					
					printf("instanced_model.node_idx %i, instanced_model.mesh_idx %i\n",instanced_model.node_idx, instanced_model.mesh_idx);
					
					if (!ok) {
						printf("couldn't instantiate static entity prototype in editor\n");
						return;
					}
					
					es->edit_object_index = level_static->entities.num;
					es->editor_mode = SELECTION;
					
					Base_Entity sel;
					sel.prototype_index = es->new_object_prototype_index;
					sel.model = instanced_model;
					sel.transform.translation = point_of_impact;
					sel.transform.rotation = quaternion(0.0f, 0.0f, 0.0f, 1.0f);
					sel.scale = vec3(1.0f, 1.0f, 1.0f);
					sel.scale_single = prot.scale_single;
					
					push_entity(ga, &level_static->entities, sel, level_static->entities.num);
					//add_static_level_entity_to_level(level_static, ns_instances, sel);
					
					calculate_entity_collection_transforms(*ns_instances, &level_static->entities);
					
					//rebuild_level_static_tree(level_static, &ga->prototypes, &ga->prototype_static_objects);
				}
			}
		}
	}
	
	
	if (es->show_bvh) {
		for (int i = 0; i < level_static->entities.tree.next_object; i++) {
			baka_AABB aabb = level_static->entities.tree.nodes[i].aabb;
			mush_draw_cube(dbg_list, aabb.mid, aabb.ex, quaternion(0.0, 0.0, 0.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0));
		}
		baka_AABB root_node_aabb = level_static->entities.tree.nodes[level_static->entities.tree.root_node_index].aabb;
		mush_draw_cube(dbg_list, root_node_aabb.mid, root_node_aabb.ex, quaternion(0.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 1.0, 1.0));
		
	}
	if (es->edit_object_index != -1) {
		Base_Entity edit_entity = level_static->entities.els[es->edit_object_index];
		debug_draw_base_entity(dbg_list, ga, edit_entity, vec4(1.0f, 0.7f, 0.0f, 0.3f));
	}
	if (es->show_all_physics_meshes) {
		for (int i = 0; i < level_static->entities.num; i++) {
			Base_Entity statent = level_static->entities.els[i];
			debug_draw_base_entity(dbg_list, ga, statent, vec4(1.0f, 1.0f, 1.0f, 0.3f));
		}
	}
	
	if (!dont_move_cam && game->keys.p_key.initial == 1) {
		//game->low_fps_mode= !game->low_fps_mode;
		game->should_reload_lib = 1;
	}
	
}