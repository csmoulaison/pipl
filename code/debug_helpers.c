void print_pipeline_def(PipelineDefinition* pipeline) {
    printf("\nPipeline\n");
    printf("    Id:          %s\n", pipeline->id);
    printf("    Outputs len: %i\n", pipeline->outputs_len);
    for(i32 i = 0; i < pipeline->outputs_len; i++) {
        printf("        %i: %s\n", i, pipeline->outputs[i]);
    }
    printf("    Input:       %s\n", pipeline->input_filename);
    printf("    Command:     %s\n", pipeline->command_template);
}

void print_asset_def(AssetDefinition* asset) {
    printf("\nAsset\n");
    printf("    Id:       %s\n", asset->id);
    printf("    Pipeline: %i\n", asset->pipeline_index);
    printf("    Args len: %i\n", asset->args_len);
    for(i32 i = 0; i < asset->args_len; i++) {
        printf("        %i: %s\n", i, asset->args[i]);
    }
}
