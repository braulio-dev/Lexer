#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Create a new parser
Parser* create_parser(Lexer* lexer) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) return NULL;

    parser->lexer = lexer;
    parser->current_token = get_next_token(lexer);
    parser->lookahead_token = get_next_token(lexer);
    parser->debug = 0;  // Debug mode disabled by default

    return parser;
}

// Set debug mode
void set_debug_mode(Parser* parser, int debug) {
    parser->debug = debug;
}

// Destroy the parser
void destroy_parser(Parser* parser) {
    if (parser) {
        if (parser->current_token) destroy_token(parser->current_token);
        if (parser->lookahead_token) destroy_token(parser->lookahead_token);
        free(parser);
    }
}

// Advance to the next token
void advance(Parser* parser) {
    if (parser->current_token) {
        destroy_token(parser->current_token);
    }
    parser->current_token = parser->lookahead_token;
    parser->lookahead_token = get_next_token(parser->lexer);
}

// Match the current token with the expected type
void match(Parser* parser, TokenType expected_type) {
    const char* type_names[] = {
        "IDENTIFIER", "KEYWORD", "OPERATOR", "LITERAL",
        "PUNCTUATION", "COMMENT", "WHITESPACE", "EOF"
    };
    
    if (parser->current_token->type == expected_type) {
        if (parser->debug) {
            fprintf(stderr, "Matched token: %s '%s' at %d:%d\n",
                    type_names[parser->current_token->type],
                    parser->current_token->value,
                    parser->current_token->line,
                    parser->current_token->column);
        }
        advance(parser);
    } else {
        fprintf(stderr, "Syntax error: Expected token type %s, got %s at line %d, column %d\n",
                type_names[expected_type], type_names[parser->current_token->type],
                parser->current_token->line, parser->current_token->column);
        exit(1);
    }
}

// Parse a class declaration
void parse_class(Parser* parser) {
    // Match class keyword
    match(parser, TOKEN_KEYWORD);
    
    // Match class name
    match(parser, TOKEN_IDENTIFIER);
    
    // Match opening brace
    match(parser, TOKEN_PUNCTUATION);
    
    // Parse class body (which will match the closing brace)
    parse_class_body(parser);
}

// Parse class body
void parse_class_body(Parser* parser) {
    int brace_count = 1; // Count the opening brace we just saw
    
    while (brace_count > 0) {
        if (parser->current_token->type == TOKEN_PUNCTUATION) {
            if (strcmp(parser->current_token->value, "{") == 0) {
                brace_count++;
                advance(parser);
            } else if (strcmp(parser->current_token->value, "}") == 0) {
                brace_count--;
                if (brace_count > 0) { // Only advance if not the last closing brace
                    advance(parser);
                }
            } else {
                advance(parser);
            }
        } else if (parser->current_token->type == TOKEN_KEYWORD) {
            if (strcmp(parser->current_token->value, "public") == 0 ||
                strcmp(parser->current_token->value, "private") == 0 ||
                strcmp(parser->current_token->value, "protected") == 0) {
                parse_method(parser);
            } else {
                advance(parser);
            }
        } else {
            advance(parser);
        }
    }
    
    // Match the final closing brace
    match(parser, TOKEN_PUNCTUATION);
}

// Parse a method declaration
void parse_method(Parser* parser) {
    int brace_count = 0;
    
    // Match access modifier
    match(parser, TOKEN_KEYWORD);
    
    // Match optional static keyword
    if (parser->current_token->type == TOKEN_KEYWORD &&
        strcmp(parser->current_token->value, "static") == 0) {
        advance(parser);
    }
    
    // Match return type
    if (parser->current_token->type == TOKEN_KEYWORD ||
        parser->current_token->type == TOKEN_IDENTIFIER) {
        advance(parser);
    }
    
    // Match method name
    match(parser, TOKEN_IDENTIFIER);
    
    // Match opening parenthesis
    match(parser, TOKEN_PUNCTUATION);
    
    // Parse parameters if any
    while (parser->current_token->type != TOKEN_PUNCTUATION ||
           strcmp(parser->current_token->value, ")") != 0) {
        advance(parser);
    }
    
    // Match closing parenthesis
    match(parser, TOKEN_PUNCTUATION);
    
    // Match opening brace
    match(parser, TOKEN_PUNCTUATION);
    brace_count = 1;
    
    // Parse method body
    while (brace_count > 0) {
        if (parser->current_token->type == TOKEN_PUNCTUATION) {
            if (strcmp(parser->current_token->value, "{") == 0) {
                brace_count++;
                advance(parser);
            } else if (strcmp(parser->current_token->value, "}") == 0) {
                brace_count--;
                if (brace_count > 0) { // Only advance if not the last closing brace
                    advance(parser);
                }
            } else {
                advance(parser);
            }
        } else {
            parse_statement(parser);
        }
    }
    
    // Match closing brace
    match(parser, TOKEN_PUNCTUATION);
}

