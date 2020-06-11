#include <pcb.h>
#include <core.h>
#include <scheduler.h>
#include <assertions.h>

void f(void) {
    struct pcb_t *const root = allocPcb();
    struct pcb_t *const child1 = allocPcb();
    struct pcb_t *const child2 = allocPcb();

    insertChild(root, child1);
    insertChild(root, child2);

    assert(list_next(&root->p_child) == &child1->p_child);
    assert(list_next(&child1->p_child) == &root->p_child);
    assert(child1->p_parent == root);

    assert(list_next(&child1->p_sib) == &child2->p_sib);
    assert(list_next(&child2->p_sib) == &child1->p_sib);
    assert(list_next(&child2->p_child) == NULL);
    assert(child2->p_parent == root);

    assert(outChild(child2) == child2);
    assert(NULL == child2->p_parent);
    assert(list_next(&child2->p_sib) == NULL);
    assert(list_next(&child2->p_child) == NULL);

    assert(list_next(&root->p_child) == &child1->p_child);
    assert(list_next(&child1->p_child) == &root->p_child);
    assert(list_next(&child1->p_sib) == NULL);
    assert(child1->p_parent == root);

    assert(outChild(child2) == NULL);
    assert(NULL == child2->p_parent);
    assert(list_next(&child2->p_sib) == NULL);
    assert(list_next(&child2->p_child) == NULL);

    assert(list_next(&root->p_child) == &child1->p_child);
    assert(list_next(&child1->p_child) == &root->p_child);
    assert(list_next(&child1->p_sib) == NULL);
    assert(child1->p_parent == root);

    assert(outChild(child1) == child1);
    assert(NULL == child1->p_parent);
    assert(list_next(&child1->p_sib) == NULL);
    assert(list_next(&child1->p_child) == NULL);
    assert(list_next(&root->p_child) == NULL);

    SYSCALL(TERMINATEPROCESS, 0, 0, 0);
}

void g(void) {
    struct pcb_t *const root = allocPcb();
    struct pcb_t *const node = allocPcb();
    struct pcb_t *const leaf = allocPcb();

    insertChild(root, node);
    insertChild(node, leaf);

    assert(list_next(&root->p_child) == &node->p_child);
    assert(list_next(&node->p_child) == &leaf->p_child);
    assert(root == node->p_parent);
    assert(node == leaf->p_parent);

    assert(removeChild(leaf) == NULL);

    assert(removeChild(node) == leaf);
    assert(NULL == leaf->p_parent);
    assert(list_next(&leaf->p_child) == NULL);
    assert(list_next(&root->p_child) == &node->p_child);
    assert(list_next(&node->p_child) == &root->p_child);

    assert(removeChild(root) == node);
    assert(NULL == node->p_parent);
    assert(list_next(&node->p_child) == NULL);
    assert(list_next(&root->p_child) == NULL);

    SYSCALL(TERMINATEPROCESS, 0, 0, 0);
}

int main(void) {
   core_boot();

   scheduler_scheduleWith(f, 1, true);
   scheduler_scheduleWith(g, 10, true);

   scheduler_dispatch();
   unreachable();
}
