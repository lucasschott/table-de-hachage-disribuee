/**
 * \file hash_table.c
 * \author Lucas Schott et Juan Manuel Torres Garcia
 */

#include "hash_table.h"





/*********************AVL*****************************************************/
/*****************************************************************************/



/**
 * \fn avl root_avlr_avl(uint16_t * x, avl g, avl d)
 * \brief root_avlr_avl deux arbres
 * \return la racine_avl
 */
avl root_avl(uint16_t * hash, struct in6_addr ip, avl left, avl right)
{
    if(compare_hash(avl_max(left)->hash,hash)<0
            && compare_hash(avl_min(right)->hash,hash)>0)
    {
        avl n = (avl) malloc(sizeof(avl_node));
        int i;
        for(i=0;i<HASH_LENGTH;i++)
            n->hash[i]=hash[i];
        n->ll=left;
        n->rl=right;
        n = balance_avl(n);
        n->address_list=malloc(sizeof(struct address));
        for(i=0;i<ADDR_LENGTH;i++)
            n->address_list->val.s6_addr[i]=ip.s6_addr[i];
        return n;
    }
    else
    {
        fprintf(stderr,"impossible d'root_avlr_avl ces deux avl\n");
        fprintf(stderr,"le resultat ne serait pas un avl\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * \fn avl sag(avl a)
 * \return sous arbre gauche
 */
avl left_sub_avl(avl a)
{
    if(is_empty_avl(a))
        return NULL;
    return a->ll;
}

/**
 * \fn avl sad(avl a)
 * \return sous arbre droit
 */
avl right_sub_avl(avl a)
{
    if(is_empty_avl(a))
        return NULL;
    return a->rl;
}

/**
 * \fn uint16_t * racine_avl(avl a)
 * \return val de la racine_avl
 */
uint16_t * get_hash(avl a)
{
    if(is_empty_avl(a))
    {
        fprintf(stderr,"ERROR : get_hash\n");
        return NULL;
    }
    return a->hash;
}

/**
 * \fn int hauteur_avl(avl a)
 */
int avl_height(avl a)
{
    if(is_empty_avl(a))
        return 0;
    int lh = avl_height(left_sub_avl(a));
    int rh = avl_height(right_sub_avl(a));
    return 1 + (lh>rh ? lh : rh);
}

int avl_nearest_leaf_height(avl a)
{
    if(is_empty_avl(a))
        return 0;
    int lh = avl_nearest_leaf_height(left_sub_avl(a));
    int rh = avl_nearest_leaf_height(right_sub_avl(a));
    return 1 + (lh>rh ? rh : lh);
}

int avl_max_difference_height(avl a)
{
    return avl_height(a)-avl_nearest_leaf_height(a);
}

/**
 * \fn bool vide_avl(avl a)
 * \return true s'il est vide_avl, false sinon
 */
bool is_empty_avl(avl a)
{
    return a==NULL;
}

/**
 * \fn bool appartient_avl(uint16_t * x, avl a)
 * \a x elt a tester
 * \a a arbre a tester
 * \return true si l'elt est dans l'arbre false sinon
 */
bool is_in_avl(uint16_t * hash, avl a)
{
    int comparison = compare_hash(get_hash(a),hash);
    if(is_empty_avl(a))
        return false;
    if(comparison == 0)
        return true;
    if(comparison == -1)
        return is_in_avl(hash, right_sub_avl(a));
    if(comparison == 1)
        return is_in_avl(hash, left_sub_avl(a));
    else
        return false;
}

/**
 * \fn uint16_t * min_arbre_avl(avl a)
 * \a a arbre a tester
 * \return la valeur minimum de l'arbre
 */
avl avl_min(avl a)
{
    if(is_empty_avl(a))
        return NULL;
    if(is_empty_avl(left_sub_avl(a)))
        return a;
    else
        return avl_min(left_sub_avl(a));
}

/**
 * \fn uint16_t * min_arbre_avl(avl a)
 * \a a arbre a tester
 * \return la valeur maximum de l'arbre
 */
avl avl_max(avl a)
{
    if(is_empty_avl(a))
        return NULL;
    if(is_empty_avl(right_sub_avl(a)))
        return a;
    else
        return avl_max(right_sub_avl(a));
}

/**
 * \fn bool est_trie_avl(avl a)
 * \brief determine si un arbre est trié
 * \a arbre a tester
 * \return true ou false
 */
bool is_sorted_avl(avl a)
{
    if(is_empty_avl(a))
    {
        fprintf(stderr,"ERROR : est_trie_avl\n");
        exit(EXIT_FAILURE);
    }
    if(is_empty_avl(right_sub_avl(a)) && is_empty_avl(left_sub_avl(a)))
        return true;
    if(is_empty_avl(left_sub_avl(a)))
    {
        if(compare_hash(avl_min(right_sub_avl(a))->hash,get_hash(a))==1
                && is_sorted_avl(right_sub_avl(a)))
            return true;
    }
    if(is_empty_avl(right_sub_avl(a)))
    {
        if(compare_hash(avl_max(left_sub_avl(a))->hash,get_hash(a))==-1
                && is_sorted_avl(left_sub_avl(a)))
            return true;
    }
    if(!is_empty_avl(left_sub_avl(a)) && !is_empty_avl(right_sub_avl(a)))
    {
        if(compare_hash(avl_max(left_sub_avl(a))->hash,get_hash(a))
                ==-1 &&
                compare_hash(avl_min(right_sub_avl(a))->hash,get_hash(a))
                ==1 &&
                is_sorted_avl(right_sub_avl(a)) &&
                is_sorted_avl(left_sub_avl(a)))
            return true;
    }
    return false;
}

/**
 * \fn void affiche_croissant(avl a)
 * \brief affiche les elements de l'arbre en ordre croissant
 * \a a arbre a afficher
 */
void disp_crescent_avl(avl a)
{
    if(is_empty_avl(a))
    {
        fprintf(stderr,"ERROR : empty avl\n");
        exit(EXIT_FAILURE);
    }
    if(!is_empty_avl(left_sub_avl(a)))
        disp_crescent_avl(left_sub_avl(a));
    printf("hash :");
    print_hash(get_hash(a));
    printf("addresses :"),
    print_address_list(a->address_list);
    printf("\n");
    if(!is_empty_avl(right_sub_avl(a)))
        disp_crescent_avl(right_sub_avl(a));
    return;
}

/**
 * \fn void inserer_avl(avl a, uint16_t * elt)
 * \brief ajoute un hash et d'une adresse dans la table
 * met a jour la date de dernier put si la donnée est déja presente dans la
 * table
 * \a a arbre ou inserer_avl la feuille
 * \a elt valeur a inserer_avl
 */
avl insert_hash_addr_put(uint16_t * hash, struct in6_addr ip ,avl a)
{
    if(is_empty_avl(a))
    {
        int i;
        a= malloc(sizeof(avl_node));
        a->ll=NULL;
        a->rl=NULL;
        a->last_put=time(NULL);
        for(i=0;i<HASH_LENGTH;i++)
            a->hash[i]=hash[i];
        a->eq = 0;
        a->address_list=NULL;
        a->address_list = head_insert_list(a->address_list,ip);
    }
    else if(compare_hash(hash,get_hash(a))==0)
    {
        if(!is_in(ip,a->address_list))
        {
            a->address_list = head_insert_list(a->address_list,ip);
        }
        a->last_put = time(NULL);
    }
    else if(compare_hash(hash,get_hash(a))==-1)
    {
        a->ll = insert_hash_addr_put(hash,ip,left_sub_avl(a));
    }
    else if(compare_hash(hash,get_hash(a))==1)
    {
        a->rl = insert_hash_addr_put(hash,ip,right_sub_avl(a));
    }
    a = balance_avl(a);
    return a;
}

/**
 * \fn void inserer_avl(avl a, uint16_t * elt)
 * \brief ajoute un hash et d'une adresse dans la table
 * ne met pas a jour la date de dernier put si la donnée est déja presente
 * dans la table
 * \a a arbre ou inserer_avl la feuille
 * \a elt valeur a inserer_avl
 */
avl insert_hash_addr_share(uint16_t * hash, struct in6_addr ip ,avl a)
{
    if(is_empty_avl(a))
    {
        int i;
        a= malloc(sizeof(avl_node));
        a->ll=NULL;
        a->rl=NULL;
        a->last_put=time(NULL);
        for(i=0;i<HASH_LENGTH;i++)
            a->hash[i]=hash[i];
        a->eq = 0;
        a->address_list = NULL;
        a->address_list = head_insert_list(a->address_list,ip);
    }
    else if(compare_hash(hash,get_hash(a))==0)
    {
        if(!is_in(ip,a->address_list))
        {
            a->address_list = head_insert_list(a->address_list,ip);
        }
    }
    else if(compare_hash(hash,get_hash(a))==-1)
    {
        a->ll = insert_hash_addr_share(hash,ip,left_sub_avl(a));
    }
    else if(compare_hash(hash,get_hash(a))==1)
    {
        a->rl = insert_hash_addr_share(hash,ip,right_sub_avl(a));
    }
    a = balance_avl(a);
    return a;
}



/**
 * \fn avl oter_max_avl(avl a)
 * utilisation de free
 */
avl remove_avl_max(avl a)
{
    avl b;
    if(is_empty_avl(a))
        b=NULL;
    else if(is_empty_avl(right_sub_avl(a)))
    {
        b = left_sub_avl(a);
    }
    else
    {
        a->rl = remove_avl_max(right_sub_avl(a));
        b=a;
    }
    b = balance_avl(b);
    return b;
}

/**
 * \fn void supprimer_avl(uint16_t * elt, avl a)
 */
avl remove_hash_addr(uint16_t * hash, struct in6_addr ip,  avl a)
{
    avl b;
    if(is_empty_avl(a))
        b=NULL;
    else
    {
        if(compare_hash(get_hash(a),hash)==0)
        {
            a->address_list = suppression_list_elt(a->address_list,ip);
            b=a;
            if(a->address_list==NULL)
            {
                if(is_empty_avl(left_sub_avl(a)))
                {
                    b=right_sub_avl(a);
                    free(a);
                }
                else if(is_empty_avl(right_sub_avl(a)))
                {
                    b=left_sub_avl(a);
                    free(a);
                }
                else
                {
                    avl tmp1 = avl_max(left_sub_avl(a));
                    avl tmp2 = a->rl;
                    free(a);
                    b=tmp1;
                    b->ll = remove_avl_max(left_sub_avl(a));
                    b->rl = tmp2;
                }
            }
        }
        else if(compare_hash(get_hash(a),hash)==1)
        {
            a->ll = remove_hash_addr(hash,ip,left_sub_avl(a));
            b=a;
        }
        else
        {
            a->rl = remove_hash_addr(hash,ip,right_sub_avl(a));
            b=a;
        }
    }
    b = balance_avl(b);
    return b; 
}


/**
 * \fn void supprimer_avl(uint16_t * elt, avl a)
 */
avl remove_hash(uint16_t * hash,  avl a)
{
    avl b;
    if(is_empty_avl(a))
        b=NULL;
    else
    {
        if(compare_hash(get_hash(a),hash)==0)
        {
            free_list(a->address_list);
            b=a;
            if(is_empty_avl(left_sub_avl(a)))
            {
                b=right_sub_avl(a);
                free(a);
            }
            else if(is_empty_avl(right_sub_avl(a)))
            {
                b=left_sub_avl(a);
                free(a);
            }
            else
            {
                avl tmp1 = avl_max(left_sub_avl(a));
                avl tmp2 = a->rl;
                free(a);
                b=tmp1;
                b->ll = remove_avl_max(left_sub_avl(a));
                b->rl = tmp2;
            }
        }
        else if(compare_hash(get_hash(a),hash)==1)
        {
            a->ll = remove_hash(hash,left_sub_avl(a));
            b=a;
        }
        else
        {
            a->rl = remove_hash(hash,right_sub_avl(a));
            b=a;
        }
    }
    b = balance_avl(b);
    return b; 
}




/**
 * \fn void supprimer_avl(uint16_t * elt, avl a)
 */
avl suppression_obsolete_hash(avl a)
{
    time_t sysdate = time(NULL);
    if(is_empty_avl(a))
        a=NULL;
    else
    {
        if(sysdate-a->last_put>30)
        {
            a = remove_hash(a->hash, a);
            a = suppression_obsolete_hash(a);
        }
        else
        {
            if(!is_empty_avl(left_sub_avl(a)))
            {
                a->ll = suppression_obsolete_hash(left_sub_avl(a));
            }
            if(!is_empty_avl(right_sub_avl(a)))
            {
                a->rl = suppression_obsolete_hash(right_sub_avl(a));
            }
        }
    }
    return a;
}



/*
 * \fn avl rotation_d_avl(avl a)
 */
avl avl_r_rotation(avl a)
{
    avl new;
    if(is_empty_avl(a))
        return a;
    if(!is_empty_avl(left_sub_avl(a)))
    {
        new=left_sub_avl(a);
        avl tmp=right_sub_avl(new);
        new->rl=a;
        new->rl->ll=tmp;
    }
    else
        new = a;
    return new;
}

/*
 * \fn avl rotation_d_avl(avl a)
 */
avl avl_l_rotation(avl a)
{
    if(is_empty_avl(a))
        return a;
    avl new;
    if(!is_empty_avl(right_sub_avl(a)))
    {
        new=right_sub_avl(a);
        avl tmp=left_sub_avl(new);
        new->ll=a;
        new->ll->rl=tmp;
    }
    else
        new = a;
    return new;
}

/*
 * \fn avl rotation_dg_avl(avl a)
 */
avl avl_rl_rotation(avl a)
{
    if(is_empty_avl(a))
    {
        fprintf(stderr,"rotation dg impossible\n");
        exit(EXIT_FAILURE);
    }
    if(!is_empty_avl(right_sub_avl(a)))
    {
        a->rl = avl_r_rotation(right_sub_avl(a));
    }
    a = avl_l_rotation(a);
    a->eq--;
    return a;
}

/*
 * \fn avl rotation_gd_avl(avl a)
 */
avl avl_lr_rotation(avl a)
{
    if(is_empty_avl(a))
    {
        fprintf(stderr,"rotation gd impossible\n");
        exit(EXIT_FAILURE);
    }
    if(!is_empty_avl(left_sub_avl(a)))
    {
        a->ll = avl_l_rotation(left_sub_avl(a));
    }
    a = avl_r_rotation(a);
    a->eq++;
    return a;
}


/**
 * \fn void liberer_arbre(avl a)
 */
void free_avl(avl a)
{
    if(is_empty_avl(a))
        return;
    if(!is_empty_avl(left_sub_avl(a)))
        free_avl(left_sub_avl(a));
    if(!is_empty_avl(right_sub_avl(a)))
        free_avl(right_sub_avl(a));
    free_list(a->address_list);
    free(a);
    return;
}

/**
 * \fn bool est_avl(avl a)
 */
bool is_avl(avl a)
{
    if(is_empty_avl(a))
        return true;
    a = define_balance_val_avl(a);
    if(avl_max_difference_height(a)<2 &&
            is_avl(left_sub_avl(a)) && is_avl(right_sub_avl(a)))
        return true;
    else
        return false;
}

/**
 * \fn avl equilibrer_avl(avl a)
 */
avl balance_avl(avl a)
{
    if(is_empty_avl(a))
        return NULL;
    a = define_balance_val_avl(a);
    while(!is_avl(a))
    {
        a->ll = balance_avl(left_sub_avl(a));
        a->rl = balance_avl(right_sub_avl(a));
        a = define_balance_val_avl(a);
        while(avl_balance_val(a)<-1)
        {
            a = avl_lr_rotation(a);
        }
        while(a->eq>1)
        {
            a = avl_rl_rotation(a);
        }
    }
    a = define_balance_val_avl(a);
    return a;
}

/**
 * \fn int def_equilibrage_avl(avl a)
 */
avl define_balance_val_avl(avl a)
{
    if(is_empty_avl(a))
        return NULL;
    int hg = avl_height(left_sub_avl(a));
    int hd = avl_height(right_sub_avl(a));
    a->eq=hd-hg;
    return a;
}

/**
 * \int val_equilibre_avl(avl a)
 */
int avl_balance_val(avl a)
{
    if(is_empty_avl(a))
    {
        fprintf(stderr,"ERROR : val_equilibre_avl\n");
        exit(EXIT_FAILURE);
    }
    return a->eq;
}

avl find_node(uint16_t * hash, avl a)
{
    if(is_empty_avl(a))
        return NULL;
    int comparison = compare_hash(get_hash(a),hash);
    if(comparison == 0)
        return a;
    if(comparison == -1)
        return find_node(hash, right_sub_avl(a));
    if(comparison == 1)
        return find_node(hash, left_sub_avl(a));
    else
        return NULL;
}


address * get_address_list(uint16_t * hash, avl table)
{
    avl node = find_node(hash,table);
    if(node==NULL)
        return NULL;
    return node->address_list;
}


/***********************LISTE*************************************************/
/*****************************************************************************/

struct in6_addr get_address_val(address * a)
{
    return a->val;
}


address * next_address(address * a)
{
    if(a==NULL)
        return NULL;
    return a->next_address;
}


address * head_insert_list(address * list_head, struct in6_addr ip)
{
    int i;
    if(list_head == NULL)
    {
        list_head  = malloc(sizeof(address));
        for(i=0;i<ADDR_LENGTH;i++)
            list_head->val.s6_addr[i]=ip.s6_addr[i];
        list_head->next_address = NULL;
        return list_head;
    }
    address * new;
    new  = malloc(sizeof(address));
    for(i=0;i<ADDR_LENGTH;i++)
        new->val.s6_addr[i]=ip.s6_addr[i];
    new->next_address = list_head;

    return new;
}


int count_addr(address * list)
{
    if(list==NULL)
        return 0;
    return 1 + count_addr(list->next_address);
}


bool is_in(struct in6_addr addr,address * address_list)
{
    if(address_list==NULL)
    {
        return false;
    }
    else if(memcmp(&address_list->val,&addr,sizeof(struct in6_addr))==0)
    {
        return true;
    }
    else
    {
        return is_in(addr,address_list->next_address);
    }
}



address *  suppression_list_elt(address * list_head, struct in6_addr ip)
{
    if(list_head==NULL)
        return NULL;
    if(compare_addr(list_head->val,ip)==0)
    {
        address * tmp = list_head->next_address;
        free(list_head);
        return tmp;
    }
    else
    {
        list_head->next_address =
            suppression_list_elt(list_head->next_address,ip);
        return list_head;
    }
}


void free_list(address * list)
{
    if(list==NULL)
        return;
    else if(list->next_address!=NULL)
    {
        free_list(list->next_address);
    }
    free(list);
}

/*
struct in6_addr * catenate_addr_in_in6_addr(uint16_t * hash,avl hash_table)
{
    address * addr_list;
    addr_list = get_address_list(hash,hash_table);
    if(addr_list==NULL)
    {
        //TODO
    }
    int nb_addr = count_addr(addr_list);
    struct in6_addr * msg = malloc(nb_addr*sizeof(struct in6_addr));
    address * addr_tmp = addr_list;
    int i=0;
    while(addr_tmp != NULL && i<nb_addr)
    {
        memcpy(&(msg[i]),&(addr_tmp->val),
                sizeof(struct in6_addr));
        addr_tmp = addr_tmp->next_address;
        i++;
    }
    return  msg;
}
*/



/******************PRINT***************************************/
/**************************************************************/



void print_address_list(address * addr)
{
    if(addr==NULL)
        return;
    if(addr->next_address==NULL)
        print_address(addr->val);
    else
    {
        print_address(addr->val);
        print_address_list(addr->next_address);
    }
}


