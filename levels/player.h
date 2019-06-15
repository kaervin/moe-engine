void create_gondola(Game_Struct * game) {
	
	bool ok = false;
	game->ps.gondola_model = get_model_from_crc32(&game->assets.model_memory, &ok, crc32_cstring("gondola.moe"));
	
	if (!ok) {
		printf("not okay dude\n");
	}
	
	// get some textures and assign their identifiers
	// the way this works will be changed probably, this is just for now
	
	unsigned int texture_ident = map_lookup_key(&game->assets.tex_ass_man.map, &ok, crc32_cstring("Material.png"));
	ta_material = fp.to_texture_asset(texture_ident);
	printf("Material texture_ident %u %u %u %u\n", ok, texture_ident, ta_material.type, ta_material.nr);
	
	texture_ident = map_lookup_key(&game->assets.tex_ass_man.map, &ok, crc32_cstring("Face.png"));
	ta_face4 = fp.to_texture_asset(texture_ident);
	printf("face texture_ident %u %u %u %u\n", ok, texture_ident, ta_face4.type, ta_face4.nr);
	
	texture_ident = map_lookup_key(&game->assets.tex_ass_man.map, &ok, crc32_cstring("Angry.png"));
	ta_angry = fp.to_texture_asset(texture_ident);
	printf("face texture_ident %u %u %u %u\n", ok, texture_ident, ta_angry.type, ta_angry.
		   nr);
	
	bool succ = instantiate_genmodel(
		game->assets.model_memory.node_stack,
		game->ps.gondola_model,
		&game->ns_instances,
		&game->ps.instantiated_gondola);
	
	if (!succ) {
		printf("couldn't instantiate gondola\n");
	}
	
	set_node_texture_asset_by_name(&game->ns_instances, &game->ps.instantiated_gondola, "Plane", ta_face4);
	set_node_texture_asset_by_name(&game->ns_instances, &game->ps.instantiated_gondola, "Cube", ta_material);
	
	game->ps.gondola_transform.translation = vec3(5.0f, 0.0f, 0.0f);
	game->ps.gondola_transform.rotation = quaternion(0.0f, 0.0f, 0.0f, 1.0f);
	game->ps.gondola_scale = vec3(1.0f, 1.0f, 1.0f);
}

v3 move_player_capsule(PlayerState ps, v3 move_direction) {
	v3 old_move_direction = move_direction;
	
	// here we check how much the character can move in this direction by doing capsule collision stuff
	
	// now resolve the final movement under the constraints of the static contacts
	// this is important for making sure capsules can't be pushed inside geometry 
	
	// if the cap is moving away from the plane the contact is considered "broken"
	// and won't be solved for 
	for (int i = 0; i < ps.player_capsule.num_contacts; i++) {
		float p_len = dot_v3(ps.player_capsule.static_contacts[i].normal, move_direction);
		if (p_len >= 0.0f) {
			ps.player_capsule.static_contacts[i].broken = 1;
		}
	}
	
	// now iterativley resolve the movement to something that doesn't break the normal constraints
	for (int times = 0; times < 8; times++) {
		for (int i = 0; i < ps.player_capsule.num_contacts; i++) {
			
			baka_contact_static i_contact = ps.player_capsule.static_contacts[i];
			
			float p_len = dot_v3(i_contact.normal, move_direction);
			
			p_len = p_len * 1.0f;
			
			if ( ps.player_capsule.static_contacts[i].broken == 0 ) {
				v3 p_dir = mul_v3f(i_contact.normal, p_len);
				move_direction = sub_v3(move_direction, p_dir);
			}
		}
	}
	
	// maybe moving a bit in the direction that goes against the normal constraint is
	// good for stability, so move it by some factor into that direction
	v3 old_to_new_character_move_direction = sub_v3(old_move_direction, move_direction);
	return add_v3(move_direction, mul_v3f(old_to_new_character_move_direction, 0.15f)); // 0.15
}


v3 move_player_capsule_edit(PlayerState ps, v3 move_direction) {
	v3 old_move_direction = move_direction;
	
	// here we check how much the character can move in this direction by doing capsule collision stuff
	
	// now resolve the final movement under the constraints of the static contacts
	// this is important for making sure capsules can't be pushed inside geometry 
	
	// if the cap is moving away from the plane the contact is considered "broken"
	// and won't be solved for 
	for (int i = 0; i < ps.player_capsule.num_contacts; i++) {
		float p_len = dot_v3(ps.player_capsule.static_contacts[i].normal, move_direction);
		if (p_len >= 0.0f) {
			ps.player_capsule.static_contacts[i].broken = 1;
		}
	}
	
	// now iterativley resolve the movement to something that doesn't break the normal constraints
	for (int times = 0; times < 8; times++) {
		for (int i = 0; i < ps.player_capsule.num_contacts; i++) {
			
			baka_contact_static i_contact = ps.player_capsule.static_contacts[i];
			
			float p_len = dot_v3(i_contact.normal, move_direction);
			
			p_len = p_len * 1.0f;
			
			if ( ps.player_capsule.static_contacts[i].broken == 0 ) {
				v3 p_dir = mul_v3f(i_contact.normal, p_len);
				
				// here we hack the movement resolution a bit to have better behaviour for y movement of our character
				if (p_dir.y < 0.0f) {
					p_dir.y = p_dir.y/20.0f;
				}
				if (p_dir.y > 0.0f) {
					p_dir.y = p_dir.y/5.0f;
				}
				
				move_direction = sub_v3(move_direction, p_dir);
			}
		}
	}
	
	// maybe moving a bit in the direction that goes against the normal constraint is
	// good for stability, so move it by some factor into that direction
	v3 old_to_new_character_move_direction = sub_v3(old_move_direction, move_direction);
	v3 ret_move_direction = add_v3(move_direction, mul_v3f(old_to_new_character_move_direction, 0.15f)); // 0.15
	
	return ret_move_direction;
	
}


