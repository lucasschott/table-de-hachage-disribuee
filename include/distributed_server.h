/**
 * \file distributed_server.h
 * \author Lucas Schott et Juan Manuel Torres Garcia
 * \brief définition des fonctinos gérant la connexion des servers
 * entre eux, ainsi que de la distribution de la table
 */

#ifndef DISTRIBUTED_SERVER_H
#define DISTRIBUTED_SERVER_H

#include "client_server.h"


/**
 * \brief les variables globales d'un server:
 * son descripteur de socket
 * son adresse IPv6
 * son numero de port
 */
int my_sockfd;
struct in6_addr my_addr;
in_port_t my_port;

/**
 * les semaphore permettant a un server d'effectuer les operations sur sa table
 * contenant les hash, et sa tables des servers connectés en toute sécurité
 */
sem_t sem_hash;
sem_t sem_server;

/*
 * headers des messages envoyés entre servers
 */
//message pour se connecter à un server
#define CONNECT_HEADER 4
//message pour partager des données entre server, sans mise a jour de la date
//de derner put
#define SHARE_DATA_HEADER 5
//message pour partager des données entre server, avec mise a jour de la date 
//de dernier put
#define SHARE_PUT_HEADER 6
//message pour envoyer à un server les adresses des autre servers déja
//connectés en eux
#define SHARE_SERVER_HEADER 7
//message pour garder a connexion entre deux server
#define KEEP_HEADER 8
//message pour demander à un server les adresse associés à un hash
#define SERVER_GET_HEADER 9


//structures de messages envoyé par le réseau
//*******************************************


//structure des messages ou un server A envoi les coodonnées d'un server B à un
//server C
typedef struct syn_t{
    uint16_t header;
    struct in6_addr addr;
    in_port_t port;
}share_server_t, connect_t, keep_t;



//stockage de données pour le maintien des
//connections server**********************


//structrure contenant les informations sur les autres servers qu'un server
//possède (n'est pas à envoyer par le réseau)
typedef struct connected_server{
    struct in6_addr addr;
    in_port_t port;
    time_t last_keep_alive;
    struct connected_server * next;
}connected_server;


//table de server

connected_server * next_server(connected_server * serv);
connected_server * insert_server(connected_server * list_head,
        struct in6_addr addr, in_port_t port);
int count_server(connected_server * list);
connected_server *  suppression_server(connected_server * list_head,
        struct in6_addr addr, in_port_t port);
void  keep_alive_server(connected_server * list_head,
        struct in6_addr addr, in_port_t port);
connected_server * suppression_dead_server(connected_server * server_table);
bool is_known(struct in6_addr addr, in_port_t port,
        connected_server * server_table);
void free_server_tab(connected_server * list);


#endif
