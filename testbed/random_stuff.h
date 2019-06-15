// Just some random stuff that was in main taken here to declutter main

typedef struct CameraState {
	mat4 projection;
	mat4 inv_projection;
	
	Transform camera_transform;
	
	v3 camera_point;
	v3 camera_right;
	
} CameraState;


typedef struct PlayerState {
	v3 character_point;
	v3 character_model_point;
	v3 character_velocity;
	
	v3 relative_character_ray_origin;
	v3 character_ray_direction;
	v3 inv_character_ray_direction;
	
	v3 character_control_direction;
	v3 character_move_direction;
	v3 character_facing_direction;
	
	bool is_jumping;
	bool on_ground;
	bool on_platform;
	
	v3 character_platform_velocity;
	
	baka_capsule player_capsule;
	
	float player_left_foot_fabrik_mix;
	float player_right_foot_fabrik_mix;
	v3 last_adjusted_foot_delta_left;
	v3 last_adjusted_foot_delta_right;
	
	v3 player_capsule_position_relative_to_player_point;
	Quat player_capsule_rotation;
	
	v3 hitnormal;
	v3 shadow_point;
	
	GenModel gondola_model;
	
	GenModel instantiated_gondola;
	Transform gondola_transform;
	v3 gondola_scale;
	
	AnimReturn ar_gondola;
	float animtime;
	
	float previous_time;
} PlayerState;

// basically all ressources for the step are inside of this struct
typedef struct Game_Struct {
	GLFWwindow* window;
	struct nk_context *ctx;
	
	mu_Context *mu_ctx;
	
	GameAssets assets;
	
	char level_to_be_loaded[PATH_MAX];
	char static_level_file_name[PATH_MAX];
	NodeStack ns_instances;
	
	GenModel *models_to_render;
	v3 *models_to_render_scales;
	uint next_model_to_render;
	uint max_models_to_render;
	
	Game_Points game_points;
	
	LevelStatic level_static;
	Terrain terra;
	
	EditorState es;
	PlayerState ps;
	CameraState cs;
	
	int level_editing_mode;
	bool low_fps_mode;
	int should_reload_lib;
	
	double current_time;
	double last_time;
	
	Rendering_Context_Gen rendering_context;
	mush_context dbg_ctx;
	
	float dt;
	
	Key_Struct keys;
	
	mush_draw_list dbg_list;
	mush_draw_list dbg_list_non_cleared;
	mush_draw_list dbg_list_static;
	
	Perf_Measurement gamestep_perf_msr[1024];
	uint num_gamestep_perf_msr;
	
	void (*sample_func_p) (void); 
	
	FP_Struct fp_struct;
} Game_Struct;


KeyState processKey(GLFWwindow *window, KeyState lastk ,int (*kf) (GLFWwindow *, int), int key_num) {
	KeyState k; 
	
	k.initial = 0;
	k.down = 0;
	k.released = 0;
	
	if(kf(window, key_num) == GLFW_PRESS)  {
		k.down = 1;
		if (lastk.down == 0) {
			k.initial = 1;
		}
	}
	else {
		if (lastk.down) {
			k.released = 1;
		}
	}
	
	return k;
} 

KeyState processButton(KeyState lastk, char button) {
	KeyState k; 
	
	k.initial = 0;
	k.down = 0;
	k.released = 0;
	
	
	if(button == GLFW_PRESS)  {
		k.down = 1;
		if (lastk.down == 0) {
			k.initial = 1;
		}
	}
	else {
		if (lastk.down) {
			k.released = 1;
		}
	}
	return k;
}

Key_Struct processInput(GLFWwindow *window, Key_Struct lastk)
{
	Key_Struct k;
	
	v3 l = vec3(0.0f,0.0f,0.0f);
	v2 r = vec2(0.0f,0.0f);
	
	if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
		int axis_count;
		const float* axis_floats = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axis_count);
		//printf("axis count: %i\n", axis_count);
		
		int btn_count;
		const unsigned char* button_chars = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &btn_count);
		// x is 0, o is 1, ^ is 2, [] is 3
		
		/*
  for (int i = 0; i < axis_count; i++) {
   printf( "%i: %f", i, axis_floats[i]);
  }
  printf("\n");
  
  for (int i = 0; i < btn_count; i++) {
   printf( "%i: %i", i, button_chars[i]);
  }
  printf("\n");
  */
		/*
  float x_val = HMM_PowerF(HMM_ABS(axis_floats[0]), 0.01);
  float y_val = HMM_PowerF(HMM_ABS(axis_floats[1]), 0.01);
  
  if (axis_floats[0] > 0) {l.x = x_val;} else {l.x = -x_val;};
  if (axis_floats[1] < 0) {l.y = y_val;} else {l.y = -y_val;};
  */
		l.x = axis_floats[0];
		l.y = -axis_floats[1];
		//printf("l.x, l.y %f %f\n", l.x, l.y);
		
		r.x = axis_floats[3];
		r.y = axis_floats[4];
		
		k.r_key = processButton(lastk.r_key, button_chars[0]);
		
	}
	
	
	
	float rot_x = 0.0f;
	if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)	 { rot_x = rot_x + 1.0f;}
	if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)	 { rot_x = rot_x - 1.0f;}
	
	float rot_y = 0.0f;
	if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)	 { rot_y = rot_y + 1.0f;}
	if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)	 { rot_y = rot_y - 1.0f;}
	
	float rot_z = 0.0f;
	if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)	 { rot_z = rot_z + 1.0f;}
	if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)	 { rot_z = rot_z - 1.0f;}
	
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)	 { l = vec3( l.x , l.y + 1.0f , l.z );}
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)	 { l = vec3( l.x , l.y - 1.0f , l.z );}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)	 { l = vec3( l.x + 1.0f , l.y , l.z );}
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)	 { l = vec3( l.x - 1.0f , l.y , l.z );}
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)	 { l = vec3( l.x , l.y , l.z - 1.0f );}
	if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)	 { l = vec3( l.x , l.y , l.z + 1.0f );}
	
	if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)	 { r = vec2( r.x , r.y + 1.0f );}
	if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)	 { r = vec2( r.x , r.y - 1.0f );}
	if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { r = vec2( r.x + 1.0f , r.y );}
	if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)	 { r = vec2( r.x - 1.0f , r.y );}
	
	int (*key_f) (GLFWwindow *, int) = &glfwGetKey;
	k.g_key = processKey(window, lastk.g_key, key_f, GLFW_KEY_G);
	k.h_key = processKey(window, lastk.h_key, key_f, GLFW_KEY_H);
	k.j_key = processKey(window, lastk.j_key, key_f, GLFW_KEY_J);
	k.n_key = processKey(window, lastk.n_key, key_f, GLFW_KEY_N);
	k.m_key = processKey(window, lastk.m_key, key_f, GLFW_KEY_M);
	k.r_key = processKey(window, lastk.r_key, key_f, GLFW_KEY_R);
	k.t_key = processKey(window, lastk.t_key, key_f, GLFW_KEY_T);
	k.p_key = processKey(window, lastk.p_key, key_f, GLFW_KEY_P);
	
	k.shift_key = processKey(window, lastk.p_key, key_f, GLFW_KEY_LEFT_SHIFT);
	
	k.F1_key = processKey(window, lastk.F1_key, key_f, GLFW_KEY_F1);
	k.F2_key = processKey(window, lastk.F2_key, key_f, GLFW_KEY_F2);
	k.F3_key = processKey(window, lastk.F3_key, key_f, GLFW_KEY_F3);
	k.F4_key = processKey(window, lastk.F4_key, key_f, GLFW_KEY_F4);
	k.F5_key = processKey(window, lastk.F5_key, key_f, GLFW_KEY_F5);
	k.F6_key = processKey(window, lastk.F6_key, key_f, GLFW_KEY_F6);
	k.F7_key = processKey(window, lastk.F7_key, key_f, GLFW_KEY_F7);
	k.F8_key = processKey(window, lastk.F8_key, key_f, GLFW_KEY_F8);
	k.F9_key = processKey(window, lastk.F9_key, key_f, GLFW_KEY_F9);
	k.F10_key = processKey(window, lastk.F10_key, key_f, GLFW_KEY_F10);
	
	double xpos, ypos;
	glfwGetCursorPos(window, &k.xpos, &k.ypos);
	
	glfwGetWindowSize(window, &k.width, &k.height);
	
	if (k.xpos < k.width && k.xpos >= 0 && k.ypos < k.height && k.ypos >= 0) {
		k.in_window = true;
	}
	else {
		k.in_window = false;
	}
	
	int (*mouse_f) (GLFWwindow *, int) = &glfwGetMouseButton;
	k.left_mouse_button = processKey(window, lastk.left_mouse_button, mouse_f, GLFW_MOUSE_BUTTON_LEFT);
	k.right_mouse_button = processKey(window, lastk.right_mouse_button, mouse_f, GLFW_MOUSE_BUTTON_RIGHT);
	
	k.l = l;
	k.r = r;
	k.rot_x = rot_x;
	k.rot_y = rot_y;
	k.rot_z = rot_z;
	
	return k;
}

