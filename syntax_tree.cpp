#include "syntax_tree.h"
#include <stdlib.h>

void make_tree_vert(struct syn_tree* top);
int build_expr(struct syn_tree* top, int l_border, int r_border, struct lex_array_t* lex_arr);
int build_mult(struct syn_tree* top, int l_border, int r_border, struct lex_array_t* lex_arr);
int build_term(struct syn_tree* top, int l_border, int r_border, struct lex_array_t* lex_arr);

struct syn_tree* make_syn_tree(struct lex_array_t* lex_arr) {
    struct syn_tree* s_tree;
    int a;
    assert(lex_arr != nullptr);

    s_tree = (struct syn_tree*) malloc(sizeof (struct syn_tree));
    s_tree->left = nullptr;
    s_tree->right = nullptr;
    a = build_expr(s_tree, 0, lex_arr->size, lex_arr);
    //При корректной работе а = 1, иначе 0

    if(a == 0) {
        destroy_tree(s_tree);
        return nullptr;
    }
    return s_tree;
}

//Во всех функциях построения синтаксического дерева:
//return 0 - ошибка, return 1 - правильное выполнение.
//проход по лексемам идет с конца массива
int build_expr(struct syn_tree* top, int l_border, int r_border, struct lex_array_t* lex_arr) {
    int i, brace_num, sep_pos;
    assert(top != nullptr);

    //поиск нужной лексемы
    sep_pos = -1; //номер нужной лексемы будет здесь
    brace_num = 0;
    //приоритет лексемы не должен повышаться скобками
    for(i = r_border - 1; i >= l_border; i--) {
        if(lex_arr->lexems[i].kind == BRACE) {
            if(lex_arr->lexems[i].lex.b == RBRAC) {
                brace_num++;
            }
            else {
                brace_num--;
            }
        }

        if(brace_num < 0) { //лишняя проверка на правильность скобочной последовательности
            return 0;
        }
        if(brace_num == 0) {
            if((lex_arr->lexems[i].kind == OP) && ((lex_arr->lexems[i].lex.op == ADD) || (lex_arr->lexems[i].lex.op == SUB))) {
                sep_pos = i;
                break;
            }
        }
    }
    if(sep_pos == -1) { //случай отсутствия искомой лексемы
        return build_mult(top, l_border, r_border, lex_arr);
    }

    //построение вершины дерева для лексемы и разделение задачи на части
    if(sep_pos == r_border - 1) {
        return 0;
    }
    else if(sep_pos == l_border) {
        if(lex_arr->lexems[sep_pos].lex.op == SUB) { //учет ситуаций наподобие (-1)
            make_tree_vert(top);
            top->left->data.kind = NUM;
            top->left->data.lex.num = 0;
            top->data.kind = OP;
            top->data.lex.op = SUB;
            if(build_expr(top->right, sep_pos + 1, r_border, lex_arr) == 0) {
                return 0;
            }
        }
        else {
            return 0;
        }
    }
    else {
        top->data = lex_arr->lexems[sep_pos];
        make_tree_vert(top);
        if(build_expr(top->left, l_border, sep_pos, lex_arr) == 0) {
            return 0;
        }
        if(build_mult(top->right, sep_pos + 1, r_border, lex_arr) == 0) {
            return 0;
        }
    }
    return 1;
}

//эта функция аналогична предыдущей
int build_mult(struct syn_tree* top, int l_border, int r_border, struct lex_array_t* lex_arr) {
    int i, brace_num, sep_pos;
    assert(top != nullptr);

    sep_pos = -1;
    brace_num = 0;
    for(i = r_border - 1; i >= l_border; i--) {
        if(lex_arr->lexems[i].kind == BRACE) {
            if(lex_arr->lexems[i].lex.b == RBRAC) {
                brace_num++;
            }
            else {
                brace_num--;
            }
        }

        if(brace_num < 0) {
            return 0;
        }
        if(brace_num == 0) {
            if((lex_arr->lexems[i].kind == OP) && ((lex_arr->lexems[i].lex.op == MUL) || (lex_arr->lexems[i].lex.op == DIV))) {
                sep_pos = i;
                break;
            }
        }
    }
    if(sep_pos == -1) {
        return build_term(top, l_border, r_border, lex_arr);
    }

    if((sep_pos == l_border) || (sep_pos == r_border - 1)) {
        return 0;
    }
    else {
        top->data = lex_arr->lexems[sep_pos];
        make_tree_vert(top);
        if(build_mult(top->left, l_border, sep_pos, lex_arr) == 0) {
            return 0;
        }
        if(build_term(top->right, sep_pos + 1, r_border, lex_arr) == 0) {
            return 0;
        }
    }
    return 1;
}

int build_term(struct syn_tree* top, int l_border, int r_border, struct lex_array_t* lex_arr) {
    int i, brace_num;
    assert(top != nullptr);

    if(l_border + 1 == r_border) { //учет одиночных чисел
        if(lex_arr->lexems[l_border].kind == NUM) {
            top->data = lex_arr->lexems[l_border];
            return 1;
        }
        else {
            return 0;
        }
    }

    if((lex_arr->lexems[l_border].kind == BRACE) && (lex_arr->lexems[l_border].lex.b == LBRAC) &&
    (lex_arr->lexems[r_border - 1].kind == BRACE) && (lex_arr->lexems[r_border - 1].lex.b == RBRAC)) { //снятие скобок со всего выражения
        brace_num = 0;
        for(i = l_border; i < r_border - 1; i++) { //проверка соответсвия выражению (expr)
            if(lex_arr->lexems[i].kind == BRACE) {
                if(lex_arr->lexems[i].lex.b == LBRAC) {
                    brace_num++;
                }
                else {
                    brace_num--;
                }
            }
            if(brace_num <= 0) {
                return 0;
            }
        }
        if(brace_num != 1) {
            return 0;
        }

        return build_expr(top, l_border + 1, r_border - 1, lex_arr);
    }
    return 0;
}

void make_tree_vert(struct syn_tree* top) {
    assert(top != nullptr);
    assert(top->left == nullptr);
    assert(top->right == nullptr);
    top->left = (struct syn_tree*) calloc(1, sizeof(struct syn_tree));
    top->left->left = nullptr;
    top->left->right = nullptr;
    top->right = (struct syn_tree*) calloc(1, sizeof(struct syn_tree));
    top->right->left = nullptr;
    top->right->right = nullptr;
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
            return l_num / r_num;
        }
    }

    assert(top->data.kind == NUM);
    return top->data.lex.num;
}
