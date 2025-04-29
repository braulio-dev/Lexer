#include "lexer.h"

// Java keywords
static const char* keywords[] = {
    "abstract", "assert", "boolean", "break", "byte", "case", "catch", "char",
    "class", "const", "continue", "default", "do", "double", "else", "enum",
    "extends", "final", "finally", "float", "for", "if", "implements", "import",
    "instanceof", "int", "interface", "long", "native", "new", "package", "private",
    "protected", "public", "return", "short", "static", "strictfp", "super", "switch",
    "synchronized", "this", "throw", "throws", "transient", "try", "void", "volatile",
    "while", NULL
};

// Create a new lexer
Lexer* create_lexer(const char* filename) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;

    lexer->file = fopen(filename, "r");
    if (!lexer->file) {
        free(lexer);
        return NULL;
    }

    lexer->filename = strdup(filename);
    lexer->current_line = 1;
    lexer->current_column = 0;
    lexer->current_char = ' ';
    lexer->current_token = NULL;

    return lexer;
}

// Destroy the lexer
void destroy_lexer(Lexer* lexer) {
    if (lexer) {
        if (lexer->file) fclose(lexer->file);
        if (lexer->filename) free(lexer->filename);
        if (lexer->current_token) destroy_token(lexer->current_token);
        free(lexer);
    }
}

// Destroy a token
void destroy_token(Token* token) {
    if (token) {
        if (token->value) free(token->value);
        free(token);
    }
}

// Check if a string is a keyword
int is_keyword(const char* str) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Check if a character is an operator
int is_operator(char c) {
    return strchr("+-*/%=&|<>!~^", c) != NULL;
}

// Check if a character is punctuation
int is_punctuation(char c) {
    return strchr("{}[]();,.:@", c) != NULL;
}

// Get the next character from the file
void next_char(Lexer* lexer) {
    lexer->current_char = fgetc(lexer->file);
    lexer->current_column++;
    
    if (lexer->current_char == '\n') {
        lexer->current_line++;
        lexer->current_column = 0;
    }
}

// Skip whitespace
void skip_whitespace(Lexer* lexer) {
    while (isspace(lexer->current_char)) {
        next_char(lexer);
    }
}

// Read an identifier or keyword
Token* read_identifier(Lexer* lexer) {
    char buffer[1024];  // Increased buffer size for qualified names
    int i = 0;
    int is_qualified = 0;
    
    while (isalnum(lexer->current_char) || lexer->current_char == '_' || 
           lexer->current_char == '.') {
        if (lexer->current_char == '.') {
            is_qualified = 1;
        }
        buffer[i++] = lexer->current_char;
        next_char(lexer);
    }
    buffer[i] = '\0';
    
    Token* token = (Token*)malloc(sizeof(Token));
    token->value = strdup(buffer);
    token->line = lexer->current_line;
    token->column = lexer->current_column - i;
    
    // If it contains dots, it's a qualified identifier
    if (is_qualified) {
        token->type = TOKEN_IDENTIFIER;
    } else {
        token->type = is_keyword(buffer) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    }
    
    return token;
}

// Read a number literal
Token* read_number(Lexer* lexer) {
    char buffer[256];
    int i = 0;
    
    while (isdigit(lexer->current_char) || lexer->current_char == '.') {
        buffer[i++] = lexer->current_char;
        next_char(lexer);
    }
    buffer[i] = '\0';
    
    Token* token = (Token*)malloc(sizeof(Token));
    token->value = strdup(buffer);
    token->line = lexer->current_line;
    token->column = lexer->current_column - i;
    token->type = TOKEN_LITERAL;
    
    return token;
}

// Read a string literal
Token* read_string(Lexer* lexer) {
    char buffer[1024];
    int i = 0;
    
    buffer[i++] = '"'; // Include opening quote
    next_char(lexer); // Skip opening quote
    
    while (lexer->current_char != '"' && lexer->current_char != EOF) {
        buffer[i++] = lexer->current_char;
        next_char(lexer);
    }
    
    if (lexer->current_char == '"') {
        buffer[i++] = '"'; // Include closing quote
    }
    
    buffer[i] = '\0';
    next_char(lexer); // Skip closing quote
    
    Token* token = (Token*)malloc(sizeof(Token));
    token->value = strdup(buffer);
    token->line = lexer->current_line;
    token->column = lexer->current_column - i;
    token->type = TOKEN_LITERAL;
    
    return token;
}

// Get the next token from the input
Token* get_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    
    if (lexer->current_char == EOF) {
        Token* token = (Token*)malloc(sizeof(Token));
        token->type = TOKEN_EOF;
        token->value = strdup("EOF");
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        return token;
    }
    
    if (isalpha(lexer->current_char) || lexer->current_char == '_') {
        return read_identifier(lexer);
    }
    
    if (isdigit(lexer->current_char)) {
        return read_number(lexer);
    }
    
    if (lexer->current_char == '"') {
        return read_string(lexer);
    }
    
    if (is_operator(lexer->current_char)) {
        char op[2] = {lexer->current_char, '\0'};
        Token* token = (Token*)malloc(sizeof(Token));
        token->value = strdup(op);
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        token->type = TOKEN_OPERATOR;
        next_char(lexer);
        return token;
    }
    
    if (is_punctuation(lexer->current_char)) {
        char punc[2] = {lexer->current_char, '\0'};
        Token* token = (Token*)malloc(sizeof(Token));
        token->value = strdup(punc);
        token->line = lexer->current_line;
        token->column = lexer->current_column;
        token->type = TOKEN_PUNCTUATION;
        next_char(lexer);
        return token;
    }
    
    // Handle comments
    if (lexer->current_char == '/') {
        next_char(lexer);
        if (lexer->current_char == '/') {
            // Single-line comment
            char buffer[1024];
            int i = 0;
            while (lexer->current_char != '\n' && lexer->current_char != EOF) {
                buffer[i++] = lexer->current_char;
                next_char(lexer);
            }
            buffer[i] = '\0';
            
            Token* token = (Token*)malloc(sizeof(Token));
            token->value = strdup(buffer);
            token->line = lexer->current_line;
            token->column = lexer->current_column - i - 2;
            token->type = TOKEN_COMMENT;
            return token;
        }
    }
    
    // If we get here, it's an unknown character
    char unknown[2] = {lexer->current_char, '\0'};
    Token* token = (Token*)malloc(sizeof(Token));
    token->value = strdup(unknown);
    token->line = lexer->current_line;
    token->column = lexer->current_column;
    token->type = TOKEN_PUNCTUATION;
    next_char(lexer);
    return token;
}

// Print a token
void print_token(Token* token) {
    const char* type_names[] = {
        "IDENTIFIER", "KEYWORD", "OPERATOR", "LITERAL",
        "PUNCTUATION", "COMMENT", "WHITESPACE", "EOF"
    };
    
    printf("Token: %s (%s) at line %d, column %d\n",
           token->value, type_names[token->type],
           token->line, token->column);
} 