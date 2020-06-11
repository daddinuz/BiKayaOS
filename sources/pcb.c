#include <primitive_types.h>
#include <assertions.h>
#include <memory.h>
#include <const_bikaya.h>
#include <listx.h>
#include <pcb.h>

static struct pcb_t pcb_table[MAX_PROC_NO];
static struct list_head pcb_free;

void initPcbs(void) {
    INIT_LIST_HEAD(&pcb_free);

    const struct pcb_t *const end = &pcb_table[MAX_PROC_NO];
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
        memclr(p, sizeof(*p));
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

    list_for_each_entry_reverse(iter, head, p_next) {
        if (p->priority <= iter->priority) {
            list_add(&p->p_next, &iter->p_next);
            not_yet_pushed = false;
            break;
        }
    }

    if (not_yet_pushed) {
        list_add(&p->p_next, head);
    }
}

struct pcb_t *headProcQ(struct list_head *const head) {
    debug_assert(NULL != head);
    return emptyProcQ(head) ? NULL
                            : container_of(list_next(head), struct pcb_t, p_next);
}

struct pcb_t *removeProcQ(struct list_head *const head) {
    debug_assert(NULL != head);
    struct pcb_t *const proc = headProcQ(head);

    if (NULL != proc) {
        list_del(&proc->p_next);
        INIT_LIST_HEAD(&proc->p_next);
    }

    return proc;
}

struct pcb_t *outProcQ(struct list_head *const head, struct pcb_t *const p) {
    debug_assert(NULL != head);
    debug_assert(NULL != p);
    struct pcb_t *iter = NULL;

    list_for_each_entry(iter, head, p_next) {
        if (p == iter) {
            list_del(&p->p_next);
            INIT_LIST_HEAD(&p->p_next);
            return p;
        }
    }

    return NULL;
}

int emptyChild(struct pcb_t *const root) {
    debug_assert(NULL != root);
    struct list_head *const next = list_next(&root->p_child);
    return NULL == next || root != container_of(next, struct pcb_t, p_child)->p_parent;
}

void insertChild(struct pcb_t *const root, struct pcb_t *const p) {
    debug_assert(NULL != root);
    debug_assert(NULL != p);
    debug_assert(NULL == p->p_parent);

    p->p_parent = root;
    if (emptyChild(root)) {
        list_add(&p->p_child, &root->p_child);
    } else {
        struct pcb_t *const child = container_of(list_next(&root->p_child), struct pcb_t, p_child);
        assert(root == child->p_parent);
        list_add_tail(&p->p_sib, &child->p_sib);
    }
}

struct pcb_t *removeChild(struct pcb_t *const root) {
    debug_assert(NULL != root);

    /**
     * LEGEND:
     *  Progeny: a circular list of p_child nodes.
     *  Founder: the root node of a progeny.
     */

    if (!emptyChild(root)) {
        struct pcb_t *const child = container_of(list_next(&root->p_child), struct pcb_t, p_child);
        struct pcb_t *founder = root;

        /**
         * Looking for the founder.
         *
         * The founder must exist: it is either someone above
         * the parent of the child or the parent of the child itself.
         *
         * The founder is the only node of its progeny that has the parent outside the progeny itself.
         */
        if (NULL != founder->p_parent) {
            list_for_each_entry_reverse(founder, &root->p_child, p_child) {
                if (NULL == founder->p_parent) {
                    break;
                }
            }
        }

        // hot potato: removing child's progeny sub-tree and update links accordingly.
        assert(NULL != founder);
        struct list_head *const last_child = list_prev(&founder->p_child);
        assert(NULL != last_child);
        struct list_head *const child_node = &child->p_child;
        assert(NULL != child_node);
        __list_del(&root->p_child, &founder->p_child);
        last_child->next = child_node;
        child_node->prev = last_child;
        // hot potato

        if (!list_empty(&child->p_sib)) {
            struct pcb_t *const sib = container_of(list_next(&child->p_sib), struct pcb_t, p_sib);
            list_del(&child->p_sib);
            INIT_LIST_HEAD(&child->p_sib);
            list_add(&sib->p_child, &root->p_child);
        }

        child->p_parent = NULL;
        return child;
    }

    return NULL;
}

struct pcb_t *outChild(struct pcb_t *const child) {
    debug_assert(NULL != child);

    if (NULL == child->p_parent) {
        return NULL;
    }

    struct pcb_t *const parent = child->p_parent;
    struct list_head *const prev = list_prev(&child->p_child);

    if (prev == &parent->p_child) {
        // child is the first child of the parent
        return removeChild(parent);
    } else {
        /*
           child is not the first child of the parent => child is a sib of the
           first child of parent (sib list is not empty)
         */
        list_del(&child->p_sib);
        INIT_LIST_HEAD(&child->p_sib);
        child->p_parent = NULL;
        return child;
    }
}

usize getPid(const struct pcb_t *const p) {
    debug_assert(NULL != p);
    debug_assert(pcb_table <= p);
    debug_assert(p < &pcb_table[MAX_PROC_NO]);
    return (p - pcb_table) + 1;
}
