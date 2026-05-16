#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> 

/* =====================================================================
 * 1. 词法分析 (Lexical Analysis)
 * ===================================================================== */

typedef enum {
    TOK_INT, TOK_FLOAT, TOK_IF, TOK_ELSE, TOK_WHILE, TOK_PRINT, 
    TOK_ID, TOK_NUM, TOK_CHAR, TOK_STR,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_LESS, TOK_GREATER, TOK_EQ, 
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE, TOK_SEMI,  
    TOK_LBRACKET, TOK_RBRACKET, TOK_DOT, TOK_COMMA,
    TOK_EOF, TOK_ERROR, TOK_STRUCT, TOK_STAR, 
    TOK_VOID, TOK_MAIN 
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[100]; 
    char category[30]; 
    int line;          
} Token;

char src[10000]; 
int pos = 0;
int currentLine = 1; 

typedef struct { 
    const char* str; 
    TokenType type; 
} KeywordMap;

KeywordMap kwDict[] = {
    {"int", TOK_INT}, {"float", TOK_FLOAT}, {"real", TOK_FLOAT}, {"char", TOK_CHAR},
    {"struct", TOK_STRUCT}, {"void", TOK_VOID}, {"main", TOK_MAIN}, 
    {"if", TOK_IF}, {"else", TOK_ELSE}, {"while", TOK_WHILE}, {"printf", TOK_PRINT} 
};

void skipWhitespace() {
    while (pos < 10000 && src[pos] != '\0' && isspace((unsigned char)src[pos])) {
        if (src[pos] == '\n') currentLine++; 
        pos++;
    }
}

TokenType checkKeyword(const char* str) {
    for(size_t i = 0; i < sizeof(kwDict)/sizeof(KeywordMap); i++) {
        if(strcmp(str, kwDict[i].str) == 0) return kwDict[i].type;
    }
    return TOK_ID; 
}

Token getNextToken() {
    Token t;
    memset(&t, 0, sizeof(Token));
    skipWhitespace();
    t.line = currentLine;
    
    if (src[pos] == '\0' || pos >= 10000) { 
        t.type = TOK_EOF; 
        strncpy(t.lexeme, "#", sizeof(t.lexeme) - 1); 
        strncpy(t.category, "Acc", sizeof(t.category) - 1); 
        return t; 
    }
    
    if (src[pos] == '"') {
        int i = 0;
        t.lexeme[i++] = src[pos++];
        while (pos < 10000 && src[pos] != '"' && src[pos] != '\0' && i < 98) {
            t.lexeme[i++] = src[pos++];
        }
        if (src[pos] == '"') t.lexeme[i++] = src[pos++];
        t.lexeme[i] = '\0';
        t.type = TOK_STR;
        strncpy(t.category, "String", sizeof(t.category) - 1);
        return t;
    }
    
    char c = src[pos];
    
    if (isalpha((unsigned char)c) || c == '_') {
        int i = 0;
        while ((isalnum((unsigned char)src[pos]) || src[pos] == '_') && i < 99) {
            t.lexeme[i++] = src[pos++];
        }
        t.lexeme[i] = '\0';
        
        t.type = checkKeyword(t.lexeme);
        if (t.type != TOK_ID) strncpy(t.category, "Keyword", sizeof(t.category) - 1);
        else strncpy(t.category, "Name", sizeof(t.category) - 1);
        return t;
    }
    
    if (isdigit((unsigned char)c)) {
        int i = 0;
        int isFloat = 0;
        if (src[pos] == '0' && (src[pos+1] == 'x' || src[pos+1] == 'X')) {
            t.lexeme[i++] = src[pos++]; 
            t.lexeme[i++] = src[pos++]; 
            while (isxdigit((unsigned char)src[pos]) && i < 99) t.lexeme[i++] = src[pos++];
            t.lexeme[i] = '\0'; 
            t.type = TOK_NUM; 
            strncpy(t.category, "Number.Hex", sizeof(t.category) - 1); 
            return t;
        }
        while ((isdigit((unsigned char)src[pos]) || src[pos] == '.') && i < 99) {
            if (src[pos] == '.') isFloat = 1;
            t.lexeme[i++] = src[pos++];
        }
        t.lexeme[i] = '\0'; 
        t.type = TOK_NUM;
        if (isFloat) strncpy(t.category, "Number.Float", sizeof(t.category) - 1);
        else strncpy(t.category, "Number.Integer", sizeof(t.category) - 1);
        return t;
    }
    
    t.lexeme[0] = src[pos]; 
    t.lexeme[1] = '\0'; 
    pos++;
    
    switch(c) {
        case '=': 
            if (src[pos] == '=') { t.type = TOK_EQ; strncat(t.lexeme, "=", sizeof(t.lexeme) - strlen(t.lexeme) - 1); pos++; }
            else { t.type = TOK_ASSIGN; }
            strncpy(t.category, "Operator", sizeof(t.category) - 1); break;
        case '<': 
            if (src[pos] == '=') { t.type = TOK_LESS; strncat(t.lexeme, "=", sizeof(t.lexeme) - strlen(t.lexeme) - 1); pos++; }
            else { t.type = TOK_LESS; }
            strncpy(t.category, "Operator", sizeof(t.category) - 1); break;
        case '>': 
            if (src[pos] == '=') { t.type = TOK_GREATER; strncat(t.lexeme, "=", sizeof(t.lexeme) - strlen(t.lexeme) - 1); pos++; }
            else { t.type = TOK_GREATER; }
            strncpy(t.category, "Operator", sizeof(t.category) - 1); break;
        case '+': t.type = TOK_PLUS; strncpy(t.category, "Operator", sizeof(t.category) - 1); break;
        case '-': t.type = TOK_MINUS; strncpy(t.category, "Operator", sizeof(t.category) - 1); break;
        case '(': t.type = TOK_LPAREN; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        case ')': t.type = TOK_RPAREN; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        case '{': t.type = TOK_LBRACE; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        case '}': t.type = TOK_RBRACE; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        case ';': t.type = TOK_SEMI; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        case '*': t.type = TOK_STAR; strncpy(t.category, "Operator", sizeof(t.category) - 1); break;
        case '[': t.type = TOK_LBRACKET; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        case ']': t.type = TOK_RBRACKET; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        case '.': t.type = TOK_DOT; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        case ',': t.type = TOK_COMMA; strncpy(t.category, "Punctuation", sizeof(t.category) - 1); break;
        default:  t.type = TOK_ERROR; strncpy(t.category, "Error", sizeof(t.category) - 1); break;
    }
    return t;
}

