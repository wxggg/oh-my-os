#include <types.h>
#include <error.h>
#include <string.h>
#include <kernel.h>
#include <kmalloc.h>
#include <rb_tree.h>

#define TREE_DEBUG

struct rb_node {
#ifdef TREE_DEBUG
	unsigned long id;
#endif

	unsigned long start;
	unsigned long end;
	void *value;

	bool red;
	struct rb_node *parent;
	struct rb_node *left;
	struct rb_node *right;
};

struct rb_tree {
#ifdef TREE_DEBUG
	unsigned long total;
#endif
	struct rb_node *root;
};


#ifdef TREE_DEBUG
static inline unsigned long node_id(struct rb_node *node)
{
	return node ? node->id : 0;
}
#endif

static inline void set_red(struct rb_node *node)
{
	if (node)
		node->red = true;
}

static inline void set_black(struct rb_node *node)
{
	if (node)
		node->red = false;
}

static inline bool is_red(struct rb_node *node)
{
	return node && node->red;
}

static inline bool is_black(struct rb_node *node)
{
	return !is_red(node);
}

unsigned long rb_node_key_start(struct rb_node *node)
{
	return node->start;
}

unsigned long rb_node_key_end(struct rb_node *node)
{
	return node->end;
}

void *rb_node_value(struct rb_node *node)
{
	return node->value;
}

#ifdef TREE_DEBUG
static void node_validate(struct rb_node *node)
{
	if (!node)
		return;

	if (node->left) {
		assert(node->start > node->left->end);
		assert(node->left->parent == node);
	}

	if (node->right) {
		assert(node->end < node->right->start);
		assert(node->right->parent == node);
	}

	assert(!(is_red(node) && is_red(node->left)));
	assert(!(is_red(node) && is_red(node->right)));
}

static void tree_dump(struct rb_node *node, uint32_t level)
{
	if (!node)
		return;

	tree_dump(node->right, level + 1);

	pr_info(repeat("    ", level),
	        node->parent ? (node == node->parent->left ? "\\" : "/") : "<",
		"----",
		"(", dec(node_id(node)), ", ", is_red(node) ? "r" : "b", ") ",
		"<", dec(node_id(node->parent)), ",", dec(node_id(node->left)), ",",
		dec(node_id(node->right)), "> ", range(node->start, node->end));

	tree_dump(node->left, level + 1);
}

static int tree_validate(struct rb_node *root, struct rb_node **fail_node)
{
	int left, right;

	if (!root)
		return 0;

	if (*fail_node)
		return -EINVAL;

	node_validate(root);

	left = tree_validate(root->left, fail_node);
	if (left < 0)
		return left;

	right = tree_validate(root->right, fail_node);
	if (right < 0)
		return right;

	if (left != right) {
		pr_err("invalid node-", dec(node_id(root)),
		       " black node node equal ", pair(left, right));
		*fail_node = root;
		return -EINVAL;
	}

	return left + (is_black(root) ? 1 : 0);
}

static void rb_tree_validate(struct rb_tree *tree)
{
	struct rb_node *fail_node = NULL;

	if (!tree->root)
		return;

	assert(is_black(tree->root));

	if (tree_validate(tree->root, &fail_node) < 0) {
		tree_dump(tree->root, 0);
		pr_err("validate node-", dec(node_id(fail_node)), " failed");
		assert(0);
	}
}
#endif

struct rb_node * rb_tree_search(struct rb_tree *tree, unsigned long key)
{
	struct rb_node *node = tree->root;
	while (node) {
		if (key < node->start)
			node = node->left;
		else if (key > node->end)
			node = node->right;
		else
			return node;
	}

	return NULL;
}

static void rotate_left(struct rb_node **root, struct rb_node *nodex)
{
	struct rb_node *nodey = nodex->right;

	nodex->right = nodey->left;
	if (nodey->left)
		nodey->left->parent = nodex;

	nodey->parent = nodex->parent;

	if (nodex == *root)
		*root = nodey;
	else if (nodex == nodex->parent->left)
		nodex->parent->left = nodey;
	else
		nodex->parent->right = nodey;

	nodey->left = nodex;
	nodex->parent = nodey;
}

