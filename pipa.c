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
    EqualOp,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,
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

typedef struct _Location {
    int startOffset;
    int endOffset;
    int startLine;
    int endLine;
    int startChar;
    int endChar;
} Location;

typedef struct _Token {
    TokenType type;
    char *text;
    Location location;
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
    IfStatement,
    LoopStatement,
    BreakStatement,
} NodeType;

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

struct IfStatementData {
    struct _Node *cond;
    struct _NodeList *consequent;
};

struct LoopStatementData {
    struct _NodeList *body;
};

typedef struct _Node {
    NodeType type;
    Location location;
    union {
        struct VarAssignData varAssign;
        struct FunCallData funCall;
        struct ProgramData program;
        struct BinOpData binOp;
        struct IfStatementData ifStatement;
        struct LoopStatementData loopStatement;
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
        type == MultiplyOp ||
        type == EqualOp ||
        type == GreaterThan ||
        type == GreaterThanOrEqual ||
        type == LessThan ||
        type == LessThanOrEqual;
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
        case EqualOp:
            printf("EqualOp");
            break;
        case LessThan:
            printf("LessThan");
            break;
        case LessThanOrEqual:
            printf("LessThanOrEqual");
            break;
        case GreaterThan:
            printf("GreaterThan");
            break;
        case GreaterThanOrEqual:
            printf("GreaterThanOrEqual");
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
            token->location.startOffset, token->location.endOffset,
            token->location.startLine, token->location.endLine,
            token->location.startChar, token->location.endChar
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
    token->location.startOffset = startOffset;
    token->location.endOffset = startOffset;
    token->location.startLine = startLine;
    token->location.endLine = startLine;
    token->location.startChar = startChar;
    token->location.endChar = startChar;
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
    token->location.startOffset = startOffset;
    token->location.endOffset = endOffset;
    token->location.startLine = startLine;
    token->location.endLine = endLine;
    token->location.startChar = startChar;
    token->location.endChar = endChar;
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
        } else if (chr == '=') {
            chr = fgetc(file);
            c++;
            i++;
            if (chr == '=') {
                token = createToken(EqualOp, NULL, i, line, c);
                tokenListAppend(&tokens, &tokensTail, token);
            } else {
                token = createToken(AssignOp, NULL, i, line, c);
                tokenListAppend(&tokens, &tokensTail, token);
                continue;
            }
        } else if (chr == '<') {
            chr = fgetc(file);
            c++;
            i++;
            if (chr == '=') {
                token = createToken(LessThanOrEqual, NULL, i, line, c);
                tokenListAppend(&tokens, &tokensTail, token);
            } else {
                token = createToken(LessThan, NULL, i, line, c);
                tokenListAppend(&tokens, &tokensTail, token);
                continue;
            }
        } else if (chr == '>') {
            chr = fgetc(file);
            c++;
            i++;
            if (chr == '=') {
                token = createToken(GreaterThanOrEqual, NULL, i, line, c);
                tokenListAppend(&tokens, &tokensTail, token);
            } else {
                token = createToken(GreaterThan, NULL, i, line, c);
                tokenListAppend(&tokens, &tokensTail, token);
                continue;
            }
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

void copyLocation(Location *src, Location *dest) {
    memcpy(src, dest, sizeof (Location));
}

void copyLocationStart(Location *src, Location *dest) {
    src->startOffset = dest->startOffset;
    src->startLine = dest->startLine;
    src->startChar = dest->startChar;
}

void copyLocationEnd(Location *src, Location *dest) {
    src->endOffset = dest->endOffset;
    src->endLine = dest->endLine;
    src->endChar = dest->endChar;
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
                case EqualOp:
                    printf("(==)");
                    break;
                case GreaterThan:
                    printf("(>)");
                    break;
                case GreaterThanOrEqual:
                    printf("(>=)");
                    break;
                case LessThan:
                    printf("(<)");
                    break;
                case LessThanOrEqual:
                    printf("(<=)");
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
        case IfStatement:
            printf("IfStatement\n");
            printAST(node->data.ifStatement.cond, level + 2);
            NodeList *consequent = node->data.ifStatement.consequent;
            while (consequent != NULL) {
                printAST(consequent->node, level + 2);
                consequent = consequent->next;
            }
            break;
        case LoopStatement:
            printf("LoopStatement\n");
            NodeList *body = node->data.loopStatement.body;
            while (body != NULL) {
                printAST(body->node, level + 2);
                body = body->next;
            }
            break;
        case BreakStatement:
            printf("BreakStatement\n");
            break;
    }

    return 0;
}

int opPrec(int opType) {
  if (opType == AddOp || opType == SubtractOp) {
    return 1;
  } else if (opType == DivideOp || opType == MultiplyOp) {
    return 2;
  } else {
    return 0;
  }
}

ParseError parseFunCall(TokenList *tokens, Node **resultNode, TokenList **tokensLeft);
ParseError parseUnaryOp(TokenList *tokens, Node **resultNode, TokenList **tokensLeft);
ParseError parseBinaryOp(TokenList *tokens, Node **resultNode, TokenList **tokensLeft);
ParseError parseIfStatement(TokenList *tokens, Node** resultNode, TokenList **tokensLeft);
ParseError parseStatements(TokenList *tokens, NodeList **statementsOut, TokenList **tokensLeft);
ParseError parseLoopStatement(TokenList *tokens, Node **resultNode, TokenList ** tokensLeft);
ParseError parseBreakStatement(TokenList *tokens, Node **resultNode, TokenList ** tokensLeft);

ParseError parseExpr(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    return parseBinaryOp(tokens, resultNode, tokensLeft);
}

ParseError parseUnaryOp(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    if (tokens == NULL) {
        *tokensLeft = tokens;
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
        copyLocation(&token->location, &node->location);
        *resultNode = node;
        *tokensLeft = tokens->next;
        return ParseSuccess;
    } else if (token->type == Id) {
        Node *node = malloc(sizeof (Node));
        node->type = Identifier;
        node->data.id = token->text;
        copyLocation(&token->location, &node->location);
        *resultNode = node;
        *tokensLeft = tokens->next;
        return ParseSuccess;
    } else if (token->type == StrLit) {
        Node *node = malloc(sizeof (Node));
        node->type = StrLiteral;
        node->data.str = token->text;
        copyLocation(&token->location, &node->location);
        *resultNode = node;
        *tokensLeft = tokens->next;
        return ParseSuccess;
    } else {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }
}

ParseError parseBinaryOp(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    Node *lhs;
    if (ParseSuccess != parseUnaryOp(tokens, &lhs, tokensLeft)) {
        return ParseNoMatch;
    }
    tokens = *tokensLeft;
    if (tokens == NULL || !isOperator(tokens->token->type)) {
        *resultNode = lhs;
        return ParseSuccess;
    }
    TokenType op = tokens->token->type;
    tokens = tokens->next;
    if (tokens == NULL) {
        *tokensLeft = NULL;
        return ParseNoMatch;
    }
    Node *rhs;
    if (ParseSuccess != parseBinaryOp(tokens, &rhs, tokensLeft)) {
        return ParseNoMatch;
    }
    Node *ret = malloc(sizeof (Node));

    copyLocationStart(&lhs->location, &ret->location);
    copyLocationEnd(&rhs->location, &ret->location);
    
    if (rhs->type == BinaryOp) {
        int rhsOp = rhs->data.binOp.op;
        if (opPrec(op) > opPrec(rhsOp)) {
            // Reshape the tree
            Node *newLhs = malloc(sizeof (Node));
            newLhs->type = BinaryOp;
            newLhs->data.binOp.lhs = lhs;
            newLhs->data.binOp.op = op;
            newLhs->data.binOp.rhs = rhs->data.binOp.lhs;
            ret->type = BinaryOp;
            ret->data.binOp.lhs = newLhs;
            ret->data.binOp.op = rhs->data.binOp.op;
            ret->data.binOp.rhs = rhs->data.binOp.rhs;
            free(rhs);
            *resultNode = ret;
            return ParseSuccess;
        }
    }
    ret->type = BinaryOp;
    ret->data.binOp.lhs = lhs;
    ret->data.binOp.rhs = rhs;
    ret->data.binOp.op = op;
    *resultNode = ret;
    return ParseSuccess;
}

ParseError parseVarAssign(
    TokenList *tokens,
    Node **resultNode,
    TokenList **tokensLeft
) {
    if (tokens == NULL) {
        *tokensLeft = NULL;
        return ParseNoMatch;
    }
    Token *typeIdToken = tokens->token;
    if (typeIdToken->type != Id) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }
    tokens = tokens->next;
    Token *varNameToken = tokens->token;
    if (varNameToken->type != Id) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }
    tokens = tokens->next;
    Token *assignToken = tokens->token;
    if (assignToken->type != AssignOp) {
        *tokensLeft = tokens;
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
    Node *varAssign = malloc(sizeof (Node));
    copyLocationStart(&typeIdToken->location, &varAssign->location);
    copyLocationEnd(&initValue->location, &varAssign->location);
    
    Node *varType = malloc(sizeof (Node));
    varType->type = TypeIdentifier;
    copyLocation(&typeIdToken->location, &varType->location);
    varType->data.id = typeIdToken->text;

    Node *varName = malloc(sizeof (Node));
    varName->type = Identifier;
    copyLocation(&varNameToken->location, &varName->location);
    varName->data.id = varNameToken->text;

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
        *tokensLeft = NULL;
        return ParseNoMatch;
    }
    Token *funName = tokens->token;
    if (funName->type != Id) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }
    tokens = tokens->next;
    if (tokens == NULL || tokens->token->type != LeftParan) {
        *tokensLeft = tokens;
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
                *tokensLeft = NULL;
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
        // TODO: free args
        *tokensLeft = tokens;
        return ParseNoMatch;
    }

    Node *retval = malloc(sizeof (Node));
    copyLocationStart(&funName->location, &retval->location);
    copyLocationEnd(&argsTail->node->location, &retval->location);
    retval->type = FunCall;
    Node *funNameNode = malloc(sizeof (Node));
    funNameNode->type = Identifier;
    copyLocation(&funName->location, &funNameNode->location);
    funNameNode->data.id = funName->text;
    retval->data.funCall.funName = funNameNode;
    retval->data.funCall.args = args;
    *resultNode = retval;
    *tokensLeft = tokens->next;
    return ParseSuccess;
}