void printLexerResult() {
    pos = 0; currentLine = 1;
    printf("\n--- 词法分析结果 ---\n");
    Token t;
    do {
        t = getNextToken();
        if (t.type != TOK_EOF) printf("(%d,%-15s,%s)\n", t.line, t.category, t.lexeme);
    } while (t.type != TOK_EOF);
}

/* =====================================================================
 * 2. 语法分析 (LL(1) Parser & SDT Engine - Auto @ Injection)
 * ===================================================================== */
Token tokens[2000];
int tCount = 0;

char ruleStr[100][100];      
char ruleLHS[100][20];      
char rhsItems[100][10][20];  
int numRules = 0;

int ll1Matrix[100][100];      
char NTs[100][20];           
int ntCount = 0;

typedef struct { const char* str; TokenType type; } TokenMap;
TokenMap tokenDict[] = {
    {"int", TOK_INT}, {"float", TOK_FLOAT}, {"char", TOK_CHAR},
    {"if", TOK_IF}, {"else", TOK_ELSE}, {"while", TOK_WHILE}, {"printf", TOK_PRINT},
    {"id", TOK_ID}, {"num", TOK_NUM}, {"str", TOK_STR},
    {"=", TOK_ASSIGN}, {"+", TOK_PLUS}, {"-", TOK_MINUS},
    {"<", TOK_LESS}, {">", TOK_GREATER}, {"==", TOK_EQ},
    {"(", TOK_LPAREN}, {")", TOK_RPAREN}, {"{", TOK_LBRACE}, {"}", TOK_RBRACE},
    {"[", TOK_LBRACKET}, {"]", TOK_RBRACKET}, {".", TOK_DOT}, {",", TOK_COMMA},
    {";", TOK_SEMI}, {"#", TOK_EOF}, {"struct", TOK_STRUCT}, {"*", TOK_STAR},
    {"void", TOK_VOID}, {"main", TOK_MAIN} 
};