static void rotate_right(struct rb_node **root, struct rb_node *nodex)
{
	struct rb_node *nodey = nodex->left;

	nodex->left = nodey->right;
	if (nodey->right)
		nodey->right->parent = nodex;

	nodey->parent = nodex->parent;

	if (nodex == *root)
		*root = nodey;
	else if (nodex == nodex->parent->left)
		nodex->parent->left = nodey;
	else
		nodex->parent->right = nodey;

	nodey->right = nodex;
	nodex->parent = nodey;
}

static void insert_fixup(struct rb_node **root, struct rb_node *nodex)
{
	struct rb_node *nodey;

	while (nodex != *root && is_red(nodex->parent)) {
		if (nodex->parent == nodex->parent->parent->left) {
			nodey = nodex->parent->parent->right;
			if (is_red(nodey)) {
				set_black(nodey);
				set_black(nodex->parent);
				set_red(nodex->parent->parent);
				nodex = nodex->parent->parent;
			} else {
				if (nodex == nodex->parent->right) {
					nodex = nodex->parent;
					rotate_left(root, nodex);
				}
				set_black(nodex->parent);
				set_red(nodex->parent->parent);
				rotate_right(root, nodex->parent->parent);
			}
		} else {
			nodey = nodex->parent->parent->left;
			if (is_red(nodey)) {
				set_black(nodey);
				set_black(nodex->parent);
				set_red(nodex->parent->parent);
				nodex = nodex->parent->parent;
			} else {
				if (nodex == nodex->parent->left) {
					nodex = nodex->parent;
					rotate_right(root, nodex);
				}
				set_black(nodex->parent);
				set_red(nodex->parent->parent);
				rotate_left(root, nodex->parent->parent);
			}
		}
	}
	set_black(*root);
}

struct rb_node * rb_tree_insert(struct rb_tree *tree, unsigned long start,
				unsigned long end, void *value)
{
	struct rb_node *node, *parent, *new_node;

	if (!tree || start > end)
		return NULL;

	node = tree->root;
	parent = NULL;

	while (node) {
		parent = node;
		if (start > node->end) {
			node = node->right;
		} else if (end < node->start) {
			node = node->left;
		} else {
			pr_err("insert ", range(start, end), " overlaps with ",
			       range(node->start, node->end));
			return NULL;
		}
	}

	new_node = (struct rb_node *) kmalloc(sizeof(struct rb_node));
	if (!new_node)
		return NULL;

#ifdef TREE_DEBUG
	new_node->id = tree->total++;
#endif
	new_node->start = start;
	new_node->end = end;
	new_node->parent = parent;
	new_node->left = NULL;
	new_node->right = NULL;
	new_node->value = value;
	set_red(new_node);

	if (parent) {
		if (new_node->end < parent->start)
			parent->left = new_node;
		else if (new_node->start > parent->end)
			parent->right = new_node;
		else
			goto fail;
	} else {
		tree->root = new_node;
	}

	insert_fixup(&tree->root, new_node);

#ifdef TREE_DEBUG
	rb_tree_validate(tree);
#endif
	return new_node;

fail:
	kfree(new_node);
	return NULL;
}

