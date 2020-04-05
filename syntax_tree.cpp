#include "syntax_tree.h"

static struct syn_tree* build_expr(struct lex_array_t* lex_arr);
static struct syn_tree* build_mult(struct lex_array_t* lex_arr);
static struct syn_tree* build_term(struct lex_array_t* lex_arr);
static bool isopadd_t(struct lex_array_t* lex_arr);
static bool isopmul_t(struct lex_array_t* lex_arr);
static bool isrbrac(struct lex_array_t* lex_arr);
static bool islbrac(struct lex_array_t* lex_arr);
static bool isnum(struct lex_array_t* lex_arr);
static bool isop(struct lex_array_t* lex_arr);
static bool isprevopsub(struct lex_array_t* lex_arr);

struct syn_tree* make_syn_tree(struct lex_array_t* lex_arr) {
    struct syn_tree* s_tree = nullptr;
    assert(lex_arr != nullptr);

    lex_arr->parsed_pos = lex_arr->size - 1;
    s_tree = build_expr(lex_arr); //error - nullptr

    if(lex_arr->parsed_pos >= 0) {
        destroy_tree(s_tree);
        return nullptr;
    }

    return s_tree;
}

static struct syn_tree* build_expr(struct lex_array_t* lex_arr) {
    struct syn_tree* top = nullptr;
    struct syn_tree* lhs = nullptr;
    struct syn_tree* rhs = nullptr;
    struct lexem_t parsed_lex;

    if(lex_arr->parsed_pos < 0) {
        return nullptr;
    }

    rhs = build_mult(lex_arr);
    if(rhs == nullptr) {
        return nullptr;
    }

    if(lex_arr->parsed_pos < 0) {
        return rhs;
    }

    if(isopadd_t(lex_arr)) {
        parsed_lex = lex_arr->lexems[lex_arr->parsed_pos];
        lex_arr->parsed_pos--;
        if((isprevopsub(lex_arr)) && ((lex_arr->parsed_pos == -1) || (islbrac(lex_arr)))){ // in case (-EXPR)
            top = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
            assert(top != nullptr);
            top->data = parsed_lex;
            top->left = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
            assert(top->left != nullptr);
            top->left->left = nullptr;
            top->left->right = nullptr;
            top->left->data.kind = NUM;
            top->left->data.lex.num = 0;
            top->right = rhs;
            return top;
        }
        lhs = build_expr(lex_arr);
        if(lhs == nullptr) {
            destroy_tree(rhs);
            return nullptr;
        }
        top = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
        assert(top != nullptr);
        top->data = parsed_lex;
        top->right = rhs;
        top->left = lhs;
        return top;
    }

    if(islbrac(lex_arr)) {
        return rhs;
    }

    destroy_tree(rhs);
    return nullptr;
}

static struct syn_tree* build_mult(struct lex_array_t* lex_arr) {
    struct syn_tree* top = nullptr;
    struct syn_tree* lhs = nullptr;
    struct syn_tree* rhs = nullptr;
    struct lexem_t parsed_lex;

    if(lex_arr->parsed_pos < 0) {
        return nullptr;
    }

    rhs = build_term(lex_arr);
    if(rhs == nullptr) {
        return nullptr;
    }

    if(lex_arr->parsed_pos < 0) {
        return rhs;
    }

    if(isop(lex_arr)) {
        if(isopmul_t(lex_arr)) {
            parsed_lex = lex_arr->lexems[lex_arr->parsed_pos];
            lex_arr->parsed_pos--;
            lhs = build_mult(lex_arr);
            if(lhs == nullptr) {
                destroy_tree(rhs);
                return nullptr;
            }
            top = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
            assert(top != nullptr);
            top->data = parsed_lex;
            top->right = rhs;
            top->left = lhs;
            return top;
        }
        else {
            return rhs;
        }
    }

    if(islbrac(lex_arr)) {
        return rhs;
    }