// [LOGIC ẨN]: Tự động sinh mô tả thuộc tính
void getActionDesc(int ruleIdx, char* desc) {
    const char* lhs = ruleLHS[ruleIdx];
    const char* str = ruleStr[ruleIdx];
    
    if (strstr(str, "e") && strlen(str) <= 6) sprintf(desc, "空归约 (无属性)");
    else if (strcmp(lhs, "Y") == 0) {
        char typeName[10] = "";
        sscanf(str, "Y -> %s", typeName);
        sprintf(desc, "Y.type=%s", typeName);
    }
    else if (strcmp(lhs, "V") == 0) sprintf(desc, "V.type=Y.type");
    else if (strcmp(lhs, "A") == 0) sprintf(desc, "构建语句 AST");
    else if (strcmp(lhs, "Q") == 0 || strcmp(lhs, "W") == 0 || strcmp(lhs, "O") == 0) {
        if (strstr(str, "=")) sprintf(desc, "emit('=')");
        else sprintf(desc, "属性传递");
    }
    else if (strcmp(lhs, "T") == 0) {
        if (strstr(str, "- num")) sprintf(desc, "T.place=-num.val");
        else if (strstr(str, "num")) sprintf(desc, "T.place=num.val");
        else sprintf(desc, "T.place=id.place");
    }
    else if (strcmp(lhs, "E") == 0 || strcmp(lhs, "N") == 0) sprintf(desc, "%s.place 传递", lhs);
    else sprintf(desc, "属性传递");
}

void printCell(const char* str, int targetWidth) {
    int w = 0;
    for (int i = 0; str[i]; ) {
        int charWidth = 1, charBytes = 1;
        if ((unsigned char)str[i] >= 0xE0) { charWidth = 2; charBytes = 3; } 
        else if ((unsigned char)str[i] >= 0xC0) { charWidth = 2; charBytes = 2; }
        if (w + charWidth > targetWidth) break;
        for(int b=0; b<charBytes; b++) printf("%c", str[i+b]);
        w += charWidth;
        i += charBytes;
    }
    while (w < targetWidth) { printf(" "); w++; }
}

int getNTId(const char* nt) {
    for(int i=0; i<ntCount; i++) { if(strcmp(NTs[i], nt) == 0) return i; }
    if (ntCount < 100) { strncpy(NTs[ntCount], nt, sizeof(NTs[ntCount]) - 1); return ntCount++; }
    return 0;
}

TokenType strToToken(const char* sym) {
    for (size_t i = 0; i < sizeof(tokenDict)/sizeof(TokenMap); i++) {
        if (strcmp(sym, tokenDict[i].str) == 0) return tokenDict[i].type;
    }
    return TOK_ERROR;
}

int isTerminal(const char* sym) {
    if (sym[0] == '@') return 1; 
    return strToToken(sym) != TOK_ERROR || strcmp(sym, "e") == 0;
}

int getFirst(const char* symbol, int* firstSet) {
    if (symbol[0] == '@') return 1; // @ Kích hoạt Empty rỗng để tính FOLLOW
    if (isTerminal(symbol)) {
        if (strcmp(symbol, "e") == 0) return 1;
        TokenType t = strToToken(symbol);
        if(t != TOK_ERROR) firstSet[t] = 1;
        return 0;
    }
    int hasEpsilon = 0;
    for(int i = 1; i <= numRules; i++) {
        if (strcmp(ruleLHS[i], symbol) == 0) {
            int prodHasEpsilon = 1;
            for(int j=0; strlen(rhsItems[i][j]) > 0; j++) {
                int subSet[50] = {0};
                int e = getFirst(rhsItems[i][j], subSet);
                for(int t=0; t<50; t++) if(subSet[t]) firstSet[t] = 1;
                if (!e) { prodHasEpsilon = 0; break; }
            }
            if (prodHasEpsilon) hasEpsilon = 1;
        }
    }
    return hasEpsilon;
}

void getFollow(const char* nt, int* followSet, int* visited) {
    int ntId = getNTId(nt);
    if(visited[ntId]) return; visited[ntId] = 1;
    if (strcmp(nt, "S") == 0) followSet[TOK_EOF] = 1; 
    
    for (int i = 1; i <= numRules; i++) {
        for (int j = 0; strlen(rhsItems[i][j]) > 0; j++) {
            if (strcmp(rhsItems[i][j], nt) == 0) {
                int k = j + 1; int allHaveEpsilon = 1;
                while (strlen(rhsItems[i][k]) > 0) {
                    if (!getFirst(rhsItems[i][k], followSet)) { allHaveEpsilon = 0; break; }
                    k++;
                }
                if (allHaveEpsilon && strcmp(ruleLHS[i], nt) != 0) getFollow(ruleLHS[i], followSet, visited);
            }
        }
    }
}

