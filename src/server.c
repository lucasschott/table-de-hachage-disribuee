/**
 * \file server.c
 * \author Lucas Schott et Juan Manuel Torres Garcia
 */

#include "client_server.h"
#include "distributed_server.h"

//variable err pour tester le retour des fonctions fthread
int err;

//variable end_signal modifié lors de la reception du signal SIGINT
volatile bool end_signal = false;

//variable indiquant si l'option verbose a été activé lors du lancement du
//programme ou non
bool verbose = false;

//adresse de la racine de la table de hachage
avl hash_table = NULL;

//adresse du premier element de la table des servers connectés
connected_server * server_table = NULL;


/**
 * usage du programme server:
 * ./server <adresse du server> <no de port du server>
 * ou
 * ./server <adresse du server> <no de port du server>
 * <adresse d'un sever auquel se connecter< <port de ce server>
 */

void exit_usage_server(void)
{
    fprintf(stderr,"usage: server <ip> <port> [<ip connexion>\
<port connexion>]\n");
    exit(EXIT_FAILURE);
}


/***********************************signaux***********************************/

/**
 * \fn void terminer_processus( __attribute__((unused)) int sig )
 * \brief met la valeur de end_signal a "true" quand le programme recoit le
 * signal SIGINT, pour terminer proprement le programme et liberant toute 
 * la mémoire alloué
 */
void terminer_processus()
{
    end_signal=true;
}

/**
 * \fn void terminer_thread()
 * \brief fermeture du thread a la reception du signal SIGUSR1
 */
void terminer_thread()
{
    pthread_exit(EXIT_SUCCESS);;
}

/**
 * \fn void init_signal(void)
 * \brief initialisation des signaux attrapés par le programme
 */
void init_signal(void)
{
    struct sigaction s;

    s.sa_handler = terminer_processus;
    s.sa_flags = SA_RESTART;
    sigemptyset(&s.sa_mask);
    PRIM(sigaction(SIGINT,&s,NULL),"sigaction");
}

/**
 * \fn void init_thread_signal(void)
 * \brief initialisation des signaux attrapés par le thread
 */
void init_thread_signal(void)
{
    struct sigaction s;
    sigemptyset(&s.sa_mask);
    sigaddset(&(s.sa_mask),SIGINT);
    THREAD(pthread_sigmask(SIG_BLOCK,&(s.sa_mask),NULL),"pthread_sigmask");
    s.sa_handler = terminer_thread;
    PRIM(sigaction(SIGUSR1,&s,NULL),"sigaction");
}


/*******************************init socket***********************************/

/**
 * \fn int open_my_socket(int local_port, struct in6_addr ip_addr)
 * \brief ouverture de la socket adapté à l'IPv6 et UDP
 */
