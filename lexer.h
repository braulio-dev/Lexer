#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    TOKEN_LITERAL,
    TOKEN_PUNCTUATION,
    TOKEN_COMMENT,
    TOKEN_WHITESPACE,
    TOKEN_EOF
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

// Lexer structure
typedef struct {
    FILE* file;
    char* filename;
    int current_line;
    int current_column;
    char current_char;
    Token* current_token;
} Lexer;

// Function declarations
Lexer* create_lexer(const char* filename);
void destroy_lexer(Lexer* lexer);
Token* get_next_token(Lexer* lexer);
void print_token(Token* token);
void destroy_token(Token* token);

// Helper functions
int is_keyword(const char* str);
int is_operator(char c);
int is_punctuation(char c);

#endif // LEXER_H 