// creates a clipped decal mesh with the static objects
// NOTE: cool and all, but doing this in the shader would maybe work better
void create_clipped_decal_mesh(LevelStatic *level_static, baka_OBB obb) {
	
	float tri_tmin_ret;
	v3 tri_normal;
	uint64_t tri_col_ids[50];
	int col_id_ret_count;
	
	baka_AABB aabb = aabb_from_obb(obb);
	
	baka_aabb_find_contacts_tree(&level_static->tri_tree, &aabb, tri_col_ids, &col_id_ret_count, 50);
	
	v3 obb_inv_scale = vec3(1.0f/obb.ex.x, 1.0f/obb.ex.y, 1.0f/obb.ex.z);
	
	// now clip every triangle and disregard those that are degenerate
	
	baka_triangle tri_stack_data_1[1000];
	baka_triangle tri_stack_data_2[1000];
	
	baka_triangle * tri_stack = tri_stack_data_1;
	baka_triangle * new_tri_stack = tri_stack_data_2;
	uint num_tri_stack = 0;
	int num_new_tris = 0;
	
	// add the triangles to the stack for clipping
	for (int i = 0; i < col_id_ret_count; i++) {
		baka_triangle orig_tri = level_static->model_tris[tri_col_ids[i]];
		
		//mush_draw_triangle(&dbg_list, orig_tri.a, orig_tri.b, orig_tri.c, Red4);
		
		baka_triangle tri;
		tri.a = mul_v3(rotate_vec3_quat(sub_v3(orig_tri.a, obb.mid), obb.inv_rot), obb_inv_scale);
		tri.b = mul_v3(rotate_vec3_quat(sub_v3(orig_tri.b, obb.mid), obb.inv_rot), obb_inv_scale);
		tri.c = mul_v3(rotate_vec3_quat(sub_v3(orig_tri.c, obb.mid), obb.inv_rot), obb_inv_scale);
		
		//mush_draw_triangle(&dbg_list, tri.a, tri.b, tri.c, Red4);
		
		tri_stack[num_tri_stack] = tri;
		num_tri_stack++;
	}
	
	
	// clip against left plane
	for (int i = 0; i < num_tri_stack; ) {
		
		//printf("iteration %i\n", i);
		
		bool mark[3];
		mark[0] = false;
		mark[1] = false;
		mark[2] = false;
		
		baka_triangle *tri = &tri_stack[i];
		
		if (tri->a.x < -1.0) {
			mark[0] = true;
		}
		if (tri->b.x < -1.0) {
			mark[1] = true;
		}
		if (tri->c.x < -1.0) {
			mark[2] = true;
		}
		
		//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
		
		int num_mark = 0;
		for (int j = 0; j < 3; j++) {
			if (mark[j]) {
				num_mark++;
			}
		}
		
		//printf("num_mark %i\n", num_mark);
		
		if (num_mark == 0) {
			new_tri_stack[num_new_tris] = tri_stack[i];
			num_new_tris++;
			i++;
			continue;
		}
		// if every x is outside then this triangle should gtfo
		if (num_mark == 3) {
			i++;
			continue;
		}
		
		// 'rotate' the triangle points so that the marked ones are in the beginning
		// only rotate and not reflect so that the winding stays the same
		for (;;) {
			if (num_mark == 1) {
				//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
				if (mark[0]) {
					break;
				}
				else {
					bool temp_mark = mark[0];
					v3 temp_v = tri->a;
					
					mark[0] = mark[2];
					mark[2] = mark[1];
					mark[1] = temp_mark;
					
					tri->a = tri->c;
					tri->c = tri->b;
					tri->b = temp_v;
				}
			}
			if (num_mark == 2) {
				//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
				if (mark[0] && mark[1]) {
					break;
				}
				else {
					bool temp_mark = mark[0];
					v3 temp_v = tri->a;
					
					mark[0] = mark[2];
					mark[2] = mark[1];
					mark[1] = temp_mark;
					
					tri->a = tri->c;
					tri->c = tri->b;
					tri->b = temp_v;
				}
			}
		}
		
		// https://www.cubic.org/docs/3dclip.htm
		if (num_mark == 1) {
			// calculate the intersection point between ab and the plane
			float da = tri->a.x - (-1.0f); 
			float db = tri->b.x - (-1.0f);
			
			float s1 = da/(da-db);
			
			v3 new_point_1;
			new_point_1.x = tri->a.x + s1*(tri->b.x - tri->a.x);
			new_point_1.y = tri->a.y + s1*(tri->b.y - tri->a.y);
			new_point_1.z = tri->a.z + s1*(tri->b.z - tri->a.z);
			
			// same for ac
			float dc = tri->c.x - (-1.0f);
			
			float s2 = da/(da-dc);
			
			v3 new_point_2;
			new_point_2.x = tri->a.x + s2*(tri->c.x - tri->a.x);
			new_point_2.y = tri->a.y + s2*(tri->c.y - tri->a.y);
			new_point_2.z = tri->a.z + s2*(tri->c.z - tri->a.z);
			
			baka_triangle new_tri_1;
			new_tri_1.a = new_point_1;
			new_tri_1.b = new_point_2;
			new_tri_1.c = tri->c;
			
			baka_triangle new_tri_2;
			new_tri_2.a = new_point_1;
			new_tri_2.b = tri->b;
			new_tri_2.c = tri->c;
			
			new_tri_stack[num_new_tris] = new_tri_1;
			num_new_tris++;
			new_tri_stack[num_new_tris] = new_tri_2;
			num_new_tris++;
		}
		if (num_mark == 2) {
			float da = tri->a.x - (-1.0f); 
			float db = tri->b.x - (-1.0f);
			float dc = tri->c.x - (-1.0f);
			
			float s1 = dc/(dc-da);
			float s2 = dc/(dc-db);
			
			v3 new_point_1;
			new_point_1.x = tri->c.x + s1*(tri->a.x - tri->c.x);
			new_point_1.y = tri->c.y + s1*(tri->a.y - tri->c.y);
			new_point_1.z = tri->c.z + s1*(tri->a.z - tri->c.z);
			
			v3 new_point_2;
			new_point_2.x = tri->c.x + s2*(tri->b.x - tri->c.x);
			new_point_2.y = tri->c.y + s2*(tri->b.y - tri->c.y);
			new_point_2.z = tri->c.z + s2*(tri->b.z - tri->c.z);
			
			baka_triangle new_tri;
			new_tri.a = new_point_1;
			new_tri.b = new_point_2;
			new_tri.c = tri->c;
			
			new_tri_stack[num_new_tris] = new_tri;
			num_new_tris++;
		}
		i++;
	}
	baka_triangle * switch_pntr = tri_stack;
	tri_stack = new_tri_stack;
	num_tri_stack = num_new_tris;
	new_tri_stack = switch_pntr;
	num_new_tris = 0;
	
	
	// clip against right plane
	for (int i = 0; i < num_tri_stack; ) {
		
		//printf("iteration %i\n", i);
		
		bool mark[3];
		mark[0] = false;
		mark[1] = false;
		mark[2] = false;
		
		baka_triangle *tri = &tri_stack[i];
		
		if (tri->a.x > 1.0) {
			mark[0] = true;
		}
		if (tri->b.x > 1.0) {
			mark[1] = true;
		}
		if (tri->c.x > 1.0) {
			mark[2] = true;
		}
		
		//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
		
		int num_mark = 0;
		for (int j = 0; j < 3; j++) {
			if (mark[j]) {
				num_mark++;
			}
		}
		
		//printf("num_mark %i\n", num_mark);
		
		if (num_mark == 0) {
			new_tri_stack[num_new_tris] = tri_stack[i];
			num_new_tris++;
			i++;
			continue;
		}
		// if every x is outside then this triangle should gtfo
		if (num_mark == 3) {
			i++;
			continue;
		}
		
		// 'rotate' the triangle points so that the marked ones are in the beginning
		// only rotate and not reflect so that the winding stays the same
		for (;;) {
			if (num_mark == 1) {
				//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
				if (mark[0]) {
					break;
				}
				else {
					bool temp_mark = mark[0];
					v3 temp_v = tri->a;
					
					mark[0] = mark[2];
					mark[2] = mark[1];
					mark[1] = temp_mark;
					
					tri->a = tri->c;
					tri->c = tri->b;
					tri->b = temp_v;
				}
			}
			if (num_mark == 2) {
				//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
				if (mark[0] && mark[1]) {
					break;
				}
				else {
					bool temp_mark = mark[0];
					v3 temp_v = tri->a;
					
					mark[0] = mark[2];
					mark[2] = mark[1];
					mark[1] = temp_mark;
					
					tri->a = tri->c;
					tri->c = tri->b;
					tri->b = temp_v;
				}
			}
		}
		
		// https://www.cubic.org/docs/3dclip.htm
		if (num_mark == 1) {
			// calculate the intersection point between ab and the plane
			float da = tri->a.x - (1.0f); 
			float db = tri->b.x - (1.0f);
			
			float s1 = da/(da-db);
			
			v3 new_point_1;
			new_point_1.x = tri->a.x + s1*(tri->b.x - tri->a.x);
			new_point_1.y = tri->a.y + s1*(tri->b.y - tri->a.y);
			new_point_1.z = tri->a.z + s1*(tri->b.z - tri->a.z);
			
			// same for ac
			float dc = tri->c.x - (1.0f);
			
			float s2 = da/(da-dc);
			
			v3 new_point_2;
			new_point_2.x = tri->a.x + s2*(tri->c.x - tri->a.x);
			new_point_2.y = tri->a.y + s2*(tri->c.y - tri->a.y);
			new_point_2.z = tri->a.z + s2*(tri->c.z - tri->a.z);
			
			baka_triangle new_tri_1;
			new_tri_1.a = new_point_1;
			new_tri_1.b = new_point_2;
			new_tri_1.c = tri->c;
			
			baka_triangle new_tri_2;
			new_tri_2.a = new_point_1;
			new_tri_2.b = tri->b;
			new_tri_2.c = tri->c;
			
			new_tri_stack[num_new_tris] = new_tri_1;
			num_new_tris++;
			new_tri_stack[num_new_tris] = new_tri_2;
			num_new_tris++;
		}
		if (num_mark == 2) {
			float da = tri->a.x - (1.0f); 
			float db = tri->b.x - (1.0f);
			float dc = tri->c.x - (1.0f);
			
			float s1 = dc/(dc-da);
			float s2 = dc/(dc-db);
			
			v3 new_point_1;
			new_point_1.x = tri->c.x + s1*(tri->a.x - tri->c.x);
			new_point_1.y = tri->c.y + s1*(tri->a.y - tri->c.y);
			new_point_1.z = tri->c.z + s1*(tri->a.z - tri->c.z);
			
			v3 new_point_2;
			new_point_2.x = tri->c.x + s2*(tri->b.x - tri->c.x);
			new_point_2.y = tri->c.y + s2*(tri->b.y - tri->c.y);
			new_point_2.z = tri->c.z + s2*(tri->b.z - tri->c.z);
			
			baka_triangle new_tri;
			new_tri.a = new_point_1;
			new_tri.b = new_point_2;
			new_tri.c = tri->c;
			
			new_tri_stack[num_new_tris] = new_tri;
			num_new_tris++;
		}
		i++;
	}
	switch_pntr = tri_stack;
	tri_stack = new_tri_stack;
	num_tri_stack = num_new_tris;
	new_tri_stack = switch_pntr;
	num_new_tris = 0;
	
	
	// clip against up  plane
	for (int i = 0; i < num_tri_stack; ) {
		
		//printf("iteration %i\n", i);
		
		bool mark[3];
		mark[0] = false;
		mark[1] = false;
		mark[2] = false;
		
		baka_triangle *tri = &tri_stack[i];
		
		if (tri->a.z > 1.0) {
			mark[0] = true;
		}
		if (tri->b.z > 1.0) {
			mark[1] = true;
		}
		if (tri->c.z > 1.0) {
			mark[2] = true;
		}
		
		//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
		
		int num_mark = 0;
		for (int j = 0; j < 3; j++) {
			if (mark[j]) {
				num_mark++;
			}
		}
		
		//printf("num_mark %i\n", num_mark);
		
		if (num_mark == 0) {
			new_tri_stack[num_new_tris] = tri_stack[i];
			num_new_tris++;
			i++;
			continue;
		}
		// if every x is outside then this triangle should gtfo
		if (num_mark == 3) {
			i++;
			continue;
		}
		
		// 'rotate' the triangle points so that the marked ones are in the beginning
		// only rotate and not reflect so that the winding stays the same
		for (;;) {
			if (num_mark == 1) {
				//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
				if (mark[0]) {
					break;
				}
				else {
					bool temp_mark = mark[0];
					v3 temp_v = tri->a;
					
					mark[0] = mark[2];
					mark[2] = mark[1];
					mark[1] = temp_mark;
					
					tri->a = tri->c;
					tri->c = tri->b;
					tri->b = temp_v;
				}
			}
			if (num_mark == 2) {
				//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
				if (mark[0] && mark[1]) {
					break;
				}
				else {
					bool temp_mark = mark[0];
					v3 temp_v = tri->a;
					
					mark[0] = mark[2];
					mark[2] = mark[1];
					mark[1] = temp_mark;
					
					tri->a = tri->c;
					tri->c = tri->b;
					tri->b = temp_v;
				}
			}
		}
		
		// https://www.cubic.org/docs/3dclip.htm
		if (num_mark == 1) {
			// calculate the intersection point between ab and the plane
			float da = tri->a.z - (1.0f); 
			float db = tri->b.z - (1.0f);
			
			float s1 = da/(da-db);
			
			v3 new_point_1;
			new_point_1.x = tri->a.x + s1*(tri->b.x - tri->a.x);
			new_point_1.y = tri->a.y + s1*(tri->b.y - tri->a.y);
			new_point_1.z = tri->a.z + s1*(tri->b.z - tri->a.z);
			
			// same for ac
			float dc = tri->c.z - (1.0f);
			
			float s2 = da/(da-dc);
			
			v3 new_point_2;
			new_point_2.x = tri->a.x + s2*(tri->c.x - tri->a.x);
			new_point_2.y = tri->a.y + s2*(tri->c.y - tri->a.y);
			new_point_2.z = tri->a.z + s2*(tri->c.z - tri->a.z);
			
			baka_triangle new_tri_1;
			new_tri_1.a = new_point_1;
			new_tri_1.b = new_point_2;
			new_tri_1.c = tri->c;
			
			baka_triangle new_tri_2;
			new_tri_2.a = new_point_1;
			new_tri_2.b = tri->b;
			new_tri_2.c = tri->c;
			
			new_tri_stack[num_new_tris] = new_tri_1;
			num_new_tris++;
			new_tri_stack[num_new_tris] = new_tri_2;
			num_new_tris++;
		}
		if (num_mark == 2) {
			float da = tri->a.z - (1.0f); 
			float db = tri->b.z - (1.0f);
			float dc = tri->c.z - (1.0f);
			
			float s1 = dc/(dc-da);
			float s2 = dc/(dc-db);
			
			v3 new_point_1;
			new_point_1.x = tri->c.x + s1*(tri->a.x - tri->c.x);
			new_point_1.y = tri->c.y + s1*(tri->a.y - tri->c.y);
			new_point_1.z = tri->c.z + s1*(tri->a.z - tri->c.z);
			
			v3 new_point_2;
			new_point_2.x = tri->c.x + s2*(tri->b.x - tri->c.x);
			new_point_2.y = tri->c.y + s2*(tri->b.y - tri->c.y);
			new_point_2.z = tri->c.z + s2*(tri->b.z - tri->c.z);
			
			baka_triangle new_tri;
			new_tri.a = new_point_1;
			new_tri.b = new_point_2;
			new_tri.c = tri->c;
			
			new_tri_stack[num_new_tris] = new_tri;
			num_new_tris++;
		}
		i++;
	}
	switch_pntr = tri_stack;
	tri_stack = new_tri_stack;
	num_tri_stack = num_new_tris;
	new_tri_stack = switch_pntr;
	num_new_tris = 0;
	
	
	// clip against dow  plane
	for (int i = 0; i < num_tri_stack; ) {
		
		//printf("iteration %i\n", i);
		
		bool mark[3];
		mark[0] = false;
		mark[1] = false;
		mark[2] = false;
		
		baka_triangle *tri = &tri_stack[i];
		
		if (tri->a.z < -1.0) {
			mark[0] = true;
		}
		if (tri->b.z < -1.0) {
			mark[1] = true;
		}
		if (tri->c.z < -1.0) {
			mark[2] = true;
		}
		
		//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
		
		int num_mark = 0;
		for (int j = 0; j < 3; j++) {
			if (mark[j]) {
				num_mark++;
			}
		}
		
		//printf("num_mark %i\n", num_mark);
		
		if (num_mark == 0) {
			new_tri_stack[num_new_tris] = tri_stack[i];
			num_new_tris++;
			i++;
			continue;
		}
		// if every x is outside then this triangle should gtfo
		if (num_mark == 3) {
			i++;
			continue;
		}
		
		// 'rotate' the triangle points so that the marked ones are in the beginning
		// only rotate and not reflect so that the winding stays the same
		for (;;) {
			if (num_mark == 1) {
				//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
				if (mark[0]) {
					break;
				}
				else {
					bool temp_mark = mark[0];
					v3 temp_v = tri->a;
					
					mark[0] = mark[2];
					mark[2] = mark[1];
					mark[1] = temp_mark;
					
					tri->a = tri->c;
					tri->c = tri->b;
					tri->b = temp_v;
				}
			}
			if (num_mark == 2) {
				//printf("mark[0] %i mark[1] %i mark[2] %i\n", mark[0], mark[1], mark[2]);
				if (mark[0] && mark[1]) {
					break;
				}
				else {
					bool temp_mark = mark[0];
					v3 temp_v = tri->a;
					
					mark[0] = mark[2];
					mark[2] = mark[1];
					mark[1] = temp_mark;
					
					tri->a = tri->c;
					tri->c = tri->b;
					tri->b = temp_v;
				}
			}
		}
		
		// https://www.cubic.org/docs/3dclip.htm
		if (num_mark == 1) {
			// calculate the intersection point between ab and the plane
			float da = tri->a.z - (-1.0f); 
			float db = tri->b.z - (-1.0f);
			
			float s1 = da/(da-db);
			
			v3 new_point_1;
			new_point_1.x = tri->a.x + s1*(tri->b.x - tri->a.x);
			new_point_1.y = tri->a.y + s1*(tri->b.y - tri->a.y);
			new_point_1.z = tri->a.z + s1*(tri->b.z - tri->a.z);
			
			// same for ac
			float dc = tri->c.z - (-1.0f);
			
			float s2 = da/(da-dc);
			
			v3 new_point_2;
			new_point_2.x = tri->a.x + s2*(tri->c.x - tri->a.x);
			new_point_2.y = tri->a.y + s2*(tri->c.y - tri->a.y);
			new_point_2.z = tri->a.z + s2*(tri->c.z - tri->a.z);
			
			baka_triangle new_tri_1;
			new_tri_1.a = new_point_1;
			new_tri_1.b = new_point_2;
			new_tri_1.c = tri->c;
			
			baka_triangle new_tri_2;
			new_tri_2.a = new_point_1;
			new_tri_2.b = tri->b;
			new_tri_2.c = tri->c;
			
			new_tri_stack[num_new_tris] = new_tri_1;
			num_new_tris++;
			new_tri_stack[num_new_tris] = new_tri_2;
			num_new_tris++;
		}
		if (num_mark == 2) {
			float da = tri->a.z - (-1.0f); 
			float db = tri->b.z - (-1.0f);
			float dc = tri->c.z - (-1.0f);
			
			float s1 = dc/(dc-da);
			float s2 = dc/(dc-db);
			
			v3 new_point_1;
			new_point_1.x = tri->c.x + s1*(tri->a.x - tri->c.x);
			new_point_1.y = tri->c.y + s1*(tri->a.y - tri->c.y);
			new_point_1.z = tri->c.z + s1*(tri->a.z - tri->c.z);
			
			v3 new_point_2;
			new_point_2.x = tri->c.x + s2*(tri->b.x - tri->c.x);
			new_point_2.y = tri->c.y + s2*(tri->b.y - tri->c.y);
			new_point_2.z = tri->c.z + s2*(tri->b.z - tri->c.z);
			
			baka_triangle new_tri;
			new_tri.a = new_point_1;
			new_tri.b = new_point_2;
			new_tri.c = tri->c;
			
			new_tri_stack[num_new_tris] = new_tri;
			num_new_tris++;
		}
		i++;
	}
	
	
	
	for (int i = 0; i < num_new_tris; i++) {
		baka_triangle orig_tri = new_tri_stack[i];
		
		baka_triangle tri;
		tri.a = add_v3(rotate_vec3_quat(mul_v3(orig_tri.a, obb.ex), obb.rot), obb.mid);
		tri.b = add_v3(rotate_vec3_quat(mul_v3(orig_tri.b, obb.ex), obb.rot), obb.mid);
		tri.c = add_v3(rotate_vec3_quat(mul_v3(orig_tri.c, obb.ex), obb.rot), obb.mid);
		
		mush_draw_triangle(dbg_list, tri.a, tri.b, tri.c, Yellow4);
	}
	
	for (int i = 0; i < num_tri_stack; i++) {
		baka_triangle orig_tri = tri_stack[i];
		
		mush_draw_triangle(dbg_list, orig_tri.a, orig_tri.b, orig_tri.c, Green4);
		
		baka_triangle tri;
		tri.a = add_v3(rotate_vec3_quat(mul_v3(orig_tri.a, obb.ex), obb.rot), obb.mid);
		tri.b = add_v3(rotate_vec3_quat(mul_v3(orig_tri.b, obb.ex), obb.rot), obb.mid);
		tri.c = add_v3(rotate_vec3_quat(mul_v3(orig_tri.c, obb.ex), obb.rot), obb.mid);
		
		//mush_draw_triangle(dbg_list, tri.a, tri.b, tri.c, Green4);
	}
	
}


