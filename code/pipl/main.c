// NOW: working on changing "asset" concept to "instance". assets ought to refer
//      to the finished output file, which is not 1:1 with instances.
      
// NOW: some disparate thoughts I was having:
//      - pipl is first and foremost a data format. this code is an interpreter
//        (and probably eventually library for processing) that format.
//      - there should probably be multiple inputs files supported.
//      - outputs should omit the ., as the . is only applicable to the filename
//        aspect of outputs, and there are other relevant aspects, such as what
//        the eventual C #define will be labeled.
//      - we should probably not make this a multi-pass situation like we have
//        now, with files parsed and THEN processed. Doing it all as a live
//        interpreter allows us to easily support dependencies between process
//        steps and makes the asset generation process a *little bit*
//        imperative, which is GOOD.

#define CSM_IMPLEMENTATION
#define CSM_SKIP_MATH
#include "csm_core/core.h"

#define DEBUG_PRINT_PARSE_OUTPUT 1

#define MAX_INPUT_FILES 64
#define MAX_PATH_LENGTH 4096
#define MAX_DEFINITIONS 256
#define MAX_INSTANCES 1024
#define MAX_INSTANCE_ID_LENGTH 256
#define MAX_ASSETS 1024
#define MAX_ARG_LENGTH 4096
#define MAX_LINE_LENGTH 4096
#define MAX_TOKEN_LENGTH 256
#define MAX_DEFINITION_OUTPUTS 8
#define MAX_INSTANCE_ARGS 16
#define OUTPUT_BUFFER_SIZE 100000000

#define LINE_END(li) (line[li] == '\n' || line[li] == '\0' || line[li] == EOF || line[li] == '#')
#define match(str1, str2) (strcmp(str1, str2) == 0)
#define copy(str1, str2) (strcpy(str1, str2))
#define expect(str, msg) { if(!consume() || !match(token, str)) \
    { fprintf(stderr, "Expected %s %s\n", str, msg); return false; }}
#define expect_any(msg) { if(!consume()) \
    { fprintf(stderr, "Expected %s\n", msg); return false; }}

typedef struct {
    char id[MAX_TOKEN_LENGTH];
    char outputs[MAX_DEFINITION_OUTPUTS][MAX_TOKEN_LENGTH];
    i32 outputs_len;
    char input_path[MAX_PATH_LENGTH];
    char command_template[MAX_ARG_LENGTH];
} Definition;

typedef struct {
    i32 definition_index;
    char id[MAX_TOKEN_LENGTH];
    char args[MAX_INSTANCE_ARGS][MAX_ARG_LENGTH];
    i32 args_len;
} Instance;

typedef struct {
    u64 buffer_index;
    i32 definition_index;
    i32 instance_index;
    i32 output_index;
} Asset;

// Configuration
char input_paths[MAX_INPUT_FILES][MAX_PATH_LENGTH];
char output_path[MAX_PATH_LENGTH] = "./pack.data";
bool use_include_path = false;
char include_path[MAX_PATH_LENGTH];
bool pack_clean = false;
// Parse state
char line[MAX_LINE_LENGTH];
char token[MAX_TOKEN_LENGTH];
i32 linei = 0;
i32 tokeni = 0;
char line_definition_id[MAX_TOKEN_LENGTH];
// Definitions
Definition definitions[MAX_DEFINITIONS];
i32 definitions_len = 0;
Instance instances[MAX_INSTANCES];
i32 instances_len = 0;
Asset assets[MAX_ASSETS];
i32 assets_len = 0;

u64 pack_last_modified = 0;
char pack_buffer[OUTPUT_BUFFER_SIZE];
u64 pack_buffer_index = 0;

#include "debug_helpers.c"

// Stores the next token if it exists, returning true if so and false if not.
bool consume() {
    if(LINE_END(linei)) {
        return false;
    }
    while(line[linei] == ' ') {
        linei++;
        if(LINE_END(linei)) {
            return false;
        }
    }
    tokeni = 0;
    while(!LINE_END(linei) && line[linei] != ' ') {
        token[tokeni] = line[linei];
        tokeni++;
        linei++;
    }
    token[tokeni] = '\0';
    return true;
}

