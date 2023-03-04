#ifndef __LIST_H__
#define __LIST_H__

struct list_node {
	struct list_node *prev;
	struct list_node *next;
};

static inline void list_init(struct list_node *head)
{
	head->prev = head->next = head;
}

static inline void list_add_to_head(struct list_node *head,
				    struct list_node *node)
{
	head->prev->next = node;
	node->next = head;
	node->prev = head->prev;
	head->prev = node;
}

static inline void list_add_to_tail(struct list_node *head,
				    struct list_node *node)
{
	head->next->prev = node;
	node->next = head->next;
	node->prev = head;
	head->next = node;
}

static inline void list_remove(struct list_node *node)
{
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

static inline bool list_empty(struct list_node *head)
{
	return head->next == head;
}

static inline struct list_node *list_next(struct list_node *head)
{
	return head->next;
}

static inline struct list_node *list_tail(struct list_node *head)
{
	return head->prev;
}

#endif