bool raycast_body(GameAssets *assets, baka_Body phys, Transform transform, v3 scale, v3 ray_origin, v3 ray_direction, float *tmin_ret, v3 *normal_ret) {
	
	baka_Shape_Stack *prototype_static_objects = &assets->prototype_static_objects ;
	
	bool didhit_return = false;
	
	float tmin;
	v3 tempnormal;
	
	float minimum_tmin = INFINITY;
	v3 hitnormal;
	
	v3 mid = transform.translation;
	Quat rot = transform.rotation;
	
	for (int j = 0; j < phys.num_shapes; j++) {
		baka_Shape next_object = prototype_static_objects->els[phys.shape_index + j];
		
		if(next_object.type_id == PHYS_OBB) {
			baka_OBB new_obb = baka_make_OBB(add_v3(rotate_vec3_quat(mul_v3(next_object.obb.mid, scale), rot), mid), mul_v3(next_object.obb.ex, scale), mul_quat(next_object.obb.rot, rot));
			
			bool didhit_obb = baka_raycast_obb_get_normal(ray_origin, ray_direction, new_obb, &tmin, &tempnormal);
			
			if (didhit_obb && tmin < minimum_tmin && tmin >= 0.0f) {
				minimum_tmin = tmin;
				hitnormal = tempnormal;
				didhit_return = true;
			}
			
		}
		
		if(next_object.type_id == PHYS_SPHERE) {
			baka_sphere new_sphere = baka_make_sphere(add_v3(rotate_vec3_quat( mul_v3f(next_object.sphere.center, scale.X), rot), mid), next_object.sphere.radius * scale.X);
			
			bool didhit_sphere = baka_raycast_sphere_get_normal(ray_origin, ray_direction, new_sphere, &tmin, &tempnormal);
			
			if (didhit_sphere && tmin < minimum_tmin && tmin >= 0.0f) {
				minimum_tmin = tmin;
				hitnormal = tempnormal;
				didhit_return = true;
			}
			
		}
		
		if(next_object.type_id == PHYS_TRIANGLE) {
			baka_triangle new_tri;
			new_tri.a = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.a, scale), rot), mid);
			new_tri.b = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.b, scale), rot), mid);
			new_tri.c = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.c, scale), rot), mid);
			
			bool didhit_tri = baka_raycast_triangle_get_normal(ray_origin, ray_direction, new_tri, &tmin, &tempnormal);
			
			if (didhit_tri && tmin < minimum_tmin && tmin >= 0.0f) {
				minimum_tmin = tmin;
				hitnormal = tempnormal;
				didhit_return = true;
			}
		}
	}
	
	*tmin_ret = minimum_tmin;
	*normal_ret = hitnormal;
	
	return didhit_return;
}


