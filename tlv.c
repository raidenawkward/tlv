/*
 * tlv.c
 * This module provides TLV (TagLengthValue) constructing
 * and parsing functions.
 *
 * Copyright (C) 1991, 1992, 1993, 1994, 1996, 1998,
 *  2000, 2001, 2002, 2003 Free Software Foundation, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this file; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA. 
 *
 * Presented by yingjun.ma and tome.huang
 * on 20180608.
 *
 * All rights reserved.
 */

#include<stdio.h>
#include<string.h>
#include<math.h>
#include<malloc.h>
#include<stddef.h>
#include <stdlib.h>

#include"tlv.h"


#define STACK_INCREASE (10)
#define STACK_INIT_SIZE (10)


typedef struct
{
    int size;
    int index;
    void** data;
} _stack;

static _stack* stack_obtain(int init_size)
{
    if (init_size < 0)
    {
        return NULL;
    }

    _stack* newstack = (_stack*)malloc(sizeof(_stack));
    newstack->index = 0;
    newstack->data = (void**)malloc(sizeof(void*)*init_size);
    newstack->size = init_size;

    return newstack;
}

static int realloc_stack(_stack* s)
{
    if (!s)
    {
        return -1;
    }

    int newsize = sizeof(void**) * (s->size + STACK_INCREASE);
    s->data = (void**) realloc(s->data, newsize);
    s->size = newsize;

    if (!s->data)
    {
        return -1;
    }

    return newsize;
}

static int stack_push(_stack* s, void* item)
{
    if (!s)
    {
        return -1;
    }

    if (s->index == s->size)
    {
        if (realloc_stack(s) < 0)
        {
            return -1;
        }
    }

    s->data[s->index++] = item;
    return s->index;
}

static void* stack_pop(_stack* s)
{
    if (!s)
    {
        return NULL;
    }

    return s->data[--s->index];
}

static size_t stack_length(const _stack* s)
{
    return s ? s->index : 0;
}

static void stack_destroy(_stack* s)
{
    if (!s)
    {
        return;
    }

    if (s->data)
    {
        free(s->data);
        s->data = NULL;
    }

    free(s);
}



tlv* tlv_obtain()
{
    tlv* newtlv = (tlv*)malloc(sizeof(tlv));
    newtlv->alength = TLV_DEF_A_LENGTH;
    newtlv->tlength = TLV_DEF_T_LENGTH;
    newtlv->llength = TLV_DEF_L_LENGTH;
    newtlv->root = NULL;
    newtlv->byteprio = TLV_BYTE_MSB;
    newtlv->dumplength = 0;

    return newtlv;
}

void tlv_set_root(tlv* t, tlvnode* root)
{
    if (t)
    {
        t->root = root;
    }
}

int tlv_node_count(const tlv* tlv)
{
    int node_count = 0;
    _stack* coStack;
    tlvnode* curnode;

    if (!tlv)
    {
        return 0;
    }

    curnode = tlv->root;
    coStack = stack_obtain(STACK_INIT_SIZE);
    stack_push(coStack, curnode);
    node_count++;
    while (coStack->index > 0)
    {
        tlvnode* child;
        curnode = (tlvnode*) stack_pop(coStack);
        child = curnode->firstChild;
        while (child)
        {
            stack_push(coStack, child);
            node_count++;
            child = child->nextSubling;
        }
    }
    return node_count;
}

tlvnode* tlv_node_obtain(tlv* tlv)
{
    if (!tlv)
    {
        return NULL;
    }

    tlvnode* newnode = (tlvnode*) malloc(sizeof(tlvnode));
    newnode->t = (tlvbyte*) malloc(tlv->tlength);
    memset(newnode->t, 0x00, tlv->tlength);

    newnode->l = (tlvbyte*)malloc(tlv->llength);
    memset(newnode->l, 0x00, tlv->llength);

    newnode->a = (tlvbyte*)malloc(tlv->alength);
    memset(newnode->a, 0x00, tlv->alength);

    newnode->length = 0;
    newnode->tlv = tlv;
    newnode->v = NULL;
    newnode->parent = NULL;
    newnode->childCount = 0;
    newnode->firstChild = newnode->nextSubling = newnode->prevSubling = NULL;

    return newnode;
}

void tlv_destroy(tlv* t)
{
    if (!t)
    {
        return;
    }

    if (t->root)
    {
        tlv_node_destroy(t, t->root);
    }

    free(t);
}

/*
 * get length from *l
 */