static void remove_fixup(struct rb_node **root, struct rb_node *parent, struct rb_node *nodex)
{
	struct rb_node *nodey;

	while (nodex != *root && is_black(nodex)) {
		assert(!(!nodex && !parent));

		if (parent) {
			if (nodex == parent->left) {
				nodey = parent->right;

				if (is_red(nodey)) {
					set_black(nodey);
					set_red(parent);
					rotate_left(root, parent);
					nodey = parent->right;
				}

				if (!nodey ||
				    (is_black(nodey->left) && is_black(nodey->right))) {
					set_red(nodey);
					nodex = parent;
				} else {
					if (is_red(nodey->left) && is_black(nodey->right)) {
						set_red(nodey);
						set_black(nodey->left);
						rotate_right(root, nodey);
						nodey = parent->right;
					}

					nodey->red = parent->red;
					set_black(parent);
					set_black(nodey->right);
					rotate_left(root, parent);

					nodex = *root;
				}
			} else {
				nodey = parent->left;

				if (is_red(nodey)) {
					set_black(nodey);
					set_red(parent);
					rotate_right(root, parent);
					nodey = parent->left;
				}

				if (!nodey || (is_black(nodey->left) && is_black(nodey->right))) {
					set_red(nodey);
					nodex = parent;
				} else {
					if (is_red(nodey->right) && is_black(nodey->left)) {
						set_red(nodey);
						set_black(nodey->right);
						rotate_left(root, nodey);
						nodey = parent->left;
					}

					nodey->red = parent->red;
					set_black(parent);
					set_black(nodey->left);
					rotate_right(root, parent);

					nodex = *root;
				}
			}
			parent = nodex->parent;
		}
	}
	set_black(nodex);
}

int rb_tree_remove(struct rb_tree *tree, unsigned long key)
{
	struct rb_node *nodex, *nodey, *nodez, *nodex_parent;
	bool fixup;

	if (!tree)
		return -EINVAL;

	nodez = rb_tree_search(tree, key);
	if (!nodez)
		return -EINVAL;

	if (!nodez->left || !nodez->right)
		nodey = nodez;
	else {
		nodey = nodez->right;
		while (nodey->left)
			nodey = nodey->left;
	}

	nodex = nodey->left ? nodey->left : nodey->right;

	if (nodex)
		nodex->parent = nodey->parent;

	if (nodey->parent) {
		if (nodey == nodey->parent->left)
			nodey->parent->left = nodex;
		else
			nodey->parent->right = nodex;
	} else {
		tree->root = nodex;
	}

	fixup = is_black(nodey);

	nodex_parent = nodey->parent;
	if (nodey != nodez) {
		nodey->parent = nodez->parent;
		if (nodez->parent) {
			if (nodez == nodez->parent->left)
				nodez->parent->left = nodey;
			else
				nodez->parent->right = nodey;
		} else {
			tree->root = nodey;
		}

		nodey->red = nodez->red;
		nodey->left = nodez->left;
		nodey->right = nodez->right;

		if (nodez->left)
			nodez->left->parent = nodey;

		if (nodez->right)
			nodez->right->parent = nodey;

		if (nodex_parent == nodez)
			nodex_parent = nodey;
	}

	if (fixup)
		remove_fixup(&tree->root, nodex_parent, nodex);

	kfree(nodez);

#ifdef TREE_DEBUG
	rb_tree_validate(tree);
#endif
	return 0;
}

struct rb_tree * rb_tree_create(void)
{
	struct rb_tree *tree;

	tree = (struct rb_tree *) kmalloc(sizeof(*tree));
	if (!tree)
		return NULL;

#ifdef TREE_DEBUG
	tree->total = 0;
#endif
	tree->root = NULL;
	return tree;
}

static void tree_delete(struct rb_node *root)
{
	if (!root)
		return;

	tree_delete(root->left);
	tree_delete(root->right);
	kfree(root);
}

void rb_tree_delete(struct rb_tree *tree)
{
	if (!tree)
		return;

	tree_delete(tree->root);
	kfree(tree);
}

static int tree_iterate(struct rb_node *node, rb_tree_pfn_callback callback, void *priv)
{
	int ret;

	if (!node)
		return 0;

	ret = tree_iterate(node->left, callback, priv);
	if (ret)
		return ret;

	ret = callback(node, priv);
	if (ret)
		return ret;

	ret = tree_iterate(node->right, callback, priv);
	if (ret)
		return ret;

	return ret;
}

int rb_tree_iterate(struct rb_tree *tree, rb_tree_pfn_callback callback, void *priv)
{
	return tree_iterate(tree->root, callback, priv);
}

