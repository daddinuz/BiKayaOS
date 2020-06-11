#include <primitive_types.h>
#include <assertions.h>
#include <const_bikaya.h>
#include <listx.h>
#include <pcb.h>
#include <asl.h>

static struct semd_t semd_table[MAX_SEM_NO];
static struct list_head semd_free;
static struct list_head semd_busy;

void initASL(void) {
    INIT_LIST_HEAD(&semd_free);
    INIT_LIST_HEAD(&semd_busy);

    const struct semd_t *const end = &semd_table[MAX_SEM_NO];
    for (struct semd_t *cur = &semd_table[0]; end > cur; ++cur) {
        mkEmptyProcQ(&cur->s_procQ);
        list_add(&cur->s_next, &semd_free);
    }
}

struct semd_t *getSemd(int *const key) {
    debug_assert(NULL != key);
    struct semd_t *iter = NULL;

    list_for_each_entry(iter, &semd_busy, s_next) {
        if (key == iter->s_key) {
            return iter;
        }
    }

    return NULL;
}

int insertBlocked(int *const key, struct pcb_t *const p) {
    debug_assert(NULL != key);
    debug_assert(NULL != p);
    struct semd_t *s = getSemd(key);

    if (NULL == s) {
        // no sem with the given key -> add a new sem removing one from semd_free.

        if (list_empty(&semd_free)) {
            // there are no sems left -> nothing to do.
            return 1;
        } else {
            // we have at least one available sem.

            struct list_head *const s_node = list_next(&semd_free);
            assert(NULL != s_node);

            // removing the sem from semd_free.
            list_del(s_node);
            INIT_LIST_HEAD(s_node);

            // adding the sem from semd_busy.
            list_add_tail(s_node, &semd_busy);

            s = container_of(s_node, struct semd_t, s_next);
            s->s_key = key;
        }
    }

    // ensure that the proc was not already associated to a sem.
    debug_assert(NULL == p->p_semkey);
    p->p_semkey = key;
    list_add_tail(&p->p_next, &s->s_procQ);
    return 0;
}

struct pcb_t *headBlocked(int *const key) {
    debug_assert(NULL != key);
    struct semd_t *s = getSemd(key);
    return (NULL == s) ? NULL : headProcQ(&s->s_procQ);
}

/**
 * This function frees (putting it back into the free list) the sem if there
 * are no procs left in its queue returning true, otherwise does nothing
 * and returns false.
 */
static bool tryFreeSemd(struct semd_t *const s) {
    debug_assert(NULL != s);

    if (emptyProcQ(&s->s_procQ)) {
        // empty proc queue -> move sem back from semd_busy to semd_free.
        list_del(&s->s_next);
        INIT_LIST_HEAD(&s->s_next);
        list_add_tail(&s->s_next, &semd_free);
        return true;
    }

    return false;
}

struct pcb_t *removeBlocked(int *const key) {
    debug_assert(NULL != key);
    struct semd_t *s = getSemd(key);

    if (NULL == s) {
        // no such process with the given key.
        return NULL;
    }

    // A semd_t can't exist without at least an associated pcb_t,
    // thus we know removing a pcb_t from the queue must not return NULL.
    struct pcb_t *const out = removeProcQ(&s->s_procQ);
    assert(NULL != out);
    out->p_semkey = NULL;

    tryFreeSemd(s);
    return out;
}

struct pcb_t *outBlocked(struct pcb_t *const p) {
    debug_assert(NULL != p);

    if (NULL != p->p_semkey) {
        // A proc with a valid semkey (!= NULL) must have the relative sem.
        struct semd_t *s = getSemd(p->p_semkey);
        assert(NULL != s);

        // remove the proc from the queue and clean its semkey before return.
        p->p_semkey = NULL;
        list_del(&p->p_next);
        INIT_LIST_HEAD(&p->p_next);

        tryFreeSemd(s);
        return p;
    }

    return NULL;
}

void outChildBlocked(struct pcb_t *const p) {
    debug_assert(NULL != p);
    struct pcb_t *child = NULL, *sib = NULL;

    // Removes procs from their relative queues in postorder traversal.
    list_for_each_entry_reverse(child, &p->p_child, p_child) {
        // Skipping the founder which is the only node of its progeny that
        // has the parent outside the progeny itself. (see removeChild in pcb.c)
        if (container_of(list_prev(&child->p_child), struct pcb_t, p_child) != child->p_parent) {
            continue;
        }

        list_for_each_entry_reverse(sib, &child->p_sib, p_sib) {
            outChildBlocked(sib);
        }

        outBlocked(child);
    }

    outBlocked(p);
}
