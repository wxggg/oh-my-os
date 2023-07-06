#ifndef __LIST_H__
#define __LIST_H__

#include <types.h>

struct list_node {
	struct list_node *prev;
	struct list_node *next;
};

static inline void list_init(struct list_node *head)
{
	head->prev = head->next = head;
}

static inline void list_insert_before(struct list_node *cur, struct list_node *node)
{
	cur->prev->next = node;
	node->next = cur;
	node->prev = cur->prev;
	cur->prev = node;
}

static inline void list_insert(struct list_node *cur, struct list_node *node)
{
	cur->next->prev = node;
	node->next = cur->next;
	node->prev = cur;
	cur->next = node;
}

static inline void list_insert_head(struct list_node *head, struct list_node *node)
{
	list_insert(head, node);
}

static inline void list_insert_tail(struct list_node *head, struct list_node *node)
{
	list_insert_before(head, node);
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

static inline unsigned long list_size(struct list_node *head)
{
	struct list_node *node = head->next;
	unsigned long size = 0;

	for (node = head->next; node != head; node = node->next)
		size++;

	return size;
}

#endif