// [CẬP NHẬT]: LL(1) Matrix Construction bảo vệ luật 'else'
void buildLL1Matrix() {
    memset(ll1Matrix, 0, sizeof(ll1Matrix)); 
    for (int i = 1; i <= numRules; i++) {
        int ntId = getNTId(ruleLHS[i]);
        int firstSet[50] = {0};
        for(int j=0; strlen(rhsItems[i][j]) > 0; j++) {
            int subSet[50] = {0};
            if (!getFirst(rhsItems[i][j], subSet)) {
                for(int t=0; t<50; t++) if(subSet[t]) firstSet[t] = 1;
                break;
            }
            for(int t=0; t<50; t++) if(subSet[t]) firstSet[t] = 1;
        }
        for (int t = 0; t < 50; t++) if (firstSet[t]) ll1Matrix[ntId][t] = i;
    }

    for (int i = 1; i <= numRules; i++) {
        int ntId = getNTId(ruleLHS[i]);
        int hasEpsilon = 1;
        for(int j=0; strlen(rhsItems[i][j]) > 0; j++) {
            int subSet[50] = {0};
            if (!getFirst(rhsItems[i][j], subSet)) { hasEpsilon = 0; break; }
        }
        if (hasEpsilon) {
            int followSet[50] = {0}, visited[100] = {0};
            getFollow(ruleLHS[i], followSet, visited);
            for (int t = 0; t < 50; t++) {
                if (followSet[t] && ll1Matrix[ntId][t] == 0) ll1Matrix[ntId][t] = i;
            }
        }
    }
}

void loadGrammar() {
    FILE *file = fopen("grammar.txt", "r");
    if (!file) { printf("\n[ERROR] 无法打开 grammar.txt 文件！\n"); exit(1); }
    char line[256]; int ruleIdx = 1;
    while (fgets(line, sizeof(line), file) && ruleIdx < 100) {
        line[strcspn(line, "\r\n")] = '\0'; 
        if (strlen(line) == 0) continue;
        
        strncpy(ruleStr[ruleIdx], line, sizeof(ruleStr[ruleIdx]) - 1); 
        char *arrow = strstr(line, "->");
        if (arrow) {
            int len = arrow - line;
            if(len > 19) len = 19;
            strncpy(ruleLHS[ruleIdx], line, len);
            ruleLHS[ruleIdx][len] = '\0';
            while(len > 0 && isspace((unsigned char)ruleLHS[ruleIdx][len-1])) ruleLHS[ruleIdx][--len] = '\0';

            char *rhs = arrow + 2; int c = 0; char *token = strtok(rhs, " ");
            int hasEpsilon = 0;
            while (token && c < 8) { 
                if (strcmp(token, "e") == 0) {
                    hasEpsilon = 1; // Đánh dấu đây là luật có Epsilon
                } else {
                    strncpy(rhsItems[ruleIdx][c++], token, sizeof(rhsItems[ruleIdx][c]) - 1); 
                }
                token = strtok(NULL, " "); 
            }
            
            // [CẬP NHẬT MỚI]: Thêm @ vào tất cả mọi luật sinh
            char actionLabel[10];
            snprintf(actionLabel, sizeof(actionLabel), "@%d", ruleIdx);
            
            if (hasEpsilon) {
                // Thay thế rỗng bằng @N trực tiếp
                strcpy(rhsItems[ruleIdx][0], actionLabel);
                strcpy(rhsItems[ruleIdx][1], ""); 
            } else {
                // Gắn @N vào cuối chuỗi
                strncpy(rhsItems[ruleIdx][c++], actionLabel, sizeof(rhsItems[ruleIdx][c]) - 1);
                strcpy(rhsItems[ruleIdx][c], ""); 
            }
        }
        ruleIdx++;
    }
    numRules = ruleIdx - 1; 
    fclose(file);
    buildLL1Matrix();
}

void fetchTokens() {
    pos = 0; currentLine = 1; tCount = 0; Token t;
    do { 
        t = getNextToken(); 
        if (tCount < 2000) tokens[tCount++] = t; 
    } while(t.type != TOK_EOF);
}

int getRule(const char* nt, TokenType t) {
    int ntId = -1;
    for(int i=0; i<ntCount; i++) { if(strcmp(NTs[i], nt) == 0) { ntId = i; break; } }
    if (ntId == -1 || t < 0 || t >= 50) return 0;
    return ll1Matrix[ntId][t];
}

