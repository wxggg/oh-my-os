#pragma once

struct rb_node;
struct rb_tree;

typedef int (*rb_tree_pfn_callback)(struct rb_node *, void *);

unsigned long rb_node_key_start(struct rb_node *node);
unsigned long rb_node_key_end(struct rb_node *node);
void *rb_node_value(struct rb_node *node);

struct rb_node * rb_tree_search(struct rb_tree *tree, unsigned long key);
struct rb_node * rb_tree_insert(struct rb_tree *tree, unsigned long start,
				unsigned long end, void *value);
int rb_tree_remove(struct rb_tree *tree, unsigned long key);

struct rb_tree * rb_tree_create(void);
void rb_tree_delete(struct rb_tree *tree);
int rb_tree_iterate(struct rb_tree *tree, rb_tree_pfn_callback callback, void *priv);