ParseError parseIfStatement(TokenList *tokens, Node** resultNode, TokenList **tokensLeft) {
    if (tokens == NULL) {
        *tokensLeft = NULL;
        return ParseNoMatch;
    }

    Token *ifKeyword = tokens->token;
    if (ifKeyword->type != Id) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }
    if (strcmp(ifKeyword->text, "if") != 0) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }
    tokens = tokens->next;
    Node *cond;
    if (ParseSuccess !=  parseExpr(tokens, &cond, tokensLeft)) {
        return ParseNoMatch;
    }
    tokens = *tokensLeft;
    if (tokens == NULL) {
        return ParseNoMatch;
    }
    if (tokens->token->type != LeftBrace) {
        return ParseNoMatch;
    }
    tokens = tokens->next;

    NodeList *statements;
    if (ParseSuccess != parseStatements(tokens, &statements, tokensLeft)) {
        return ParseNoMatch;
    }
    tokens = *tokensLeft;
    if (tokens == NULL || tokens->token->type != RightBrace) {
        return ParseNoMatch;
    }
    Token *rightBrace = tokens->token;
    tokens = tokens->next;
    *tokensLeft = tokens;
    Node *retval = malloc(sizeof (Node));
    copyLocationStart(&ifKeyword->location, &retval->location);
    copyLocationEnd(&rightBrace->location, &retval->location);
    retval->type = IfStatement;
    retval->data.ifStatement.cond = cond;
    retval->data.ifStatement.consequent = statements;
    *resultNode = retval;
    return ParseSuccess;
}