// TODO: we can definitely make the call to a raycast simpler now
bool raycast_entity_collection_return_normal_and_ids(GameAssets *assets, Entity_Collection *col, v3 ray_origin, v3 ray_direction, float *tmin_ret, v3 *normal_ret, uint64_t *collision_ids, int *collision_ids_count, int collision_ids_max, uint64_t *closest_id) {
	
	baka_Shape_Stack *prototype_static_objects = &assets->prototype_static_objects;
	PrototypeStack *prototypes = &assets->prototypes;
	bool didhit_return = false;
	
	float tmin;
	v3 tempnormal;
	
	float minimum_tmin = INFINITY;
	v3 hitnormal;
	
	bool didhit = baka_raycast_tree_return_normal(
		&col->tree, ray_origin, ray_direction, &tmin, &tempnormal, collision_ids, collision_ids_count, collision_ids_max);
	
	for (int i = 0; i < *collision_ids_count; i++) {
		uint64_t col_id = collision_ids[i]; 
		
		Base_Entity to_test = col->els[col_id];
		baka_Body body = prototypes->prototypes[to_test.prototype_index].body;
		
		v3 mid = to_test.transform.translation;
		Quat rot = to_test.transform.rotation;
		v3 scale = to_test.scale;
		
		bool entity_hit = raycast_body(assets, body, to_test.transform, to_test.scale, ray_origin, ray_direction, &tmin, &tempnormal);
		
		if (entity_hit && tmin < minimum_tmin && tmin >= 0.0f) {
			*closest_id = col_id;
			minimum_tmin = tmin;
			hitnormal = tempnormal;
			didhit_return = true;
		}
		
	}
	*tmin_ret = minimum_tmin;
	*normal_ret = hitnormal;
	
	return didhit_return;
}

