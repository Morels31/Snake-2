#include <stdlib.h>

struct nodeS{
    struct nodeS *prev;
    struct nodeS *next;
    int v;
};

typedef struct nodeS nod;

nod *initList(void){
    nod *node = (nod *) calloc(1, sizeof(nod));
    if(!node){
        printf("Allocation Error\n");
        exit(1);
    }
    node->prev=node;
    node->next=node;
    node->v=-1;
    return node;
}

nod *initNode(int val){
    nod *node = (nod *) calloc(1, sizeof(nod));
    if(!node){
        printf("Allocation Error\n");
        exit(1);
    }
    node->v=val;
    return node;
}

void linkk(nod *prevv, nod *node, nod *nextt){
    node->prev=prevv;
    node->next=nextt;
    prevv->next=node;
    nextt->prev=node;
}

void appendRight(nod *list, int val){
    nod *node=initNode(val);
    linkk(list->prev, node, list);
}

void appendLeft(nod *list, int val){
    nod *node=initNode(val);
    linkk(list, node, list->next);
}

int popRight(nod *list){
    if(list->next==list) return -1;
    nod *node=list->prev;
    int val = node->v;
    node->prev->next=list;
    list->prev=node->prev;
    free(node);
    return val;
}

int popLeft(nod *list){
    if(list->next==list) return -1;
    nod *node=list->next;
    int val = node->v;
    node->next->prev=list;
    list->next=node->next;
    free(node);
    return val;
}