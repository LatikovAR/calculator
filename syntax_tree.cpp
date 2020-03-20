#include "syntax_tree.h"

struct syn_tree* build_expr(struct lex_array_t* lex_arr, int cur_pos);
struct syn_tree* build_mult(struct lex_array_t* lex_arr, int cur_pos);
struct syn_tree* build_term(struct lex_array_t* lex_arr, int cur_pos);
int check_brace(struct lex_array_t* lex_arr);

struct syn_tree* make_syn_tree(struct lex_array_t* lex_arr) {
    struct syn_tree* s_tree;
    assert(lex_arr != nullptr);

    if(check_brace(lex_arr) == 0) {
        return nullptr;
    }

    s_tree = build_expr(lex_arr, lex_arr->size - 1);
    //Ошибка - nullptr

    return s_tree;
}

//Во всех функциях построения синтаксического дерева:
//return nullptr - ошибка
//проход по лексемам идет с конца массива
struct syn_tree* build_expr(struct lex_array_t* lex_arr, int cur_pos) {
    int brace_num, start_pos = cur_pos;
    struct syn_tree* top = nullptr;
    struct syn_tree* lhs = nullptr;
    struct syn_tree* rhs = nullptr;

    rhs = build_mult(lex_arr, cur_pos);
    if(rhs == nullptr) {
        return nullptr;
    }

    brace_num = 0;
    //приоритет лексемы не должен повышаться скобками
    while(cur_pos >= 0) {
        if(lex_arr->lexems[cur_pos].kind == BRACE) {
            if(lex_arr->lexems[cur_pos].lex.b == RBRAC) {
                brace_num++;
            }
            else {
                brace_num--;
            }
        }
        if(brace_num < 0) {
            cur_pos = -1;
            break;
        }

        if(brace_num == 0) {
            if((lex_arr->lexems[cur_pos].kind == OP) &&
              ((lex_arr->lexems[cur_pos].lex.op == ADD) || (lex_arr->lexems[cur_pos].lex.op == SUB))) {
                break;
            }
        }
        cur_pos--;
    }
    if(cur_pos == -1) { //случай отсутствия искомой лексемы
        return rhs;
    }
    if(cur_pos == start_pos) {
        destroy_tree(rhs);
        return nullptr;
    }

    //построение левого потомка
    if((cur_pos == 0) || ((lex_arr->lexems[cur_pos - 1].kind == BRACE) && (lex_arr->lexems[cur_pos - 1].lex.b == LBRAC))) {
        if(lex_arr->lexems[cur_pos].lex.op == SUB) { //учет ситуаций наподобие (-1)
            lhs = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
            lhs->data.kind = NUM;
            lhs->data.lex.num = 0;
        }
        else {
            destroy_tree(rhs);
            return nullptr;
        }
    }
    else {
        lhs = build_expr(lex_arr, cur_pos - 1);
        if(lhs == nullptr) {
            destroy_tree(rhs);
            return nullptr;
        }
    }

    //построение вершины дерева
    top = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
    top->data = lex_arr->lexems[cur_pos];
    top->right = rhs;
    top->left = lhs;
    return top;
}

//эта функция аналогична предыдущей
struct syn_tree* build_mult(struct lex_array_t* lex_arr, int cur_pos) {
    int brace_num, start_pos = cur_pos;
    struct syn_tree* top = nullptr;
    struct syn_tree* lhs = nullptr;
    struct syn_tree* rhs = nullptr;

    rhs = build_term(lex_arr, cur_pos);
    if(rhs == nullptr) {
        return nullptr;
    }

    brace_num = 0;
    while(cur_pos >= 0) {
        if(lex_arr->lexems[cur_pos].kind == BRACE) {
            if(lex_arr->lexems[cur_pos].lex.b == RBRAC) {
                brace_num++;
            }
            else {
                brace_num--;
            }
        }
        if(brace_num < 0) {
            cur_pos = -1;
            break;
        }

        if(brace_num == 0) {
            if(lex_arr->lexems[cur_pos].kind == OP) {
                if((lex_arr->lexems[cur_pos].lex.op == MUL) || (lex_arr->lexems[cur_pos].lex.op == DIV)) {
                    break;
                }
                else { //операция низшего приоритета встретилась первой -> нет ветвления
                    cur_pos = -1;
                    break;
                }
            }
        }
        cur_pos--;
    }
    if(cur_pos == -1) {
        return rhs;
    }

    if((cur_pos == start_pos) || (cur_pos == 0)) {
        destroy_tree(rhs);
        return nullptr;
    }

    lhs = build_mult(lex_arr, cur_pos - 1);
    if(lhs == nullptr) {
        destroy_tree(rhs);
        return nullptr;
    }

    top = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
    top->data = lex_arr->lexems[cur_pos];
    top->right = rhs;
    top->left = lhs;
    return top;
}

struct syn_tree* build_term(struct lex_array_t* lex_arr, int cur_pos) {
    struct syn_tree* top = nullptr;

    if(lex_arr->lexems[cur_pos].kind == NUM) { //запись числа
        if((cur_pos > 0) && (lex_arr->lexems[cur_pos - 1].kind == NUM)) { //костыль на два числа подряд
            return nullptr;
        }
        top = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
        top->data = lex_arr->lexems[cur_pos];
        return top;
    }

    if((lex_arr->lexems[cur_pos].kind == BRACE) && (lex_arr->lexems[cur_pos].lex.b == RBRAC)) { //снятие скобок
        assert(cur_pos > 0);
        top = build_expr(lex_arr, cur_pos - 1);
        return top;
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