static size_t tlv_node_get_length(tlv* t, tlvnode* node)
{
    size_t i;
    size_t length = 0;

    if (!t || !node)
    {
        return -1;
    }


    if (t->byteprio == TLV_BYTE_MSB)
    {
        for (i = 0; i < t->llength; i++)
        {
            length = (length + node->l[i] / 16) * 16;
            length = (length + node->l[i] % 16) * 16;
        }
    }
    else if (t->byteprio == TLV_BYTE_LSB)
    {
        for (i = t->llength - 1; i >= 0; i--)
        {
            length = (length + node->l[i] / 16) * 16;
            length = (length + node->l[i] % 16) * 16;
            if (i == 0)
            {
                break;
            }
        }
    }
    else
    {
        // do nothing
    }

    return length / 16;
}

/*
 * set *l from length
 */
static void tlv_node_set_l(tlv* t, tlvnode* node, size_t length)
{
    size_t i;
    int a;
    int b1;
    int b2;

    if (!t || !node)
    {
        return;
    }

    for (i = 0; i < t->llength; i++)
    {
        node->l[i] = 0;
    }

    a = length;

    if (t->byteprio == TLV_BYTE_MSB)
    {
        for (i = t->llength - 1; i >= 0; i--)
        {
            b1 = a % 16;
            a = a / 16;
            b2 = a % 16;
            a = a / 16;
            node->l[i] = b2 * 16 + b1;
            if (i == 0)
            {
                break;
            }
        }
    }
    else if (t->byteprio == TLV_BYTE_LSB)
    {
        for (i = 0; i < t->llength; i++)
        {
            b1 = a % 16;
            a = a / 16;
            b2 = a % 16;
            a = a / 16;
            node->l[i] = b2 * 16 + b1;
        }
    }
    else
    {
        // do nothing
    }

}

/*
 * load bytes into tlv node tree.
 * defination for tlv should be set into original tlv struct.
 * if succeed, tlv node tree should be set to tlv->root.
 * returns the node count.
 */
size_t tlv_loads(tlv* tlv, tlvbyte* bytes, size_t size)
{
    int byteshandled = -1;
    int nodecount = 0;
    tlvnode* curparent;

    if (!tlv || !bytes || size <= 0)
    {
        return 0;
    }

    _stack* lstack = stack_obtain(STACK_INIT_SIZE);

    size_t index = 0;
    tlv_node_attr_t attr = TLV_NODE_ATTR_IS_STRUCTUAL;

    while (index < size)
    {
        if (byteshandled >= 0)
        {
            if (index >= byteshandled)
            {
                break;
            }
        }

        size_t i;
        int hasChild;

        tlvnode *node = tlv_node_obtain(tlv);
        if (index == 0)
        {
            tlv->root = node;
            curparent = tlv->root;
        }
        else
        {
            tlv_node_add_child(tlv, curparent, node);
        }

        nodecount += 1;

        for (i = 0; i < tlv->alength; i++)
        {
            node->a[i] = bytes[index++];
        }

        for (i = 0; i < tlv->tlength; i++)
        {
            node->t[i] = bytes[index++];
        }

        for (i = 0; i < tlv->llength; i++)
        {
            node->l[i] = bytes[index++];
        }

        node->length = tlv_node_get_length(tlv, node);

        hasChild = tlv_node_get_attributes(tlv, node, attr);
        if (hasChild) //have child
        {
            int* n = (int*)malloc(sizeof(int));

            *n = index + node->length;
            stack_push(lstack, n);
            curparent = node;
        }
        else //have no child
        {
            size_t vi;
            node->v = (tlvbyte*)malloc(node->length);
            for (vi = 0; vi < node->length; vi++)
            {
                node->v[vi] = bytes[index++];
            }

            while (1)
            {
                int n;
                int* pindex = (int*)stack_pop(lstack);
                n = *pindex;

                if (n == index)
                {
                    curparent = curparent->parent;
                    free(pindex);
                    if (curparent == NULL)
                    {
                        break;
                    }
                }
                else
                {
                    stack_push(lstack, pindex);
                    break;
                }
            }
        }

        if (node == tlv->root)
        {
            byteshandled = tlv->alength
                        + tlv->tlength  
                        + tlv->llength
                        + tlv->root->length;
        }

    } // index < size

    //tlv_layout(tlv);

    stack_destroy(lstack);
    return byteshandled;
}


static size_t write_buf_from_index(tlv* tlv, tlvnode* node, tlvbyte* buf, size_t bufsize, size_t index)
{
    size_t i;
    int hasChild;
    tlv_node_attr_t attr = TLV_NODE_ATTR_IS_STRUCTUAL;

    if (!tlv || !node || !buf || bufsize <= 0 || index < 0)
    {
        return 0;
    }

    for (i = 0; i < tlv->alength; i++)
    {
        buf[index++] =  node->a[i];
        if (index >= bufsize)
            return -1;
    }

    for (i = 0; i < tlv->tlength; i++)
    {
        buf[index++] =  node->t[i];
        if (index >= bufsize)
            return -1;
    }

    for (i = 0;i < tlv->llength;i++)
    {
        buf[index++] =  node->l[i];
        if (index >= bufsize)
            return -1;
    }

    hasChild = tlv_node_get_attributes(tlv, node,attr);
    if (!hasChild)
    {
        for (i = 0;i < node->length; i++)
        {

            buf[index++] = node->v[i]; 
            if (index >= bufsize)
            {
                return -1;
            }
        }   
    }

    return index;
}

