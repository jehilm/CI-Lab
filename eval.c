/**************************************************************************
 * C S 429 EEL interpreter
 * 
 * eval.c - This file contains the skeleton of functions to be implemented by
 * you. When completed, it will contain the code used to evaluate an expression
 * based on its AST.
 * 
 * Copyright (c) 2021. S. Chatterjee, X. Shen, T. Byrd. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/ 

#include "ci.h"

extern bool is_binop(token_t);
extern bool is_unop(token_t);
char *strrev(char *str);

/* infer_type() - set the type of a non-root node based on the types of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated.
 * (STUDENT TODO)
 */

static void infer_type(node_t *nptr) {
    if(nptr == NULL || nptr == NT_LEAF) return;
    
    for(int i = 0; i < 3; i++) {
        infer_type(nptr->children[i]);
    }

    if(nptr->children[0] != NULL && nptr->children[1] != NULL && nptr->children[2] == NULL) {
        if(nptr->tok == TOK_PLUS || nptr->tok == TOK_BMINUS || nptr->tok == TOK_TIMES || nptr->tok == TOK_DIV || nptr->tok == TOK_MOD) {
            if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
                nptr->type = INT_TYPE;
            }
            else if(nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE && nptr->tok == TOK_PLUS) {
                nptr->type = STRING_TYPE;
            }
            else if(nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == INT_TYPE && nptr->tok == TOK_TIMES) {
                nptr->type = STRING_TYPE;
            }
            else {
                handle_error(ERR_TYPE);
            }
        }
        else if(nptr->tok == TOK_LT || nptr->tok == TOK_GT || nptr->tok == TOK_EQ) {
            if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
                nptr->type = BOOL_TYPE;
            }
            else if(nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE) {
                nptr->type = BOOL_TYPE;
            }
            else {
                handle_error(ERR_TYPE);
            }
        }
        else if(nptr->tok == TOK_AND || nptr->tok == TOK_OR) {
            if(nptr->children[0]->type == BOOL_TYPE && nptr->children[1]->type == BOOL_TYPE) {
                nptr->type = BOOL_TYPE;
            }
            else {
                handle_error(ERR_TYPE);
            }
        }
        else if(nptr->tok == TOK_ASSIGN) {
            if(nptr->children[0]->type == ID_TYPE && (nptr->children[1]->type == STRING_TYPE || nptr->children[1]->type == INT_TYPE || nptr->children[1]->type == BOOL_TYPE)) {
                nptr->children[0]->type = nptr->children[1]->type;
            }
        }
        else {
            handle_error(ERR_TYPE);
        }
    }
    else if(nptr->children[0] != NULL && nptr->children[1] == NULL && nptr->children[2] == NULL) {
        if(nptr->tok == TOK_UMINUS) {
            if(nptr->children[0]->type == INT_TYPE) {
                nptr->type = INT_TYPE;
            }
            else if(nptr->children[0]->type == STRING_TYPE) {
                nptr->type = STRING_TYPE;
            }
            else {
                handle_error(ERR_TYPE);
            }
        }
        else if(nptr->tok == TOK_NOT) {
            if(nptr->children[0]->type == BOOL_TYPE) {
                nptr->type = BOOL_TYPE;
            }
            else {
                handle_error(ERR_TYPE);
            }
        }
    }
    else if(nptr->children[0] != NULL && nptr->children[1] != NULL && nptr->children[2] != NULL) {
        if(nptr->tok == TOK_COLON) {
            if(nptr->children[0]->type == BOOL_TYPE && nptr->children[1]->type == nptr->children[2]->type) {
                nptr->type = nptr->children[1]->type;
            }
            else {
                handle_error(ERR_TYPE);
            }
        }
        else {
            handle_error(ERR_TYPE);
        }
    }

    return;
}

/* infer_root() - set the type of the root node based on the types of children
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The type field of the node is updated. 
 */

static void infer_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        infer_type(nptr->children[1]);
    } else {
        for (int i = 0; i < 3; ++i) {
            infer_type(nptr->children[i]);
        }
        if (nptr->children[0] == NULL) {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        nptr->type = nptr->children[0]->type;
    }
    return;
}

/* eval_node() - set the value of a non-root node based on the values of children
 * Parameter: A node pointer, possibly NULL.
 * Return value: None.
 * Side effect: The val field of the node is updated.
 * (STUDENT TODO) 
 */

