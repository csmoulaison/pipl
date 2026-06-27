# PIPL (Packed Instance Pipeline Language)
PIPL is a game asset packing program which reads a .pipl file containing a list
of pipeline and asset instance definitions and spits out a packed asset file.

Usage is as follows:

    pipl <input_filename ... > -o <output_filename> <arguments ... >

The list of input files are assumed to be written in the PIPL language, and the
output file is the final packed asset file. Arguments are as follows:

    -o, --output <file>:    As seen above, the output filename. REQUIRED.

    -i, --include <dir>:    Sets additional directories for includes in PIPL
                            files, in addition to the working directory.

    -c, --clean:            If the output file already exists and this argument
                            is NOT used, PIPL tracks which source files are
                            newer than the output and only updates those assets.

# The .pipl Format

PIPL files contain source code written in the PIPL language. The language allows
users to define pipelines, which encode a conversion operation between source
files and asset files.

## Asset and Pipeline Definitions

A pipeline definition looks like this:

    model = .mesh .texture : _$.blend sh process_blend.sh _$ _0
    font  = .font .texture : _$.ttf   sh process_font.sh _$

It starts with an identifier and equals sign, followed by a space-separated list
of output assets, followed by a colon, the name of the source input filename,
followed by a command to be run on the command line. Both the source filename
and the command can include any number of arguments, numbered _0, _1, and so on,
as well as some implicit arguments used for special values. These are: _$ for
the asset name and _# for the asset index.

An asset definition looks like this:

    model cube flat
    model sphere smooth
    font bembo

It is simply a pipeline identifier and an asset name, followed by a space-
separated list of arguments.

## Including Files

PIPL files can include other PIPL files using the following directive:

    include file.pipl

This has the effect of inserting all the data in the file as plain text at the
position of the include directive, just like in C. Paths are relative to the
working directory, or any include directories defined using the --include
argument to the pipl invocation.