static bool raycast_entity_collection_return_normal(GameAssets *assets, Entity_Collection *col, v3 ray_origin, v3 ray_direction, float *tmin_ret, v3 *normal_ret, uint64_t *id_ret) {
	START_TIME;
	// TODO: Just saying 50  ain't right, think of something better
	int collision_ids_max = 50;
	uint64_t collision_ids[collision_ids_max];
	int collision_ids_count = 0;
	bool ret = raycast_entity_collection_return_normal_and_ids(assets, col, ray_origin, ray_direction, tmin_ret, normal_ret, collision_ids, &collision_ids_count, collision_ids_max, id_ret);
	END_TIME;
	return ret; 
}


// creates collision points of the capsule with some entity_collection
void collide_capsule_entity_collection(GameAssets *assets, Entity_Collection *col, baka_capsule *capsule, v3 capsule_position, Quat capsule_rotation) {
	START_TIME;
	//capsule->num_contacts = 0;
	
	int collision_ids_max = 50;
	uint64_t collision_ids[collision_ids_max];
	int collision_ids_count = 0;
	
	baka_AABB capsule_aabb = aabb_from_capsule(*capsule, capsule_position, capsule_rotation);
	
	baka_aabb_find_contacts_tree(
		&col->tree, &capsule_aabb, collision_ids, &collision_ids_count, collision_ids_max);
	
	for (int i = 0; i < collision_ids_count; i++) {
		uint64_t col_id = collision_ids[i]; 
		
		Base_Entity b = col->els[col_id];
		baka_Body body = assets->prototypes.prototypes[b.prototype_index].body;
		
		v3 mid = b.transform.translation;
		Quat rot = b.transform.rotation;
		v3 scale = b.scale;
		
		for (int j = 0; j < body.num_shapes; j++) {
			baka_Shape next_object = assets->prototype_static_objects.els[body.shape_index + j];
			
			if(next_object.type_id == PHYS_OBB) {
				baka_OBB new_obb = baka_make_OBB(add_v3(rotate_vec3_quat(mul_v3(next_object.obb.mid, scale), rot), mid), mul_v3(next_object.obb.ex, scale), mul_quat(next_object.obb.rot, rot));
				
				baka_does_contact_capsule_obb(
					capsule,
					capsule_position,
					capsule_rotation,
					new_obb.mid,
					new_obb.ex,
					new_obb.rot);
			}
			
			if(next_object.type_id == PHYS_SPHERE) {
				baka_sphere new_sphere = baka_make_sphere(add_v3(rotate_vec3_quat( mul_v3f(next_object.sphere.center, scale.X), rot), mid), next_object.sphere.radius * scale.X);
				
				baka_does_contact_capsule_sphere(
					capsule,
					capsule_position,
					capsule_rotation,
					new_sphere.center,
					new_sphere.radius);
				
			}
			
			if(next_object.type_id == PHYS_TRIANGLE) {
				baka_triangle new_tri;
				new_tri.a = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.a, scale), rot), mid);
				new_tri.b = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.b, scale), rot), mid);
				new_tri.c = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.c, scale), rot), mid);
				
				baka_does_contact_capsule_triangle(
					capsule,
					capsule_position,
					capsule_rotation,
					new_tri);
				
			}
		}
	}
	END_TIME;
	return;
}


