/*
 * tlv.h
 * This module provides TLV (TagLengthValue) constructing
 * and parsing functions.
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
 * along with this file; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA. 
 *
 * Presented by yingjun.ma and tome.huang
 * on 20180608.
 *
 * All rights reserved.
 */

#ifndef __TLV_H
#define __TLV_H

#include <stdlib.h>


// default lengths the attribute, tag and length
// fields take
#define TLV_DEF_A_LENGTH 1
#define TLV_DEF_T_LENGTH 2
#define TLV_DEF_L_LENGTH 2



typedef unsigned char tlvbyte;

typedef enum {
    TLV_BYTE_MSB = 0,       // Big Endian
    TLV_BYTE_LSB  = 1       // Little Endian
} tlv_byte_prio_order_t;


typedef struct tlvnode {
    tlvbyte* a;
    tlvbyte* t;
    tlvbyte* l;
    tlvbyte* v;

    /*
     * length: optinal field: the value from *l
     * the below tlv node functions make sure this
     * field will be updated once the value in *l changes
     */
    size_t length;

    struct tlv* tlv;

    struct tlvnode* parent;

    size_t childCount;
    struct tlvnode* firstChild;
    struct tlvnode* nextSubling;
    struct tlvnode* prevSubling;
} tlvnode;


typedef struct tlv {
    tlvnode* root;

    size_t alength; // bytes of attribute
    size_t tlength; // bytes of tag
    size_t llength; // bytes of length

    tlv_byte_prio_order_t byteprio;

    size_t dumplength; // how much bytes does it take when tlv was dumpped to buffer

} tlv;


typedef enum tlv_node_attr {
    // when set to 0, this node has no child
    // if node contains child, it should have no value
    TLV_NODE_ATTR_IS_STRUCTUAL          =       0x02
} tlv_node_attr_t;
 


////////////////////////////// TLV FUNCTIONS BELOW //////////////////////////////

tlv* tlv_obtain();
void tlv_destroy(tlv* tlv);

/*
 * load bytes into tlv node tree.
 * defination for tlv should be set into original tlv struct.
 * if succeed, tlv node tree should be set to tlv->root.
 * returns how much bytes was handled in bytes buffer.
 */
size_t tlv_loads(tlv* tlv, tlvbyte* bytes, size_t size);

/*
 * dump tlv struct into bytes.
 * returns byte length.
 */
size_t tlv_dumps(tlv* tlv, tlvbyte* buf, size_t bufsize);

/*
 * calculate all the *l, length values in tlv tree
 * returns bytes of total dump size
 *
 * * NOTICE: should be invoked before dump tlv to buffer
 * * OR: set tlv->autolayout = 1, then the tlv_layout
 *       will be invoked each time the length might be changed
 */
size_t tlv_layout(tlv* tlv);

/*
 * specify tree entrance for tlv
 */
void tlv_set_root(tlv* tlv, tlvnode* root);

/*
 * returns how many nodes does current tlv contain
 */
int tlv_node_count(const tlv* tlv);


////////////////////////////// TLV FUNCTIONS ABOVE //////////////////////////////
////////////////////////////// TLV NODE FUNCTIONS BELOW //////////////////////////////

tlvnode* tlv_node_obtain(tlv* tlv);
void tlv_node_destroy(tlv* tlv, tlvnode* node);

size_t tlv_node_read_t(const tlv* tlv, const tlvnode* node, tlvbyte* buf, size_t bufsize);
size_t tlv_node_write_t(tlv* tlv, tlvnode* node, tlvbyte* tvalue, size_t tlength);
size_t tlv_node_write_v(tlv* tlv, tlvnode* node, tlvbyte* vvalue, size_t vlength);

/*
 * set attribute bit value
 * attr: which bit
 * value: 0 or 1 on specified bit
 */
void tlv_node_set_attributes(tlv* tlv, tlvnode* node, tlv_node_attr_t attr, int value);

/*
 * get attribute bit value
 * attr: which bit
 * returns: 0 or 1 on specified bit
 */
int tlv_node_get_attributes(const tlv* tlv, const tlvnode* node, tlv_node_attr_t attr);

/*
 * add child to parent according to the standard in tlv
 *
 * *NOTICE: this operation will erase the *v in parent
 * returns: 0 if succeed
 */
int tlv_node_add_child(tlv* tlv, tlvnode* parent, tlvnode* child);

/*
 * pick child off from parent
 *
 * *NOTICE: the removed child will not be destroyed in this function
 * returns: 0 if succeed
 */
int tlv_node_remove_child(tlv* tlv, tlvnode* child);

/*
 * callback which is used for traversing tlv node tree
 *
 * tlv: traversed subject
 * node: current visiting node
 * returns: 0 if you want the traversing continue
 */
int tlv_node_traverse_callback(tlv* tlv, tlvnode* node);

/*
 * traverse in tlv node tree
 *
 * works with tlv_node_traverse_callback
 * returns: total count of visited nodes
 */
int tlv_node_traverse(tlv* t, int (*callback)(tlv*, tlvnode*));


////////////////////////////// TLV NODE FUNCTIONS BELOW //////////////////////////////

#endif // __TLV_H