v3 baka_resolve_capsule_position_dynamic(
baka_capsule *cap,
v3 cap_pos)
{
	START_TIME;
	v3 displacement_direction = vec3(0.0f, 0.0f, 0.0f);
	
	for (int times = 0; times < 8; times++) {
		
	    for (int i = 0; i < cap->num_dyn_contacts; i++) {
			
	        // TODO: optimize this
	        // there might be 100s of things here we do twice
	        // also all the names are probably wrong, so calculate on a piece of paper again
			
	        baka_contact_static i_contact = cap->dyn_contacts[i];
			
	        v3 penetration_direction = mul_v3f(i_contact.normal, i_contact.depth);
			
	        v3 contact_point_on_capsule = sub_v3(i_contact.point, penetration_direction);
			
	        v3 current_contact_point = sub_v3(contact_point_on_capsule, displacement_direction);
			
	        float i_contact_plane_distance = dot_v3(i_contact.normal, i_contact.point);
			
	        float new_displacement_distance = DistPointPlane(
	            current_contact_point,
	            i_contact.normal,
	            i_contact_plane_distance) * 0.9f; // - 0.001f;
			
	        // maybe this is unnecessary
	        if (new_displacement_distance > 0.0f) {
	            displacement_direction = add_v3(
	                displacement_direction,
	                mul_v3f(i_contact.normal,new_displacement_distance));
	        } 
	    }
	}
	END_TIME;
    return displacement_direction;
}

void move_platform(Game_Struct *game) {
	GameAssets *ga = &game->assets;
	
	for (int i = 0; i < platform_collection.entities.num; i++) {
		Platform *p = &platform_collection.platforms[i];
		Base_Entity *e = &platform_collection.entities.els[i];
		
		v3 target = p->p1;
		if (p->direction) {
			target = p->p2;
		}
		
		v3 dir = (sub_v3(target, e->transform.translation));
		v3 dir_speed = mul_v3f(dir,0.01f);
		if (HMM_LengthVec3(dir_speed) >= 0.1) {
			dir_speed = mul_v3f(normalize_v3(dir_speed), 0.1f);
		}
		e->transform.translation = add_v3(e->transform.translation, dir_speed);
		p->vel = dir_speed;
		
		if (v3_about_equal(e->transform.translation, target, 0.1f)) {
			p->direction = !p->direction;
		}
		// TODO: when updating all platform entities, it will be better to just completely rebuild the tree after every entity has been udated
		update_base_entity(&game->assets, &platform_collection.entities, i);
	}
	
	for (int i = 0; i < platform_collection.entities.tree.next_object; i++) {
		baka_AABB aabb = platform_collection.entities.tree.nodes[i].aabb;
		mush_draw_cube(dbg_list, aabb.mid, aabb.ex, quaternion(0.0, 0.0, 0.0, 1.0), vec4(1.0, 1.0, 1.0, 1.0));
	}
	baka_AABB root_node_aabb = platform_collection.entities.tree.nodes[platform_collection.entities.tree.root_node_index].aabb;
	mush_draw_cube(dbg_list, root_node_aabb.mid, root_node_aabb.ex, quaternion(0.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 1.0, 1.0));
	
	
}

bool player_stands_on_platform(Game_Struct *game, v3 character_ray_origin, v3 character_ray_direction, float *tmin_ret, v3 *normal_ret, uint *pid) {
	uint64_t platform_id;
	bool ret = raycast_entity_collection_return_normal(&game->assets, &platform_collection.entities, character_ray_origin, character_ray_direction, tmin_ret, normal_ret, &platform_id);
	*pid = platform_id;
	
	return ret;
}

// the player_step is where the main functionality of the game would happen
// meaning player movement, logic, collision response and such

