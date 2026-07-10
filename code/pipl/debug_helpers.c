void print_definition(Definition* definition) {
    printf("\nDefinition\n");
    printf("    Id:          %s\n", definition->id);
    printf("    Outputs len: %i\n", definition->outputs_len);
    for(i32 i = 0; i < definition->outputs_len; i++) {
        printf("        %i: %s\n", i, definition->outputs[i]);
    }
    printf("    Input:       %s\n", definition->input_path);
    printf("    Command:     %s\n", definition->command_template);
}

void print_instance(Instance* instance) {
    printf("\nAsset\n");
    printf("    Id:       %s\n", instance->id);
    printf("    Definition: %i\n", instance->definition_index);
    printf("    Args len: %i\n", instance->args_len);
    for(i32 i = 0; i < instance->args_len; i++) {
        printf("        %i: %s\n", i, instance->args[i]);
    }
}
