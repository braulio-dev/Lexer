#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Parser structure
typedef struct {
    Lexer* lexer;
    Token* current_token;
    Token* lookahead_token;
} Parser;

// Function declarations
Parser* create_parser(Lexer* lexer);
void destroy_parser(Parser* parser);
void parse(Parser* parser);
void match(Parser* parser, TokenType expected_type);
void advance(Parser* parser);

// Grammar rule functions
void parse_class(Parser* parser);
void parse_class_body(Parser* parser);
void parse_method(Parser* parser);
void parse_statement(Parser* parser);
void parse_expression(Parser* parser);
void parse_method_call(Parser* parser);

#endif // PARSER_H 