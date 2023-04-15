#include "radix_tree.h"
#include <cstdint>
#include <stdlib.h>
#include <string.h>
//
// Created by jz on 23-3-23.
//
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

// insert a new entry into the tree
// return 0 if success
// return -1 if fail
void radix_tree_insert(struct radix_tree *tree, uint64_t entry, void *item) {
  struct radix_tree_node *node = tree->root;
  size_t height = tree->height;
  size_t index = entry >> (height * RADIX_TREE_MAP_SHIFT);
  assert(index < RADIX_TREE_MAP_SIZE);
  while (height > 0) {
    if (node->children[index] == nullptr) {
      radix_tree_node *new_node = (struct radix_tree_node *) malloc(sizeof(struct radix_tree_node));
      new_node->parent = node;
      memset(new_node->children, 0, 64 * sizeof(void *));
      node->children[index] = new_node;
    }
    node = (radix_tree_node *) node->children[index];
    height--;
    index = (entry >> (height * RADIX_TREE_MAP_SHIFT)) & RADIX_TREE_MAP_MASK;
  }
  node->children[index] = item;
}

// find the entry in the tree
// return 0 if success
// return -1 if fail
void* radix_tree_find(struct radix_tree *tree, size_t entry) {
  struct radix_tree_node *node = tree->root;
  size_t height = tree->height;
  size_t index = entry >> (height * RADIX_TREE_MAP_SHIFT);
  assert(index < RADIX_TREE_MAP_SIZE);
  while (height > 0) {
    if (node->children[index] == nullptr) {
      return nullptr;
    }
    node = (radix_tree_node *)node->children[index];
    height--;
    index = (entry >> (height * RADIX_TREE_MAP_SHIFT)) & RADIX_TREE_MAP_MASK;
  }

  return node->children[index];
}

// delete the entry in the tree
// return 0 if success
// return -1 if fail
int radix_tree_delete(struct radix_tree *tree, size_t entry) {
  struct radix_tree_node *node = tree->root;
  size_t height = tree->height;
  size_t index = entry >> (height * 8);
  assert(index < 256);
  while (height > 0) {
    if (node->children[index] == nullptr) {
      return -1;
    }
    node = (radix_tree_node *)node->children[index];
    height--;
    index = (entry >> (height * RADIX_TREE_MAP_SHIFT)) & RADIX_TREE_MAP_MASK;
  }

  node->children[index] = nullptr;
  return 0;
}

// test the tree
void radix_tree_test() {
  struct radix_tree tree;
  tree.root = (struct radix_tree_node *) malloc(sizeof(struct radix_tree_node));
  tree.root->parent = nullptr;
  memset(tree.root->children, 0, sizeof(tree.root->children));
  tree.height = 2;
  for (size_t i = 0; i < 100; i++) {
    radix_tree_insert(&tree, i, (void *) (i + 1));
  }
  for (size_t i = 0; i < 100; i++) {
    if (radix_tree_find(&tree, i) != (void *)(i + 1)) {
      printf("find %ld failed\n", i);
    }
  }
  for (size_t i = 0; i < 100; i++) {
    if (radix_tree_delete(&tree, i) != 0) {
      printf("delete %ld failed\n", i);
    }
  }
  for (size_t i = 0; i < 100; i++) {
    if (radix_tree_find(&tree, i) != nullptr) {
      printf("delete %ld failed\n", i);
    }
  }
}

