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
int radix_tree_insert_internel(struct radix_tree_node *node, uint64_t entry,
                               void *item, size_t height, size_t parent_index) {
  // split the node
  if (node->is_leaf) {
    struct radix_tree_node *new_node =
        (struct radix_tree_node *)malloc(sizeof(struct radix_tree_node));
    memset(new_node, 0, sizeof(radix_tree_node));
    new_node->is_leaf = false;
    new_node->parent = node->parent;
    // 需要获得上一级的index，也就是当前node在parent的index
    node->parent->children[parent_index] = new_node;
    // 把当前node插入分裂之后的new_node
    size_t index = node->rest >> (height * RADIX_TREE_MAP_SHIFT);
    new_node->children[index] = node;
    // rest减少一级
    node->rest = entry - (index << (height * RADIX_TREE_MAP_SHIFT));
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
    new_node->rest = entry - (index << (height * RADIX_TREE_MAP_MASK));
    new_node->element = item;
    node->children[index] = new_node;
    return 0;
  }

  if (height == 0) {
    // 已经到了最后一级
    return -1;
  }
  return radix_tree_insert_internel(
      node->children[index], entry - (index << (height * RADIX_TREE_MAP_SHIFT)),
      item, height - 1, index);
}

int radix_tree_insert(struct radix_tree *tree, uint64_t entry, void *item) {
  struct radix_tree_node *node = tree->root;
  size_t height = tree->height;
  return radix_tree_insert_internel(node, entry, item, height, -1);
}

void *radix_tree_find_internel(struct radix_tree_node *node, size_t entry,
                               size_t height) {
  size_t index = entry >> (height * RADIX_TREE_MAP_SHIFT);
  assert(index < RADIX_TREE_MAP_SIZE);
  if (node->is_leaf) {
    size_t rest = entry - (index << (height * RADIX_TREE_MAP_SHIFT));
    if (rest == node->rest)
      return node->element;
    return nullptr;
  }

  if (node->children[index] == nullptr) {
    return nullptr;
  }

  if (height == 0) {
    return nullptr;
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
  return radix_tree_find_internel(node, entry, height);
}
// void* radix_tree_find(struct radix_tree *tree, size_t entry) {
//   struct radix_tree_node *node = tree->root;
//   size_t height = tree->height;
//   size_t index = entry >> (height * RADIX_TREE_MAP_SHIFT);
//   assert(index < RADIX_TREE_MAP_SIZE);
//   while (height > 0) {
//     if (node->children[index] == nullptr) {
//       return nullptr;
//     }
//     node = (radix_tree_node *)node->children[index];
//     height--;
//     index = (entry >> (height * RADIX_TREE_MAP_SHIFT)) & RADIX_TREE_MAP_MASK;
//   }

//   return node->children[index];
// }

// delete the entry in the tree
// return 0 if success
// return -1 if fail
// int radix_tree_delete(struct radix_tree *tree, size_t entry) {
//   struct radix_tree_node *node = tree->root;
//   struct radix_tree_node *parent = nullptr;
//   size_t height = tree->height;
//   size_t index = entry >> (height * RADIX_TREE_MAP_SHIFT);
//   assert(index < RADIX_TREE_MAP_SIZE);
//   while (height > 0) {
//     if (node->children[index] == nullptr) {
//       return -1;
//     } else if (height == 1 &&
//                ((radix_tree_node *)(node->children[index]))->children[0] !=
//                    nullptr &&
//                ((radix_tree_node *)(node->children[index]))->children[1] ==
//                    nullptr) {
//       // Merge current node with its only child
//       struct radix_tree_node *child =
//           (radix_tree_node *)(((radix_tree_node *)(node->children[index]))
//                                   ->children[0]);
//       node->children[index] = child;
//       child->parent = node;
//       free(((radix_tree_node *)(node->children[index]))->children[0]);
//       ((radix_tree_node *)(node->children[index]))->children[0] = nullptr;
//     } else {
//       parent = node;
//       node = (radix_tree_node *)node->children[index];
//     }
//     height--;
//     index = (entry >> (height * RADIX_TREE_MAP_SHIFT)) & RADIX_TREE_MAP_MASK;
//   }
//   if (node == nullptr) {
//     return -1;
//   }
//   if (parent == nullptr) {
//     tree->root = (radix_tree_node *)node->children[index];
//   } else {
//     parent->children[index] = node->children[index];
//     if (node->children[index] != nullptr) {
//       ((radix_tree_node *)(node->children[index]))->parent = parent;
//     }
//   }
//   free(node);
//   return 0;
// }
// int radix_tree_delete(struct radix_tree *tree, size_t entry) {
//   struct radix_tree_node *node = tree->root;
//   size_t height = tree->height;
//   size_t index = entry >> (height * 8);
//   assert(index < 256);
//   while (height > 0) {
//     if (node->children[index] == nullptr) {
//       return -1;
//     }
//     node = (radix_tree_node *)node->children[index];
//     height--;
//     index = (entry >> (height * RADIX_TREE_MAP_SHIFT)) & RADIX_TREE_MAP_MASK;
//   }

//   node->children[index] = nullptr;
//   return 0;
// }

// test the tree
void radix_tree_test() {
  struct radix_tree tree;
  tree.root = (struct radix_tree_node *)malloc(sizeof(struct radix_tree_node));
  tree.root->parent = nullptr;
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
  /* for (size_t i = 0; i < 100; i++) { */
  /*   if (radix_tree_delete(&tree, i) != 0) { */
  /*     printf("delete %ld failed\n", i); */
  /*   } */
  /* } */
  /* for (size_t i = 0; i < 100; i++) { */
  /*   if (radix_tree_find(&tree, i) != nullptr) { */
  /*     printf("delete %ld failed\n", i); */
  /*   } */
  /* } */
}
