#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "lx.h"

struct syn_tree {
    struct syn_tree* left;
    struct syn_tree* right;
    struct lexem_t data;
};

struct syn_tree* make_syn_tree(struct lex_array_t* lex_arr);
void destroy_tree(struct syn_tree* top);
int calculate_tree(struct syn_tree* top);

#endif // SYNTAX_TREE_H
