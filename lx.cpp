#include "lx.h"

struct lex_array_t lex_string(const char *str) {
    int i, j, isprev_number;
    struct lexem_t* lex_copy;
    assert(str != NULL);

    struct lex_array_t larr = { (struct lexem_t*) malloc(ICAP * sizeof(struct lexem_t)), 0, ICAP };
    assert(larr.lexems != NULL);

    i = 0;
    j = 0;
    isprev_number = 0;
    while(str[i] != '\0') {
        if(larr.capacity == j) {
            larr.capacity *= 2;
            lex_copy = (struct lexem_t*) realloc(larr.lexems, (size_t) larr.capacity);
            assert(lex_copy != nullptr);
            larr.lexems = lex_copy;
        }
        if((isprev_number == 1) && (isdigit(str[i]) == 0)) {
            isprev_number = 0;
            j++;
        }
        if(isspace(str[i]) != 0) {
            i++;
            continue;
        }
        if(isdigit(str[i]) != 0) {
            if(isprev_number == 0) {
                isprev_number = 1;
                larr.lexems[j].kind = NUM;
                larr.lexems[j].lex.num = (str[i] - '0');
            }
            else {
                larr.lexems[j].lex.num *= 10;
                larr.lexems[j].lex.num += (str[i] - '0');
            }
            i++;
            continue;
        }
        if(str[i] == '(') {
            larr.lexems[j].kind = BRACE;
            larr.lexems[j].lex.b = LBRAC;
            j++;
            i++;
            continue;
        }
        if(str[i] == ')') {
            larr.lexems[j].kind = BRACE;
            larr.lexems[j].lex.b = RBRAC;
            j++;
            i++;
            continue;
        }
        if((str[i] == '+') || (str[i] == '-') || (str[i] == '/') || (str[i] == '*')) {
            larr.lexems[j].kind = OP;
            if(str[i] == '+') {
                larr.lexems[j].lex.op = ADD;
            }
            if(str[i] == '-') {
                larr.lexems[j].lex.op = SUB;
            }
            if(str[i] == '/') {
                larr.lexems[j].lex.op = DIV;
            }
            if(str[i] == '*') {
                larr.lexems[j].lex.op = MUL;
            }
            j++;
            i++;
            continue;
        }
        free(larr.lexems);
        larr.lexems = nullptr;
        return larr;
    }
    if(isprev_number == 1) {
        j++;
    }
    larr.size = j;
    return larr;
}

static void print_op(enum operation_t opcode) {
    switch(opcode) {
        case ADD: printf("PLUS "); break;
        case SUB: printf("MINUS "); break;
        case MUL: printf("MUL "); break;
        case DIV: printf("DIV "); break;
        default: assert(0 && "unknown opcode");
    }
}

static void print_brace(enum braces_t bracetype) {
    switch(bracetype) {
        case LBRAC: printf("LBRAC "); break;
        case RBRAC: printf("RBRAC "); break;
        default: assert(0 && "unknown bracket");
    }
}

static void print_num(int n) {
    printf("NUMBER: %d ", n);
}

void print_lexem(struct lexem_t lxm) {
    switch(lxm.kind) {
        case OP: print_op(lxm.lex.op); break;
        case BRACE: print_brace(lxm.lex.b); break;
        case NUM: print_num(lxm.lex.num); break;
        default: assert(0 && "unknown lexem");
    }
}

void dump_lexarray(struct lex_array_t pl) {
    int i;
    assert(pl.lexems != NULL);
    for (i = 0; i < pl.size; ++i) {
        print_lexem(pl.lexems[i]);
    }
    printf("\n");
}