void expand_template(Instance* instance, char* template, char* out) {
    Definition* definition = &definitions[instance->definition_index];
    i32 template_i = 0;
    i32 out_i = 0;
    while(template[template_i] != '\0') {
        if(template[template_i] == '!') {
            char* expand_str = NULL;
            switch(template[template_i + 1]) {
                case '$': 
                    expand_str = instance->id; 
                    break;
                case '0': 
                    assert(instance->args_len > 0); 
                    expand_str = instance->args[0]; 
                    break;
                case '1': 
                    assert(instance->args_len > 1); 
                    expand_str = instance->args[1]; 
                    break;
                case '2': 
                    assert(instance->args_len > 2); 
                    expand_str = instance->args[2]; 
                    break;
                case '3': 
                    assert(instance->args_len > 3); 
                    expand_str = instance->args[3]; 
                    break;
                case '4': 
                    assert(instance->args_len > 4); 
                    expand_str = instance->args[4]; 
                    break;
                case '5': 
                    assert(instance->args_len > 5); 
                    expand_str = instance->args[5]; 
                    break;
                case '6': 
                    assert(instance->args_len > 6); 
                    expand_str = instance->args[6]; 
                    break;
                case '7': 
                    assert(instance->args_len > 7); 
                    expand_str = instance->args[7]; 
                    break;
                case '8': 
                    assert(instance->args_len > 8); 
                    expand_str = instance->args[8]; 
                    break;
                case '9': 
                    assert(instance->args_len > 9); 
                    expand_str = instance->args[9]; 
                    break;
                default: {
                    fprintf(stderr, "Unexpected symbol following '_' in template expansion.\n");
                    exit(1);
                }
            }
            assert(expand_str != NULL);
            copy(&(out[out_i]), expand_str);
            out_i += strlen(expand_str);
            template_i += 2;
        } else {
            out[out_i] = template[template_i];
            template_i++;
            out_i++;
        }
    }
}

bool interpret_pipl(FILE* pipl, char* path) {
    while(fgets(line, MAX_LINE_LENGTH, pipl)) {
        linei = 0;
        if(consume()) { 
            if(match(token, "include")) {
                expect_any("filename following include directive");
                FILE* include = fopen(token, "r");
                if(include == NULL) {
                    fprintf(stderr, "Could not open file '%s' included in '%s'.\n", token, path);
                    return false;
                }
                if(interpret_pipl(include, token) == false) {
                    return false;
                }
                continue;
            }

            copy(line_definition_id, token);
            i32 existing_definition_index = -1;
            for(i32 i = 0; i < definitions_len; i++) {
                if(match(line_definition_id, definitions[i].id)) {
                    existing_definition_index = i;
                    break;
                }
            }
            assert(consume());
            if(match(token, "=")) {
                // Parse a definition
                if(existing_definition_index != -1) {
                    fprintf(stderr, "Definition '%s' already exists, and redefinitions are not allowed.\n", line_definition_id);
                    panic();
                }

                Definition* definition = &definitions[definitions_len];
                memset(definition, 0, sizeof(Definition));
                definitions_len++;
                copy(definition->id, line_definition_id);

                while(true) {
                    expect_any(": or asset type before end of line following definition definition");
                    if(match(token, ":")) {
                        break;
                    }
                    copy(definition->outputs[definition->outputs_len], token);
                    definition->outputs_len++;
                }

                expect_any("source input filename following : in definition definition");
                copy(definition->input_path, token);

                expect_any("command line template following input source filename in definition definition");
                copy(definition->command_template, token);
                definition->command_template[tokeni] = ' ';
                i32 stri = tokeni + 1;
                while(consume()) {
                    copy(&definition->command_template[stri], token);
                    stri += tokeni;
                    definition->command_template[stri] = ' ';
                    stri++;
                }
#if DEBUG_PRINT_PARSE_OUTPUT
                print_definition(definition);
#endif
            } else {
                // Parse an instance
                if(existing_definition_index == -1) {
                    fprintf(stderr, "Definition '%s' not recognized and new definitions must be followed by '='.\n", line_definition_id);
                    panic();
                }

                Instance* instance = &instances[instances_len];
                memset(instance, 0, sizeof(Instance));
                instances_len++;
                instance->definition_index = existing_definition_index;
                copy(instance->id, token);

                while(consume()) {
                    copy(instance->args[instance->args_len], token);
                    instance->args_len++;
                }
#if DEBUG_PRINT_PARSE_OUTPUT
                print_instance(instance);
#endif

                // Process instance if newer than output pack file.
                Definition* definition = &definitions[instance->definition_index];
                char input_path[MAX_PATH_LENGTH] = "\0";
                expand_template(instance, definition->input_path, input_path);
                struct stat input_stat;
                u64 input_last_modified = UINT64_MAX;
                if(stat(input_path, &input_stat) == 0) {
                	u64 input_last_modified = input_stat.st_mtim.tv_sec;
                }
                if(pack_last_modified < input_last_modified) {
                	char command[MAX_LINE_LENGTH] = "\0";
                	expand_template(instance, definition->command_template, command);
                	system(command);
                }


                // Write to output buffer and store asset info
                for(i32 i = 0; i < definition->outputs_len; i++) {
                    Asset* asset = &assets[assets_len];
                    asset->buffer_index = pack_buffer_index;
                    asset->definition_index = instance->definition_index;
                    asset->instance_index = instances_len - 1;
                    asset->output_index = i;
                    assets_len++;

                    char asset_fname[4096];
                    sprintf(asset_fname, "%s.%s", instance->id, definition->outputs[i]);
                    FILE* asset_file = fopen(asset_fname, "r");
                    if(asset_file == NULL) {
                        fprintf(stderr, "Can't open asset file '%s' for writing.\n", asset_fname);
                        return false;
                    }
                    char write_buf[256];
                    u64 bytes_read = 0;
                    while((bytes_read = fread(write_buf, 1, sizeof(write_buf), asset_file)) > 0) {
                        memcpy(
                            &(pack_buffer[pack_buffer_index]), 
                            write_buf,
                            bytes_read);
                        pack_buffer_index += bytes_read;
                    }
                    fclose(asset_file);
                }
            }
        }
    }
    return true;
}