PlayerState player_step(Game_Struct *game, float dt) {//LevelStatic level_static, Terrain terra, GameAssets ga, baka_capsule *caps, v3 *cap_positions, Quat * cap_rotations, int num_cap, PlayerState ps, CameraState *cs, float dt, Key_Struct keys) {
	
	move_platform(game);
	
	float movement_speed = 4.0f * 10.0f * 0.016f / 4.0f;
	float run_speed_modifier = 1.4f;
	
	float jumping_speed = 1.2f * 75.0f * 1.2f * 1.1f;
	float base_gravity = 1.7 * 1.5f * 1.5f * 1.3f;
	float jump_gravity = 1.0f * 1.5 * 1.5f  * 1.3f;
	float float_gravity = 0.5f * 1.3 * 1.5f * 1.3f;
	
	PlayerState ps = game->ps;
	Terrain terra = game->terra;
	Key_Struct keys = game->keys;
	LevelStatic level_static = game->level_static;
	GameAssets *ga = &game->assets;
	CameraState *cs = &game->cs;
	NodeStack *ns_instances = &game->ns_instances;
	ModelMemory model_memory = game->assets.model_memory;
	baka_Shape_Stack prototype_static_objects = game->assets.prototype_static_objects;
	PrototypeStack prototypes = game->assets.prototypes;
	
	// figure out in which direction to move
	v3 camera_point_plane = vec3(cs->camera_point.X, 0.0f, cs->camera_point.Z);
	camera_point_plane = normalize_v3(camera_point_plane);
	
	v3 new_d_forward = mul_v3f(camera_point_plane, keys.l.y*(-1.0f));
	v3 new_d_right = mul_v3f(cs->camera_right, keys.l.x);
	
	v3 new_d_move = add_v3(new_d_forward, new_d_right);
	
	// for keyboard input we have to normalize the input controls, for joystick we don't (but we do it anyway) 
	if (HMM_LengthVec3(new_d_move) > 1.0f) {
		new_d_move = mul_v3f(normalize_v3(new_d_move), movement_speed);
	} else {
		new_d_move = mul_v3f(new_d_move, movement_speed);
	}
	
	if (keys.shift_key.down == 1) {
		new_d_move = mul_v3f(new_d_move, run_speed_modifier);
	}
	
	if (ps.is_jumping) {
		ps.character_control_direction = lerp_vec3(ps.character_control_direction, new_d_move, 0.014f);//0.010f);
	}
	else {
		ps.character_control_direction = lerp_vec3(ps.character_control_direction, new_d_move, 0.1f);
	}
	
	if (HMM_LengthVec3(ps.character_control_direction) > 0.05f) {
		ps.character_facing_direction = normalize_v3(ps.character_control_direction);
	}
	
	v3 last_move_direction = ps.character_move_direction;
	
	ps.character_move_direction = ps.character_control_direction;
	
	mush_draw_segment(
		dbg_list, ps.character_point, add_v3(ps.character_point, ps.hitnormal),Red4, Red4);
	
	// figure out the quaternion which is needed to rotate the movement so that it is parallel to the ground
	Quat move_quaternion_from_raynormal = quaternion_rotation_from_vectors(vec3(0.0f, 1.0f, 0.0f), ps.hitnormal);
	
	bool should_slide = false;
	if (dot_v3(ps.hitnormal, vec3(0.0f, 1.0f, 0.0f)) < 0.5f) {
		should_slide = true;
	}
	//ps.character_move_direction = rotate_vec3_quat(ps.character_move_direction, move_quaternion_from_raynormal);
	
	
	
	// ---   jumping logic   ---
	
	if (ps.on_ground) {
		ps.character_velocity.Y = 0;
		ps.character_move_direction.Y = 0.0f;
		ps.character_move_direction = rotate_vec3_quat(ps.character_move_direction, move_quaternion_from_raynormal);
		
		if (keys.r_key.initial) {
			ps.character_velocity.Y = jumping_speed;
			//ps.character_move_direction.Y = jumping_speed;
			ps.is_jumping = true;
		}
	}
	else {
		if (keys.r_key.down) {
			ps.character_velocity.Y = ps.character_velocity.Y - (float_gravity);
			//ps.character_move_direction.Y = last_move_direction.Y - (float_gravity);
		}
		else {
			ps.character_velocity.Y = ps.character_velocity.Y - (jump_gravity);
			//ps.character_move_direction.Y = last_move_direction.Y - (jump_gravity);
		}
		
		if (ps.character_move_direction.Y < 0.0f) {
			ps.character_velocity.Y = ps.character_velocity.Y - (base_gravity);
			//ps.character_move_direction.Y = last_move_direction.Y - (base_gravity);
		}
	}
	
	ps.character_move_direction.Y = ps.character_velocity.Y * dt;
	
	v3 character_move_direction_old = ps.character_move_direction;
	
	
	// ---    capsule vs player collision   ---
	/*
 v3 cap_displacement[num_cap];
 caps[0] = ps.player_capsule;
 //cap_positions[0] = add_v3(ps.character_point, ps.player_capsule_position_relative_to_player_point);
 cap_positions[0] = ps.character_point;
 cap_rotations[0] = quaternion(0.0,0.0,0.0,1.0);
 
 for (int i = 0; i < num_cap; i++) {
  caps[i].num_dyn_contacts = 0;
 }
 
 test_character_resolution(caps, cap_positions, cap_rotations, cap_displacement, num_cap);
 
 for (int i = 0; i < num_cap; i++) {
 
  cap_displacement[i] = baka_resolve_capsule_position_dynamic(&caps[i], cap_positions[i]);
  // draw the contacts
  for (int j = 0; j < caps[i].num_dyn_contacts; j++)
  {
   mush_draw_sphere(dbg_list, caps[i].dyn_contacts[j].point, 0.2f, Yellow4);
  }
 }
 
 for (int i = 1; i < num_cap; i++) {
  cap_displacement[i].y = 0.0f;
  cap_displacement[i] = mul_v3f(cap_displacement[i], 0.2f);
  // TODO: here we can also restrict the capsule movement, like we do with the player capsule
  cap_positions[i] = add_v3(cap_positions[i], cap_displacement[i]);
 }
 //cap_displacement[0].y = cap_displacement[0].y * 0.9f;
 cap_displacement[0] = mul_v3f(cap_displacement[0], 0.2f);//0.8f);
 
 if (cap_displacement[0].y > 0.0f) {
  //cap_displacement[0].y = HMM_MIN(cap_displacement[0].y, 0.1f);
  if (ps.character_move_direction.y < 0.0f) {
   ps.character_move_direction.y -= ps.character_move_direction.y * 0.04f;
  }
 }
 
 //ps.character_move_direction = add_v3(ps.character_move_direction, cap_displacement[0]);
 
 v3 disp_cps = move_player_capsule_edit(ps, cap_displacement[0]);
 v3 mv_cps = move_player_capsule_edit(ps, ps.character_move_direction);
 */
	//ps.character_move_direction = mv_cps;
	
	// here we update the position of the character and then find the appropriate point to "stand on" via a raycast
	v3 old_character_point = ps.character_point;
	
	//ps.character_point = add_v3(ps.character_point, disp_cps);
	
	ps.character_point = add_v3(ps.character_point, ps.character_move_direction);
	mush_draw_sphere(dbg_list, ps.character_point, 0.2f, vec4(1.0f, 1.0f, 1.0f, 1.0f));
	
	v3 character_ray_origin = add_v3(ps.character_point, ps.relative_character_ray_origin);
	mush_draw_sphere(dbg_list, character_ray_origin, 0.1f, Yellow4);
	
	mush_draw_segment(dbg_list, ps.character_point, add_v3(ps.character_point, mul_v3f(ps.character_move_direction, 30.0f)), Blue4, Yellow4);
	
	float animation_mix = HMM_MIN(1.0f, 5.0*HMM_LengthVec3(ps.character_control_direction));
	
	float tmin_ret;
	float minimum_tmin;
	
	v3 tempnormal;
	
	float depth_at_test_point = terra_depth_normal_at_point(terra, character_ray_origin.X, character_ray_origin.Z, &ps.hitnormal);
	//mush_draw_sphere(dbg_list, vec3(character_ray_origin.X, depth_at_test_point, character_ray_origin.Z), 0.1f, vec4(1.0f, 0.0f, 0.0f, 0.3f));
	
	minimum_tmin = INFINITY;
	
	float tmin_terrain = character_ray_origin.Y - depth_at_test_point;
	
	if (tmin_terrain < minimum_tmin) {
		minimum_tmin = tmin_terrain;
	}
	
	uint64_t hit_id;
	
	bool didhit = raycast_entity_collection_return_normal(ga, &level_static.entities, character_ray_origin, ps.character_ray_direction, &tmin_ret, &tempnormal, &hit_id);
	
	if (didhit && tmin_ret < minimum_tmin) {
		minimum_tmin = tmin_ret;
		ps.hitnormal = tempnormal;
	}
	
	uint platform_id;
	
	bool didhit_platform = player_stands_on_platform(game, character_ray_origin, ps.character_ray_direction, &tmin_ret, &tempnormal, &platform_id);
	
	ps.on_platform = false;
	if (didhit_platform && tmin_ret < minimum_tmin) {
		minimum_tmin = tmin_ret;
		ps.hitnormal = tempnormal;
		ps.on_platform = true;
	} else {
		ps.character_platform_velocity = mul_v3f(ps.character_platform_velocity, (1-(1*dt)));
	}
	
	ps.shadow_point = add_v3(character_ray_origin, mul_v3f(ps.character_ray_direction, minimum_tmin));
	
	// the "stickyness" keeps us on the ground in situations where its the right thing to do
	float stickyness_constant = 0.6f;
	if (ps.is_jumping) {
		stickyness_constant = 0.0f;
		
		if (ps.character_move_direction.Y > 0.0f) {
			stickyness_constant = -3.0f;
		}
	}
	
	
	if ( minimum_tmin <= ps.relative_character_ray_origin.Y + stickyness_constant) {
		
		if (ps.on_platform) {
			ps.character_platform_velocity = platform_collection.platforms[platform_id].vel;
		}
		
		v3 draw_raycast_hitpoint = add_v3(character_ray_origin, mul_v3f(ps.character_ray_direction, minimum_tmin));
		
		mush_draw_sphere(
			dbg_list,
			draw_raycast_hitpoint,
			0.1f,
			Blue4);
		
		ps.character_point = add_v3(character_ray_origin, mul_v3f(ps.character_ray_direction, minimum_tmin));
		
		ps.character_move_direction.Y = 0.0f;
		ps.on_ground = 1;
		ps.is_jumping = false;
		
		if (!ps.on_platform) {
			ps.character_platform_velocity = vec3(0.0f, 0.0f, 0.0f);
		}
	} else {
		
		mush_draw_sphere(
			dbg_list,
			add_v3(character_ray_origin, mul_v3f(ps.character_ray_direction, minimum_tmin)),
			0.3f,
			Red4);
		
		ps.on_ground = 0;
	}
	
	ps.character_point = add_v3(ps.character_point, ps.character_platform_velocity);
	
	
	
	// now get new contacts and resolve the final position to not be inside some collision object
	v3 capsule_world_position = add_v3(ps.character_point, ps.player_capsule_position_relative_to_player_point);
	
	
	ps.player_capsule.num_contacts = 0;
	collide_capsule_entity_collection(&game->assets, &platform_collection.entities, &ps.player_capsule, capsule_world_position, ps.player_capsule_rotation);
	collide_capsule_entity_collection(ga, &level_static.entities, &ps.player_capsule, capsule_world_position, ps.player_capsule_rotation);
	
	
	// draw the contacts
	for (int i = 0; i < ps.player_capsule.num_contacts; ++i)
	{
		mush_draw_sphere(dbg_list, ps.player_capsule.static_contacts[i].point, 0.2f, Red4);
	}
	
	v3 displacement_direction = baka_resolve_capsule_position(&ps.player_capsule, capsule_world_position);
	
	// NOTE: not having displacement in the negative direction could be good for stability reasons, but TODO: this needs some more testing and appropriate test cases 
	/*
if (displacement_direction.y < 0.0f) {
displacement_direction.y = 0.0f;
}
*/
	// if we collide with something upwards, then reset vertical velocity under the condition that the causing normal is pointing downwards enough
	if (displacement_direction.y < 0.0f && !ps.on_ground) {
		if (ps.character_velocity.y > 0.0f) {
			for (int i = 0; i < ps.player_capsule.num_contacts; ++i)
			{
				float nrm = dot_v3(ps.player_capsule.static_contacts[i].normal, vec3(0.0f, -1.0f, 0.0f));
				ps.character_velocity.y = ps.character_velocity.y - 2.0f*nrm;
			}
		}
	}
	
	ps.character_point = add_v3(ps.character_point, displacement_direction);
	
	
	// draw the capsule (more or less)
	mush_draw_sphere(
		dbg_list,
		add_v3(capsule_world_position, rotate_vec3_quat(ps.player_capsule.p, ps.player_capsule_rotation)),
		ps.player_capsule.radius,
		vec4(1.0f,1.0,1.0f,0.2f));
	
	mush_draw_sphere(
		dbg_list,
		add_v3(capsule_world_position, rotate_vec3_quat(ps.player_capsule.q, ps.player_capsule_rotation)),
		ps.player_capsule.radius,
		vec4(1.0f,1.0,1.0f,0.2f));
	
	// the point of the displayed model is lerped, so that it will move more smoothly
	
	// split character model point into vertical and horizontal movement
	v3 character_model_point_vert = vec3(0.0f, ps.character_model_point.Y, 0.0f);
	v3 character_point_vert = vec3(0.0f, ps.character_point.Y, 0.0f);
	
	v3 character_model_point_horiz = vec3(ps.character_model_point.X, 0.0f, ps.character_model_point.Z);
	v3 character_point_horiz = vec3(ps.character_point.X, 0.0f, ps.character_point.Z);
	
	character_model_point_horiz = lerp_vec3(character_model_point_horiz, character_point_horiz, 0.6f);
	
	// if the character model point gets adjusted downwards
	if ( (character_model_point_vert.Y - character_point_vert.Y) > 0.0f || ps.is_jumping || ps.on_platform ) {
		//character_model_point_vert = lerp_vec3(character_model_point_vert, character_point_vert, 1.0f);
		character_model_point_vert = character_point_vert;
	} else {
		character_model_point_vert = lerp_vec3(character_model_point_vert, character_point_vert, 0.15f);
	}
	
	ps.character_model_point = vec3(character_model_point_horiz.X, character_model_point_vert.Y, character_model_point_horiz.Z);
	
	//ps.character_model_point = lerp_vec3(ps.character_model_point, ps.character_point, 0.15f);
	
	ps.gondola_transform.translation = ps.character_model_point;
	
	ps.gondola_transform.rotation = quaternion_rotation_from_vectors(vec3(0.0f, 0.0f, 1.0f), ps.character_facing_direction);
	
	
	if (keys.r_key.down) {
		set_node_texture_asset_by_name(ns_instances, &ps.instantiated_gondola, "Plane", ta_angry);
	} else {
		set_node_texture_asset_by_name(ns_instances, &ps.instantiated_gondola, "Plane", ta_face4);
	}
	
	// adjust camera
	
	// first rotate around y axis
	Quat quat_right = quaternionFromAxisAngle(vec3(0.0f,1.0f,0.0f), keys.r.x*-0.01);
	
	cs->camera_point = rotate_vec3_quat(cs->camera_point, quat_right);
	cs->camera_right = rotate_vec3_quat(cs->camera_right, quat_right);
	
	// then rotate around camera right
	Quat quat_up = quaternionFromAxisAngle(cs->camera_right, keys.r.y*-0.01);
	
	cs->camera_point = rotate_vec3_quat(cs->camera_point, quat_up);
	
	// animate our player character
	
	float currentTime = ps.previous_time + dt;
	
	if (!ps.is_jumping) {
		ps.animtime = fmod(ps.animtime + dt*1.7f, ps.ar_gondola.as.anims[1].last_time);
	}
	else {
		ps.animtime = fmod(ps.animtime + dt*1.0f, ps.ar_gondola.as.anims[1].last_time);
	}
	
	ps.previous_time = ps.previous_time + dt;
	
	CombinedAnimationState c_animstate;
	for (int j = 0; j < 4; j++) {
		c_animstate.animation_states[j].anim_index = 0;
		c_animstate.animation_states[j].time = ps.animtime;
	}
	
	c_animstate.weight = vec4(animation_mix, 1-animation_mix, 0.0f, 0.0f);
	c_animstate.animation_states[0].anim_index = 1;
	c_animstate.animation_states[1].anim_index = 2;
	
	animate_model_test(
		&model_memory.node_stack,
		ps.gondola_model,
		ns_instances,
		ps.instantiated_gondola,
		&ps.ar_gondola.as,
		c_animstate);
	
	// calculate_object_transforms calculates the node-local transforms according to the hierachy
	calculate_object_transforms(
		*ns_instances,
		&ps.instantiated_gondola,
		&ps.gondola_transform,
		1);
	
	// FOOT IK
	
	float tmin_left_foot;
	v3 normal_left_foot;
	v3 left_foot_pos = ns_instances->nodes[ps.instantiated_gondola.node_idx + 7].object_transform.translation;
	left_foot_pos = add_v3(left_foot_pos, vec3(0.0f, -0.3f, 0.0f));
	v3 left_foot_ray_origin = left_foot_pos;
	left_foot_ray_origin.Y = left_foot_pos.Y+4.0f;
	v3 left_foot_delta = sub_v3(left_foot_pos, left_foot_ray_origin);
	
	didhit =  raycast_entity_collection_return_normal(ga, &level_static.entities, left_foot_ray_origin, left_foot_delta, &tmin_left_foot, &normal_left_foot, &hit_id);
	
	if (didhit && tmin_left_foot <= 1.0f) {
		ps.player_left_foot_fabrik_mix = lerp(ps.player_left_foot_fabrik_mix, 0.1f, 1.0f);
		v3 adjusted_foot_delta = add_v3(left_foot_ray_origin, mul_v3f(left_foot_delta, tmin_left_foot));
		adjusted_foot_delta = sub_v3(adjusted_foot_delta ,left_foot_pos);
		adjusted_foot_delta = add_v3(adjusted_foot_delta, vec3(0.0f, 0.0f, 0.0f));
		adjusted_foot_delta.Y = HMM_MIN(adjusted_foot_delta.Y, 2.0f);
		ps.last_adjusted_foot_delta_left = adjusted_foot_delta;
		
		game->fp_struct.solve_IK_fabrik(ns_instances, &ps.instantiated_gondola, 7, 3, adjusted_foot_delta, 3, ps.player_left_foot_fabrik_mix);
		
		mush_draw_segment(dbg_list, left_foot_ray_origin, add_v3(left_foot_ray_origin, mul_v3f(left_foot_delta, tmin_left_foot)),Yellow4, Yellow4);
		
		mush_draw_sphere(dbg_list, left_foot_pos, 0.12f, Red4);
		
	} else {
		ps.player_left_foot_fabrik_mix = lerp(ps.player_left_foot_fabrik_mix, 0.1f, 0.0f);
		
		solve_IK_fabrik(ns_instances, &ps.instantiated_gondola, 7, 3, ps.last_adjusted_foot_delta_left, 3, ps.player_left_foot_fabrik_mix);
		
	}
	
	float tmin_right_foot;
	v3 normal_right_foot;
	v3 right_foot_pos = ns_instances->nodes[ps.instantiated_gondola.node_idx + 10].object_transform.translation;
	right_foot_pos = add_v3(right_foot_pos, vec3(0.0f, -0.3f, 0.0f));
	v3 right_foot_ray_origin = right_foot_pos;
	right_foot_ray_origin.Y = right_foot_pos.Y+4.0f;
	v3 right_foot_delta = sub_v3(right_foot_pos, right_foot_ray_origin);
	
	didhit = raycast_entity_collection_return_normal(ga, &level_static.entities, right_foot_ray_origin, right_foot_delta, &tmin_right_foot, &normal_right_foot, &hit_id);
	if (didhit && tmin_right_foot <= 1.0f) {
		ps.player_right_foot_fabrik_mix = lerp(ps.player_right_foot_fabrik_mix, 0.1f, 1.0f);
		v3 adjusted_foot_delta = add_v3(right_foot_ray_origin, mul_v3f(right_foot_delta, tmin_right_foot));
		adjusted_foot_delta = sub_v3(adjusted_foot_delta ,right_foot_pos);
		adjusted_foot_delta = add_v3(adjusted_foot_delta, vec3(0.0f, 0.0f, 0.0f));
		adjusted_foot_delta.Y = HMM_MIN(adjusted_foot_delta.Y, 2.0f);
		ps.last_adjusted_foot_delta_right = adjusted_foot_delta;
		
		game->fp_struct.solve_IK_fabrik(ns_instances, &ps.instantiated_gondola, 10, 3, adjusted_foot_delta, 13, ps.player_right_foot_fabrik_mix);
		
		mush_draw_segment(dbg_list, right_foot_ray_origin, add_v3(right_foot_ray_origin, mul_v3f(right_foot_delta, tmin_right_foot)),Yellow4, Yellow4);
	} else {
		ps.player_right_foot_fabrik_mix = lerp(ps.player_right_foot_fabrik_mix, 0.1f, 0.0f);
		
		solve_IK_fabrik(ns_instances, &ps.instantiated_gondola, 10, 3, ps.last_adjusted_foot_delta_right, 13, ps.player_right_foot_fabrik_mix);
		
	}
	
	if (!ps.on_ground) {
		
		solve_IK_fabrik(ns_instances, &ps.instantiated_gondola, 7, 3, vec3(0.0f, 0.4f, 0.0f), 3, 1.0f);
		
		solve_IK_fabrik(ns_instances, &ps.instantiated_gondola, 10, 3, vec3(0.0f, 0.6f, 0.0f), 13, 1.0f);
		
	}
	
	return ps;
}


