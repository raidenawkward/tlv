#include "tlv.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define TAG_LEN 64

int print_tlv_node(tlv* tlv, tlvnode* node)
{
    int level = 0;
    char* tag = NULL;
    char* value = NULL;
    tlvnode* nptr;
    size_t i;

    if (!tlv || !node)
    {
        return 0;
    }

    nptr = node;
    while (nptr)
    {
        ++level;
        nptr = nptr->parent;
    }

    if (tlv->tlength)
    {
        tag = (char*) malloc(sizeof(char) * tlv->tlength);
        memset(tag, 0x00, tlv->tlength);
        tlv_node_read_t(tlv, node, (tlvbyte*)tag, tlv->tlength);
    }

    if (!node->childCount)
    {
        if (node->v && node->length)
        {
            value = (char*) malloc(sizeof(char) * node->length);
            memcpy(value, node->v, node->length);
        }
    }

    for (i = 0; i < level; ++i)
    {
        printf("\t");
    }

    printf("t(%ld): %s, l: %ld, v: %s\n",
            node->childCount,
            tag == NULL ? "" : tag,
            node->length,value == NULL ? " " : value);

    free(tag);
    free(value);

    return 0;
}

void print_tlvnode_tree(tlv* tlv, tlvnode* node)
{
    size_t i;
    char* tag = NULL;
    char* value = NULL;
    int level = 0;
    tlvnode* nptr;

    if (!tlv || !node)
    {
        return;
    }

    nptr = node;
    while (nptr)
    {
        ++level;
        nptr = nptr->parent;
    }

    if (tlv->tlength)
    {
        tag = (char*) malloc(sizeof(char) * tlv->tlength);
        memset(tag, 0x00, tlv->tlength);
        tlv_node_read_t(tlv, node, (tlvbyte*)tag, tlv->tlength);
    }

    if (!node->childCount)
    {
        if (node->v && node->length)
        {
            value = (char*) malloc(sizeof(char) * node->length);
            memcpy(value, node->v, node->length);
        }
    }

    for (i = 0; i < level; ++i)
    {
        printf("\t");
    }

    printf("t(%ld): %s, l: %ld, v: %s\n", node->childCount, tag == NULL ? "" : tag, node->length, value == NULL ? " " : value);

    free(tag);
    free(value);

    if (node->firstChild != NULL)
    {
        print_tlvnode_tree(tlv, node->firstChild);
    }

    if (node->nextSubling != NULL)
    {
        print_tlvnode_tree(tlv, node->nextSubling);
    }
}

tlv* make_test_tlv()
{
    tlv* t;
    tlvnode* root;
    tlvnode* lv1_1;
    tlvnode* lv1_1_1;
    tlvnode* lv1_2;
    tlvnode* lv1_3;
    tlvnode* lv1_4;

    t = tlv_obtain();
    t->tlength = TAG_LEN;

    root = tlv_node_obtain(t);
    tlv_node_write_t(t, root, (tlvbyte*)"root node\0", 10);
    tlv_set_root(t, root);

    lv1_1 = tlv_node_obtain(t);
    tlv_node_write_t(t, lv1_1, (tlvbyte*)"node lv1_1\0", 11);
    tlv_node_add_child(t, root, lv1_1);

    lv1_1_1 = tlv_node_obtain(t);
    tlv_node_write_t(t, lv1_1_1, (tlvbyte*)"node lv1_1_1\0", 13);
    tlv_node_write_v(t, lv1_1_1, (tlvbyte*)"value of lv1_1_1\0", 17);
    tlv_node_add_child(t, lv1_1, lv1_1_1);

    lv1_2 = tlv_node_obtain(t);
    tlv_node_write_t(t, lv1_2, (tlvbyte*)"node lv1_2\0", 11);
    tlv_node_add_child(t, root, lv1_2);

    lv1_3 = tlv_node_obtain(t);
    tlv_node_write_t(t, lv1_3, (tlvbyte*)"node lv1_3\0", 11);
    tlv_node_add_child(t, root, lv1_3);

    lv1_4 = tlv_node_obtain(t);
    tlv_node_write_t(t, lv1_4, (tlvbyte*)"node lv1_4\0", 11);
    tlv_node_write_v(t, lv1_4, (tlvbyte*)"value of lv1_4\0", 17);
    tlv_node_add_child(t, root, lv1_4);

    tlv_layout(t);

    return t;
}

void write_tlv_to_file(tlv* tlv, const char* path)
{
    FILE* fp = fopen(path, "w");
    tlvbyte* buf = (tlvbyte*) malloc(sizeof(tlvbyte) * tlv->dumplength);

    memset(buf, 0x00, tlv->dumplength);
    tlv_dumps(tlv, buf, tlv->dumplength);
    fwrite(buf, tlv->dumplength, 1, fp);

    free(buf);
    fclose(fp); 
}


tlv* read_tlv_from_file(const char* path)
{
    tlv* tlv = tlv_obtain();
    tlv->tlength = TAG_LEN;

    FILE* fp = fopen(path, "rb");
    tlvbyte buf[4096];
    memset(buf, 0x00, sizeof(buf));

    fread(buf, sizeof(buf), 1, fp);
    tlv_loads(tlv, buf, sizeof(buf));
    fclose(fp); 

    return tlv;
}


int main()
{
    tlv* t = make_test_tlv();
    tlv* t1;
    tlv* t2;
    tlvbyte dumpbuf[4096];

    print_tlvnode_tree(t, t->root);
    tlv_node_traverse(t, print_tlv_node);

    memset(dumpbuf, 0x00, sizeof(dumpbuf));
    tlv_dumps(t, dumpbuf, t->dumplength);

    t1 = tlv_obtain();
    t1->tlength = t->tlength;
    tlv_loads(t1, (tlvbyte*)dumpbuf, t->dumplength);

    tlvnode* rmchild = t1->root->firstChild->nextSubling;
    tlv_node_remove_child(t1, rmchild);
    tlv_node_add_child(t1, t1->root->firstChild->nextSubling->nextSubling, rmchild);

    tlv_layout(t1);

    t2 = tlv_obtain();
    t2->tlength = TAG_LEN;
    memset(dumpbuf, 0x00, sizeof(dumpbuf));
    tlv_dumps(t1, dumpbuf, t1->dumplength);
    tlv_loads(t2, dumpbuf, t1->dumplength);

    printf("\n\n=========================\n\n");

    print_tlvnode_tree(t2, t2->root);

    tlv_node_traverse(t2, print_tlv_node);

    //write_tlv_to_file(t1, "tlv.out");

    tlv_destroy(t);
    tlv_destroy(t1);

    return 0;
}