int open_my_socket(int local_port, struct in6_addr ip_addr)
{
    int sockfd;

    socklen_t addrlen = sizeof(struct sockaddr_in6);

    struct sockaddr_in6 my_sockaddr;

    // creation de la socket
    if((sockfd = socket(AF_INET6,SOCK_DGRAM,IPPROTO_UDP)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // initialisation des parametres voulu
    my_sockaddr.sin6_family      = AF_INET6;
    my_sockaddr.sin6_port        = htons(local_port);
    int i;
    for(i=0;i<16;i++)
        my_sockaddr.sin6_addr.s6_addr[i] = htons(ip_addr.s6_addr[i]);
    my_sockaddr.sin6_flowinfo    = 0;
    my_sockaddr.sin6_scope_id    = 0;

    // lier la structure sockaddr_in6 avec sa socket
    if(bind(sockfd,(struct sockaddr *) &my_sockaddr,addrlen) == -1)
    {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}



/****************************receive********************************/


/**
 * \fn msg_t * receive_m(int my_sockfd,
 * struct sockaddr_storage ** client_sockaddr, socklen_t ** client_addrlen)
 * \brief attend 1 secondes la reception d'un message sur la socket
 * \return le pointeur sur le message recu
 * \pre pointeur sur un pointeur de structure sockaddr_storage pour pouvoir
 * stocker les coordonnée d'un client pour pouvoir lui répondre ulterieurement
 * \post le pointeur sur le message recu est à liberer après utiliation
 */
msg_t * receive_m(int my_sockfd, struct sockaddr_storage ** client_sockaddr,
        socklen_t ** client_addrlen)
{
    msg_t * msg = calloc(1,MSG_MAX_LENGTH);

    **client_addrlen = sizeof(**client_sockaddr);

    struct timeval timeVal;
    fd_set readSet;

    FD_ZERO(&readSet);
    FD_SET(my_sockfd, &readSet);
    timeVal.tv_sec = 1;
    timeVal.tv_usec = 0;

    //attente une seconde le reception d'un message
    if (select(my_sockfd+1, &readSet, NULL, NULL, &timeVal))
    {
        // reception de la chaine de caracteres
        recvfrom(my_sockfd,msg,MSG_MAX_LENGTH,MSG_DONTWAIT,
                (struct sockaddr*) *client_sockaddr, *client_addrlen);
    }
    else
    {
        //si aucun message n'est recu au bout d'une seconde, le header du
        //message est défini comme vide
        msg->header=EMPTY_HEADER;
    }

    return msg;
}



/****************************send***********************************/


/**
 * \fn int responde_to(int my_sockfd, struct sockaddr_storage * client_sockaddr,
 * socklen_t * client_addrlen, msg_t * msg_snd)
 * \brief envoyer le message msg_snd a l'address client_sockaddr de taille
 * client_addr_len par my_socket
 */
int responde_to(int my_sockfd, struct sockaddr_storage * client_sockaddr,
        socklen_t * client_addrlen, msg_t * msg_snd)
{
    *client_addrlen = sizeof(*client_sockaddr);

    //definition de la taille du message a envoyer en fonction
    //du header du message
    int size;
    if(msg_snd->header==PUT_HEADER)
    {size=sizeof(put_t);}
    else if(msg_snd->header==GET_HEADER)
    {size=sizeof(get_t);}
    else if(msg_snd->header==CONNECT_HEADER)
    {size=sizeof(connect_t);}
    else if(msg_snd->header==KEEP_HEADER)
    {size=sizeof(keep_t);}
    else if(msg_snd->header==SHARE_DATA_HEADER)
    {size=sizeof(share_data_t);}
    else if(msg_snd->header==SHARE_SERVER_HEADER)
    {size=sizeof(share_server_t);}
    else if(msg_snd->header==RESPONSE_HEADER)
    {size=sizeof(response_t);}

    //envoi du message 
    if(sendto(my_sockfd,msg_snd, size, 0,
                (const struct sockaddr*) client_sockaddr,
                *client_addrlen) == -1)
    {
        perror("sendto");
        close(my_sockfd);
        exit(EXIT_FAILURE);
    }
    return 0;
}


/**
 * \fn struct addrinfo get_address_info(char * addr,char * port)
 * \brief obtenir les informatiques sur un hote
 * \return struct addrinfo accessible de l'hote
 */
struct addrinfo * get_address_info(char * addr,char * port)
{
    int sockfd;
    int status;
    bool sockSuccess = false;
    struct addrinfo hints,*host_info, *host;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    //obtenir les info a partir de son addres (ou nom de domaine) et du port
    if ((status = getaddrinfo(addr, port,&hints, &host)) != 0)
    {
        return NULL;
    }

    //trouver les infos sur la struct addrinfo accessible
    host_info = host;
    while((host_info != NULL) && !sockSuccess)
    {
        if ((sockfd = socket(host_info->ai_family, host_info->ai_socktype,
                        host_info->ai_protocol)) == -1)
        {
            perror("socket:");
            sockSuccess = false; // Echec ouverture socket
            host_info = host_info->ai_next;// Enregistrement d'adresse suivant
        }
        else // La prise réseau est valide
            sockSuccess = true;
    }
    if (host_info == NULL)
    {
        VERB(fputs("Création de socket impossible", stderr));
        return NULL;
    }

    struct addrinfo * new_host = malloc(sizeof(struct addrinfo));
    memcpy(new_host,host_info,sizeof(struct addrinfo));
    new_host->ai_next=NULL;

    freeaddrinfo(host);

    return new_host;
}



/**
 * \fn void send_to_server(int my_sockfd,struct in6_addr server_addr,
 * in_port_t server_port,msg_t * msg, int msg_len)
 * \brief envoi d'une message a un autre sever dont on connais l'ip et le port
 */
void send_to_server(int my_sockfd,struct in6_addr server_addr,
        in_port_t server_port,msg_t * msg, int msg_len)
{
    if(memcmp(&server_addr,&my_addr,sizeof(struct in6_addr))==0
            && server_port==my_port)
    {
        return;
    }

    char * ip = in6_addr_to_char_addr(server_addr);
    char * port = malloc(6*sizeof(char));
    snprintf(port,6,"%d",server_port);


    struct addrinfo * server_info = get_address_info(ip,port);
    if(server_info==NULL)
    {
        VERB(printf("envoi impossible"));
    }

    if(sendto(my_sockfd,msg,msg_len,0,server_info->ai_addr,
                server_info->ai_addrlen)==-1)
    {
        perror("sendto");
        close(my_sockfd);;
        exit(EXIT_FAILURE);
    }

    free(ip);
    free(port);
    free(server_info);
}



/*************************answer to client*******************************/

/**
 * \fn void answer_to_client(get_t get_rcvd,
 * struct sockaddr_storage * client_sockaddr,
 * socklen_t * client_addrlen)
 * \brief répond a un client dont on connait sa struct sockaddr_storage
 * envoi des ip associés au hash deandé par le client
 */
void answer_to_client(get_t get_rcvd,
        struct sockaddr_storage * client_sockaddr,
        socklen_t * client_addrlen)
{
    uint16_t nb_addr = count_addr(get_address_list(
                get_rcvd.hash,hash_table));

    response_t * response_snd = malloc(sizeof(response_t)+
            sizeof(struct in6_addr)*nb_addr);

    int i;

    response_snd->header = RESPONSE_HEADER;
    for(i=0;i<HASH_LENGTH;i++)
        response_snd->hash[i] = get_rcvd.hash[i];

    //obtenir la liste des adresse IP associés au hash demandé par le client
    address * addr = get_address_list(response_snd->hash,hash_table);

    //si le server ne dispose pas de ce hash, il va demander au autre server
    //connectés
    if(addr != NULL)
    {
        put_t msg;
        msg.header = SERVER_GET_HEADER;
        for(i=0;i<HASH_LENGTH;i++)
            msg.hash[i]=get_rcvd.hash[i];

        VERB(printf("ask other server\n"));
        connected_server * server = server_table;

        while(server!=NULL)
        {
            struct in6_addr client_addr = server->addr;
            in_port_t client_port = server->port;
            send_to_server(my_sockfd,client_addr,client_port,
                    (msg_t*) &msg,sizeof(msg_t));
            server = server->next;
        }
    }

    //ensuite il va tant qu'il y a des adresses assocés au hash, les envoyer
    //au client, une adresse par message
    while(addr != NULL)
    {
        fflush(NULL);
        fflush(NULL);
        VERB(printf("envoi\n"));
        fflush(NULL);
        for(i=0;i<ADDR_LENGTH;i++)
        {
            response_snd->addr.s6_addr[i]=addr->val.s6_addr[i];
        }
        responde_to(my_sockfd,client_sockaddr,client_addrlen,
            (msg_t*) response_snd);
        addr=addr->next_address;
    }
    free(response_snd);
}


/**
 * \fn void answer_to_server(get_t get_rcvd,
 * struct sockaddr_storage * client_sockaddr,
 * socklen_t * client_addrlen)
 * \brief repondre a un sever quand celui-ci demande de lui envoyer les
 * adresses associés a un hash, lui envoyer les addresses correspondante
 */
void answer_to_server(get_t get_rcvd,
        struct sockaddr_storage * client_sockaddr,
        socklen_t * client_addrlen)
{
    uint16_t nb_addr = count_addr(get_address_list(
                get_rcvd.hash,hash_table));

    response_t * response_snd = malloc(sizeof(response_t)+
            sizeof(struct in6_addr)*nb_addr);

    int i;

    response_snd->header = SHARE_DATA_HEADER;
    for(i=0;i<HASH_LENGTH;i++)
        response_snd->hash[i] = get_rcvd.hash[i];

    address * addr = get_address_list(response_snd->hash,hash_table);

    while(addr != NULL)
    {
        fflush(NULL);
        fflush(NULL);
        VERB(printf("envoi\n"));
        fflush(NULL);
        for(i=0;i<ADDR_LENGTH;i++)
        {
            response_snd->addr.s6_addr[i]=addr->val.s6_addr[i];
        }
        responde_to(my_sockfd,client_sockaddr,client_addrlen,
            (msg_t*) response_snd);
        addr=addr->next_address;
    }

    free(response_snd);
}




/**
 * \fn void share_all_servers(int my_sockfd,struct in6_addr addr,
 * in_port_t port)
 * \brief envoyer sa liste de server connu à un autre server
 */
void share_all_servers(int my_sockfd,struct in6_addr addr, in_port_t port)
{
    if(memcmp(&addr,&my_addr,sizeof(struct in6_addr))==0
            && port==my_port)
    {
        return;
    }
    connected_server * tmp = server_table;
    share_server_t msg;
    while(tmp!=NULL)
    {
        msg.header = SHARE_SERVER_HEADER;
        msg.addr = tmp->addr;
        msg.port = tmp->port;
        send_to_server(my_sockfd,addr,port,(msg_t*) &msg,
                sizeof(share_server_t));
        tmp=tmp->next;
    }
}

/**
 * \fn void share_all_hashs(int my_sockfd,struct in6_addr addr,
 * in_port_t port, avl hash_node)
 * \brief envoyer sa liste de hash connu à un autre server, dont l'adresse et
 * le port sont donénes en parametres
 */
void share_all_hashs(int my_sockfd,struct in6_addr addr, in_port_t port,
        avl hash_node)
{
    if(hash_node==NULL)
    {
        return;
    }
    if(memcmp(&addr,&my_addr,sizeof(struct in6_addr))==0
            && port==my_port)
    {
        return;
    }
    share_data_t msg;
    msg.header = SHARE_DATA_HEADER;
    int i;
    for(i=0;i<HASH_LENGTH;i++)
        msg.hash[i] = hash_node->hash[i];
    address * tmp = hash_node->address_list;

    //parcourir la liste des adresses associés au hash courant
    while(tmp!=NULL)
    {
        msg.addr = tmp->val;
        //l'envoyer au server
        send_to_server(my_sockfd,addr,port,(msg_t*) &msg,sizeof(share_data_t));
        tmp = tmp->next_address;
    }

    //repeter recursivement cette fonction pour envoyer toutes les adresses
    //de tout les hashs de la table
    
    share_all_hashs(my_sockfd,addr,port,left_sub_avl(hash_node));

    share_all_hashs(my_sockfd,addr,port,right_sub_avl(hash_node));
}

//thread numero 2 qui va verifier toutes les 20 secondes, si les données de la 
//table dont obsoletes, si les servers connectés donnent toujours des signes
//de vie, et envoi toute les 10 secondes et messages "keep_alive" a tous les
//autres servers connectés
void * fthread(void __attribute__((unused)) * arg)
{
    //iitialisation des signaux pour le thread
    init_thread_signal();

    struct in6_addr client_addr;
    in_port_t client_port;

    keep_t msg_keep;
    msg_keep.header = KEEP_HEADER;
    msg_keep.addr = my_addr;
    msg_keep.port = my_port;

    connected_server * server;

    char * char_addr;

    while(true)
    {

        //envoi des keep alive à tous les servers connectés
        PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
        server = server_table;
        while(server!=NULL)
        {
            char_addr = in6_addr_to_char_addr(server->addr);
            VERB(printf("send keep_alive to addr %s\n", char_addr));
            free(char_addr);
            VERB(printf("port %d\n", server->port));

            client_addr = server->addr;
            client_port = server->port;
            send_to_server(my_sockfd,client_addr,client_port,
                    (msg_t*) &msg_keep,sizeof(keep_t));
            server = server->next;
        }
        PRIM(sem_post(&sem_server),"sem_post(sem_server)");

        //supprimer tous les servers morts (ce qu'in n'ont pas renvoyé de
        //keep_alive depuis plus de 30 secondes
        VERB(printf("suppression des servers morts\n"));

        PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
        server_table=suppression_dead_server(server_table);
        PRIM(sem_post(&sem_server),"sem_post(sem_server)");

        //suprimer les hash obsoletes, (qui n'ont pas recu de put depuis plus
        //de 30 secondes
        VERB(printf("suppression des hashs obsoletes\n"));

        PRIM(sem_wait(&sem_hash),"sem_wait(sem_hash)");
        hash_table = suppression_obsolete_hash(hash_table);
        PRIM(sem_post(&sem_hash),"sem_post(sem_hash)");

        sleep(5);
    }

    VERB(printf("fin thread boucle\n"));

    pthread_exit(EXIT_SUCCESS);
}




/********************************main********************************/

int main(int argc, char ** argv)
{
    init_signal();

    //verification des options utilisées, -v
    int c_option;
    while( (c_option = getopt(argc, argv, "v")) != EOF )
    {
        switch(c_option)
        {
            case 'v':
                verbose = true;
                break;
            case '?':
                break;
            default:
                break;
        }
    }

    //lecture des arguments
    if( argc-optind+1 < 3 || argc-optind+1 == 4 || argc-optind+1 >5 )
        exit_usage_server();


    char *  my_addr_name = argv[1+optind-1];
    
    char * my_char_port = argv[2+optind-1];
    my_port = atoi(my_char_port);

    //obtenir les info sur son adresse (utile si l'on utilise un nom de domaine
    //ou localhost pour identifier son adresse
    struct addrinfo * server_info = get_address_info(my_addr_name,my_char_port);
    if(server_info==NULL)
        exit_usage_server();
    char my_ip[256];
    getnameinfo(server_info->ai_addr, server_info->ai_addrlen, my_ip,
            sizeof(my_ip), NULL, 0, NI_NUMERICHOST);
    free(server_info);

    struct in6_addr * tmp_my_addr = char_to_in6_addr(my_ip);
    my_addr = * tmp_my_addr;
    free(tmp_my_addr);

    my_sockfd = open_my_socket(my_port, my_addr);

    msg_t * msg_rcvd = NULL;


    char * char_addr = in6_addr_to_char_addr(my_addr);
    VERB(printf("mon addresse est : %s\n", char_addr));
    VERB(printf("mon port est : %d\n", my_port));

    //initialisation des semaphores
    PRIM(sem_init(&sem_server,1,1),"sem_init(sem_server)");
    PRIM(sem_init(&sem_hash,1,1),"sem_init(sem_hash)");

    //s'il y a 5 arguments c'est que l'on souhaite que le server se connecte
    //un a autre sever
    if(argc-optind+1 == 5)
    {
        char * connected_server_ip = argv[3+optind-1];
        char * connected_server_port = argv[4+optind-1];
        if(strtol(connected_server_port,NULL,10)==0)
            exit_usage_server();

        //obtenir les infos sur le server auquel se conencter
        server_info = get_address_info(connected_server_ip,
                connected_server_port);
        if(server_info==NULL)
            exit_usage_server();
        char ip_server[256];
        getnameinfo(server_info->ai_addr, server_info->ai_addrlen, ip_server,
                sizeof(ip_server),NULL, 0, NI_NUMERICHOST);

        free(server_info);
        struct in6_addr * tmp_server_ip = char_to_in6_addr(ip_server);
        struct in6_addr server_ip = * tmp_server_ip;
        free(tmp_server_ip);
        in_port_t server_port = strtol(connected_server_port,NULL,10);
        char * char_other_addr = in6_addr_to_char_addr(server_ip);

        VERB(printf("demande de connection a l'adresse %s\n", char_other_addr));
        VERB(printf("port %d\n", server_port));

        connect_t msg_connect;
        msg_connect.header = CONNECT_HEADER;
        msg_connect.addr = my_addr;
        msg_connect.port = my_port;

        //envoyer un message connect a ce server
        send_to_server(my_sockfd,server_ip,server_port,
                (msg_t *) &msg_connect,sizeof(connect_t));
        free(char_other_addr);
    }

    //stockage des sockaddr des clients et server lors de la receptiond d'un
    //message,
    //utilisé pour pour pouvoir répondre au client
    struct sockaddr_storage * client_sockaddr =
        malloc(sizeof(struct sockaddr_storage));
    socklen_t * client_addrlen = malloc(sizeof(socklen_t));
    struct in6_addr client_addr;
    in_port_t client_port;


    /**********mise en place du nouveau thread:**********/
    //nouveau thread : boucle jusqu'a l'arret du server, envoi des keep_alive,
    //vérifi l'obsolescence des donnée, et des servers.

    pthread_t * tid = malloc(sizeof(pthread_t));
    THREAD(pthread_create(tid,NULL,fthread,NULL),"pthread_create");


    //thread courant: repond aux requetes


    msg_rcvd=receive_m(my_sockfd,&client_sockaddr,&client_addrlen);

    //boucle d'actions et de réponse aux requetes de clients et de servers
    while(!end_signal)
    {
        //si le message recu est un message put 
        if(msg_rcvd->header==PUT_HEADER)
        {
            put_t * put_rcvd = (put_t*) msg_rcvd;

            VERB(print_put_reception(put_rcvd->hash, put_rcvd->addr));

            //insertion de la nouvelle donnée dans la table ou mise a jour de
            //la date de dernier put si ell existe deja
            PRIM(sem_wait(&sem_hash),"sem_wait(sem_hash)");

            hash_table = insert_hash_addr_put(
                    put_rcvd->hash,put_rcvd->addr,hash_table);
            PRIM(sem_post(&sem_hash),"sem_post(sem_hash)");


            PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
            PRIM(sem_wait(&sem_hash),"sem_wait(sem_hash)");

            share_put_t msg_snd;
            msg_snd.header = SHARE_PUT_HEADER;
            int i;
            for(i=0;i<HASH_LENGTH;i++)
                msg_snd.hash[i]=put_rcvd->hash[i];
            msg_snd.addr=put_rcvd->addr;

            //partager cette donnée recu avec tous les autres server
            connected_server * server = server_table;
            while(server!=NULL)
            {
                VERB(printf("send share put data\n"));
                
                client_addr = server->addr;
                client_port = server->port;
                send_to_server(my_sockfd,client_addr,client_port,
                        (msg_t*) &msg_snd,sizeof(share_put_t));
                server = server->next;
            }

            PRIM(sem_post(&sem_hash),"sem_post(sem_hash)");
            PRIM(sem_post(&sem_server),"sem_post(sem_server)");

        }
        else if(msg_rcvd->header==GET_HEADER)
        {
            get_t * get_rcvd = (get_t *) msg_rcvd;

            VERB(printf("reception get\n"));
            
            //repondre a la requete get d'un client et lui envoyer des adresses
            PRIM(sem_wait(&sem_hash),"sem_wait(sem_hash)");
            answer_to_client(*get_rcvd,client_sockaddr,client_addrlen);
            PRIM(sem_post(&sem_hash),"sem_post(sem_hash)");
        }
        else if(msg_rcvd->header==SERVER_GET_HEADER)
        {
            get_t * get_rcvd = (get_t *) msg_rcvd;

            VERB(printf("reception get\n"));
            
            //repondre a la requete get d'un server et lui envoyer des adresse
            PRIM(sem_wait(&sem_hash),"sem_wait(sem_hash)");
            answer_to_server(*get_rcvd,client_sockaddr,client_addrlen);
            PRIM(sem_post(&sem_hash),"sem_post(sem_hash)");
        }
        else if(msg_rcvd->header==CONNECT_HEADER)
        {
            //message d'un server qui tente de se connecter
            connect_t * connect_rcvd = (connect_t*) msg_rcvd;

            VERB(printf("reception connect\n"));

            if(memcmp(&connect_rcvd->addr,&my_addr,sizeof(struct in6_addr))==0
                    && connect_rcvd->port==my_port)
            {
                VERB(printf("auto_connect\n"));
            }
            else
            {
                PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                bool connue = is_known(connect_rcvd->addr,
                        connect_rcvd->port,server_table);
                PRIM(sem_post(&sem_server),"sem_post(sem_server)");

                if(!connue)
                {
                    // si le server est inconnu
                    VERB(print_connection(connect_rcvd->addr,
                                connect_rcvd->port));

                    //on lui partage notre liste de server connecté pour qu'il
                    //puisse s'y connecter
                    PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                    share_all_servers(my_sockfd,connect_rcvd->addr,
                            connect_rcvd->port);
                    PRIM(sem_post(&sem_server),"sem_post(sem_server)");

                    VERB(printf("insertion du nouveau server dans le base\n"));

                    //on l'ajoute dans notre table des servers connectés
                    PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                    server_table = insert_server(server_table,
                            connect_rcvd->addr,connect_rcvd->port);
                    PRIM(sem_post(&sem_server),"sem_post(sem_server)");

                    VERB(printf("se connecter à ce nouveau server\n"));

                    //lui repondre en lui envoyant aussi un message connect
                    connect_t msg_connect;
                    msg_connect.header = CONNECT_HEADER;
                    msg_connect.addr = my_addr;
                    msg_connect.port = my_port;
                    responde_to(my_sockfd,client_sockaddr,client_addrlen,
                            (msg_t*) &msg_connect);

                    VERB(printf("lui partager les hash\n"));

                    //lui partager tous les hashs ainsi que les adresses
                    //associées que l'on a  dans la base
                    PRIM(sem_wait(&sem_hash),"sem_wait(sem_hash)");
                    share_all_hashs(my_sockfd,connect_rcvd->addr,
                            connect_rcvd->port,hash_table);
                    PRIM(sem_post(&sem_hash),"sem_post(sem_hash)");
                }
                else
                {
                    VERB(printf("server déja connecté\n"));
                }
            }
        }
        else if(msg_rcvd->header==KEEP_HEADER)
        {
            keep_t * keep_rcvd = (keep_t*) msg_rcvd;

            VERB(print_keep_recv(keep_rcvd->addr,keep_rcvd->port));
            
            if(memcmp(&keep_rcvd->addr,&my_addr,sizeof(struct in6_addr))==0
                    && keep_rcvd->port==my_port)
            {
                fprintf(stderr,"self keep\n");
            }
            else
            {
                //mettre a jour la date de dernier keep_alive du server
                //connecté qui nous envoi ce message
                PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                keep_alive_server(server_table,keep_rcvd->addr,keep_rcvd->port);
                PRIM(sem_post(&sem_server),"sem_post(sem_server)");
            }
        }
        else if(msg_rcvd->header==SHARE_DATA_HEADER)
        {
            share_data_t * share_data_rcvd = (share_data_t *) msg_rcvd;

            VERB(print_share_data_rcv(share_data_rcvd->hash,
                        share_data_rcvd->addr));

            //inserer les données recu dans la table sans mettre a jour la date
            //de last put si la donnée estd éja stocké
            PRIM(sem_wait(&sem_hash),"sem_wait(sem_hash)");
            hash_table = insert_hash_addr_share(
                    share_data_rcvd->hash,share_data_rcvd->addr,
                    hash_table);
            PRIM(sem_post(&sem_hash),"sem_post(sem_hash)");

        }
        else if(msg_rcvd->header==SHARE_PUT_HEADER)
        {
            share_put_t * share_put_rcvd = (share_put_t *) msg_rcvd;

            VERB(print_share_put_rcv(share_put_rcvd->hash,
                        share_put_rcvd->addr));

            //inserer elas donées recu dans la table, en mettant a jour la date
            //de last put si la données est deja dans la table
            PRIM(sem_wait(&sem_hash),"sem_wait(sem_hash)");
            hash_table = insert_hash_addr_put(
                    share_put_rcvd->hash,share_put_rcvd->addr,
                    hash_table);
            PRIM(sem_post(&sem_hash),"sem_post(sem_hash)");

        }
        else if(msg_rcvd->header==SHARE_SERVER_HEADER)
        {
            share_server_t * share_rcvd = (share_server_t *) msg_rcvd;

            //recevoir des coordonnées d'un server auquel se conecter

            VERB(printf("reception share server\n"));

            PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
            bool connue = is_known(share_rcvd->addr,
                    share_rcvd->port,server_table);
            PRIM(sem_post(&sem_server),"sem_post(sem_server)");

            if(!connue)
            {
                VERB(printf("insertion du server dans la base\n"));

                //ajouter le server dans la tabel des severs
                PRIM(sem_wait(&sem_server),"sem_wait(sem_server)");
                server_table = insert_server(server_table,share_rcvd->addr,
                        share_rcvd->port);
                PRIM(sem_post(&sem_server),"sem_post(sem_server)");

                //lui envoyer un message pour se connecter
                connect_t msg_connect;
                msg_connect.header = CONNECT_HEADER;
                msg_connect.addr = my_addr;
                msg_connect.port = my_port;
                send_to_server(my_sockfd,share_rcvd->addr,share_rcvd->port,
                        (msg_t*) &msg_connect,sizeof(connect_t));
            }
        }
        else if(msg_rcvd->header==EMPTY_HEADER)
        {
        }

        free(msg_rcvd);

        msg_rcvd=receive_m(my_sockfd,&client_sockaddr,&client_addrlen);
    }

    //fin de l'execution de la boucle si le processus recoit le signal SIGINT

    //envoi du signal SIGUSR1 au 2eme thread pour lui dire de s'arreter
    THREAD(pthread_kill(*tid,SIGUSR1),"pthread_kill");;
    THREAD(pthread_join(*tid,NULL),"pthread_join");

    //liberer toute la mémoire encore alloué
    free(client_addrlen);
    free(client_sockaddr);
    free_avl(hash_table);
    free_server_tab(server_table);
    free(tid);
    free(msg_rcvd);;
    free(char_addr);

    PRIM(sem_destroy(&sem_server),"sem_destroy(sem_server)");
    PRIM(sem_destroy(&sem_hash),"sem_destroy(sem_hash)");

    printf("\n");
    VERB(printf("arrêt du server\n"));

    exit(EXIT_SUCCESS);
}
