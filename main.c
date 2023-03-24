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

#include "tc.h"

// Syntax Analysis
ast_node_t* parse(struct list_head* tokens) {};

// Semantic Analysis
void analyze_semantics(ast_node_t* ast, symbol_table_t* symbol_table) {};

// Code Generation
void generate_code(ast_node_t* ast, code_generator_t* code_generator) {};

// Optimization
void optimize_code(code_generator_t* code_generator, optimization_pass_t*
		   optimization_pass) {};

// Debugging
void debug_code(code_generator_t* code_generator, debug_info_t*
		debug_info) {};

static char* read_file(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: cannot open file %s.\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*) malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "Error: cannot allocate memory for file %s.\n", filename);
        exit(1);
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    fclose(file);
    return buffer;
}

static void close_file(char* source_code)
{
    free(source_code);
}

int main(int argc, char** argv)
{
    // Ensure that an input file has been specified
    if (argc < 2) {
        fprintf(stderr, "Error: no input file specified.\n");
        exit(1);
    }

    // Read the input file
    char* source_code = read_file(argv[1]);

    // Perform lexical analysis
    struct list_head* tokens_list = lex(source_code);

    // Perform syntax analysis
    ast_node_t* ast = parse(tokens_list);

    // Perform semantic analysis
    symbol_table_t* symbol_table = (symbol_table_t*)
	    malloc(sizeof(symbol_table_t));
    analyze_semantics(ast, symbol_table);

    // Generate code
    code_generator_t* code_generator =
	    (code_generator_t*) malloc(sizeof(code_generator_t));
    generate_code(ast, code_generator);

    // Optimize code
    optimization_pass_t* optimization_pass =
	    (optimization_pass_t*) malloc(sizeof(optimization_pass_t));
    optimize_code(code_generator, optimization_pass);

    // Debug code
    debug_info_t* debug_info = (debug_info_t*) malloc(sizeof(debug_info_t));
    debug_code(code_generator, debug_info);

    // Close the input file
    close_file(source_code);

    // Free memory
    // free(tokens);
    // free(ast);
    free(symbol_table);
    free(code_generator);
    free(optimization_pass);
    free(debug_info);

    // Exit program
    return 0;
}
