#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum _TokenType {
    IntLit,
    StrLit,
    Id,
    AssignOp,
    AddOp,
    SubtractOp,
    DivideOp,
    MultiplyOp,
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
} TokenType;

typedef enum _TokenizeErrorType {
    LexSuccess = 0,
    IdTooLong,
    NumberTooLong,
    StrTooLong,
    CommentTooLong,
    UnknownChar,
} TokenizeErrorType;

typedef struct _Token {
    TokenType type;
    char *text;
    
    // TODO replace with LocationInfo
    int startOffset;
    int endOffset;
    int startLine;
    int endLine;
    int startChar;
    int endChar;
} Token;

typedef struct _TokenList {
    Token *token;
    struct _TokenList *next;
} TokenList;

typedef struct _TokenizeErrorInfo {
    int offset;
    int line;
    int character;
} TokenizeErrorInfo;

typedef enum _NodeType {
    VarAssign = 1,
    FunCall,
    IntLiteral,
    StrLiteral,
    Identifier,
    TypeIdentifier,
    Program,
    BinaryOp,
} NodeType;

typedef struct _LocationInfo {
    int startOffset;
    int endOffset;
    int startLine;
    int endLine;
    int startChar;
    int endChar;
} LocationInfo;

typedef enum _ParseError {
    ParseSuccess = 0,
    ParseNoMatch,
    ParseUnrecoverable,
    ParseExtraTokens,
} ParseError;

struct VarAssignData {
    struct _Node *varType;
    struct _Node *varName;
    struct _Node *initValue;
};

struct FunCallData {
    struct _Node *funName;
    struct _NodeList *args;
};

struct ProgramData {
    struct _NodeList *statements;
};

struct BinOpData {
    struct _Node *lhs;
    int op; // TokenType that ends in Op
    struct _Node *rhs;
};

typedef struct _Node {
    NodeType type;
    LocationInfo location;
    union {
        struct VarAssignData varAssign;
        struct FunCallData funCall;
        struct ProgramData program;
        struct BinOpData binOp;
        char *id;
        int val;
        char *str;
    } data;
} Node;

typedef struct _NodeList {
    Node *node;
    struct _NodeList *next;
} NodeList;

#define BUFFER_LEN 100

int isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isDigit(char c) {
    return c >= '0' && c <= '9';
}

int isOperator(TokenType type) {
    return type == AddOp ||
        type == SubtractOp ||
        type == DivideOp ||
        type == MultiplyOp;
}

