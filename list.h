#ifndef LIST_H
#define LIST_H

#include <stddef.h>

#define __LOCAL(var, line) __ ## var ## line
#define _LOCAL(var, line) __LOCAL(var, line)
#define LOCAL(var) _LOCAL(var, __LINE__)

#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) *__mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })

struct list_node {
	struct list_node *next;
	struct list_node *prev;
};

struct list_head {
	struct list_node n;
};

#define LIST_HEAD_INIT(name) { { &(name.n), &(name.n) } }
#define LIST_NODE_INIT { NULL, NULL }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)
#define LIST_NODE(name) \
	struct list_node name = LIST_NODE_INIT

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->n.next = &list->n;
	list->n.prev = &list->n;
}

static inline void INIT_LIST_NODE(struct list_node *list)
{
	list->next = NULL;
	list->prev = NULL;
}

#define list_first_entry(head, type, member) \
	list_entry((head)->n.next, type, member)

static inline int list_empty(const struct list_head *head)
{
	return head->n.next == &head->n;
}

static inline int list_linked(const struct list_node *node)
{
	return node->next != NULL;
}

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_for_each(pos, head)					\
	for (typeof(pos) LOCAL(n) = (pos = (head)->n.next, pos->next);	\
	     pos != &(head)->n;						\
	     pos = LOCAL(n), LOCAL(n) = pos->next)

#define list_for_each_entry(pos, head, member)				\
	for (typeof(pos) LOCAL(n) = (pos = list_entry((head)->n.next,	\
						      typeof(*pos),	\
						      member),		\
				     list_entry(pos->member.next,	\
						typeof(*pos),		\
						member));		\
	     &pos->member != &(head)->n;				\
	     pos = LOCAL(n), LOCAL(n) = list_entry(LOCAL(n)->member.next, \
						   typeof(*LOCAL(n)),	\
						   member))

#define list_for_each_entry_reverse(pos, head, member)			\
    for (typeof(pos) LOCAL(n) = (pos = list_entry((head)->n.prev,	\
                              typeof(*pos),	\
                              member),		\
                     list_entry(pos->member.prev,	\
                        typeof(*pos),		\
                        member));		\
         &pos->member != &(head)->n;				\
         pos = LOCAL(n), LOCAL(n) = list_entry(LOCAL(n)->member.prev, \
                           typeof(*LOCAL(n)),	\
                           member))

static inline void __list_add(struct list_node *one,
                              struct list_node *prev,
                              struct list_node *next)
{
	next->prev = one;
	one->next = next;
	one->prev = prev;
	prev->next = one;
}

static inline void list_add(struct list_node *one, struct list_head *head)
{
	__list_add(one, &head->n, head->n.next);
}

static inline void list_add_tail(struct list_node *one, struct list_head *head)
{
	__list_add(one, head->n.prev, &head->n);
}

static inline void __list_del(struct list_node *prev, struct list_node *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void __list_del_entry(struct list_node *entry)
{
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_node *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = entry->prev = NULL;
}

static inline void list_move(struct list_node *list, struct list_head *head)
{
	__list_del_entry(list);
	list_add(list, head);
}

static inline void list_move_tail(struct list_node *list,
                                  struct list_head *head)
{
	__list_del_entry(list);
	list_add_tail(list, head);
}

static inline void __list_splice(const struct list_head *list,
                                 struct list_node *prev,
                                 struct list_node *next)
{
	struct list_node *first = list->n.next;
	struct list_node *last = list->n.prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

static inline void list_splice_init(struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, &head->n, head->n.next);
		INIT_LIST_HEAD(list);
	}
}

static inline void list_splice_tail_init(struct list_head *list,
					 struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head->n.prev, &head->n);
		INIT_LIST_HEAD(list);
	}
}

// Get the size of a list
#define list_size(head) ({ \
    size_t count = 0; \
    struct list_node *pos; \
    list_for_each(pos, head) { \
        count++; \
    } \
    count; \
})

// Get the first entry of list and remove it from the list
#define list_entry_grab(head, type, member) ({ \
    type *first_entry = list_entry((head)->n.next, type, member); \
    list_del(&(first_entry->member)); \
    first_entry; \
})

// Get the next entry of list
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

/* hlist, mostly useful for hash tables */

// This is djb2 hash function from http://www.cse.yorku.ca/~oz/hash.html
static inline unsigned long djb2_hash(char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

// FNV-1 hash function from http://www.isthe.com/chongo/tech/comp/fnv/
static inline unsigned long fnv1_hash(char *str)
{
    unsigned long hash = 2166136261;
    for (int i = 0; str[i] != '\0'; i++) {
        hash = hash ^ str[i];
        hash = hash * 16777619;
    }
    return hash;
}

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define LIST_POISON1 ((struct hlist_node *) 0x00100100)
#define LIST_POISON2 ((struct hlist_node **) 0x00200200)

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

static inline int hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* next must be != NULL */
static inline void hlist_add_before(struct hlist_node *n,
		struct hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static inline void hlist_add_after(struct hlist_node *n,
		struct hlist_node *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if (next->next)
		next->next->pprev  = &next->next;
}

#define hlist_entry(ptr, type, member) container_of(ptr, type, member)

#define hlist_for_each(pos, head)					\
	for (typeof(pos) LOCAL(n) = (pos = (head)->first, NULL);	\
	     pos && (LOCAL(n) = pos->next, 1);				\
	     pos = LOCAL(n))						\

/*
 * hlist_for_each_entry - iterate over list of given type
 * @tpos:       the type * to use as a loop cursor.
 * @pos:        the &struct hlist_node to use as a loop cursor.
 * @head:       the head for your list.
 * @member:     the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(tpos, pos, head, member)			\
	for (typeof(pos) LOCAL(n) = (pos = (head)->first, NULL);	\
	     pos && (LOCAL(n) = pos->next, 1) &&			\
		     (tpos = hlist_entry(pos, typeof(*tpos), member), 1); \
	     pos = LOCAL(n))

#endif /* LIST_H */