// Parse a statement
void parse_statement(Parser* parser) {
    // Handle variable declarations
    if (parser->current_token->type == TOKEN_KEYWORD &&
        (strcmp(parser->current_token->value, "int") == 0 ||
         strcmp(parser->current_token->value, "String") == 0)) {
        // Save the type
        char* type = strdup(parser->current_token->value);
        advance(parser); // Skip type
        
        // Get variable name
        if (parser->current_token->type != TOKEN_IDENTIFIER) {
            fprintf(stderr, "Syntax error: Expected IDENTIFIER after type '%s' at line %d, column %d\n",
                    type, parser->current_token->line, parser->current_token->column);
            free(type);
            exit(1);
        }
        char* name = strdup(parser->current_token->value);
        advance(parser); // Skip variable name
        
        // Handle initialization
        if (parser->current_token->type == TOKEN_OPERATOR &&
            strcmp(parser->current_token->value, "=") == 0) {
            advance(parser); // Skip =
            
            // Handle the value
            if (parser->current_token->type == TOKEN_LITERAL ||
                parser->current_token->type == TOKEN_IDENTIFIER) {
                advance(parser);
            } else {
                fprintf(stderr, "Syntax error: Expected value after '=' in declaration of '%s %s' at line %d, column %d\n",
                        type, name, parser->current_token->line, parser->current_token->column);
                free(type);
                free(name);
                exit(1);
            }
        }
        
        // Expect semicolon
        if (parser->current_token->type != TOKEN_PUNCTUATION ||
            strcmp(parser->current_token->value, ";") != 0) {
            fprintf(stderr, "Syntax error: Expected ';' after variable declaration '%s %s' at line %d, column %d\n",
                    type, name, parser->current_token->line, parser->current_token->column);
            free(type);
            free(name);
            exit(1);
        }
        advance(parser); // Skip semicolon
        
        free(type);
        free(name);
        return;
    }
    
    // Handle if statements
    if (parser->current_token->type == TOKEN_KEYWORD &&
        strcmp(parser->current_token->value, "if") == 0) {
        advance(parser);
        match(parser, TOKEN_PUNCTUATION); // (
        parse_expression(parser);
        match(parser, TOKEN_PUNCTUATION); // )
        match(parser, TOKEN_PUNCTUATION); // {
        
        while (parser->current_token->type != TOKEN_PUNCTUATION ||
               strcmp(parser->current_token->value, "}") != 0) {
            parse_statement(parser);
        }
        match(parser, TOKEN_PUNCTUATION); // }
        
        // Handle else
        if (parser->current_token->type == TOKEN_KEYWORD &&
            strcmp(parser->current_token->value, "else") == 0) {
            advance(parser);
            match(parser, TOKEN_PUNCTUATION); // {
            while (parser->current_token->type != TOKEN_PUNCTUATION ||
                   strcmp(parser->current_token->value, "}") != 0) {
                parse_statement(parser);
            }
            match(parser, TOKEN_PUNCTUATION); // }
        }
        return;
    }
    
    // Handle method calls and assignments
    if (parser->current_token->type == TOKEN_IDENTIFIER) {
        char* identifier = strdup(parser->current_token->value);
        advance(parser);
        
        // Handle dot notation (e.g., System.out.println)
        while (parser->current_token->type == TOKEN_PUNCTUATION &&
               strcmp(parser->current_token->value, ".") == 0) {
            advance(parser); // Skip dot
            if (parser->current_token->type == TOKEN_IDENTIFIER) {
                advance(parser);
            } else {
                fprintf(stderr, "Syntax error: Expected identifier after '.' at line %d, column %d\n",
                        parser->current_token->line, parser->current_token->column);
                free(identifier);
                exit(1);
            }
        }
        
        // Method call
        if (parser->current_token->type == TOKEN_PUNCTUATION &&
            strcmp(parser->current_token->value, "(") == 0) {
            parse_method_call(parser);
            if (parser->current_token->type != TOKEN_PUNCTUATION ||
                strcmp(parser->current_token->value, ";") != 0) {
                fprintf(stderr, "Syntax error: Expected ';' after method call at line %d, column %d\n",
                        parser->current_token->line, parser->current_token->column);
                free(identifier);
                exit(1);
            }
            advance(parser); // Skip semicolon
            free(identifier);
            return;
        }
        
        // Assignment
        if (parser->current_token->type == TOKEN_OPERATOR &&
            strcmp(parser->current_token->value, "=") == 0) {
            advance(parser);
            parse_expression(parser);
            if (parser->current_token->type != TOKEN_PUNCTUATION ||
                strcmp(parser->current_token->value, ";") != 0) {
                fprintf(stderr, "Syntax error: Expected ';' after assignment to '%s' at line %d, column %d\n",
                        identifier, parser->current_token->line, parser->current_token->column);
                free(identifier);
                exit(1);
            }
            advance(parser); // Skip semicolon
            free(identifier);
            return;
        }
        
        free(identifier);
    }
    
    // Expression statement
    parse_expression(parser);
    if (parser->current_token->type != TOKEN_PUNCTUATION ||
        strcmp(parser->current_token->value, ";") != 0) {
        fprintf(stderr, "Syntax error: Expected ';' after expression at line %d, column %d\n",
                parser->current_token->line, parser->current_token->column);
        exit(1);
    }
    advance(parser); // Skip semicolon
}

