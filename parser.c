#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum TokenType {
    IntLit,
    StrLit,
    Id,
    AssignOp,
    LeftParan,
    RightParan,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    LessThan,
    GreaterThan,
    Dot,
    Newline,
    Comma,
    Comment,
};

enum TokenizeErrorType {
    IdTooLong = 1,
    NumberTooLong,
    StrTooLong,
    CommentTooLong,
    UnknownChar,
};

struct Token {
    enum TokenType type;
    char *text;
    int startOffset;
    int endOffset;
    int startLine;
    int endLine;
    int startChar;
    int endChar;
};

struct TokenList {
    struct Token *token;
    struct TokenList *next;
};

struct TokenizeErrorInfo {
    int offset;
    int line;
    int character;
};

#define BUFFER_LEN 100

int isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isDigit(char c) {
    return c >= '0' && c <= '9';
}

int printToken(struct Token *token, int details) {
    printf("Token(");
    switch (token->type) {
        case Id:
            printf("ID,");
            break;
        case AssignOp:
            printf("AssignOp");
            break;
        case IntLit:
            printf("IntLit,");
            break;
        case StrLit:
            printf("StrLit,");
            break;
        case LeftParan:
            printf("LeftParan");
            break;
        case RightParan:
            printf("RightParan");
            break;
        case LeftBrace:
            printf("LeftBrace");
            break;
        case RightBrace:
            printf("RightBrace");
            break;
        case LeftBracket:
            printf("LeftBracket");
            break;
        case RightBracket:
            printf("RightBracket");
            break;
        case LessThan:
            printf("LessThan");
            break;
        case GreaterThan:
            printf("GreaterThan");
            break;
        case Dot:
            printf("Dot");
            break;
        case Comma:
            printf("Comma");
            break;
        case Comment:
            printf("Comment");
            break;
        case Newline:
            printf("Newline");
            break;
        default:
            printf("Unknown");
    }
    if (token->text != NULL) {
        printf("%s", token->text);
    }
    if (details) {
        printf(",%d,%d,%d,%d,%d,%d", 
            token->startOffset, token->endOffset,
            token->startLine, token->endLine,
            token->startChar, token->endChar
        );
    }
    printf(")");
    printf("\n");
    
    return 0;
}

void tokenListAppend(
    struct TokenList **tokens, 
    struct TokenList **tokensTail, 
    struct Token *token
) {
    // Append to the end of tokens linked list
    if (*tokens == NULL) {
        *tokens = malloc(sizeof (struct TokenList));
        *tokensTail = *tokens;
    } else {
        (*tokensTail)->next = malloc(sizeof (struct TokenList));
        (*tokensTail) = (*tokensTail)->next;
    }
    (*tokensTail)->token = token;
    (*tokensTail)->next = NULL;
}

struct Token *createToken(
    enum TokenType type, 
    char *text,
    int startOffset,
    int startLine,
    int startChar
) {
    struct Token *token = malloc(sizeof (struct Token));
    token->type = type;
    token->text = text;
    token->startOffset = startOffset;
    token->endOffset = startOffset;
    token->startLine = startLine;
    token->endLine = startLine;
    token->startChar = startChar;
    token->endChar = startChar;
    return token;
}

struct Token *createTokenLong(
    enum TokenType type, 
    char *text,
    int startOffset,
    int endOffset,
    int startLine,
    int endLine,
    int startChar,
    int endChar
) {
    struct Token *token = malloc(sizeof (struct Token));
    token->type = type;
    token->text = text;
    token->startOffset = startOffset;
    token->endOffset = endOffset;
    token->startLine = startLine;
    token->endLine = endLine;
    token->startChar = startChar;
    token->endChar = endChar;
    return token;
}

