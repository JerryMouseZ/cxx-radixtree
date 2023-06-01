#ifndef _radix_tree_H
#define _radix_tree_H
#include <cstdint>
#include <stdint.h>

#define RADIX_TREE_MAP_SHIFT 6
#define RADIX_TREE_MAP_SIZE (1 << RADIX_TREE_MAP_SHIFT)
#define RADIX_TREE_MAP_MASK (RADIX_TREE_MAP_SIZE - 1)
#define RADIX_TREE_TAG_LONGS 2

/* struct radix_tree_node { */
/*   unsigned int prefix; */
/*   uint8_t prefix_len; */
/*   uint8_t height; */
/*   uint8_t count; // child count */
/*   radix_tree_node *parent; */
/*   void *slot[64]; */
/*   /1* 每个bit表示一个tag，有三种tag, 0表示脏，1表示需要写回, 2表示需要删除
 * *1/ */
/*   /1* uint64_t tags[3]; *1/ */
/* }; */

/* struct radix_tree_root { */
/*   unsigned int height; */
/*   struct radix_tree_node *node; */
/* }; */

// 256 * 30 = 7680
struct radix_tree_node {
  struct radix_tree_node *parent;
  uint64_t rest;
  /* uint8_t height; */
  void *element;
  bool is_leaf;
  radix_tree_node *children[64];
};

struct radix_tree {
  uint8_t height;
  struct radix_tree_node *root;
};

radix_tree_node *radix_tree_node_alloc();
radix_tree *radix_tree_init();
int radix_tree_insert(struct radix_tree *root, unsigned long index, void *item);

void *radix_tree_find(struct radix_tree *root, unsigned long index);

int radix_tree_delete(struct radix_tree *root, unsigned long index);

void radix_tree_test();
// 先不考虑tag
/* void *radix_tree_tag_set(struct radix_tree_root *root, unsigned long index,
 */
/*                          unsigned int tag); */

/* void *radix_tree_tag_clear(struct radix_tree_root *root, unsigned long index,
 */
/*                            unsigned int tag); */
#endif
