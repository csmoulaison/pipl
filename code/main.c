#define CSM_IMPLEMENTATION
#define CSM_SKIP_MATH
#include "csm_core/core.h"

#define MAX_INPUT_FILES 64
#define MAX_PATH_LENGTH 4096
#define MAX_PIPELINES 256
#define MAX_ASSETS 4096
#define MAX_ASSET_NAME 256
#define MAX_ARG_LENGTH 4096
#define MAX_LINE_LENGTH 4096
#define MAX_TOKEN_LENGTH 4096

#define LINE_END(li) (line[li] == '\n' || line[li] == '\0' || line[li] == EOF)
#define match(str1, str2) (strcmp(str1, str2) == 0)
#define copy(str1, str2) (strcpy(str1, str2))

// Configuration
char input_paths[MAX_INPUT_FILES][MAX_PATH_LENGTH];
bool output_set = false;
char output_path[MAX_PATH_LENGTH];
bool use_include_path = false;
char include_path[MAX_PATH_LENGTH];
bool pack_clean = false;
// State
char line[MAX_LINE_LENGTH];
char token[MAX_TOKEN_LENGTH];
i32 linei = 0;
i32 tokeni = 0;
char line_pipeline[MAX_TOKEN_LENGTH];

char line_asset_name[MAX_ASSET_NAME] = "\0";
i32  line_asset_index = -1;

// Stores the next token if it exists, returning true if so and false if not.
bool consume() {
    if(LINE_END(linei)) return false;
    while(line[linei] == ' ') {
        linei++;
        assert(LINE_END(linei));
    }
    tokeni = 0;
    while(!LINE_END(linei)) {
        token[tokeni] = line[linei];
        tokeni++;
        linei++;
    }
    return true;
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
        if(consume()) { 
            copy(line_pipeline, token);
            assert(consume);
            if(match(token, "=")) { // pipeline definition
                // TODO: New pipeline.
                while(!match(token, ":")) {
                    // TODO: Replace below with expect() macro.
                    if(!consume()) {
                        fprintf(stderr, "Expected : before end of line following pipeline definition.\n");
                        goto invalid_usage;
                    }
                    // TODO: Store each asset file extension.
                }

                // TODO: Store input filename template.
                // TODO: Then store command line template.
                 
            } else { // asset definition
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