static void eval_node(node_t *nptr) {
    if(nptr == NULL || nptr == NT_LEAF) return;
    
    for(int i = 0; i < 3; i++) {
        eval_node(nptr->children[i]);
    }

    if(nptr->tok == TOK_PLUS) {
        if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
            nptr->val.ival = nptr->children[0]->val.ival + nptr->children[1]->val.ival;
        }
        else if(nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE) {
            nptr->val.sval = (char *)malloc(strlen(nptr->children[0]->val.sval) + strlen(nptr->children[1]->val.sval) + 2);
            strcpy(nptr->val.sval, nptr->children[0]->val.sval);
            strcat(nptr->val.sval, nptr->children[1]->val.sval);
        }
    }
    else if(nptr->tok == TOK_BMINUS) {
        if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
            nptr->val.ival = nptr->children[0]->val.ival - nptr->children[1]->val.ival;
        }
    }
    else if(nptr->tok == TOK_TIMES) {
        if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
            nptr->val.ival = nptr->children[0]->val.ival * nptr->children[1]->val.ival;
        }
        else if(nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == INT_TYPE) {
            if(nptr->children[1]->val.ival < 0) {
                handle_error(ERR_EVAL);
            }
            else {
                nptr->val.sval = (char *)malloc((strlen(nptr->children[0]->val.sval) + 1) * nptr->children[1]->val.ival);
                strcpy(nptr->val.sval, nptr->children[0]->val.sval);
                for(int i = 0; i < nptr->children[1]->val.ival - 1; i++) {
                    strcat(nptr->val.sval, nptr->children[0]->val.sval);
                }
            }
        }
    }
    else if(nptr->tok == TOK_DIV) {
        if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
            if(nptr->children[1]->val.ival == 0) {
                handle_error(ERR_EVAL);
            }
            else {
                nptr->val.ival = nptr->children[0]->val.ival / nptr->children[1]->val.ival;
            }
        }
    }
    else if(nptr->tok == TOK_MOD) {
        if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
            if(nptr->children[1]->val.ival == 0) {
                handle_error(ERR_EVAL);
            }
            else {
                nptr->val.ival = nptr->children[0]->val.ival % nptr->children[1]->val.ival;
            }
        }
    }
    else if(nptr->tok == TOK_LT) {
        if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
            nptr->val.bval = nptr->children[0]->val.ival < nptr->children[1]->val.ival;
        }
        else if(nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE) {
            if(strcmp(nptr->children[1]->val.sval, nptr->children[0]->val.sval) > 0) {
                nptr->val.bval = true;
            }
            else {
                nptr->val.bval = false;
            }
        }
    }
    else if(nptr->tok == TOK_GT) {
        if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
            nptr->val.bval = nptr->children[0]->val.ival > nptr->children[1]->val.ival;
        }
        else if(nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE) {
            if(strcmp(nptr->children[0]->val.sval, nptr->children[1]->val.sval) > 0) {
                nptr->val.bval = true;
            }
            else {
                nptr->val.bval = false;
            }
        }
    }
    else if(nptr->tok == TOK_EQ) {
        if(nptr->children[0]->type == INT_TYPE && nptr->children[1]->type == INT_TYPE) {
            nptr->val.bval = nptr->children[0]->val.ival == nptr->children[1]->val.ival;
        }
        else if(nptr->children[0]->type == STRING_TYPE && nptr->children[1]->type == STRING_TYPE) {
            if(strcmp(nptr->children[1]->val.sval, nptr->children[0]->val.sval) == 0) {
                nptr->val.bval = true;
            }
            else {
                nptr->val.bval = false;
            }
        }
    }
    else if(nptr->tok == TOK_AND) {
        if(nptr->children[0]->type == BOOL_TYPE && nptr->children[1]->type == BOOL_TYPE) {
            nptr->val.bval = nptr->children[0]->val.bval && nptr->children[1]->val.bval;
        }
    }
    else if(nptr->tok == TOK_OR) {
        if(nptr->children[0]->type == BOOL_TYPE && nptr->children[1]->type == BOOL_TYPE) {
            nptr->val.bval = nptr->children[0]->val.bval || nptr->children[1]->val.bval;
        }
    }
    else if(nptr->tok == TOK_UMINUS) {
        if(nptr->children[0]->type == INT_TYPE) {
            if(nptr->children[0]->val.ival == 0) {
                nptr->val.ival = 0;
            }
            else {
                nptr->val.ival = nptr->children[0]->val.ival * -1;
            }
        }
        else if(nptr->children[0]->type == STRING_TYPE) {
            nptr->val.sval = strrev(nptr->children[0]->val.sval);
        }
    }
    else if(nptr->tok == TOK_NOT) {
        if(nptr->children[0]->type == BOOL_TYPE) {
            nptr->val.bval = !(nptr->children[0]->val.bval);
        }
    }
    else if(nptr->tok == TOK_COLON) {
        if(nptr->children[0]->type == BOOL_TYPE) {
            if(nptr->children[1]->type == INT_TYPE && nptr->children[2]->type == INT_TYPE) {
                if(nptr->children[0]->val.bval == true) {
                    nptr->val.ival = nptr->children[1]->val.ival;
                }
                else {
                    nptr->val.ival = nptr->children[2]->val.ival;
                }
            }
            else if(nptr->children[1]->type == STRING_TYPE && nptr->children[2]->type == STRING_TYPE) {
                if(nptr->children[0]->val.bval == true) {
                    nptr->val.sval = (char *)malloc(strlen(nptr->children[1]->val.sval) + 1);
                    strcpy(nptr->val.sval, nptr->children[1]->val.sval);
                }
                else {
                    nptr->val.sval = (char *)malloc(strlen(nptr->children[2]->val.sval) + 1);
                    strcpy(nptr->val.sval, nptr->children[2]->val.sval);
                }
            }
            else if(nptr->children[1]->type == BOOL_TYPE && nptr->children[2]->type == BOOL_TYPE) {
                if(nptr->children[0]->val.bval == true) {
                    nptr->val.bval = nptr->children[1]->val.bval;
                }
                else {
                    nptr->val.bval = nptr->children[2]->val.bval;
                }
            }
        }
    }
    else if(nptr->tok == TOK_ASSIGN) {
        if(nptr->children[1]->type == INT_TYPE) {
            nptr->val.ival = nptr->children[1]->val.ival;
        }
        if(nptr->children[1]->type == BOOL_TYPE) {
            nptr->val.bval = nptr->children[1]->val.bval;
        }
        if(nptr->children[1]->type == STRING_TYPE) {
            nptr->children[0]->val.sval = (char *)malloc(strlen(nptr->children[1]->val.sval)+1);
            strcpy(nptr->val.sval, nptr->children[1]->val.sval);
        }
    }
    return;
}