i32 main(i32 argc, char** argv) {
    // =====================================================
    // Process progam arguments and initialize state
    // =====================================================
     
    i32 argi = 1;
    // Store input filenames
    i32 input_file_index = 0;
    while(argi < argc && argv[argi][0] != '-') {
        if(argi >= argc) break;
        copy(input_paths[input_file_index], argv[argi]);
        argi++;
    }
    // Process other arguments
    while(argi < argc) {
        char* arg = argv[argi];
        if(match(arg, "-o") || match(arg, "--output")) {
            assert(argi < argc);
            argi++; 
            copy(output_path, argv[argi]);
        } else if(match(arg, "-i") || match(arg, "--include")) {
            assert(argi < argc);
            argi++; 
            copy(include_path, argv[argi]);
            use_include_path = true;
        } else if(match(arg, "-c") || match(arg, "--clean")) {
            pack_clean = true;
        }
        argi++;
    }
    // Verify arguments
    if(use_include_path) {
        fprintf(stderr, "Custom include paths are not functional yet.\n");
        goto invalid_usage;
    }
    // Find last modified time of pack
	struct stat out_stat;
	if(stat(output_path, &out_stat) == 0) {
    	pack_last_modified = out_stat.st_mtim.tv_sec;
	}

    // =====================================================
    // Interpret .pipl file/s
    // TODO: Parse multiple files.
    // =====================================================

    FILE* pipl = fopen(input_paths[0], "r");
    if(pipl == NULL) {
        fprintf(stderr, "Could not open file '%s'.\n", input_paths[0]);
        goto error;
    }
    if(interpret_pipl(pipl, input_paths[0]) == false) {
        goto error;
    }

    // =====================================================
    // Pack asset file
    // TODO: Pack assets into file and generate C files.
    //       - Pack files and store their offsets.
    //       - In C file, emit pack to byte array, #define
    //         the offsets as handles, and emit array of
    //         source filenames to check for modifications.
    // =====================================================

    FILE* out_file = fopen(output_path, "w");
    if(out_file == NULL) {
        fprintf(stderr, "Could not open output path '%s' for writing.\n", output_path);
        goto error;
    }

    for(i32 i = 0; i < instances_len; i++) {
        Instance* asset = &instances[i];
        u64 packed_size = 0;
        Definition* definition = &definitions[asset->definition_index];
        for(i32 j = 0; j < definition->outputs_len; j++) {
            char asset_fname[4096];
            sprintf(asset_fname, "%s.%s", asset->id, definition->outputs[j]);
            FILE* asset_file = fopen(asset_fname, "r");
            char write_buf[256];
            u64 bytes_read = 0;
            while((bytes_read = fread(write_buf, 1, sizeof(write_buf), asset_file)) > 0) {
                fwrite(write_buf, 1, bytes_read, out_file);
                packed_size += bytes_read;
            }
            fclose(asset_file);
        }
    }
    fclose(out_file);

    return 0;

    // =====================================================
    // Handle errors
    // =====================================================
error:
    fprintf(stderr, "PIPL terminated with error.\n");
    return 1;
invalid_usage:
    fprintf(stderr, "\nUsage:\n\n \
    pipl <input_filename ... > -o <output_filename> <arguments ... >\n\n\
Arguments:\n\n\
    -o, --output <file>:    As seen above, the output filename. REQUIRED.\n\n\
    -i, --include <dir>:    Sets additional directories for includes in PIPL\n\
                            files, in addition to the working directory.\n\n\
    -c, --clean:            If the output file already exists and this argument\n\
                            is NOT used, PIPL tracks which source files are\n\
                            newer than the output and only updates those assets.\n"
    );
    return 1;
}