ParseError parseLoopStatement(TokenList *tokens, Node **resultNode, TokenList ** tokensLeft) {
    if (tokens == NULL) {
        *tokensLeft = NULL;
        return ParseNoMatch;
    }

    Token *loopKeyword = tokens->token;
    if (loopKeyword->type != Id) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }

    if (strcmp(loopKeyword->text, "loop") != 0) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }

    tokens = tokens->next;
    if (tokens->token->type != LeftBrace) {
        return ParseNoMatch;
    }
    tokens = tokens->next;

    NodeList *statements;
    if (ParseSuccess != parseStatements(tokens, &statements, tokensLeft)) {
        return ParseNoMatch;
    }
    tokens = *tokensLeft;
    if (tokens == NULL || tokens->token->type != RightBrace) {
        return ParseNoMatch;
    }
    Token *rightBrace = tokens->token;
    tokens = tokens->next;
    *tokensLeft = tokens;
    Node *retval = malloc(sizeof (Node));
    copyLocationStart(&loopKeyword->location, &retval->location);
    copyLocationEnd(&rightBrace->location, &retval->location);
    retval->type = LoopStatement;
    retval->data.loopStatement.body = statements;
    *resultNode = retval;
    return ParseSuccess;
}

ParseError parseBreakStatement(TokenList *tokens, Node **resultNode, TokenList ** tokensLeft) {
    if (tokens == NULL) {
        *tokensLeft = NULL;
        return ParseNoMatch;
    }

    Token *breakKeyword = tokens->token;
    if (breakKeyword->type != Id) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }

    if (strcmp(breakKeyword->text, "break") != 0) {
        *tokensLeft = tokens;
        return ParseNoMatch;
    }
    *tokensLeft = tokens->next;

    Node *retval = malloc(sizeof (Node));
    retval->type = BreakStatement;
    copyLocationStart(&breakKeyword->location, &retval->location);
    copyLocationEnd(&breakKeyword->location, &retval->location);
    *resultNode = retval;
    return ParseSuccess;
}

