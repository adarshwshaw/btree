/**
 * Copyright (c) 2024 Adarsh Shaw
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.  

 *
 * File: msin.c
 * Author: Adarsh Shaw
 * Date: 2024-08-13
 * Description: Simple b tree implementation
 * Dependencies: NA
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
typedef unsigned int uint;
#define NOT_IMP assert(FALSE && "NOT IMPLEMENTED");

typedef struct {
    uint itemSize;
    uint len;
    uint cap;
} Array_hdr;

typedef struct Node {
    _Bool isleaf;
    int *data;
    struct Node **childrens;
} Node;

typedef struct BTree {
    Node *root;
    size_t degree;
} BTree;

#define MAX_CELLS(degree) degree * 2 - 1 /*returns the max keys in the node*/

#define ARR_HDR(arr) ((Array_hdr *)(arr))[-1]

#define ARR_LEN(arr) ARR_HDR((arr)).len

#define ARRAY_INSERT(arr, idx, data)                                           \
    do {                                                                       \
        assert(ARR_LEN((arr)) < ARR_HDR((arr)).cap && "Error: Overflow");      \
        size_t itemSize = ARR_HDR(arr).itemSize;                               \
        size_t bytes = itemSize * (ARR_LEN(arr) - (idx));                      \
        char *src = (char *)(arr + idx);                                       \
        char *dst = src + itemSize;                                            \
        memmove(dst, src, bytes);                                              \
        (arr)[(idx)] = (data);                                                 \
        ARR_LEN((arr)) += 1;                                                   \
    } while (0)

#define ARRAY_REMOVE(arr, idx)                                                 \
    do {                                                                       \
        assert(ARR_LEN((arr)) > idx && "Error: underflow");                    \
        size_t itemSize = ARR_HDR((arr)).itemSize;                             \
        size_t bytes = itemSize * (ARR_LEN((arr)) - (idx) + 1);                \
        char *src = (char *)((arr) + idx + 1);                                 \
        char *dst = src - itemSize;                                            \
        memmove(dst, src, bytes);                                              \
        ARR_LEN((arr)) -= 1;                                                   \
    } while (0)

void *Array_init(uint itemSize, uint cap) {
    Array_hdr *arr = calloc(1, sizeof(*arr) + itemSize * cap);
    arr->cap = cap;
    arr->itemSize = itemSize;
    return &arr[1];
}

Node *Node_init(size_t degree) {
    Node *node = calloc(1, sizeof(*node));
    node->data =
        Array_init(sizeof(*node->data),
                   MAX_CELLS(degree)); // calloc(max_cell, sizeof(*node->data));
    node->childrens =
        Array_init(sizeof(*node->childrens),
                   MAX_CELLS(degree) +
                       1); // calloc(max_cell + 1, sizeof(*node->childrens));
    node->isleaf = TRUE;
    return node;
}

void Node_dump(Node *node, int lvl);

BTree *BTree_init(size_t degree) {
    BTree *tree = malloc(sizeof(*tree));
    tree->degree = degree;
    tree->root = Node_init(degree);
    return tree;
}

Node *Node_split(Node *node, size_t degree) {
    Node *tmp = Node_init(degree);
    tmp->isleaf = FALSE;
    ARRAY_INSERT(tmp->childrens, 0, node);
    return tmp;
}

void Node_splitChild(Node *root, int idx, int degree) {
    Node *child = root->childrens[idx];
    Node *newnode = Node_init(degree);
    newnode->isleaf = child->isleaf;
    // move keys from degree to (2*degree-1)
    memmove(newnode->data, child->data + degree,
            ARR_HDR(child->data).itemSize * (MAX_CELLS(degree) - degree));
    // for (int i = degree; i < ARR_LEN(child->data); i++) {
    //     ARRAY_INSERT(newnode->data, i - degree, child->data[i]);
    // }
    ARR_LEN(child->data) = degree;
    ARR_LEN(newnode->data) = MAX_CELLS(degree) - degree;
    // Node_dump(newnode, 0);

    // move middle element to root
    ARRAY_INSERT(root->data, idx, child->data[degree - 1]);
    ARR_LEN(child->data)--;
    ARRAY_INSERT(root->childrens, idx + 1, newnode);

    // move childrens if not leaf node
    if (!child->isleaf) {
        memmove(newnode->childrens, child->childrens + degree,
                ARR_HDR(child->childrens).itemSize *
                    (ARR_LEN(child->childrens) - degree));
        ARR_LEN(child->childrens) = degree;
        ARR_LEN(newnode->childrens) = MAX_CELLS(degree) + 1 - degree;
    }
}

void Node_insertNotFull(Node *node, int data, int degree) {
    int i = ARR_LEN(node->data) - 1;
    if (node->isleaf) {
        while (i >= 0 && node->data[i] > data) {
            i--;
        }
        i++;
        ARRAY_INSERT(node->data, i, data);
    } else {
        while (i >= 0 && node->data[i] > data) {
            i--;
        }
        i++;
        if (ARR_LEN(node->childrens[i]->data) == MAX_CELLS(degree)) {
            Node_splitChild(node, i, degree);
            if (data > node->data[i])
                i++;
        }
        Node_insertNotFull(node->childrens[i], data, degree);
    }
}

void BTree_insert(BTree *tree, int data) {
    if (ARR_LEN(tree->root->data) == MAX_CELLS(tree->degree)) {
        tree->root = Node_split(tree->root, tree->degree);
        Node_splitChild(tree->root, 0, tree->degree);
        Node_insertNotFull(tree->root, data, tree->degree);
    } else {
        Node_insertNotFull(tree->root, data, tree->degree);
    }
}

void Node_deleteFromLeaf(Node *node, int idx) { ARRAY_REMOVE(node->data, idx); }