void new_character_resolution(baka_capsule *cap, v3 *cap_positions, Quat * cap_rotations,v3 * cap_displacement, int num_cap) {
	START_TIME;
	
	baka_aabb_binary_tree cap_tree;
	aabb_tree_node nds[num_cap*2];
	int fi[num_cap*2];
	cap_tree.nodes = nds;
	cap_tree.root_node_index = -1;
	cap_tree.max_object = num_cap*2;
	cap_tree.next_object = 0;
	cap_tree.free_indices = fi;
	cap_tree.next_free_index = 0;
	
	for (int i = 0; i < num_cap; i++) {
		baka_capsule * cap_i = &cap[i]; 
		v3 p1 = add_v3(rotate_vec3_quat(cap_i->p, cap_rotations[i]), cap_positions[i]);
		v3 q1 = add_v3(rotate_vec3_quat(cap_i->q, cap_rotations[i]), cap_positions[i]);
		
		baka_AABB i_aabb = aabb_from_capsule(*cap_i, cap_positions[i], cap_rotations[i]);
		
		int collision_ids_max = 50;
		uint64_t collision_ids[collision_ids_max];
		int collision_ids_count = 0;
		
		baka_aabb_find_contacts_tree(&cap_tree, &i_aabb, collision_ids, &collision_ids_count, collision_ids_max);
		
		int node_index_i = tree_insert_aabb(&cap_tree, i_aabb, i);
		
		for (int j = 0; j < collision_ids_count; j++) {
			int cap_j_idx = collision_ids[j];
			baka_capsule * cap_j = &cap[cap_j_idx];
			
			v3 p2 = add_v3(rotate_vec3_quat(cap_j->p, cap_rotations[cap_j_idx]), cap_positions[cap_j_idx]);
			v3 q2 = add_v3(rotate_vec3_quat(cap_j->q, cap_rotations[cap_j_idx]), cap_positions[cap_j_idx]);
			
			float s;
			float t;
			v3 c1;
			v3 c2;
			
			float ret_len = baka_closestpt_segment_segment(p1, q1, p2, q2, &s, &t, &c1, &c2);
			float combined_radius = cap_i->radius + cap_j->radius;
			
			v3 c1_to_c2 = sub_v3(c1, c2);
			v3 c2_to_c1 = sub_v3(c2, c1);
			
			float c1_to_c2_sqlen = dot_v3(c1_to_c2, c1_to_c2);
			float c1_to_c2_len = HMM_SquareRootF(c1_to_c2_sqlen);
			float c1_to_cap_j_len = c1_to_c2_len - cap_j->radius;
			float c2_to_cap_i_len = c1_to_c2_len - cap_i->radius;
			
			if (c1_to_cap_j_len > cap_i->radius) {
				continue;
			}
			
			if (c2_to_cap_i_len > cap_j->radius) {
				continue;
			}
			
			float penetration_depth_i = c1_to_cap_j_len - cap_i->radius;
			float penetration_depth_j = c2_to_cap_i_len - cap_j->radius;
			
			if (cap_i->num_dyn_contacts < MAX_CONTACTS) {
				
				cap_i->dyn_contacts[cap_i->num_dyn_contacts].point = add_v3(c1, mul_v3f(normalize_v3(c1_to_c2), cap_i->radius));
				cap_i->dyn_contacts[cap_i->num_dyn_contacts].normal = normalize_v3(c1_to_c2);
				cap_i->dyn_contacts[cap_i->num_dyn_contacts].depth = penetration_depth_i;
				cap_i->dyn_contacts[cap_i->num_dyn_contacts].broken = 0;
				cap_i->num_dyn_contacts++;
			}
			
			if (cap_j->num_dyn_contacts < MAX_CONTACTS) {
				// TODO: this can be optimized by using calculate results from cap_i
				cap_j->dyn_contacts[cap_j->num_dyn_contacts].point = add_v3(c2, mul_v3f(normalize_v3(c2_to_c1), cap_j->radius));
				cap_j->dyn_contacts[cap_j->num_dyn_contacts].normal = normalize_v3(c2_to_c1);
				cap_j->dyn_contacts[cap_j->num_dyn_contacts].depth = penetration_depth_j;
				cap_j->dyn_contacts[cap_j->num_dyn_contacts].broken = 0;
				cap_j->num_dyn_contacts++;
			}
		}
		
		/*
  for (int j = i+1; j < num_cap; j++) {
   baka_capsule * cap_j = &cap[j];
   
   v3 p2 = add_v3(rotate_vec3_quat(cap_j->p, cap_rotations[j]), cap_positions[j]);
   v3 q2 = add_v3(rotate_vec3_quat(cap_j->q, cap_rotations[j]), cap_positions[j]);
   
   float s;
   float t;
   v3 c1;
   v3 c2;
   
   float ret_len = baka_closestpt_segment_segment(p1, q1, p2, q2, &s, &t, &c1, &c2);
   float combined_radius = cap_i->radius + cap_j->radius;
   
   v3 c1_to_c2 = sub_v3(c1, c2);
   v3 c2_to_c1 = sub_v3(c2, c1);
   
   float c1_to_c2_sqlen = dot_v3(c1_to_c2, c1_to_c2);
   float c1_to_c2_len = HMM_SquareRootF(c1_to_c2_sqlen);
   float c1_to_cap_j_len = c1_to_c2_len - cap_j->radius;
   float c2_to_cap_i_len = c1_to_c2_len - cap_i->radius;
   
   if (c1_to_cap_j_len > cap_i->radius) {
 continue;
   }
   
   if (c2_to_cap_i_len > cap_j->radius) {
 continue;
   }
   
   float penetration_depth_i = c1_to_cap_j_len - cap_i->radius;
   float penetration_depth_j = c2_to_cap_i_len - cap_j->radius;
   
   if (cap_i->num_dyn_contacts < MAX_CONTACTS) {
   
 cap_i->dyn_contacts[cap_i->num_dyn_contacts].point = add_v3(c1, mul_v3f(normalize_v3(c1_to_c2), cap_i->radius));
 cap_i->dyn_contacts[cap_i->num_dyn_contacts].normal = normalize_v3(c1_to_c2);
 cap_i->dyn_contacts[cap_i->num_dyn_contacts].depth = penetration_depth_i;
 cap_i->dyn_contacts[cap_i->num_dyn_contacts].broken = 0;
 cap_i->num_dyn_contacts++;
   }
   
   if (cap_j->num_dyn_contacts < MAX_CONTACTS) {
 // TODO: this can be optimized by using calculate results from cap_i
 cap_j->dyn_contacts[cap_j->num_dyn_contacts].point = add_v3(c2, mul_v3f(normalize_v3(c2_to_c1), cap_j->radius));
 cap_j->dyn_contacts[cap_j->num_dyn_contacts].normal = normalize_v3(c2_to_c1);
 cap_j->dyn_contacts[cap_j->num_dyn_contacts].depth = penetration_depth_j;
 cap_j->dyn_contacts[cap_j->num_dyn_contacts].broken = 0;
 cap_j->num_dyn_contacts++;
   }
  }*/
	}
	END_TIME;
}