void printLL1Table() {
    printf("\n表1 文法G[S]的部分LL(1)分析表 (包含 @ 语义动作)\n");
    printf("----------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    const char* colNames[] = {"int", "float", "id", "num", "=", "struct", "void", "main", "else", "*", "-", "#"};
    int numCols = 12;

    printf(" "); printCell("", 4); printf(" |");
    for(int j=0; j<numCols; j++) { printf(" "); printCell(colNames[j], 11); printf(" |"); }
    printf("\n----------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    for(int i=0; i<ntCount; i++) {
        int hasRule = 0;
        for(int j=0; j<numCols; j++) {
            TokenType t = strToToken(colNames[j]);
            if(t != TOK_ERROR && ll1Matrix[i][t] > 0) hasRule = 1;
        }
        
        if (hasRule) {
            printf(" "); printCell(NTs[i], 4); printf(" |"); 
            for(int j=0; j<numCols; j++) {
                TokenType t = strToToken(colNames[j]);
                int ruleId = (t != TOK_ERROR) ? ll1Matrix[i][t] : 0;
                if (ruleId > 0) {
                    // [ĐÃ HIỂN THỊ @ VÀO BẢNG 1]: Nén chuỗi có bao gồm cả ký hiệu @
                    char shortRule[40] = "";
                    snprintf(shortRule, sizeof(shortRule), "%s->", ruleLHS[ruleId]);
                    for(int k=0; strlen(rhsItems[ruleId][k]) > 0; k++) {
                        strcat(shortRule, rhsItems[ruleId][k]);
                    }
                    printf(" "); printCell(shortRule, 11); printf(" |");
                } else {
                    printf(" "); printCell("", 11); printf(" |");
                }
            }
            printf("\n");
        }
    }
    printf("----------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

void runSyntaxProcess() {
    fetchTokens();
    char stack[100][20]; int top = -1;
    strcpy(stack[++top], "#"); strcpy(stack[++top], "S");
    
    int step = 0; int ip = 0;
    
    printf("\n表2 符号串的LL(1)分析过程及语义动作\n");
    printf("-----------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    printf(" "); printCell("步骤", 5); printf(" | ");
    printCell("分析栈", 50); printf(" | ");  // Cột Stack được mở rộng khổng lồ để chứa các ký tự @
    printCell("符号串", 35); printf(" | ");
    printCell("产生式", 25); printf(" | ");
    printCell("下一步动作", 35); printf("\n");
    printf("-----------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    while(top >= 0 && ip < tCount) {
        char X[20]; strncpy(X, stack[top], sizeof(X) - 1); X[sizeof(X) - 1] = '\0';
        Token a = tokens[ip];
        
        char stkStr[300] = ""; 
        // [CẬP NHẬT BẢNG 2]: Cho phép in toàn bộ các ký hiệu @ đang bị dồn lại trong ngăn xếp
        for(int i=0; i<=top; i++) { 
            strncat(stkStr, stack[i], sizeof(stkStr) - strlen(stkStr) - 1); 
        }
        
        char inStr[200] = ""; 
        for(int i=ip; i<tCount && i<ip+8; i++) {
            if (tokens[i].type == TOK_ID) strncat(inStr, "id ", sizeof(inStr) - strlen(inStr) - 1);
            else if (tokens[i].type == TOK_NUM) strncat(inStr, "num ", sizeof(inStr) - strlen(inStr) - 1);
            else if (tokens[i].type == TOK_INT) strncat(inStr, "int ", sizeof(inStr) - strlen(inStr) - 1);
            else if (tokens[i].type == TOK_FLOAT) strncat(inStr, "float ", sizeof(inStr) - strlen(inStr) - 1);
            else { strncat(inStr, tokens[i].lexeme, sizeof(inStr) - strlen(inStr) - 1); strncat(inStr, " ", sizeof(inStr) - strlen(inStr) - 1); }
        }
        if (ip + 8 < tCount) strncat(inStr, "...", sizeof(inStr) - strlen(inStr) - 1);

        char stepStr[10]; snprintf(stepStr, sizeof(stepStr), "%d", step++);
        char action[150] = "";
        char prodStr[50] = "";

        if (isTerminal(X)) {
            if (X[0] == '@') {
                int rId = atoi(X + 1);
                char desc[100] = "";
                getActionDesc(rId, desc);
                strcpy(prodStr, ""); // Bỏ trống cột sản xuất khi thực thi Action
                snprintf(action, sizeof(action), "执行%s，%s", X, desc); 
                
                printf(" "); printCell(stepStr, 5); printf(" | ");
                printCell(stkStr, 50); printf(" | ");
                printCell(inStr, 35); printf(" | ");
                printCell(prodStr, 25); printf(" | ");
                printCell(action, 35); printf("\n");
                top--;
            }
            else {
                TokenType expectedType = strToToken(X);
                if (expectedType != TOK_ERROR && expectedType == a.type) {
                    if (strcmp(X, "#") == 0) {
                        strcpy(prodStr, "acc");
                        strcpy(action, "分析成功");
                    } else {
                        strcpy(prodStr, "");
                        snprintf(action, sizeof(action), "%s匹配", X); 
                    }
                    
                    printf(" "); printCell(stepStr, 5); printf(" | ");
                    printCell(stkStr, 50); printf(" | ");
                    printCell(inStr, 35); printf(" | ");
                    printCell(prodStr, 25); printf(" | ");
                    printCell(action, 35); printf("\n");
                    top--; ip++;
                } else { 
                    printf("\n[ERROR] 语法错误！期望 '%s' 但找到 '%s' (行 %d)\n", X, a.lexeme, a.line); 
                    return; 
                }
            }
        } else {
            int rule = getRule(X, a.type);
            if (rule > 0) {
                top--;
                int count = 0; 
                char revPush[100] = "";
                while(strlen(rhsItems[rule][count]) > 0 && count < 10) count++;
                
                for(int i = count-1; i >= 0; i--) {
                    if (top < 99) {
                        strncpy(stack[++top], rhsItems[rule][i], sizeof(stack[top]) - 1);
                        strcat(revPush, rhsItems[rule][i]);
                        strcat(revPush, " ");
                    }
                }
                
                // In luật sinh bao gồm cả @
                snprintf(prodStr, sizeof(prodStr), "%s->", ruleLHS[rule]);
                for(int i=0; i<count; i++) {
                    strcat(prodStr, rhsItems[rule][i]);
                }
                
                snprintf(action, sizeof(action), "%s弹栈，%s逆序压栈", X, revPush);
                
                printf(" "); printCell(stepStr, 5); printf(" | ");
                printCell(stkStr, 50); printf(" | ");
                printCell(inStr, 35); printf(" | ");
                printCell(prodStr, 25); printf(" | ");
                printCell(action, 35); printf("\n");
                
            } else { 
                printf("\n[ERROR] 语法错误！非终结符 '%s' 没有对应向前看符号 '%s' 的产生式 (行 %d)\n", X, a.lexeme, a.line); 
                return; 
            }
        }
    }
    printf("-----------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

/* =====================================================================
 * 3. 语义分析及中间代码生成 (IR Generation)
 * ===================================================================== */
typedef struct {
    char op[20];
    char arg1[50];
    char arg2[50];
    char res[50];
} Quadruple;

Quadruple quads[500];
int nextquad = 0;
char symbolTable[100][50];
int symCount = 0;
int tempCount = 1;
int labelCount = 1;
Token lookahead;

void resetSemantic() {
    pos = 0; currentLine = 1; nextquad = 0; tempCount = 1; symCount = 0; labelCount = 1;
}

void advanceIR() { lookahead = getNextToken(); }
void matchIR(TokenType exp) { if (lookahead.type == exp) advanceIR(); }

void emit(const char* op, const char* a1, const char* a2, const char* r) {
    if (nextquad < 500) {
        strncpy(quads[nextquad].op, op, sizeof(quads[nextquad].op) - 1); 
        strncpy(quads[nextquad].arg1, a1, sizeof(quads[nextquad].arg1) - 1);
        strncpy(quads[nextquad].arg2, a2, sizeof(quads[nextquad].arg2) - 1); 
        strncpy(quads[nextquad].res, r, sizeof(quads[nextquad].res) - 1); 
        nextquad++;
    }
}

void checkDecl(char* name) {
    for (int i=0; i<symCount; i++) if (strcmp(symbolTable[i], name) == 0) return;
    if (symCount < 100) strncpy(symbolTable[symCount++], name, 49);
}

void parseStatementIR();

void parseDeclIR() {
    advanceIR();
    char id[50]; strncpy(id, lookahead.lexeme, sizeof(id) - 1); id[sizeof(id) - 1] = '\0';
    if (symCount < 100) strncpy(symbolTable[symCount++], id, 49);
    advanceIR(); 
    
    if (lookahead.type == TOK_ASSIGN) {
        advanceIR();
        char val[50]; strncpy(val, lookahead.lexeme, sizeof(val) - 1); val[sizeof(val) - 1] = '\0';
        advanceIR();
        emit("=", val, "-", id);
    }
    matchIR(TOK_SEMI);
}

void parseAssignIR() {
    char target[50]; strncpy(target, lookahead.lexeme, sizeof(target) - 1); target[sizeof(target) - 1] = '\0';
    checkDecl(target); advanceIR();
    matchIR(TOK_ASSIGN);
    char op1[50]; strncpy(op1, lookahead.lexeme, sizeof(op1) - 1); op1[sizeof(op1) - 1] = '\0';
    if (lookahead.type == TOK_ID) checkDecl(op1); advanceIR();
    
    if (lookahead.type == TOK_SEMI) {
        emit("=", op1, "-", target); matchIR(TOK_SEMI);
    } else {
        char op[5]; strncpy(op, lookahead.lexeme, sizeof(op) - 1); advanceIR();
        char op2[50]; strncpy(op2, lookahead.lexeme, sizeof(op2) - 1); op2[sizeof(op2) - 1] = '\0';
        if (lookahead.type == TOK_ID) checkDecl(op2); advanceIR();
        matchIR(TOK_SEMI);
        char tName[10]; snprintf(tName, sizeof(tName), "T%d", tempCount++);
        emit(op, op1, op2, tName);
        emit("=", tName, "-", target);
    }
}

void parseCondIR(char* condStr, size_t maxLen) {
    char a1[50], o[5], a2[50];
    strncpy(a1, lookahead.lexeme, sizeof(a1) - 1); a1[sizeof(a1) - 1] = '\0';
    checkDecl(a1); advanceIR();
    strncpy(o, lookahead.lexeme, sizeof(o) - 1); advanceIR();
    strncpy(a2, lookahead.lexeme, sizeof(a2) - 1); a2[sizeof(a2) - 1] = '\0';
    advanceIR();
    snprintf(condStr, maxLen, "%s %s %s", a1, o, a2);
}

void parseBlockIR() {
    matchIR(TOK_LBRACE);
    while(lookahead.type != TOK_RBRACE && lookahead.type != TOK_EOF) parseStatementIR();
    matchIR(TOK_RBRACE);
}

void parseWhileIR() {
    int startIdx = nextquad;
    matchIR(TOK_WHILE); matchIR(TOK_LPAREN);
    char cond[100]; parseCondIR(cond, sizeof(cond)); matchIR(TOK_RPAREN);
    
    int falseIdx = nextquad;
    emit("j<false>", cond, "-", "?");
    parseBlockIR();
    
    char loopStart[10]; snprintf(loopStart, sizeof(loopStart), "%d", startIdx);
    emit("j", "-", "-", loopStart);
    
    char jumpTarget[10]; snprintf(jumpTarget, sizeof(jumpTarget), "%d", nextquad);
    strncpy(quads[falseIdx].res, jumpTarget, sizeof(quads[falseIdx].res) - 1);
}

void parseIfIR() {
    matchIR(TOK_IF); matchIR(TOK_LPAREN);
    char cond[100]; parseCondIR(cond, sizeof(cond)); matchIR(TOK_RPAREN);
    
    int falseIdx = nextquad;
    emit("j<false>", cond, "-", "?");
    parseBlockIR();
    
    int endIdx = nextquad;
    emit("j", "-", "-", "?");
    
    char jumpTarget[10]; snprintf(jumpTarget, sizeof(jumpTarget), "%d", nextquad);
    strncpy(quads[falseIdx].res, jumpTarget, sizeof(quads[falseIdx].res) - 1);
    
    if (lookahead.type == TOK_ELSE) {
        matchIR(TOK_ELSE); parseBlockIR();
    }
    snprintf(jumpTarget, sizeof(jumpTarget), "%d", nextquad);
    strncpy(quads[endIdx].res, jumpTarget, sizeof(quads[endIdx].res) - 1);
}

void parseIOIR() {
    matchIR(TOK_PRINT); matchIR(TOK_LPAREN);
    char id[50]; strncpy(id, lookahead.lexeme, sizeof(id) - 1); id[sizeof(id) - 1] = '\0';
    checkDecl(id); advanceIR();
    matchIR(TOK_RPAREN); matchIR(TOK_SEMI);
    emit("print", "-", "-", id);
}

void parseStatementIR() {
    if (lookahead.type == TOK_INT || lookahead.type == TOK_FLOAT) parseDeclIR();
    else if (lookahead.type == TOK_ID) parseAssignIR();
    else if (lookahead.type == TOK_IF) parseIfIR();
    else if (lookahead.type == TOK_WHILE) parseWhileIR();
    else if (lookahead.type == TOK_PRINT) parseIOIR();
    else advanceIR(); 
}

void runSemanticIR() {
    resetSemantic();
    advanceIR();
    while(lookahead.type != TOK_EOF) parseStatementIR();
    
    printf("\n--- 中间代码 ---\n");
    printf("格式: (序号)\t(操作符, 操作数1, 操作数2, 结果)\n");
    for(int i=0; i<nextquad; i++) 
        printf("(%d)\t(%s, %s, %s, %s)\n", i, quads[i].op, quads[i].arg1, quads[i].arg2, quads[i].res);
}

/* =====================================================================
 * 4. 目标代码生成 (Target Assembly Generation)
 * ===================================================================== */
void generateAssembly() {
    resetSemantic(); advanceIR();
    while(lookahead.type != TOK_EOF) parseStatementIR(); 
    
    printf("\n; --- 目标机器代码 ---\n");
    printf("section .data\n");
    for(int i=0; i<symCount; i++) {
        if(symbolTable[i][0] != '\0' && symbolTable[i][0] != 'T') {
            printf("    %s dd 0\n", symbolTable[i]); 
        }
    } 
    printf("\nsection .text\nglobal _start\n_start:\n");
    
    for (int i = 0; i < nextquad; i++) {
        Quadruple q = quads[i];
        printf("\nL%d:\t; (%s, %s, %s, %s)\n", i, q.op, q.arg1, q.arg2, q.res);
        if (strcmp(q.op, "=") == 0) {
            if (isdigit((unsigned char)q.arg1[0])) printf("\tMOV DWORD [%s], %s\n", q.res, q.arg1);
            else { printf("\tMOV EAX, [%s]\n\tMOV [%s], EAX\n", q.arg1, q.res); }
        }
        else if (strcmp(q.op, "+") == 0 || strcmp(q.op, "-") == 0) {
            printf("\tMOV EAX, [%s]\n", q.arg1);
            if (strcmp(q.op, "+") == 0) {
                if(isdigit((unsigned char)q.arg2[0])) printf("\tADD EAX, %s\n", q.arg2); else printf("\tADD EAX, [%s]\n", q.arg2);
            } else {
                if(isdigit((unsigned char)q.arg2[0])) printf("\tSUB EAX, %s\n", q.arg2); else printf("\tSUB EAX, [%s]\n", q.arg2);
            }
            printf("\tMOV [%s], EAX\n", q.res);
        }
        else if (strcmp(q.op, "j<false>") == 0) {
            char op1[50], op[10], op2[50]; 
            if (sscanf(q.arg1, "%49s %9s %49s", op1, op, op2) == 3) {
                printf("\tMOV EAX, [%s]\n", op1);
                if (isdigit((unsigned char)op2[0])) printf("\tCMP EAX, %s\n", op2); else printf("\tCMP EAX, [%s]\n", op2);
                if (strcmp(op, "<") == 0) printf("\tJGE L%s\n", q.res);
                else if (strcmp(op, ">") == 0) printf("\tJLE L%s\n", q.res);
                else if (strcmp(op, "==") == 0) printf("\tJNE L%s\n", q.res);
            }
        }
        else if (strcmp(q.op, "j") == 0) printf("\tJMP L%s\n", q.res);
        else if (strcmp(q.op, "print") == 0) printf("\tMOV EAX, [%s]\n\tCALL sys_print_int\n", q.res);
    }
    printf("\nL%d:\n\tMOV EAX, 1\n\tINT 0x80\n", nextquad);
}

/* =====================================================================
 * 5. 跨平台主程序 (Main Engine)
 * ===================================================================== */
void crossClear() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int main() {
    loadGrammar();

    const char* filename = "code.txt"; 
    FILE *file = fopen(filename, "r");
    if (file == NULL) { 
        printf("Error: 无法打开源代码文件 '%s'\n", filename); 
        return 1; 
    }
    size_t bytesRead = fread(src, 1, sizeof(src) - 1, file);
    src[bytesRead] = '\0'; 
    fclose(file);

    int choice;
    do {
        crossClear();
        printf("\n======================================================\n");
        printf("             编译原理课程设计 \n");
        printf("======================================================\n");
        printf("  1. 词法分析\n");
        printf("  2. 语法分析\n");
        printf("  3. 语义分析及中间代码生成\n");
        printf("  4. 目标代码生成\n");
        printf("  0. 退出系统\n");
        printf("======================================================\n");
        printf("请选择功能 (0-4): ");
        
        if (scanf("%d", &choice) != 1) { 
            while (getchar() != '\n'); 
            choice = -1; 
        }

        switch(choice) {
            case 1: 
                crossClear();
                printLexerResult(); 
                break;
            case 2: 
                crossClear();
                printLL1Table(); 
                runSyntaxProcess(); 
                break;
            case 3: 
                crossClear();
                runSemanticIR(); 
                break;
            case 4: 
                crossClear();
                generateAssembly(); 
                break;
            case 0: break;
            default: printf("\n无效选择，请重新输入！\n");
        }
        
        if (choice != 0) {
            printf("\n[按回车键返回主菜单...]\n");
            while (getchar() != '\n'); 
            getchar();
        }
    } while (choice != 0);
    
    return 0;
}