void enemy_collection_step(Game_Struct *game, float dt) {
	START_TIME;
	//move_platform(game);
	
	float movement_speed = 4.0f * 10.0f * dt;
	float run_speed_modifier = 1.4f;
	
	float jumping_speed = 1.2f * 75.0f * dt;
	float base_gravity = 1.7 * 1.5f;
	float jump_gravity = 1.0f * 1.5;
	float float_gravity = 0.5f * 1.3;
	
	Terrain terra = game->terra;
	Key_Struct keys = game->keys;
	LevelStatic level_static = game->level_static;
	GameAssets *ga = &game->assets;
	CameraState *cs = &game->cs;
	NodeStack *ns_instances = &game->ns_instances;
	ModelMemory model_memory = game->assets.model_memory;
	baka_Shape_Stack prototype_static_objects = game->assets.prototype_static_objects;
	PrototypeStack prototypes = game->assets.prototypes;
	
	for (int i = 0; i < enemy_collection.entities.num; i++) {
		PlayerState *ps = &enemy_collection.enemies[i];
		Base_Entity *b_ent = &enemy_collection.entities.els[i];
		
		v3 new_d_move = vec3(0,0,0);
		
		if (ps->is_jumping) {
			ps->character_control_direction = lerp_vec3(ps->character_control_direction, new_d_move, 0.007f);
		}
		else {
			ps->character_control_direction = lerp_vec3(ps->character_control_direction, new_d_move, 0.1f);
		}
		if (HMM_LengthVec3(ps->character_control_direction) > 0.05f) {
			ps->character_facing_direction = normalize_v3(ps->character_control_direction);
		}
		v3 last_move_direction = ps->character_move_direction;
		ps->character_move_direction = ps->character_control_direction;
		
		Quat move_quaternion_from_raynormal = quaternion_rotation_from_vectors(vec3(0.0f, 1.0f, 0.0f), ps->hitnormal);
		
		bool should_slide = false;
		if (dot_v3(ps->hitnormal, vec3(0.0f, 1.0f, 0.0f)) < 0.5f) {
			should_slide = true;
		}
		
		if (ps->on_ground) {
			ps->character_move_direction.Y = 0.0f;
			ps->character_move_direction = rotate_vec3_quat(ps->character_move_direction, move_quaternion_from_raynormal);
			
			if (0) {
				ps->character_move_direction.Y = jumping_speed;
				ps->is_jumping = true;
			}
		}
		else {
			if (0) {
				ps->character_move_direction.Y = last_move_direction.Y - (float_gravity*dt);
			}
			else {
				ps->character_move_direction.Y = last_move_direction.Y - (jump_gravity*dt);
			}
			
			if (ps->character_move_direction.Y < 0.0f) {
				ps->character_move_direction.Y = last_move_direction.Y - (base_gravity*dt);
			}
		}
		v3 character_move_direction_old = ps->character_move_direction;
		
		// here we update the position of the character and then find the appropriate point to "stand on" via a raycast
		v3 old_character_point = ps->character_point;
		
		ps->character_point = add_v3(ps->character_point, ps->character_move_direction);
		v3 character_ray_origin = add_v3(ps->character_point, ps->relative_character_ray_origin);
		
		float animation_mix = HMM_MIN(1.0f, 5.0*HMM_LengthVec3(ps->character_control_direction));
		
		float tmin_ret;
		float minimum_tmin;
		v3 tempnormal;
		
		float depth_at_test_point = terra_depth_normal_at_point(terra, character_ray_origin.X, character_ray_origin.Z, &ps->hitnormal);
		
		
		minimum_tmin = INFINITY;
		float tmin_terrain = character_ray_origin.Y - depth_at_test_point;
		
		if (tmin_terrain < minimum_tmin) {
			minimum_tmin = tmin_terrain;
		}
		
		uint64_t hit_id;
		bool didhit = raycast_entity_collection_return_normal(ga, &level_static.entities, character_ray_origin, ps->character_ray_direction, &tmin_ret, &tempnormal, &hit_id);
		if (didhit && tmin_ret < minimum_tmin) {
			minimum_tmin = tmin_ret;
			ps->hitnormal = tempnormal;
		}
		
		uint platform_id;
		bool didhit_platform = player_stands_on_platform(game, character_ray_origin, ps->character_ray_direction, &tmin_ret, &tempnormal, &platform_id);
		
		ps->on_platform = false;
		if (didhit_platform && tmin_ret < minimum_tmin) {
			minimum_tmin = tmin_ret;
			ps->hitnormal = tempnormal;
			ps->on_platform = true;
		} else {
			ps->character_platform_velocity = mul_v3f(ps->character_platform_velocity, (1-(1*dt)));
		}
		ps->shadow_point = add_v3(character_ray_origin, mul_v3f(ps->character_ray_direction, minimum_tmin));
		
		// the "stickyness" keeps us on the ground in situations where its the right thing to do
		float stickyness_constant = 0.6f;
		if (ps->is_jumping) {
			stickyness_constant = 0.0f;
			
			if (ps->character_move_direction.Y > 0.0f) {
				stickyness_constant = -3.0f;
			}
		}
		
		if ( minimum_tmin <= ps->relative_character_ray_origin.Y + stickyness_constant) {
			
			if (ps->on_platform) {
				ps->character_platform_velocity = platform_collection.platforms[platform_id].vel;
			}
			
			v3 draw_raycast_hitpoint = add_v3(character_ray_origin, mul_v3f(ps->character_ray_direction, minimum_tmin));
			
			ps->character_point = add_v3(character_ray_origin, mul_v3f(ps->character_ray_direction, minimum_tmin));
			
			ps->character_move_direction.Y = 0.0f;
			ps->on_ground = 1;
			ps->is_jumping = false;
			
			if (!ps->on_platform) {
				ps->character_platform_velocity = vec3(0.0f, 0.0f, 0.0f);
			}
		} else {
			ps->on_ground = 0;
		}
		ps->character_point = add_v3(ps->character_point, ps->character_platform_velocity);
		
		v3 capsule_world_position = add_v3(ps->character_point, ps->player_capsule_position_relative_to_player_point);
		
		ps->player_capsule.num_contacts = 0;
		collide_capsule_entity_collection(&game->assets, &platform_collection.entities, &ps->player_capsule, capsule_world_position, ps->player_capsule_rotation);
		collide_capsule_entity_collection(ga, &level_static.entities, &ps->player_capsule, capsule_world_position, ps->player_capsule_rotation);
		
		v3 displacement_direction = baka_resolve_capsule_position(&ps->player_capsule, capsule_world_position);
		ps->character_point = add_v3(ps->character_point, displacement_direction);
		
		v3 character_model_point_vert = vec3(0.0f, ps->character_model_point.Y, 0.0f);
		v3 character_point_vert = vec3(0.0f, ps->character_point.Y, 0.0f);
		
		v3 character_model_point_horiz = vec3(ps->character_model_point.X, 0.0f, ps->character_model_point.Z);
		v3 character_point_horiz = vec3(ps->character_point.X, 0.0f, ps->character_point.Z);
		
		character_model_point_horiz = lerp_vec3(character_model_point_horiz, character_point_horiz, 0.6f);
		
		// if the character model point gets adjusted downwards
		if ( (character_model_point_vert.Y - character_point_vert.Y) > 0.0f || ps->is_jumping || ps->on_platform ) {
			character_model_point_vert = character_point_vert;
		} else {
			character_model_point_vert = lerp_vec3(character_model_point_vert, character_point_vert, 0.15f);
		}
		
		ps->character_model_point = vec3(character_model_point_horiz.X, character_model_point_vert.Y, character_model_point_horiz.Z);
		
		ps->gondola_transform.translation = ps->character_model_point;
		ps->gondola_transform.rotation = quaternion_rotation_from_vectors(vec3(0.0f, 0.0f, 1.0f), ps->character_facing_direction);
		
		
		float currentTime = ps->previous_time + dt;
		if (!ps->is_jumping) {
			ps->animtime = fmod(ps->animtime + dt*1.7f, ps->ar_gondola.as.anims[1].last_time);
		}
		else {
			ps->animtime = fmod(ps->animtime + dt*1.0f, ps->ar_gondola.as.anims[1].last_time);
		}
		ps->previous_time = currentTime;
		
		CombinedAnimationState c_animstate;
		for (int j = 0; j < 4; j++) {
			c_animstate.animation_states[j].anim_index = 0;
			c_animstate.animation_states[j].time = ps->animtime;
		}
		
		c_animstate.weight = vec4(animation_mix, 1-animation_mix, 0.0f, 0.0f);
		c_animstate.animation_states[0].anim_index = 1;
		c_animstate.animation_states[1].anim_index = 2;
		
		animate_model_test(&model_memory.node_stack, game->ps.gondola_model, ns_instances, b_ent->model, &ps->ar_gondola.as, c_animstate);
		
		b_ent->transform = ps->gondola_transform;
		// calculate_object_transforms calculates the node-local transforms according to the hierachy
		//calculate_object_transforms(*ns_instances, &b_ent->model, &ps->gondola_transform, 1);
	}
	END_TIME;
}


