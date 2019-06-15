bool (*instantiate_genmodel)(NodeStack ns_src , GenModel gm_src , NodeStack * ns_dest , GenModel * gm_dest ) = NULL;
void (*set_node_texture_asset_by_name)(NodeStack * ns , GenModel * genmodel , const char * node_name , texture_asset tex ) = NULL;
struct texture_asset (*to_texture_asset)(unsigned int value ) = NULL;
void (*animate_model_test)(NodeStack * ns_src , GenModel gm_src , NodeStack * ns_result , GenModel gm_result , AnimStack * as , CombinedAnimationState c_animstate ) = NULL;
void (*solve_IK_fabrik)(NodeStack * ns , GenModel * model , uint target_node , uint chain_length , v3 target_pos_delta , uint pole_target , float mix ) = NULL;
GenModel (*get_model_from_crc32)(ModelMemory * mm , bool * ok , uint key ) = NULL;
void (*calculate_object_transforms)(NodeStack ns , GenModel * gma , Transform * world_transforms , uint num_models ) = NULL;
