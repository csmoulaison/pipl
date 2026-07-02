void print_pipeline_def(PipelineDefinition* pipeline) {
    printf("\nPIPELINE CREATED:\n");
    printf("pipeline id: %s\n", pipeline->id);
    printf("pipeline outputs len: %i\n", pipeline->outputs_len);
    for(i32 i = 0; i < pipeline->outputs_len; i++) {
        printf("    output: %s\n", pipeline->outputs[i]);
    }
    printf("pipeline input filename: %s\n", pipeline->input_filename);
    printf("pipeline command template: %s\n", pipeline->command_template);
}
