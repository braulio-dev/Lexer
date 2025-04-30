#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    int debug_mode = 0;
    char* filename = NULL;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            debug_mode = 1;
        } else if (filename == NULL) {
            filename = argv[i];
        } else {
            fprintf(stderr, "Usage: %s [-d|--debug] <java_file>\n", argv[0]);
            return 1;
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "Usage: %s [-d|--debug] <java_file>\n", argv[0]);
        return 1;
    }

    // Create lexer
    Lexer* lexer = create_lexer(filename);
    if (!lexer) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return 1;
    }

    // Create parser
    Parser* parser = create_parser(lexer);
    if (!parser) {
        fprintf(stderr, "Error: Could not create parser\n");
        destroy_lexer(lexer);
        return 1;
    }

    // Set debug mode if requested
    if (debug_mode) {
        set_debug_mode(parser, 1);
    }

    // Parse the input
    parse(parser);

    // Cleanup
    destroy_parser(parser);
    destroy_lexer(lexer);

    return 0;
} 