ParseError parseStatement(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    if (ParseSuccess == parseVarAssign(tokens, resultNode, tokensLeft)) {
        return ParseSuccess;
    }
    if (ParseSuccess == parseFunCall(tokens, resultNode, tokensLeft)) {
        return ParseSuccess;
    }
    if (ParseSuccess == parseIfStatement(tokens, resultNode, tokensLeft)) {
        return ParseSuccess;
    }
    if (ParseSuccess == parseLoopStatement(tokens, resultNode, tokensLeft)) {
        return ParseSuccess;
    }
    if (ParseSuccess == parseBreakStatement(tokens, resultNode, tokensLeft)) {
        return ParseSuccess;
    }
    return ParseNoMatch;
}

ParseError parseStatements(TokenList *tokens, NodeList **statementsOut, TokenList **tokensLeft) {
    NodeList *statements = NULL;
    NodeList *statementsTail = NULL;
    while (tokens->token->type == Newline) {
        tokens = tokens->next;
    }
    while (1) {
        if (tokens != NULL && RightBrace == tokens->token->type) {
            break;
        }
        Node *stmtNode;
        TokenList *stmtTokensLeft;
        if (ParseSuccess != parseStatement(tokens, &stmtNode, &stmtTokensLeft)) {
            return ParseNoMatch;
        }
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
        tokens = stmtTokensLeft;
        while (tokens != NULL && tokens->token->type == Newline) {
            tokens = tokens->next;
        }
        if (tokens == NULL) {
            break;
        }
    }
    (*tokensLeft) = tokens;
    (*statementsOut) = statements;
    return ParseSuccess;
}

ParseError parse(TokenList *tokens, Node **resultNode, TokenList **tokensLeft) {
    NodeList *statements = NULL;
    if (ParseSuccess != parseStatements(tokens, &statements, tokensLeft)) {
        return ParseNoMatch;
    }

    Node *program = malloc(sizeof (Node));
    program->type = Program;
    program->data.program.statements = statements;
    *resultNode = program;
    if (*tokensLeft != NULL) {
        *resultNode = program;
        return ParseExtraTokens;
    }
    return ParseSuccess;
}

void reportParseError(char *filename, int parseResult, TokenList *tokensLeft) {
    printf("Parse error:\n");
    Token *token = tokensLeft == NULL ? NULL : tokensLeft->token;
    FILE *file = fopen(filename, "r");
    char *line = NULL;
    size_t lineCap = 0;
    int lineNo = 1;
    printf("Unexpected ");
    if (token == NULL) {
        char *lastLine = NULL;
        printf("end of file\n");
        while (1) {
            int read = getline(&line, &lineCap, file);
            if (read == -1) {
                break;
            }
            if (lastLine != NULL) {
                free(lastLine);
                lastLine = NULL;
            }
            lastLine = malloc(read);
            strcpy(lastLine, line);
            lineNo++;
        }
        printf("%*d  %s\n", 3, lineNo, lastLine);
        free(lastLine);
        printf("     ");
        for (int i = 0; i < strlen(lastLine); i++) {
            printf(" ");
        }
        printf("^\n");
    } else {
        printToken(token, 0);
        int printMore = 0;
        while (1) {
            int read = getline(&line, &lineCap, file);
            if (read == -1) {
                break;
            }
            if (printMore > 0) {
                printf("%*d  %s", 3, lineNo, line);
                printMore--;
            }
            if (token != NULL && token->location.startLine == lineNo) {
                printf("%*d  %s", 3, lineNo, line);
                printf("     ");
                for (int i = 1; i < token->location.startChar; i++) {
                    printf(" ");
                }
                printf("^\n");
                printMore = 4;
            }
            lineNo++;
        }
        printf("\n");
    }
    fclose(file);
}

void parseCommand(char *filename) {
    FILE *file = fopen(filename, "r");
    TokenList *tokens;
    TokenizeErrorInfo errorInfo;
    TokenizeErrorType lexResult = tokenize(file, &tokens, &errorInfo);
    fclose(file);
    if (lexResult != LexSuccess) {
        printf("Lex failed\n");
        return;
    }

    Node *resultNode;
    TokenList *tokensLeft;
    
    int result = parse(tokens, &resultNode, &tokensLeft);
    if (result == ParseSuccess) {
        printAST(resultNode, 0);
    } else {
        reportParseError(filename, result, tokensLeft);
    }
    
}

void lexCommand(char *filename) {
    FILE *file = fopen(filename, "r");
    TokenList *tokens;
    TokenizeErrorInfo errorInfo;
    TokenizeErrorType err = tokenize(file, &tokens, &errorInfo);
    fclose(file);
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
    if (file == NULL) {
        printf("Failed to open %s\n", filename);
        exit(1);
    }
    if (strcmp(command, "lex") == 0) {
        lexCommand(filename);
    } else if (strcmp(command, "parse") == 0) {
        parseCommand(filename);
    }
}
