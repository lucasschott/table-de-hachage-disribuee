/**
 * \file client_server.h
 * \author Lucas Schott et Juan Manuel Torres Garcia
 * \brief fichier contenant les stuctures et d'autres informations dont les
 * clients et les servers vont se servir. Comme les header des messages qu'ils
 * s'envoient, les structures de ces messages etc.
 */

#ifndef CLIENT_SERVER_H
#define CLIENT_SERVER_H

#include <unistd.h>
#include <netdb.h>

#include <pthread.h>
#include <semaphore.h>

#include <signal.h>

#include "hash_table.h"
#include "affichage.h"


//taille max des message sur le réseaux, en plafonnnant le nombre d'adresse
//envoyé par le server a 20
#define MSG_MAX_LENGTH 100


/**
 * definitions des headers des messages qui circulent par le réseau entre
 * les clients et les servers
 */

//vide 
#define EMPTY_HEADER 0
//quand client envoi un hash et une adresse à un server
#define PUT_HEADER 1
//quand un cliend demande des adresses à un server
#define GET_HEADER 2
//quand un server envoi une addresse à un client
#define RESPONSE_HEADER 3


//macro pour tester le retour des primitives systeme
#define PRIM(r, msg) do{if((r)==-1){perror(msg);exit(EXIT_FAILURE);}}while(0)
//marco pour tester le retour des fonctions pthread
#define THREAD(r,msg) do{if((err = r)!=0){fprintf(stderr,strerror(err));\
    exit(EXIT_FAILURE);}}while(0)
//maco pour effectuer une action uniquement si l'option verbose à été donnée en 
//parametre au programme
#define VERB(action) do{if(verbose){action;}}while(0)



//structures de messages envoyé par le réseau
//*******************************************

//strucure d'un message contenant juste le header
typedef struct msg_t{
    uint8_t header;
}msg_t;

//structure d'un message contenant un hash
typedef struct get_t{
    uint16_t header;
    uint16_t hash[HASH_LENGTH];
}get_t;

//structure d'un message contenant un hash et une adresse
typedef struct put_t{
    uint16_t header;
    uint16_t hash[HASH_LENGTH];
    struct in6_addr addr;
}put_t, share_data_t, share_put_t, response_t;

#endif
