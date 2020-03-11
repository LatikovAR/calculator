#include "lx.h"
#include "syntax_tree.h"

enum { MAXLEN = 1024 };

void test_dump() {
    struct lexem_t lxs[5] = {
        {BRACE, {.b = LBRAC}}, {NUM, {.num = 2}}, {OP, {.op = MUL}},
        {NUM, {.num = 2}}, {BRACE, {.b = RBRAC}}
    };

    struct lex_array_t larr = {lxs, 5, 5};
    dump_lexarray(larr);
}

int main() {
    int i, result;
    char c;
    struct lex_array_t larr;
    struct syn_tree* s_tree;
    char inp[MAXLEN] = {0};

    i = 0;
    scanf("%c", &c);
    while(c != '\n') {
        inp[i] = c;
        i++;
        scanf("%c", &c);
    }
    inp[i] = '\0';

    larr = lex_string(inp);

    if (larr.lexems == nullptr) {
        printf("LEX_ERROR\n");
        return 0;
    }

    dump_lexarray(larr);

    s_tree = make_syn_tree(&larr);
    if(s_tree == nullptr) {
        printf("SYN_ERROR\n");
        return 0;
    }

    result = calculate_tree(s_tree);
    destroy_tree(s_tree);
    printf("RESULT: %d", result);

    free(larr.lexems);
    return 0;
}