    destroy_tree(rhs);
    return nullptr;
}

static struct syn_tree* build_term(struct lex_array_t* lex_arr) {
    struct syn_tree* top = nullptr;

    if(isnum(lex_arr)) {
        top = (struct syn_tree*) calloc(1, sizeof(struct syn_tree));
        assert(top != nullptr);
        top->data = lex_arr->lexems[lex_arr->parsed_pos];
        top->left = nullptr;
        top->right = nullptr;
        lex_arr->parsed_pos--;
        return top;
    }

    if(isrbrac(lex_arr)) {
        lex_arr->parsed_pos--;
        top = build_expr(lex_arr);
        if((lex_arr->parsed_pos < 0) ||
           (!islbrac(lex_arr))) {
            destroy_tree(top);
            return nullptr;
        }
        lex_arr->parsed_pos--;
        return top;
    }

    return nullptr;
}

void destroy_tree(struct syn_tree* top) {
    if(top != nullptr) {
        if(top->left != nullptr) {
            destroy_tree(top->left);
        }
        if(top->right != nullptr) {
            destroy_tree(top->right);
        }
        free(top);
    }
}

int calculate_tree(struct syn_tree* top) {
    int l_num, r_num;
    assert(top != nullptr);

    //print_lexem(top->data);
    if((top->left != nullptr) && (top->right != nullptr)) {
        l_num = calculate_tree(top->left);
        r_num = calculate_tree(top->right);
        assert(top->data.kind == OP);
        if(top->data.lex.op == ADD) {
            return l_num + r_num;
        }
        if(top->data.lex.op == SUB) {
            return l_num - r_num;
        }
        if(top->data.lex.op == MUL) {
            return l_num * r_num;
        }
        if(top->data.lex.op == DIV) {
            if(r_num == 0) {
                assert(0 && "MATH_ERROR");
            }
            return l_num / r_num;
        }
    }

    assert(top->data.kind == NUM);
    return top->data.lex.num;
}

static bool isopadd_t(struct lex_array_t* lex_arr) {
    return ((lex_arr->lexems[lex_arr->parsed_pos].kind == OP) &&
            ((lex_arr->lexems[lex_arr->parsed_pos].lex.op == ADD) ||
            (lex_arr->lexems[lex_arr->parsed_pos].lex.op == SUB))) ? true : false;
}

static bool isopmul_t(struct lex_array_t* lex_arr) {
    return ((lex_arr->lexems[lex_arr->parsed_pos].kind == OP) &&
            ((lex_arr->lexems[lex_arr->parsed_pos].lex.op == MUL) ||
            (lex_arr->lexems[lex_arr->parsed_pos].lex.op == DIV))) ? true : false;
}

static bool isrbrac(struct lex_array_t* lex_arr) {
    return ((lex_arr->lexems[lex_arr->parsed_pos].kind == BRACE) &&
            (lex_arr->lexems[lex_arr->parsed_pos].lex.b == RBRAC)) ? true : false;
}

static bool islbrac(struct lex_array_t* lex_arr) {
    return ((lex_arr->lexems[lex_arr->parsed_pos].kind == BRACE) &&
            (lex_arr->lexems[lex_arr->parsed_pos].lex.b == LBRAC)) ? true : false;
}

static bool isnum(struct lex_array_t* lex_arr) {
    return (lex_arr->lexems[lex_arr->parsed_pos].kind == NUM) ? true : false;
}

static bool isop(struct lex_array_t* lex_arr) {
    return (lex_arr->lexems[lex_arr->parsed_pos].kind == OP) ? true : false;
}

static bool isprevopsub(struct lex_array_t* lex_arr) {
    return ((lex_arr->lexems[lex_arr->parsed_pos + 1].kind == OP) &&
            (lex_arr->lexems[lex_arr->parsed_pos + 1].lex.op == SUB)) ? true : false;
}