/*
// creates collision points of the capsule with the level
void collide_capsule(LevelStatic level_static, PrototypeStack prototypes, baka_Shape_Stack prototype_static_objects,
   baka_aabb_binary_tree *tree, baka_capsule *capsule, v3 capsule_position, Quat capsule_rotation) {
 START_TIME;
 //capsule->num_contacts = 0;
 
 int collision_ids_max = 50;
 uint64_t collision_ids[collision_ids_max];
 int collision_ids_count = 0;
 
 baka_AABB capsule_aabb = aabb_from_capsule(*capsule, capsule_position, capsule_rotation);
 
 baka_aabb_find_contacts_tree(
  tree, &capsule_aabb, collision_ids, &collision_ids_count, collision_ids_max);
  
 for (int i = 0; i < collision_ids_count; i++) {
  uint64_t col_id = collision_ids[i]; 
  
  StaticLevelEntity to_test = level_static.level_entities[col_id];
  baka_Body body = prototypes.prototypes[to_test.prototype_index].body;
  
  v3 mid = to_test.transform.translation;
  Quat rot = to_test.transform.rotation;
  v3 scale = to_test.scale;
  
  for (int j = 0; j < body.num_shapes; j++) {
   baka_Shape next_object = prototype_static_objects.els[body.shape_index + j];
   
   if(next_object.type_id == PHYS_OBB) {
 baka_OBB new_obb = baka_make_OBB(add_v3(rotate_vec3_quat(mul_v3(next_object.obb.mid, scale), rot), mid), mul_v3(next_object.obb.ex, scale), mul_quat(next_object.obb.rot, rot));
 
 baka_does_contact_capsule_obb(
  capsule,
  capsule_position,
  capsule_rotation,
  new_obb.mid,
  new_obb.ex,
  new_obb.rot);
   }
   
   if(next_object.type_id == PHYS_SPHERE) {
 baka_sphere new_sphere = baka_make_sphere(add_v3(rotate_vec3_quat( mul_v3f(next_object.sphere.center, scale.X), rot), mid), next_object.sphere.radius * scale.X);
 
 baka_does_contact_capsule_sphere(
  capsule,
  capsule_position,
  capsule_rotation,
  new_sphere.center,
  new_sphere.radius);
  
   }
   
   if(next_object.type_id == PHYS_TRIANGLE) {
 baka_triangle new_tri;
 new_tri.a = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.a, scale), rot), mid);
 new_tri.b = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.b, scale), rot), mid);
 new_tri.c = add_v3(rotate_vec3_quat( mul_v3(next_object.tri.c, scale), rot), mid);
 
 baka_does_contact_capsule_triangle(
  capsule,
  capsule_position,
  capsule_rotation,
  new_tri);
  
   }
  }
 }
 END_TIME;
 return;
}
*/