int printToken(Token *token, int details) {
    printf("Token(");
    switch (token->type) {
        case Id:
            printf("ID,");
            break;
        case AssignOp:
            printf("AssignOp");
            break;
        case AddOp:
            printf("AddOp");
            break;
        case SubtractOp:
            printf("SubtractOp");
            break;
        case DivideOp:
            printf("DivideOp");
            break;
        case MultiplyOp:
            printf("MultiplyOp");
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
    TokenList **tokens, 
    TokenList **tokensTail, 
    Token *token
) {
    // Append to the end of tokens linked list
    if (*tokens == NULL) {
        *tokens = malloc(sizeof (TokenList));
        *tokensTail = *tokens;
    } else {
        (*tokensTail)->next = malloc(sizeof (TokenList));
        (*tokensTail) = (*tokensTail)->next;
    }
    (*tokensTail)->token = token;
    (*tokensTail)->next = NULL;
}

Token *createToken(
    TokenType type, 
    char *text,
    int startOffset,
    int startLine,
    int startChar
) {
    Token *token = malloc(sizeof (Token));
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

Token *createTokenLong(
    TokenType type, 
    char *text,
    int startOffset,
    int endOffset,
    int startLine,
    int endLine,
    int startChar,
    int endChar
) {
    Token *token = malloc(sizeof (Token));
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
    TokenList **tokensRetval,
    TokenizeErrorInfo *errorInfo
) {
    char chr;
    char buffer[100];
    
    TokenList *tokens = NULL;
    TokenList *tokensTail = NULL;
    Token *token = NULL;
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
        } else if (chr == '+') {
            token = createToken(AddOp, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '-') {
            token = createToken(SubtractOp, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '/') {
            token = createToken(DivideOp, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
        } else if (chr == '*') {
            token = createToken(MultiplyOp, NULL, i, line, c);
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
            token = createToken(Newline, NULL, i, line, c);
            tokenListAppend(&tokens, &tokensTail, token);
            line++;
            c = 0;
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

int printAST(Node *node, int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    switch (node->type) {
        case VarAssign:
            {
                printf("VarAssign\n");
                struct VarAssignData *data = &(node->data.varAssign);
                printAST(data->varType, level + 1);
                printAST(data->varName, level + 1);
                printAST(data->initValue, level + 1);
                break;
            }
        case FunCall:
            {
                printf("FunCall\n");
                struct FunCallData *data = &(node->data.funCall);
                printAST(data->funName, level + 1);
                NodeList *args = data->args;
                for (int i = 0; i < level + 1; i++) {
                    printf("  ");
                }
                printf("Args:\n");
                while (args != NULL) {
                    printAST(args->node, level + 2);
                    args = args->next;
                }
                break;
            }
        case IntLiteral:
            printf("IntLiteral(%d)\n", node->data.val);
            break;
        case StrLiteral:
            printf("StrLiteral(%s)\n", node->data.str);
            break;
        case Identifier:
            printf("Identifier(%s)\n", node->data.id);
            break;
        case TypeIdentifier:
            printf("TypeIdentifier(%s)\n", node->data.id);
            break;
        case BinaryOp:
            printf("BinaryOp");
            switch (node->data.binOp.op) {
                case AddOp:
                    printf("(+)");
                    break;
                case SubtractOp:
                    printf("(-)");
                    break;
                case DivideOp:
                    printf("(/)");
                    break;
                case MultiplyOp:
                    printf("(*)");
                    break;
                default:
                    printf("(?)");
                    break;
            }
            printf("\n");
            printAST(node->data.binOp.lhs, level + 1);
            printAST(node->data.binOp.rhs, level + 1);
            break;
        case Program:
            printf("Program\n");
            NodeList *statements = node->data.program.statements;
            while (statements != NULL) {
                printAST(statements->node, level + 2);
                statements = statements->next;
            }
            break;
    }
    
    return 0;
}

ParseError parseFunCall(TokenList *tokens, Node **resultNode, TokenList **tokensLeft);
ParseError parseUnaryOp(TokenList *tokens, Node **resultNode, TokenList **tokensLeft);
ParseError parseBinaryOp(TokenList *tokens, Node **resultNode, TokenList **tokensLeft);

ParseError parseExpr(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    return parseBinaryOp(tokens, resultNode, tokensLeft);
}

ParseError parseUnaryOp(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    if (tokens == NULL) {
        return ParseNoMatch;
    }
    
    Node *node;
    if (ParseSuccess == parseFunCall(tokens, &node, tokensLeft)) {
        *resultNode = node;
        return ParseSuccess;
    }
    Token *token = tokens->token;
    if (token->type == IntLit) {
        Node *node = malloc(sizeof (Node));
        node->type = IntLiteral;
        node->data.val = atoi(token->text);
        *resultNode = node;
        *tokensLeft = tokens->next;
        return ParseSuccess;
    } else if (token->type == Id) {
        Node *node = malloc(sizeof (Node));
        node->type = Identifier;
        node->data.id = token->text;
        *resultNode = node;
        *tokensLeft = tokens->next;
        return ParseSuccess;
    } else if (token->type == StrLit) {
        Node *node = malloc(sizeof (Node));
        node->type = StrLiteral;
        node->data.str = token->text;
        *resultNode = node;
        *tokensLeft = tokens->next;
        return ParseSuccess;
    } else {
        return ParseNoMatch;
    }
}

ParseError parseBinaryOp(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    Node *lhs;
    if (ParseSuccess == parseUnaryOp(tokens, &lhs, tokensLeft)) {
        tokens = *tokensLeft;
        if (tokens == NULL || !isOperator(tokens->token->type)) {
            *resultNode = lhs;
            return ParseSuccess;
        }
        TokenType opType = tokens->token->type;
        tokens = tokens->next;
        if (tokens == NULL) return ParseNoMatch;
        Node *rhs;
        if (ParseSuccess == parseBinaryOp(tokens, &rhs, tokensLeft)) {
            Node *ret = malloc(sizeof (Node));
            ret->type = BinaryOp;
            ret->data.binOp.lhs = lhs;
            ret->data.binOp.rhs = rhs;
            ret->data.binOp.op = opType;
            *resultNode = ret;
            return ParseSuccess;
        }
    }
    return ParseNoMatch;
}

ParseError parseVarAssign(
    TokenList *tokens, 
    Node **resultNode, 
    TokenList **tokensLeft
) {
    if (tokens == NULL) {
        return ParseNoMatch;
    }
    Token *typeIdToken = tokens->token;
    if (typeIdToken->type != Id) {
        return ParseNoMatch;
    }
    tokens = tokens->next;
    Token *varNameToken = tokens->token;
    if (varNameToken->type != Id) {
        return ParseNoMatch;
    }
    tokens = tokens->next;
    Token *assignToken = tokens->token;
    if (assignToken->type != AssignOp) {
        return ParseNoMatch;
    }
    tokens = tokens->next;
    Token *initValueToken = tokens->token;
    Node *initValue;
    TokenList *left;
    int result = parseExpr(tokens, &initValue, &left);
    if (result != ParseSuccess) {
        return result;
    }
    
    *tokensLeft = left;
    
    // Create the node
    Node *varType = malloc(sizeof (Node));
    varType->type = TypeIdentifier;
    // TODO copy location info
    // varType->location.startOffset = typeIdToken.startOffset;
    // varType->location.endOffset = typeIdToken.endOffset;
    // varType->location.startLine = typeIdToken.startLine;
    // varType->location.endLine = typeIdToken.endLine;
    // varType->location.startChar = typeIdToken.startChar;
    // varType->location.endChar = typeIdToken.endChar;
    varType->data.id = typeIdToken->text;
    
    Node *varName = malloc(sizeof (Node));
    varName->type = Identifier;
    varName->data.id = varNameToken->text;
    
    Node *varAssign = malloc(sizeof (Node));
    varAssign->type = VarAssign;
    varAssign->data.varAssign.varType = varType;
    varAssign->data.varAssign.varName = varName;
    varAssign->data.varAssign.initValue = initValue;
    
    *resultNode = varAssign;
    return 0;
}

ParseError parseFunCall(
    TokenList *tokens, 
    Node **resultNode,
    TokenList **tokensLeft
) {
    if (tokens == NULL) {
        return ParseNoMatch;
    }
    Token *funName = tokens->token;
    if (funName->type != Id) {
        return ParseNoMatch;
    }
    tokens = tokens->next;
    if (tokens->token->type != LeftParan) {
        return ParseNoMatch;
    }
    
    tokens = tokens->next;
    NodeList *args = NULL;
    NodeList *argsTail = NULL;
    while (1) {
        Node *arg;
        TokenList *left;
        int result = parseExpr(tokens, &arg, &left);
        if (result == ParseSuccess) {
            tokens = left;
            if (tokens == NULL) {
                return ParseNoMatch;
            }
            NodeList *next = malloc(sizeof (NodeList));
            next->node = arg;
            next->next = NULL;
            if (args == NULL) {
                args = next;
                argsTail = next;
            } else {
                argsTail->next = next;
                argsTail = next;
            }
            if (tokens->token->type != Comma) {
                break;
            } else {
                tokens = tokens->next;
            }
        } else {
            return result;
        }
    }
    if (tokens->token->type != RightParan) {
        return ParseNoMatch;
    }
    
    Node *retval = malloc(sizeof (Node));
    retval->type = FunCall;
    Node *funNameNode = malloc(sizeof (Node));
    funNameNode->type = Identifier;
    funNameNode->data.id = funName->text;
    retval->data.funCall.funName = funNameNode;
    retval->data.funCall.args = args;
    *resultNode = retval;
    *tokensLeft = tokens->next;
    return ParseSuccess;
}

ParseError parseStatement(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    if (ParseSuccess == parseVarAssign(tokens, resultNode, tokensLeft)) {
        return ParseSuccess;
    }
    if (ParseSuccess == parseFunCall(tokens, resultNode, tokensLeft)) {
        return ParseSuccess;
    }
    return ParseNoMatch;
}

ParseError parse(TokenList *tokens, Node **resultNode) {
    NodeList *statements = NULL;
    NodeList *statementsTail = NULL;
    while (1) {
        Node *stmtNode;
        TokenList *left;
        if (ParseSuccess == parseStatement(tokens, &stmtNode, &left)) {
            NodeList *next = malloc(sizeof (NodeList));
            next->node = stmtNode;
            next->next = NULL;
            if (statements == NULL) {
                statements = next;
                statementsTail = next;
            } else {
                statementsTail->next = next;
                statementsTail = next;
            }
            tokens = left;
        } else {
            return ParseNoMatch;
        }
        if (tokens == NULL) {
            break;
        }
        if (tokens->token->type == Newline) {
            tokens = tokens->next;
            continue;
        } else {
            break;
        }
    }
    
    Node *program = malloc(sizeof (Node));
    program->type = Program;
    program->data.program.statements = statements;
    *resultNode = program;
    if (tokens != NULL) {
        *resultNode = program;
        return ParseExtraTokens;
    }
    return ParseSuccess;
}

void parseCommand(FILE *file) {
    TokenList *tokens;
    TokenizeErrorInfo errorInfo;
    TokenizeErrorType lexResult = tokenize(file, &tokens, &errorInfo);
    
    if (lexResult != LexSuccess) {
        printf("Lex failed\n");
        return;
    }
    
    Node *resultNode;
    int result = parse(tokens, &resultNode);
    if (result == ParseSuccess) {
        printAST(resultNode, 0);
    } else if (result == ParseExtraTokens) {
        printAST(resultNode, 0);
        printf("Parse failed: extra token(s) at the end\n");
    } else if (result == ParseNoMatch) {
        printf("Parse failed: no match\n");
    } else if (result == ParseUnrecoverable) {
        printf("Parse failed: unrecoverable\n");
    } else {
        printf("Parse failed\n");
    }
}

void lexCommand(FILE *file) {
    TokenList *tokens;
    TokenizeErrorInfo errorInfo;
    TokenizeErrorType err = tokenize(file, &tokens, &errorInfo);
    if (err != 0) {
        printf("Tokenize error: %d\n", err);
        printf("Line %d, char %d, offset %d\n", errorInfo.line, errorInfo.character, errorInfo.offset);
        exit(1);
    }
    
    while (tokens != NULL) {
        printToken(tokens->token, 0);
        tokens = tokens->next;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: pipa <command> <filename>\n");
        printf("  where command is one of: lex and parse\n");
        exit(1);
    }
    
    char* command = argv[1];
    char* filename = argv[2];
    FILE *file = fopen(filename, "r");
    if (strcmp(command, "lex") == 0) {
        lexCommand(file);
    } else if (strcmp(command, "parse") == 0) {
        parseCommand(file);
    }
    fclose(file);
    
    
    
}