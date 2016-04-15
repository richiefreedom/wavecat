#ifndef _LIST_H_
#define _LIST_H_

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

typedef struct list_head list_head_t;

static inline void INIT_LIST_HEAD(list_head_t *list) {
	list->next = list;
	list->prev = list;
}

static inline void __list_add(list_head_t *new, list_head_t *prev,
		list_head_t *next) {
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(list_head_t *new, list_head_t *head) {
	__list_add(new, head, head->next);
}

static inline void list_add_tail(list_head_t *new, list_head_t *head) {
	__list_add(new, head->prev, head);
}

static inline void __list_del(list_head_t * prev, list_head_t * next) {
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(list_head_t *entry) {
	__list_del(entry->prev, entry->next);
}

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_entry(ptr, type, member) \
	(type *) ((unsigned char *) ptr - \
			(unsigned char *) (&((type *)0)->member))

#define list_is_empty(head) \
	((head)->next == (head))

#define DECLARE_LIST_HEAD(head) \
	list_head_t head = { .next = &head, .prev = &head }

#endif
