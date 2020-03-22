#include "syntax_tree.h"

struct syn_tree* build_expr(struct lex_array_t* lex_arr);
struct syn_tree* build_mult(struct lex_array_t* lex_arr);
struct syn_tree* build_term(struct lex_array_t* lex_arr);
int check_brace(struct lex_array_t* lex_arr);

struct syn_tree* make_syn_tree(struct lex_array_t* lex_arr) {
    struct syn_tree* s_tree;
    assert(lex_arr != nullptr);

    if(check_brace(lex_arr) == 0) {
        return nullptr;
    }

    lex_arr->parsed_pos = lex_arr->size - 1;
    s_tree = build_expr(lex_arr); //error - nullptr

    return s_tree;
}

struct syn_tree* build_expr(struct lex_array_t* lex_arr) {
    struct syn_tree* top = nullptr;
    struct syn_tree* lhs = nullptr;
    struct syn_tree* rhs = nullptr;

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

    if((lex_arr->lexems[lex_arr->parsed_pos].kind == OP) &&
      ((lex_arr->lexems[lex_arr->parsed_pos].lex.op == ADD) ||
      (lex_arr->lexems[lex_arr->parsed_pos].lex.op == SUB))) {
        top = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
        top->data = lex_arr->lexems[lex_arr->parsed_pos];
        lex_arr->parsed_pos--;
        if((lex_arr->lexems[lex_arr->parsed_pos + 1].lex.op == SUB) &&
           ((lex_arr->parsed_pos == -1) ||
           ((lex_arr->lexems[lex_arr->parsed_pos].kind == BRACE) &&
           (lex_arr->lexems[lex_arr->parsed_pos].lex.b == LBRAC)))){ // in case (-EXPR)
            top->left = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
            top->left->left = nullptr;
            top->left->right = nullptr;
            top->left->data.kind = NUM;
            top->left->data.lex.num = 0;
            top->right = rhs;
            lex_arr->parsed_pos--;
            return top;
        }
        lhs = build_expr(lex_arr);
        if(lhs == nullptr) {
            free(top);
            destroy_tree(rhs);
            return nullptr;
        }
        top->right = rhs;
        top->left = lhs;
        return top;
    }

    if((lex_arr->lexems[lex_arr->parsed_pos].kind == BRACE) &&
      (lex_arr->lexems[lex_arr->parsed_pos].lex.b == LBRAC)) {
        lex_arr->parsed_pos--;
        return rhs;
    }

    destroy_tree(rhs);
    return nullptr;
}

struct syn_tree* build_mult(struct lex_array_t* lex_arr) {
    struct syn_tree* top = nullptr;
    struct syn_tree* lhs = nullptr;
    struct syn_tree* rhs = nullptr;

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

    if(lex_arr->lexems[lex_arr->parsed_pos].kind == OP) {
        if((lex_arr->lexems[lex_arr->parsed_pos].lex.op == MUL) ||
          (lex_arr->lexems[lex_arr->parsed_pos].lex.op == DIV)) {
            top = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
            top->data = lex_arr->lexems[lex_arr->parsed_pos];
            lex_arr->parsed_pos--;
            lhs = build_mult(lex_arr);
            if(lhs == nullptr) {
                free(top);
                destroy_tree(rhs);
                return nullptr;
            }
            top->right = rhs;
            top->left = lhs;
            return top;
        }
        else {
            return rhs;
        }
    }

    if((lex_arr->lexems[lex_arr->parsed_pos].kind == BRACE) &&
      (lex_arr->lexems[lex_arr->parsed_pos].lex.b == LBRAC)) {
        return rhs;
    }

    destroy_tree(rhs);
    return nullptr;
}

struct syn_tree* build_term(struct lex_array_t* lex_arr) {
    struct syn_tree* top = nullptr;

    if(lex_arr->lexems[lex_arr->parsed_pos].kind == NUM) {
        top = (struct syn_tree*) calloc(1, sizeof(struct syn_tree));
        top->data = lex_arr->lexems[lex_arr->parsed_pos];
        top->left = nullptr;
        top->right = nullptr;
        lex_arr->parsed_pos--;
        return top;
    }

    if((lex_arr->lexems[lex_arr->parsed_pos].kind == BRACE) &&
       (lex_arr->lexems[lex_arr->parsed_pos].lex.b == RBRAC)) {
        lex_arr->parsed_pos--;
        return build_expr(lex_arr);
    }

    return nullptr;
}

void destroy_tree(struct syn_tree* top) {
    assert(top != nullptr);
    if(top->left != nullptr) {
        destroy_tree(top->left);
    }
    if(top->right != nullptr) {
        destroy_tree(top->right);
    }
    free(top);
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
                printf("MATH_ERROR\n");
            }
            return l_num / r_num;
        }
    }

    assert(top->data.kind == NUM);
    return top->data.lex.num;
}

int check_brace(struct lex_array_t* lex_arr) {
    int i, cur_br_num;
    assert(lex_arr != nullptr);

    cur_br_num = 0;
    //Число встретившихся "(" - ")", должно быть не меньше 0 всегда.
    //В конце должно быть 0.
    for(i = 0; i < lex_arr->size; i++) {
        if(lex_arr->lexems[i].kind == BRACE) {
            if(lex_arr->lexems[i].lex.b == LBRAC) {
                cur_br_num++;
            }
            else if(lex_arr->lexems[i].lex.b == RBRAC) {
                cur_br_num--;
            }
            else {
                return 0;
            }
            if(cur_br_num < 0) {
                return 0;
            }
        }
    }
    if(cur_br_num > 0) {
        return 0;
    }
    return 1;
}
