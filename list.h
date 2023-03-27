#ifndef LIST_H
#define LIST_H

#include <stddef.h>

struct list_node {
    struct list_node *prev, *next;
};

struct list_head {
    struct list_node node;
};

// Initialize a list head
#define LIST_HEAD_INIT(name) { { &(name.node), &(name.node) } }

// Declare and initialize a list head
#define LIST_HEAD(name) \\
    struct list_head name = LIST_HEAD_INIT(name)

// Initialize a list node
static inline void INIT_LIST_NODE(struct list_node *node)
{
    node->next = NULL;
    node->prev = NULL;
}

// Initialize a list head
static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->node.next = &(head->node);
    head->node.prev = &(head->node);
}

// Add a new node to the list
static inline void list_add(struct list_node *new,
                            struct list_node *prev,
                            struct list_node *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

// Add a new node to the head of the list
static inline void list_add_head(struct list_node *new, struct list_head *head)
{
    list_add(new, &(head->node), head->node.next);
}

// Add a new node to the tail of the list
static inline void list_add_tail(struct list_node *new, struct list_head *head)
{
    list_add(new, head->node.prev, &(head->node));
}

// Remove a node from the list
static inline void __list_del(struct list_node *prev, struct list_node *next)
{
    next->prev = prev;
    prev->next = next;
}

// Remove a node from the list
static inline void list_del(struct list_node *entry)
{
    __list_del(entry->prev, entry->next);
}

// Remove a node from the list and initialize it
static inline void list_del_init(struct list_node *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_NODE(entry);
}

// Move a node to the head of another list
static inline void list_move(struct list_node *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_head(list, head);
}

// Move a node to the tail of another list
static inline void list_move_tail(struct list_node *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

// Check if the list is empty
static inline int list_empty(const struct list_head *head)
{
    return head->node.next == &(head->node);
}

// Get the struct for this entry
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

// Get the struct for this entry
#define container_of(ptr, type, member) ({ \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member)); })

// Iterate over a list
#define list_for_each(pos, head) \
    for (pos = (head)->node.next; pos != &(head->node); pos = pos->next)

// Iterate over a list safe against removal of list entry
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->node.next, n = pos->next; pos != &(head->node); \
            pos = n, n = pos->next)

// Iterate over list of given type
#define list_for_each_entry(pos, head, member) \
    for (pos = list_entry((head)->node.next, typeof(*pos), member); \
         &pos->member != &(head)->node; \
         pos = list_entry(pos->member.next, typeof(*pos), member))

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
    type *first_entry = list_entry((head)->node.next, type, member); \
    list_del(&(first_entry->member)); \
    first_entry; \
})

// Iterate over list of given type safe against removal of list entry
#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_entry((head)->node.next, typeof(*pos), member), \
	 n = list_entry(pos->member.next, typeof(*pos), member); \
	 &pos->member != &(head)->node; \
	 pos = n, n = list_entry(n->member.next, typeof(*n), member))

#endif /* LIST_H */
