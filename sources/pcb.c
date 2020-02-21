#include <primitive_types.h>
#include <assertions.h>
#include <const.h>
#include <listx.h>
#include <pcb.h>

static struct pcb_t pcb_table[MAXPROC];
static struct list_head pcb_free;

/**
 * Clears the memory starting from ptr for the following s bytes,
 * setting each of them to zero.
 */
static inline void memclr(void *const ptr, usize s) {
    debug_assert(NULL != ptr);
    for (u8 *p = ptr; s--; *(p++) = 0);
}

void initPcbs(void) {
    INIT_LIST_HEAD(&pcb_free);

    const struct pcb_t *const end = &pcb_table[MAXPROC];
    for (struct pcb_t *cur = &pcb_table[0]; end > cur; ++cur) {
        list_add(&cur->p_next, &pcb_free);
    }
}

void freePcb(struct pcb_t *const p) {
    debug_assert(NULL != p);
    list_add_tail(&p->p_next, &pcb_free);
}

struct pcb_t *allocPcb(void) {
    struct list_head *node = list_next(&pcb_free);

    if (NULL != node) {
        struct pcb_t *p = container_of(node, struct pcb_t, p_next);
        list_del(node);
        memclr(p, sizeof(p));
        INIT_LIST_HEAD(&p->p_next);
        INIT_LIST_HEAD(&p->p_child);
        INIT_LIST_HEAD(&p->p_sib);
        return p;
    }

    return NULL;
}

void mkEmptyProcQ(struct list_head *const head) {
    debug_assert(NULL != head);
    INIT_LIST_HEAD(head);
}

int emptyProcQ(struct list_head *const head) {
    debug_assert(NULL != head);
    return list_empty(head);
}

void insertProcQ(struct list_head *const head, struct pcb_t *const p) {
    debug_assert(NULL != head);
    debug_assert(NULL != p);
    bool not_yet_pushed = true;
    struct pcb_t *iter = NULL;

    list_for_each_entry(iter, head, p_next) {
        if (p->priority > iter->priority) {
            list_add(&p->p_next, list_prev(&iter->p_next));
            not_yet_pushed = false;
            break;
        }
    }

    if (not_yet_pushed) {
        list_add_tail(&p->p_next, head);
    }
}

struct pcb_t *headProcQ(struct list_head *const head) {
    debug_assert(NULL != head);
    return emptyProcQ(head) ? NULL
                            : container_of(list_next(head), struct pcb_t, p_next);
}

struct pcb_t *removeProcQ(struct list_head *const head) {
    debug_assert(NULL != head);
    struct pcb_t *tmp = headProcQ(head);

    if (NULL != tmp) {
        list_del(list_next(head));
    }

    return tmp;
}

struct pcb_t *outProcQ(struct list_head *const head, struct pcb_t *const p) {
    debug_assert(NULL != head);
    debug_assert(NULL != p);
    struct pcb_t *iter = NULL;

    list_for_each_entry(iter, head, p_next) {
        if (p == iter) {
            return removeProcQ(list_prev(&iter->p_next));
        }
    }

    return NULL;
}

int emptyChild(struct pcb_t *const this) {
    debug_assert(NULL != this);
    return list_empty(&this->p_child);
}

void insertChild(struct pcb_t *const prnt, struct pcb_t *const p) {
    debug_assert(NULL != prnt);
    debug_assert(NULL != p);
    debug_assert(NULL == p->p_parent);

    p->p_parent = prnt;
    if (emptyChild(prnt)) {
        list_add(&p->p_child, &prnt->p_child);
    } else {
        struct pcb_t *child = container_of(list_next(&prnt->p_child), struct pcb_t, p_child);
        list_add_tail(&p->p_sib, &child->p_sib);
    }
}

struct pcb_t *removeChild(struct pcb_t *const p) {
    debug_assert(NULL != p);

    /**
     * LEGEND:
     *  Progeny: a circular list of p_child nodes.
     *  Founder: the root node of a progeny.
     */

    struct list_head *orphan_node = list_next(&p->p_child);
    if (NULL != orphan_node) {
        // p has at least a child & orphan_node is the first child

        struct pcb_t *orphan = container_of(orphan_node, struct pcb_t, p_child);
        struct list_head *sib_node = list_next(&orphan->p_sib);
        struct pcb_t *founder = NULL;    // Founder of p's progeny

        /**
         * Looking for the founder (founder).
         *
         * The founder must exist: it is either someone above
         * the parent of the orphan or the parent of the orphan itself.
         *
         * The founder is the only node of its progeny that has the parent outside the progeny itself.
         */
        list_for_each_entry(founder, &orphan->p_child, p_child) {
            if (container_of(list_prev(&founder->p_child), struct pcb_t, p_child) != founder->p_parent) {
                break;
            }
        }

        // hot potato: removing orphan's progeny sub-tree and update links accordingly.
        assert(NULL != founder);
        struct list_head *cur_prev = list_prev(&founder->p_child);
        assert(NULL != cur_prev);
        __list_del(&p->p_child, &founder->p_child);
        cur_prev->next = orphan_node;
        orphan_node->prev = cur_prev;
        // hot potato

        orphan->p_parent = NULL;
        if (NULL != sib_node) {
            list_del(list_prev(sib_node));
            list_add(&container_of(sib_node, struct pcb_t, p_sib)->p_child, &p->p_child);
        }

        return orphan;
    }

    return NULL;
}

struct pcb_t *outChild(struct pcb_t *const p) {
    debug_assert(NULL != p);

    if (NULL == p->p_parent) {
        return NULL;
    }

    struct pcb_t *parent = p->p_parent;
    struct list_head *prev = list_prev(&p->p_child);

    if (NULL != prev && container_of(prev, struct pcb_t, p_child) == parent) {
        // p is the first child of the parent
        return removeChild(parent);
    } else {
        // p is not the first child of the parent => p is a sib of the first child of parent (sib list is not empty)
        list_del(&p->p_sib);
        p->p_parent = NULL;
        return p;
    }
}
