/*
 * COPYRIGHT (C) Liu Yuan <namei.unix@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Liu Yuan <namei.unix@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "tc.h"

static char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: cannot open file %s.\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char*) malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: cannot allocate memory for file %s.\n", filename);
        exit(1);
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}

static void generate_machine_code(char *code, char *la)
{
    char cmd[1024];
    FILE *fp = fopen("a.s", "w");
    fprintf(fp, "%s", code);
    fclose(fp);
    if (la)
        sprintf(cmd, "gcc -o a.tc a.s -l%s", la);
    else
        sprintf(cmd, "gcc -o a.tc a.s");
    system(cmd);
    //remove("a.s");
}

int main(int argc, char **argv)
{
    char *source_code = NULL;
    char *linker_arg = NULL;
    int opt, need_free = 0;

    // Parse command line options
    while ((opt = getopt(argc, argv, "s:l:")) != -1) {
        switch (opt) {
        case 's':
            source_code = optarg;
            break;
        case 'l':
            linker_arg = optarg;
            break;
        default:
            panic("Usage: %s [-s source_code] [-l linker arg] [input_file]\n", argv[0]);
        }
    }

    // Ensure that an input file or source code has been specified
    if (optind >= argc && !source_code)
        // optind >= argc means no input file
        panic("Error: no input file specified.\n");

    // Read the input file
    if (!source_code) {
        source_code = read_file(argv[optind]);
        need_free = 1;
    }

    // Perform lexical analysis
    struct list_head *tokens_list = lex(source_code);
    if (need_free)
        free(source_code);

    // Perform syntax analysis
    cast_node_t *ast = parse(tokens_list);

    // Perform semantic analysis
    analyze_semantics(ast);

    // Generate code
    struct strbuf *code = generate_code(ast);

    // Optimize code
    optimize_code(code);

    generate_machine_code(code->buf, linker_arg);
    // Debug code
    //debug_code(code_generator, debug_info);

    strbuf_release(code);
    // Free memory
    // free(tokens);
    // free(ast);

    // Exit program
    return 0;
}
