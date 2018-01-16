/**
 * \file hash_table.h
 * \author Lucas Schott et Juan Manuel Torres Garcia
 */

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdbool.h>
#include <limits.h>
#include <time.h>

#include "addr_and_hash.h"



typedef struct address {
    struct in6_addr val;
    struct address * next_address;
} address;



typedef struct s_avl_node {
    uint16_t hash[HASH_LENGTH];
    address * address_list;
    time_t last_put;
    int eq;
    struct s_avl_node *ll, *rl;
} avl_node, *avl;




/***************************UTILISATION DE LA TABLE***************************/
/*****************************************************************************/




address * get_address_list(uint16_t * hash, avl table);
struct in6_addr get_address_val(address * a);
address * next_address(address * a);

avl insert_hash_addr_put(uint16_t * hash, struct in6_addr ip, avl table);
avl insert_hash_addr_share(uint16_t * hash, struct in6_addr ip ,avl a);
avl remove_hash_addr(uint16_t * hash, struct in6_addr ip, avl table);
avl remove_hash(uint16_t * hash,  avl a);
avl suppression_obsolete_hash(avl a);






/*********************************FONCTION INTERNES***************************/
/*****************************************************************************/




/********************************AVL******************************************/

//CREATION ALV///////////////////////////////////
avl root_avl(uint16_t * hash, struct in6_addr ip, avl g, avl d);

//INFORMATION SUR ARBRE//////////////////////////

uint16_t * get_hash(avl a);
avl left_sub_avl(avl a);
avl right_sub_avl(avl a);
bool is_empty_avl(avl a);
bool is_in_avl(uint16_t * hash, avl a);
bool is_sorted_avl(avl a);
avl avl_min(avl a);
avl avl_max(avl a);
int avl_height(avl a);
int avl_nearest_leaf_height(avl a);
bool is_avl(avl a);
int avl_balance_val(avl a);
int avl_max_difference_height(avl a);
avl find_node(uint16_t * hash, avl a);

//MODIFICATION///////////////////////////////////

avl remove_avl_max(avl a);
avl avl_r_rotation(avl a);
avl avl_l_rotation(avl a);
avl avl_rl_rotation(avl a);
avl avl_lr_rotation(avl a);
avl define_balance_val_avl(avl a);
avl balance_avl(avl a);

//AFFICHAGE//////////////////////////////////////

void disp_crescent_avl(avl a);

//LIBERATION ARBRE///////////////////////////////

void free_avl(avl a);



/*************************************LISTE***********************************/


address * head_insert_list(address * list, struct in6_addr ip);
address * suppression_list_elt(address * list, struct in6_addr ip);
void free_list(address * list);
int count_addr(address * list);
bool is_in(struct in6_addr addr,address * address_list);

struct in6_addr * catenate_addr_in_in6_addr(uint16_t * hash, avl hash_table);



/************************************PRINT************************************/

void print_address_list(address * list_head_adr);

#endif

