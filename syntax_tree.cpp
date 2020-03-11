#include "syntax_tree.h"

void make_tree_vert_left(struct syn_tree* top);
void make_tree_vert_right(struct syn_tree* top);
int build_v(struct lex_array_t* lex_arr, int lex_arr_pos, struct syn_tree* top);
int isbrace_order(struct lex_array_t* lex_arr);
int calculate_tree(struct syn_tree* top);

struct syn_tree* make_syn_tree(struct lex_array_t* lex_arr) {
    struct syn_tree* s_tree;
    int a;
    assert(lex_arr != nullptr);

    if(isbrace_order(lex_arr) == 0) {
        return nullptr;
    }
    //Проверка правильности расстановки скобок
    //Сделать ее отдельно - самый простой вариант для моей реализации

    s_tree = (struct syn_tree*) calloc(1, sizeof (struct syn_tree));
    s_tree->left = nullptr;
    s_tree->right = nullptr;
    a = build_v(lex_arr, lex_arr->size - 1, s_tree);
    //При корректной работе а = -1, иначе -2

    if(a == -2) {
        destroy_tree(s_tree);
        return nullptr;
    }
    return s_tree;
}

int isbrace_order(struct lex_array_t* lex_arr) {
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

void make_tree_vert_right(struct syn_tree* top) {
    top->right = (struct syn_tree*) calloc(1, sizeof(struct syn_tree));
    top->right->left = nullptr;
    top->right->right = nullptr;
}

void make_tree_vert_left(struct syn_tree* top) {
    top->left = (struct syn_tree*) calloc(1, sizeof(struct syn_tree));
    top->left->left = nullptr;
    top->left->right = nullptr;
}

//Эта функция рекурсивно строит синтаксическое дерево
//Проход по массиву лексем идет в обратном порядке
int build_v(struct lex_array_t* lex_arr, int lex_arr_pos, struct syn_tree* top) {
    static int work_mode = 0;
    //Функция работает в 2 режимах:
    //work_mode = 0 - полное построение вершины дерева
    //work_mode = 1 - достраивание вершины с учетом наличия правого потомка
    struct syn_tree t_buff;
    //Для хранения копии вершины
    assert(lex_arr != nullptr);
    assert(top != nullptr);
    if(lex_arr_pos < 0) {
        return lex_arr_pos;
    }

    if(work_mode == 0) { //построение правого потомка (число или рекурсивный спуск в него)
        if(lex_arr->lexems[lex_arr_pos].kind == NUM) {
            make_tree_vert_right(top);
            top->right->data = lex_arr->lexems[lex_arr_pos];
            lex_arr_pos--;
        }
        else if ((lex_arr->lexems[lex_arr_pos].kind == BRACE) || (lex_arr->lexems[lex_arr_pos].lex.b == RBRAC)) {
            make_tree_vert_right(top);
            lex_arr_pos--;
            lex_arr_pos = build_v(lex_arr, lex_arr_pos, top->right);
        }
        else { //синтаксическая ошибка
            return -2;
        }

        if(lex_arr_pos < 0) {
            return lex_arr_pos;
        }
    }

    work_mode = 0;

    if(lex_arr->lexems[lex_arr_pos].kind == OP) { //Заполнение текущей вершины или подъем из нее.
        top->data.kind = OP;
        top->data.lex.op = lex_arr->lexems[lex_arr_pos].lex.op;
        lex_arr_pos--;
    }
    else if((lex_arr->lexems[lex_arr_pos].kind == BRACE) && (lex_arr->lexems[lex_arr_pos].lex.b == LBRAC)) {
        return lex_arr_pos - 1;
    }
    else { //ошибка
        return -2;
    }

    if(top->data.lex.op == SUB) {//дополнение выражений наподобие (-1) до (0 - 1).
        if(lex_arr_pos < 0) {
            make_tree_vert_left(top);
            top->left->data.kind = NUM;
            top->left->data.lex.num = 0;
            return lex_arr_pos;
        }
        if ((lex_arr->lexems[lex_arr_pos].kind == BRACE) && (lex_arr->lexems[lex_arr_pos].lex.b == LBRAC)) {
            make_tree_vert_left(top);
            top->left->data.kind = NUM;
            top->left->data.lex.num = 0;
            return lex_arr_pos - 1;
        }
    }

    if(lex_arr_pos < 0) {
        return -2;
    }

    //Построение левого потомка (число или рекурсивный спуск в него).
    if((lex_arr->lexems[lex_arr_pos].kind == BRACE) && (lex_arr->lexems[lex_arr_pos].lex.b == RBRAC)) {
        make_tree_vert_left(top);
        lex_arr_pos--;
        lex_arr_pos = build_v(lex_arr, lex_arr_pos, top->left);
    }
    else if(lex_arr->lexems[lex_arr_pos].kind == NUM) {
        make_tree_vert_left(top);
        top->left->data = lex_arr->lexems[lex_arr_pos];
        lex_arr_pos--;
    }
    else { //ошибка
        return -2;
    }

    if(lex_arr_pos < 0) {
        return lex_arr_pos;
    }

    //Далее идет подготовка дерева к добавлению следующей операции.
    if((lex_arr->lexems[lex_arr_pos].kind == BRACE) && (lex_arr->lexems[lex_arr_pos].lex.b == LBRAC)) { //"(" - подъем из вершины
        return lex_arr_pos - 1;
    }
    else {
        work_mode = 1; //Следующий запуск функции - достраивание вершины
        //В случае низкого приоритета операции текущей вершины:
        //Левый потомок перевешивается на своего правого потомка
        //Далее левый потомок будет достраиваться
        if((top->data.lex.op == ADD) || (top->data.lex.op == SUB)) {
            t_buff = *(top->left);
            make_tree_vert_left(top->left);
            make_tree_vert_right(top->left);
            *(top->left->right) = t_buff;
            lex_arr_pos = build_v(lex_arr, lex_arr_pos, top->left);
        }
        //В случае высокого приоритета текущей операции:
        //Текущая вершина перевешивается на своего правого потомка
        //Далее достраивается текущая вершина
        else {
            t_buff = *(top->right);
            make_tree_vert_left(top->right);
            make_tree_vert_right(top->right);
            *(top->right->right) = t_buff;
            top->right->data = top->data;
            top->right->left = top->left;
            top->left = nullptr;
            lex_arr_pos = build_v(lex_arr, lex_arr_pos, top);
        }
    }
    return lex_arr_pos;
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

    if(top->right != nullptr) {
        return calculate_tree(top->right);
    }

    assert(top->data.kind == NUM);
    return top->data.lex.num;
}