void Node_deleteFromNonLeaf(Node *node, int idx, int degree) {
    NOT_IMP
    //     if (ARR_LEN(node->childrens[idx]->data) >= degree){
    // 	Node* cur = node->childrens[idx];
    // 	while (!cur->isleaf){
    // 	    cur= cur->childrens[ARR_LEN(cur->data)];
    // 	}
    // 	node->data[idx]=cur->data[ARR_LEN(cur->data)-1];
    // 	Node_delete(node->childrens[idx],node->data[idx]);
    //     }else if (ARR_LEN(node->childrens[idx+1]->data) >= degree){
    // 	Node* cur = node->childrens[idx+1];
    // 	while (!cur->isleaf){
    // 	    cur= cur->childrens[0];
    // 	}
    // 	node->data[idx]=cur->data[0];
    // 	Node_delete(node->childrens[idx+1],node->data[idx]);
    //     }else{
    // 	assert(FALSE && "Merge Non implemented"
    //     }
}
void Node_merge(Node *node, int idx, size_t degree) { NOT_IMP }
void Node_fill(Node *node, int idx, size_t degree) {
    // check previous or later sibling if one of them has more than degree the
    // pull one from there
    if (idx != 0 && ARR_LEN(node->childrens[idx - 1]->data) >= degree) {
        Node *child = node->childrens[idx];
        Node *sibling = node->childrens[idx - 1];

        ARRAY_INSERT(child->data, 0, node->data[idx - 1]);
        if (!child->isleaf) {
            ARRAY_INSERT(child->childrens, 0,
                         sibling->childrens[ARR_LEN(sibling->childrens) - 1]);
        }
        node->data[idx - 1] = sibling->data[ARR_LEN(sibling->data) - 1];
        ARRAY_REMOVE(sibling->data, ARR_LEN(sibling->data) - 1);
        ARRAY_REMOVE(sibling->childrens, ARR_LEN(sibling->childrens) - 1);

    }

    else if (idx != ARR_LEN(node->data) &&
             ARR_LEN(node->childrens[idx + 1]->data) >= degree) {
        Node *child = node->childrens[idx];
        Node *sibling = node->childrens[idx + 1];

        ARRAY_INSERT(child->data, ARR_LEN(child->data), node->data[idx]);
        if (!child->isleaf) {
            ARRAY_INSERT(child->childrens, ARR_LEN(child->childrens),
                         sibling->childrens[0]);
            ARRAY_REMOVE(sibling->childrens, 0);
        }
        node->data[idx] = sibling->data[0];
        ARRAY_REMOVE(sibling->data, 0);

    } else {
        if (idx != ARR_LEN(node->data)) {
            Node_merge(node, idx, degree);
        } else {
            Node_merge(node, idx - 1, degree);
        }
    }
}

void Node_delete(Node *node, int data, size_t degree) {
    int idx = 0;
    while (idx < ARR_LEN(node->data) && node->data[idx] < data) {
        idx++;
    }
    printf("to delete idx=%d\n", idx);
    if (node->data[idx] == data) {
        if (node->isleaf) {
            Node_deleteFromLeaf(node, idx);
        } else {
            Node_deleteFromNonLeaf(node, idx, degree);
        }
    } else {

        if (node->isleaf) {
            printf("Error: key not found!");
            return;
        }
        _Bool flag = idx == ARR_LEN(node->data);
        if (ARR_LEN(node->childrens[idx]->data) < degree) {
            Node_fill(node, idx, degree);
        }

        if (flag && idx > ARR_LEN(node->data)) {
            Node_delete(node->childrens[idx - 1], data, degree);
        } else {
            Node_delete(node->childrens[idx], data, degree);
        }
    }
}
//

void BTree_delete(BTree *tree, int data) {
    printf("starting delete process for %d\n", data);
    Node_delete(tree->root, data, tree->degree);
}

void Node_dump(Node *node, int lvl) {
    printf("level %d:%d %d:", lvl, ARR_LEN(node->childrens),
           ARR_LEN(node->data));
    for (int i = 0; i < ARR_LEN(node->data); i++) {
        printf("%d ", node->data[i]);
    }
    printf("\n");
    for (int i = 0; i < ARR_LEN(node->childrens); i++) {
        Node_dump(node->childrens[i], lvl + 1);
    }
}

void BTree_dump(BTree *tree) { Node_dump(tree->root, 0); }

void node_jobj(FILE *fp, Node *node) {
    fwrite("{\n", 1, 2, fp);
    fprintf(fp, "\"keys\": [ ");
    for (int i = 0; i < ARR_LEN(node->data); i++) {
        if (i != ARR_LEN(node->data) - 1)
            fprintf(fp, "%d, ", node->data[i]);
        else
            fprintf(fp, "%d ", node->data[i]);
    }
    fprintf(fp, "],\n");

    fprintf(fp, "\"childrens\": [ ");
    for (int i = 0; i < ARR_LEN(node->childrens); i++) {
        node_jobj(fp, node->childrens[i]);
        if (i == ARR_LEN(node->childrens))
            fprintf(fp, ",");
    }
    fprintf(fp, "]");
    fwrite("}\n", 1, 2, fp);
}

int main(int argc, char **argv) {
    BTree *tree = BTree_init(3);
    for (int i = 0; i < 10; i++) {
        BTree_insert(tree, i);
    }
    BTree_dump(tree);
    FILE *fp = fopen("data.json", "w");
    node_jobj(fp, tree->root);
    fclose(fp);

    for (int i = 9; i > 0; i--) {
        BTree_delete(tree, i);
        printf("deleted %d\n", i);
        BTree_dump(tree);
        if (i == 9)
            i = 5;
    }
    return 0;
}
