void fill_function_pointers() {
fp_struct.instantiate_genmodel = instantiate_genmodel;
fp_struct.set_node_texture_asset_by_name = set_node_texture_asset_by_name;
fp_struct.to_texture_asset = to_texture_asset;
fp_struct.animate_model_test = animate_model_test;
fp_struct.solve_IK_fabrik = solve_IK_fabrik;
fp_struct.get_model_from_crc32 = get_model_from_crc32;
fp_struct.calculate_object_transforms = calculate_object_transforms;
}