// show info about some genmodel such as its Nodes and corresponding transforms
// TODO: make this work again
void ui_show_genmodel(struct nk_context *ctx, GenModel *gm, NodeStack *ns, String_Stack *ss, GenMeshStack *gms) {
	/*
 if (nk_begin(ctx, "info", nk_rect(5, 160, 250, 300),
  NK_WINDOW_BORDER|NK_WINDOW_SCALABLE))
 {
  //nk_layout_row_dynamic(ctx, 20, 1);
  
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
   
   //nk_label(ctx, index_string, NK_TEXT_LEFT);
   //nk_label(ctx, node_name, NK_TEXT_LEFT);
   //nk_label(ctx, parent_string, NK_TEXT_LEFT);
   
   //nk_label(ctx, mesh_string, NK_TEXT_LEFT);
   if(ns->nodes[gm->node_idx + i].mesh != -1) {
 const char* mesh_name = (const char *)gms->names[gm->mesh_idx + ns->nodes[gm->node_idx + i].mesh].chars;
 
 //nk_label(ctx, mesh_name, NK_TEXT_LEFT);
   }
   
   
   //nk_label(ctx, skin_string, NK_TEXT_LEFT);
   //nk_label(ctx, texture_asset_string, NK_TEXT_LEFT);
   //nk_label(ctx, transf_transl_string, NK_TEXT_LEFT);
   //nk_label(ctx, object_transl_string, NK_TEXT_LEFT);
   
  }
 }
 nk_end(ctx);
 */
}


// Render our NodeHierachy,
// We might also consider just rendering the skeleton by looking at skins or sonething
// but it's not necessary right now
void render_skeleton(NodeStack *ns, GenModel *skeleton) {
	
	
	for(int i = 0; i < skeleton->num_nodes; i++) {
		int nidx = skeleton->node_idx + i;
		Node* current_node = &ns->nodes[nidx];
		
		v3 startpoint = current_node->object_transform.translation;
		Quat rotation = current_node->object_transform.rotation;
		
		v3 endpoint = vec3(0.0f, 1.0f, 0.0f);
		endpoint = rotate_vec3_quat(endpoint, rotation);
		endpoint = add_v3(endpoint, startpoint);
		
		mush_draw_sphere(
			dbg_list,
			startpoint,
			0.1f,
			vec4(0.0f, 0.0f, 1.0f, 0.5f));
		
		mush_draw_segment(
			dbg_list,
			startpoint,
			endpoint,
			vec4(1.0f, 0.0f, 0.0f, 1.0f),
			vec4(1.0f, 0.0f, 0.0f, 0.1f));
	} 
}