int tokenize(
    FILE *file, 
    struct TokenList **tokensRetval,
    struct TokenizeErrorInfo *errorInfo
) {
    char chr;
    char buffer[100];
    
    struct TokenList *tokens = NULL;
    struct TokenList *tokensTail = NULL;
    struct Token *token = NULL;
    chr = fgetc(file);
    int i = 0;
    int c = 0;
    int line = 1;
    while (1) {
        if (chr == EOF) {
            break;
        } else if (chr == ' ') {
           // do nothing
        } else if (isDigit(chr)) {
            int startOffset = i;
            int startLine = line;
            int startChar = c;
            // get identifier or keyword
            int j = 0;
            while (1) {
                if (j >= BUFFER_LEN) {
                    errorInfo->offset = i;
                    errorInfo->line = line;
                    errorInfo->character = c;
                    return NumberTooLong;
                }
                buffer[j++] = chr;
                chr = fgetc(file);
                c++;
                i++;
                if (!isDigit(chr)) {
                    break;
                }
            }
            // new id token
            char *text = malloc(sizeof(char) * (j + 1));
            strncpy(text, buffer, j);
            text[j] = 0;
            token = createTokenLong(
                IntLit, text,
                startOffset, i,
                startLine, line,
                startChar, c
            );
            tokenListAppend(&tokens, &tokensTail, token);
            continue;
        } else if (isAlpha(chr)) {
            int startOffset = i;
            int startLine = line;
            int startChar = c;
            int j = 0;
            while (1) {
                if (j >= BUFFER_LEN) {
                    errorInfo->offset = i;
                    errorInfo->line = line;
                    errorInfo->character = c;
                    return IdTooLong;
                }
                buffer[j++] = chr;
                chr = fgetc(file);
                c++;
                i++;
                if (!isAlpha(chr)) {
                    break;
                }
            }
            // new id token
            char *text = malloc(sizeof(char) * (j + 1));
            strncpy(text, buffer, j);
            text[j] = 0;
            token = createTokenLong(
                Id, text,
                startOffset, i,
                startLine, line,
                startChar, c
            );
            tokenListAppend(&tokens, &tokensTail, token);
            continue;
        } else if (chr == '"') {
            int startOffset = i;
            int startLine = line;
            int startChar = c;
            int j = 0;
            chr = fgetc(file);
            c++;
            i++;
            while (1) {
                if (j >= BUFFER_LEN) {
                    errorInfo->offset = i;
                    errorInfo->line = line;
                    errorInfo->character = c;
                    return StrTooLong;
                }
                if (chr == '\\') {
                    chr = fgetc(file);
                    c++;
                    i++;
                    if (chr == '"') {
                        buffer[j++] = chr;
                    } else if (chr == 't') {
                        buffer[j++] = '\t';
                    } else if (chr == 'n') {
                        buffer[j++] = '\n';
                    } else {
                        buffer[j++] = chr;
                    }
                } else if (chr == '"') {
                    chr = fgetc(file);
                    c++;
                    i++;
                    break;
                } else {
                    buffer[j++] = chr;
                }
                chr = fgetc(file);
                c++;
                i++;
            }
            char *text = malloc(sizeof(char) * (j + 1));
            strncpy(text, buffer, j);
            text[j] = 0;
            token = createTokenLong(
                StrLit, text,
                startOffset, i,
                startLine, line,
                startChar, c
            );
            tokenListAppend(&tokens, &tokensTail, token);
            continue;
        } else if (chr == '=') {
            token = createToken(AssignOp, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '(') {
            token = createToken(LeftParan, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == ')') {
            token = createToken(RightParan, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '{') {
            token = createToken(LeftBrace, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '}') {
            token = createToken(RightBrace, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '[') {
            token = createToken(LeftBracket, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == ']') {
            token = createToken(RightBracket, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '<') {
            token = createToken(LessThan, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '>') {
            token = createToken(GreaterThan, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '.') {
            token = createToken(Dot, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == ',') {
            token = createToken(Comma, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '#') {
            int startOffset = i;
            int startLine = line;
            int startChar = c;
            int j = 0;
            chr = fgetc(file);
            i++;
            c++;
            while (1) {
                if (j >= BUFFER_LEN) {
                    errorInfo->offset = i;
                    errorInfo->line = line;
                    errorInfo->character = c;
                    return CommentTooLong;
                }
                if (chr == '\n') {
                    chr = fgetc(file);
                    i++;
                    c++;
                    break;
                } else if (chr == EOF) {
                    break;
                } else {
                    buffer[j++] = chr;
                }
                chr = fgetc(file);
                i++;
                c++;
            }
            char *text = malloc(sizeof(char) * (j + 1));
            strncpy(text, buffer, j);
            text[j] = 0;
            token = createTokenLong(
                Comment, text,
                startOffset, i,
                startLine, line,
                startChar, c
            );
            tokenListAppend(&tokens, &tokensTail, token);
            continue;
        } else if (chr == '\n') {
            line++;
            c = 0;
            token = createToken(Newline, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else {
            errorInfo->offset = i;
            errorInfo->line = line;
            errorInfo->character = c;
            return UnknownChar;
        }
        
        chr = fgetc(file);
        c++;
        i++;
    }
    
    (*tokensRetval) = tokens;
    
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Please provide a file name\n");
        exit(1);
    }
    
    char* filename = argv[1];
    FILE *file = fopen(filename, "r");
    
    printf("Processing %s\n", filename);
    struct TokenList *tokens;
    struct TokenizeErrorInfo errorInfo;
    enum TokenizeErrorType err = tokenize(file, &tokens, &errorInfo);
    if (err != 0) {
        printf("Tokenize error: %d\n", err);
        printf("Line %d, char %d, offset %d\n", errorInfo.line, errorInfo.character, errorInfo.offset);
        exit(1);
    }
    
    while (tokens != NULL) {
        printToken(tokens->token, 1);
        tokens = tokens->next;
    }
    
}