static tlvnode* tlv_node_last_child(tlvnode* n)
{
    tlvnode* curnode;

    if (!n)
    {
        return NULL;
    }

    curnode = n->firstChild;
    if (curnode)
    {
        while (curnode->nextSubling)
        {
            curnode = curnode->nextSubling;
        }

        return curnode;
    }

    return NULL;
}

/*
 * dump tlv struct into bytes.
 * returns byte length.
 */
size_t tlv_dumps(tlv* tlv, tlvbyte* buf, size_t bufsize)
{
    size_t needsize;
    size_t buf_index = 0;

    if (!tlv || !buf || bufsize < 0)
    {
        return -1;
    }

    _stack* dumpsStack = stack_obtain(STACK_INIT_SIZE);

    tlvnode* curnode = tlv->root;

    needsize = tlv->tlength + tlv->llength + tlv->root->length;

    stack_push(dumpsStack, curnode);

    while (dumpsStack->index > 0)
    {
        tlvnode* child;
        curnode = (tlvnode*) stack_pop(dumpsStack);
        child = tlv_node_last_child(curnode);
        while (child)
        {
            stack_push(dumpsStack, child);
            child = child->prevSubling;
        }
        buf_index = write_buf_from_index(tlv, curnode, buf, bufsize, buf_index);
        if (buf_index < 0)
        {
            return needsize;
        }

    }

    stack_destroy(dumpsStack);
    return buf_index;
}



static void free_tlv_node(tlvnode* node)
{
    if (!node)
    {
        return;
    }

    if (node->a)
    {
        free(node->a);
        node->a = NULL;
    }

    if (node->t)
    {
        free(node->t);
        node->t = NULL;
    }

    if (node->l)
    {
        free(node->l);
        node->l = NULL;
    }

    if (node->v)
    {
        free(node->v);
        node->v = NULL;
    }

    free(node);
}

void tlv_node_destroy(tlv* tlv, tlvnode* node)
{
    if (!tlv || !node)
    {
        return;
    }

    _stack* desStack = stack_obtain(STACK_INIT_SIZE);
    tlvnode* curnode = node;
    stack_push(desStack, curnode);
    while (desStack->index > 0)
    {
        tlvnode* child;
        curnode = (tlvnode*) stack_pop(desStack);
        if (!curnode)
        {
            break;
        }

        child = curnode->firstChild;

        while (child)
        {
            stack_push(desStack, child);
            child = child->nextSubling;
        }
        free_tlv_node(curnode);
    }

    stack_destroy(desStack);
}

void tlv_node_set_attributes(tlv* tlv, tlvnode* node, tlv_node_attr_t attr, int value)
{
    if (!tlv || !node)
    {
        return;
    }

    int index_t = attr / 8;
    int index_byte = attr % 8;
    tlvbyte b = 0x01;
    b = b << (7 - index_byte);

    if (value == 0)
    {
        b = ~b;
        node->a[index_t] = node->a[index_t] & b;
    }
    else if (value == 1)
    {

        node->a[index_t] = node->a[index_t] | b;
    }
    else
    {
        // do nothing
    }
}

int tlv_node_get_attributes(const tlv* tlv, const tlvnode* node, tlv_node_attr_t attr)
{
    if (!tlv || !node)
    {
        return 0;
    }

    int index_t = attr / 8;
    int index_byte = attr % 8;

    tlvbyte b = 0x01;
    b = b << (7 - index_byte);

    return (node->a[index_t]) & b;
}

int tlv_node_add_child(tlv* tlv, tlvnode* parent, tlvnode* child)
{
    if (!tlv || !parent || !child)
    {
        return -1;
    }

    tlv_node_set_attributes(tlv, parent, TLV_NODE_ATTR_IS_STRUCTUAL, 1);

    if (parent->v)
    {
        free(parent->v);
        parent->v = NULL;
        parent->length = 0;
        tlv_node_set_l(tlv, parent, 0);
    }

    tlvnode* lastChild = tlv_node_last_child(parent);
    if (lastChild)
    {
        lastChild->nextSubling = child;
        child->prevSubling = lastChild;
        child->nextSubling = NULL;
    }
    else // parent without child
    {
        parent->firstChild = child;
    }

    child->parent = parent;
    ++parent->childCount;

    return 0;
}