/* eval_root() - set the value of the root node based on the values of children 
 * Parameter: A pointer to a root node, possibly NULL.
 * Return value: None.
 * Side effect: The val dield of the node is updated. 
 */

void eval_root(node_t *nptr) {
    if (nptr == NULL) return;
    // check running status
    if (terminate || ignore_input) return;

    // check for assignment
    if (nptr->type == ID_TYPE) {
        eval_node(nptr->children[1]);
        if (terminate || ignore_input) return;
        
        if (nptr->children[0] == NULL) {
            logging(LOG_ERROR, "failed to find child node");
            return;
        }
        put(nptr->children[0]->val.sval, nptr->children[1]);
        return;
    }

    for (int i = 0; i < 2; ++i) {
        eval_node(nptr->children[i]);
    }
    if (terminate || ignore_input) return;
    
    if (nptr->type == STRING_TYPE) {
        (nptr->val).sval = (char *) malloc(strlen(nptr->children[0]->val.sval) + 1);
        if (! nptr->val.sval) {
            logging(LOG_FATAL, "failed to allocate string");
            return;
        }
        strcpy(nptr->val.sval, nptr->children[0]->val.sval);
    } else {
        nptr->val.ival = nptr->children[0]->val.ival;
    }
    return;
}

/* infer_and_eval() - wrapper for calling infer() and eval() 
 * Parameter: A pointer to a root node.
 * Return value: none.
 * Side effect: The type and val fields of the node are updated. 
 */

void infer_and_eval(node_t *nptr) {
    infer_root(nptr);
    eval_root(nptr);
    return;
}

/* strrev() - helper function to reverse a given string 
 * Parameter: The string to reverse.
 * Return value: The reversed string. The input string is not modified.
 * (STUDENT TODO)
 */

char *strrev(char *str) {
    char *new_str = (char *)malloc(strlen(str) + 1);
    for(int i = strlen(str) - 1, j = 0; i >= 0; i--, j++) {
        new_str[j] = str[i];
    }
    new_str[strlen(str)] = '\0';
    return new_str;
}