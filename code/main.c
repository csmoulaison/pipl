#define CSM_IMPLEMENTATION
#define CSM_SKIP_MATH
#include "csm_core/core.h"

#define MAX_INPUT_FILES 64
#define MAX_PATH_LENGTH 4096
#define MAX_PIPELINES 256
#define MAX_ASSETS 1024
#define MAX_ASSET_NAME 256
#define MAX_ARG_LENGTH 4096
#define MAX_LINE_LENGTH 4096
#define MAX_TOKEN_LENGTH 256
#define MAX_PIPELINE_OUTPUTS 8

#define LINE_END(li) (line[li] == '\n' || line[li] == '\0' || line[li] == EOF)
#define match(str1, str2) (strcmp(str1, str2) == 0)
#define copy(str1, str2) (strcpy(str1, str2))
#define expect(str, msg) { if(!consume() || !match(token, str)) \
    { fprintf(stderr, "Expected %s %s\n", str, msg); goto invalid_usage; }}
#define expect_any(msg) { if(!consume()) \
    { fprintf(stderr, "Expected %s\n", msg); goto invalid_usage; }}

typedef struct {
    char id[MAX_TOKEN_LENGTH];
    char outputs[MAX_PIPELINE_OUTPUTS][MAX_TOKEN_LENGTH];
    i32 outputs_len;
    char input_filename[MAX_PATH_LENGTH];
    char command_template[MAX_ARG_LENGTH];
} PipelineDefinition;

typedef struct {
    i32 pipeline_index;
    char id[MAX_TOKEN_LENGTH];
    char args[MAX_ARG_LENGTH];
    i32 args_len;
} AssetDefinition;

// Configuration
char input_paths[MAX_INPUT_FILES][MAX_PATH_LENGTH];
bool output_set = false;
char output_path[MAX_PATH_LENGTH];
bool use_include_path = false;
char include_path[MAX_PATH_LENGTH];
bool pack_clean = false;
// Parse state
char line[MAX_LINE_LENGTH];
char token[MAX_TOKEN_LENGTH];
i32 linei = 0;
i32 tokeni = 0;
char line_pipeline[MAX_TOKEN_LENGTH];
char line_asset_name[MAX_ASSET_NAME] = "\0";
i32  line_asset_index = -1;
// Definitions
PipelineDefinition pipeline_defs[MAX_PIPELINES];
i32 pipelines_len = 0;
AssetDefinition asset_defs[MAX_ASSETS];
i32 assets_len = 0;

#include "debug_helpers.c"

// Stores the next token if it exists, returning true if so and false if not.
bool consume() {
    if(LINE_END(linei)) {
        return false;
    }
    while(line[linei] == ' ') {
        linei++;
        assert(!LINE_END(linei));
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

bool consume_to_line_end(char* str) {
}

i32 main(i32 argc, char** argv) {
    // =====================================================
    // Process progam arguments
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
            output_set = true;
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
    if(!output_set) {
        fprintf(stderr, "ERROR: No output filename set.\n");
        goto invalid_usage;
    }
    if(use_include_path) {
        fprintf(stderr, "ERROR: file includes not functional yet.\n");
        goto invalid_usage;
    }

    // =====================================================
    // Parse .pipl file
    // TODO: Parse multiple files.
    // =====================================================

    FILE* pipl = fopen(input_paths[0], "r");
    if(pipl == NULL) {
        fprintf(stderr, "ERROR: Could not open file \"%s\"\n", input_paths[0]);
        goto invalid_usage;
    }

    while(fgets(line, MAX_LINE_LENGTH, pipl)) {
        linei = 0;
        if(consume()) { 
            copy(line_pipeline, token);
            assert(consume());
            if(match(token, "=")) {
                // Parse pipeline definition
                PipelineDefinition* pipeline = &pipeline_defs[pipelines_len];
                memset(pipeline, 0, sizeof(PipelineDefinition));
                pipelines_len++;
                // Pipeline id
                copy(pipeline->id, line_pipeline);
                // Pipeline outputs
                while(true) {
                    expect_any(": or asset type before end of line following pipeline definition");
                    if(match(token, ":")) {
                        break;
                    }
                    copy(pipeline->outputs[pipeline->outputs_len], token);
                    pipeline->outputs_len++;
                }
                // Source input filename
                expect_any("source input filename following : in pipeline definition");
                copy(pipeline->input_filename, token);
                // Command line template
                expect_any("command line template following input source filename in pipeline definition");
                copy(pipeline->command_template, token);
                pipeline->command_template[tokeni] = ' ';
                i32 stri = tokeni + 1;
                while(consume()) {
                    copy(&pipeline->command_template[stri], token);
                    stri += tokeni;
                    pipeline->command_template[stri] = ' ';
                    stri++;
                }
#if 1
                print_pipeline_def(pipeline);
#endif
            } else { // asset definition
                fprintf(stderr, "Expected pipeline definition, got '%s'.\n", token);
                panic();
                // TODO: Check existence of the pipeline with the id.
                // TODO: Store next token as asset name.
                // TODO: Check if source file exists and is newer than pack.
                // TODO: Store argument strings.
                // TODO: Expand pipeline command line template and run.
                // TODO: Ensure that the specified file has been created.
                //          (maybe store old file modified time to verify)
            } // TODO: Check for include directive and read that if so. Implies
              // recursive function call for .pipl parsing.
        }
    }

    // =====================================================
    // Pack asset file
    // =====================================================

    // TODO: Pack assets into a single file.
    // TODO: Generate C source for asset references, counts,
    //       and pack structure. It can all be hardcoded
    //       because it can all be generated.
    //          (also generate list of filenames to check
    //           for triggering hot reloading from the host
    //           program)

    return 0;

    // =====================================================
    // Handle errors
    // =====================================================
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