// Parse a method call
void parse_method_call(Parser* parser) {
    // Match opening parenthesis
    match(parser, TOKEN_PUNCTUATION); // (
    
    // Parse arguments if any
    while (parser->current_token->type != TOKEN_PUNCTUATION ||
           strcmp(parser->current_token->value, ")") != 0) {
        if (parser->current_token->type == TOKEN_LITERAL ||
            parser->current_token->type == TOKEN_IDENTIFIER) {
            advance(parser);
        }
        // Skip commas between arguments
        if (parser->current_token->type == TOKEN_PUNCTUATION &&
            strcmp(parser->current_token->value, ",") == 0) {
            advance(parser);
        }
    }
    
    // Match closing parenthesis
    match(parser, TOKEN_PUNCTUATION); // )
}

// Parse an expression
void parse_expression(Parser* parser) {
    // Handle literals
    if (parser->current_token->type == TOKEN_LITERAL) {
        advance(parser);
        return;
    }
    
    // Handle identifiers and method calls
    if (parser->current_token->type == TOKEN_IDENTIFIER) {
        advance(parser);
        
        // Handle dot notation (e.g., System.out.println)
        while (parser->current_token->type == TOKEN_PUNCTUATION &&
               strcmp(parser->current_token->value, ".") == 0) {
            advance(parser); // Skip dot
            if (parser->current_token->type == TOKEN_IDENTIFIER) {
                advance(parser);
            }
        }
        
        // Handle method calls
        if (parser->current_token->type == TOKEN_PUNCTUATION &&
            strcmp(parser->current_token->value, "(") == 0) {
            parse_method_call(parser);
            return;
        }
        
        // Handle operators
        if (parser->current_token->type == TOKEN_OPERATOR) {
            char* op = strdup(parser->current_token->value);
            advance(parser);
            
            // Handle the right-hand side of the operator
            if (parser->current_token->type == TOKEN_LITERAL ||
                parser->current_token->type == TOKEN_IDENTIFIER) {
                advance(parser);
            } else {
                fprintf(stderr, "Syntax error: Expected value after operator '%s' at line %d, column %d\n",
                        op, parser->current_token->line, parser->current_token->column);
                free(op);
                exit(1);
            }
            free(op);
            return;
        }
        return;
    }
    
    // Handle operators
    if (parser->current_token->type == TOKEN_OPERATOR) {
        char* op = strdup(parser->current_token->value);
        advance(parser);
        
        // Handle the operand
        if (parser->current_token->type == TOKEN_LITERAL ||
            parser->current_token->type == TOKEN_IDENTIFIER) {
            advance(parser);
        } else {
            fprintf(stderr, "Syntax error: Expected value after operator '%s' at line %d, column %d\n",
                    op, parser->current_token->line, parser->current_token->column);
            free(op);
            exit(1);
        }
        free(op);
        return;
    }
    
    // Handle parenthesized expressions
    if (parser->current_token->type == TOKEN_PUNCTUATION &&
        strcmp(parser->current_token->value, "(") == 0) {
        advance(parser);
        parse_expression(parser);
        if (parser->current_token->type == TOKEN_PUNCTUATION &&
            strcmp(parser->current_token->value, ")") == 0) {
            advance(parser);
        } else {
            fprintf(stderr, "Syntax error: Expected ')' at line %d, column %d\n",
                    parser->current_token->line, parser->current_token->column);
            exit(1);
        }
        return;
    }
}

// Main parse function
void parse(Parser* parser) {
    while (parser->current_token->type != TOKEN_EOF) {
        if (parser->current_token->type == TOKEN_KEYWORD &&
            strcmp(parser->current_token->value, "class") == 0) {
            parse_class(parser);
        } else {
            advance(parser);
        }
    }
    fprintf(stderr, "Parsing completed successfully!\n");
} 