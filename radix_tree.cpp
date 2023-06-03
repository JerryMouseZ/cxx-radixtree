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

int radix_tree_insert_internel(struct radix_tree_node *node, uint64_t entry,
                               void *item, size_t height, size_t parent_index) {
  // split the node
  if (node->is_leaf) {
    struct radix_tree_node *new_node =
        (struct radix_tree_node *)malloc(sizeof(struct radix_tree_node));
    memset(new_node, 0, sizeof(radix_tree_node));
    new_node->is_leaf = false;
    new_node->height = height + 1;
    node->height = height;
    new_node->parent = node->parent;
    // 需要获得上一级的index，也就是当前node在parent的index
    node->parent->children[parent_index] = new_node;
    node->parent = new_node;
    // 把当前node插入分裂之后的new_node
    size_t index = node->rest >> (height * RADIX_TREE_MAP_SHIFT);
    new_node->children[index] = node;
    // rest减少一级
    node->rest = node->rest - (index << (height * RADIX_TREE_MAP_SHIFT));
    node = new_node;
  }

  size_t index = entry >> (height * RADIX_TREE_MAP_SHIFT);
  assert(index < RADIX_TREE_MAP_SIZE);
  if (node->children[index] == nullptr) {
    struct radix_tree_node *new_node =
        (radix_tree_node *)malloc(sizeof(radix_tree_node));
    memset(new_node, 0, sizeof(radix_tree_node));
    new_node->parent = node;
    new_node->is_leaf = true;
    new_node->height = height;
    new_node->rest = entry - (index << (height * RADIX_TREE_MAP_SHIFT));
    new_node->element = item;
    node->children[index] = new_node;
    return 0;
  }

  if (height == 0) {
    // 已经到了最后一级，进入这个分支说明之前已经有这个元素了
    return -1;
  }

  return radix_tree_insert_internel(
      node->children[index], entry - (index << (height * RADIX_TREE_MAP_SHIFT)),
      item, height - 1, index);
}

// insert a new entry into the tree
// return 0 if success
// return -1 if fail
int radix_tree_insert(struct radix_tree *tree, uint64_t entry, void *item) {
  struct radix_tree_node *node = tree->root;
  size_t height = tree->height;
  return radix_tree_insert_internel(node, entry, item, height, -1);
}

radix_tree_node *radix_tree_find_internel(struct radix_tree_node *node,
                                          size_t entry, size_t height) {
  size_t index = entry >> (height * RADIX_TREE_MAP_SHIFT);
  assert(index < RADIX_TREE_MAP_SIZE);
  if (node->is_leaf) {
    size_t rest = entry - (index << (height * RADIX_TREE_MAP_SHIFT));
    if (rest == node->rest)
      return node;
    return nullptr;
  }

  if (node->children[index] == nullptr) {
    return nullptr;
  }

  if (height == 0) {
    return node->children[index];
  }

  return radix_tree_find_internel(
      node->children[index], entry - (index << (height * RADIX_TREE_MAP_SHIFT)),
      height - 1);
}

// find the entry in the tree
// return 0 if success
// return -1 if fail
void *radix_tree_find(struct radix_tree *tree, size_t entry) {
  struct radix_tree_node *node = tree->root;
  size_t height = tree->height;
  node = radix_tree_find_internel(node, entry, height);
  if (node)
    return node->element;
  return nullptr;
}

// delete the entry in the tree
// return 0 if success
// return -1 if fail
int radix_tree_delete(struct radix_tree *tree, unsigned long entry) {
  radix_tree_node *node = tree->root;
  size_t height = tree->height;
  node = radix_tree_find_internel(node, entry, height);
  if (node == nullptr)
    return -1;
  size_t index = entry >> (node->height * RADIX_TREE_MAP_SHIFT);
  index &= RADIX_TREE_MAP_MASK;
  radix_tree_node *backup = node->parent->children[0];
  node->parent->children[index] = nullptr;
  free(backup);
  return 0;
}

void radix_tree_free_internel(radix_tree_node *node, size_t height) {
  for (int i = 0; i < RADIX_TREE_MAP_SIZE; i++) {
    if (node->children[i] != nullptr) {
      if (height == 0) {
        free(node->children[i]);
      } else {
        radix_tree_free_internel(node->children[i], height - 1);
      }
    }
  }
}

void radix_tree_free(struct radix_tree *tree) {
  radix_tree_node *node = tree->root;
  size_t height = tree->height;
  radix_tree_free_internel(node, height);
}

// test the tree
void radix_tree_test() {
  struct radix_tree tree;
  tree.root = (struct radix_tree_node *)malloc(sizeof(struct radix_tree_node));
  tree.root->parent = nullptr;
  tree.root->height = 3;
  memset(tree.root->children, 0, sizeof(tree.root->children));
  tree.height = 2;
  for (size_t i = 0; i < 100; i++) {
    radix_tree_insert(&tree, i, (void *)(i + 1));
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
