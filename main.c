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
    BTree *tree = BTree_init(2);
    for (int i = 0; i < 10; i++) {
        BTree_insert(tree, i);
    }
    BTree_dump(tree);
    FILE *fp = fopen("data.json", "w");
    node_jobj(fp, tree->root);
    fclose(fp);
    return 0;
}