int tlv_node_remove_child(tlv* tlv, tlvnode* child)
{
    if (!tlv || !child)
    {
        return -1;
    }

    tlvnode* parent = child->parent;
    if (!parent)
    {
        return -1;
    }

    tlvnode* curChild = parent->firstChild;

    while (curChild)
    {
        if (curChild == child)
        {
            if (curChild == parent->firstChild)
            {
                parent->firstChild = curChild->nextSubling;
                if (curChild->nextSubling)
                {
                    curChild->nextSubling->prevSubling = NULL;
                }
            }
            else
            {
                curChild->prevSubling->nextSubling = curChild->nextSubling;
                if (curChild->nextSubling)
                {
                    curChild->nextSubling->prevSubling = curChild->prevSubling;
                }
            }

            --(parent->childCount);
            child->parent = child->prevSubling = child->nextSubling = NULL;

            if (!parent->childCount)
            {
                tlv_node_set_attributes(tlv, parent, TLV_NODE_ATTR_IS_STRUCTUAL, 0);

                parent->length = 0;
                tlv_node_set_l(tlv, parent, 0);
            }

            break;
        }
        curChild =curChild->nextSubling;
    }

    return 0;
}

/*
 * calculate all the *l, length values in tlv tree
 * returns bytes of total dump size
 */
size_t tlv_layout(tlv* tlv)
{
    size_t dumplen = 0;
    tlvnode* curnode;
    _stack* laStack;

    if (!tlv)
    {
        return 0;
    }

    laStack = stack_obtain(STACK_INIT_SIZE);
    stack_push(laStack, tlv->root);

    while (laStack->index > 0)
    {
        curnode = (tlvnode*) stack_pop(laStack);

        if (!curnode)
        {
            continue;
        }

        if (curnode->firstChild != NULL)
        {
            curnode->length = 0;
            curnode = curnode->firstChild;
            while (curnode)
            {
                stack_push(laStack, curnode);
                curnode = curnode->nextSubling;
            }
        }
        else 
        {
            tlvnode* p = curnode->parent;
            tlv_node_set_l(tlv, curnode, curnode->length);
            while (p)
            {
                tlvnode* c = p->firstChild;
                p->length = 0;
                while (c)
                {
                    p->length += c->length + tlv->alength + tlv->tlength + tlv->llength;
                    c = c->nextSubling;
                }

                tlv_node_set_l(tlv, p, p->length);
                curnode = p;
                p = p->parent;
            }
        }
    }

    stack_destroy(laStack);

    dumplen =  tlv->root->length + tlv->tlength + tlv->llength;
    tlv->dumplength = dumplen;

    return dumplen;
}

size_t tlv_node_read_t(const tlv* tlv, const tlvnode* node, tlvbyte* buf, size_t bufsize)
{
    size_t size = 0;

    if (!tlv || !node || !buf || bufsize < 0)
    {
        return -1;
    }

    if (tlv->tlength)
    {
        size = bufsize < tlv->tlength ? bufsize : tlv->tlength;
        memcpy(buf, node->t, size);
    }

    return size;
}

size_t tlv_node_write_t(tlv* tlv, tlvnode* node, tlvbyte* tvalue, size_t tlength)
{
    size_t size;

    if (!tlv || !node || !tvalue || tlength < 0)
    {
        return 0;
    }

    size = tlength < tlv->tlength? tlength : tlv->tlength;
    memcpy(node->t, tvalue, size);

    return size;
}

size_t tlv_node_write_v(tlv* tlv, tlvnode* node, tlvbyte* vvalue, size_t vlength)
{
    size_t i;

    if (!tlv || !node || !vvalue || vlength < 0)
    {
        return 0;
    }

    node->length = vlength;
    node->v = (tlvbyte*)malloc(vlength);

    for (i = 0; i < vlength; i++)
    {
        node->v[i] = vvalue[i];
    }

    return vlength;
}


int tlv_node_traverse(tlv* t, int (*callback)(tlv*, tlvnode*))
{
    int visited = 0;
    tlvnode* root;
    _stack* stack;

    if (!t)
    {
        return 0;
    }

    root = t->root;

    if (!root)
    {
        return 0;
    }

    stack = stack_obtain(STACK_INIT_SIZE);
    stack_push(stack, (void*)root);

    while (stack_length(stack) > 0)
    {
        tlvnode* node = (tlvnode*) stack_pop(stack);

        if (!node)
        {
            continue;
        }

        ++visited;
        int visitres = 0;

        if (callback)
        {
            visitres = callback(t, node);
        }

        if (visitres)
        {
            break;
        }

        if (node->childCount > 0)
        {
            tlvnode* child = tlv_node_last_child(node);
            while (child)
            {
                stack_push(stack, (void*) child);
                child = child->prevSubling;
            }
        }
    }

    if (stack)
    {
        stack_destroy(stack);
    }

    return visited;
}

