#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <java_file>\n", argv[0]);
        return 1;
    }

    // Create lexer
    Lexer* lexer = create_lexer(argv[1]);
    if (!lexer) {
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        return 1;
    }

    // Create parser
    Parser* parser = create_parser(lexer);
    if (!parser) {
        fprintf(stderr, "Error: Could not create parser\n");
        destroy_lexer(lexer);
        return 1;
    }

    // Parse the input
    parse(parser);

    // Cleanup
    destroy_parser(parser);
    destroy_lexer(lexer);

    return 0;
} 