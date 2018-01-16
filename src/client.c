/**
 * \file client.c
 * \author Lucas Schott et Juan Manuel Torres Garcia
 * \brief fichier principale contenant le main du programme client
 */

#include "client_server.h"


//variable indiquant si l'option verbose a été activé lors du lancement du
//programme ou non
bool verbose = false;


/**
 * usage du programme client:
 *
 * ./client <adresse du server> <no de port du server> put <hash a envoyer>
 * <adresse associé>
 *
 * ./cleint <adresse du server> <no de port du server> get <hash>
 */
void exit_usage_client(void)
{
    fprintf(stderr,"usage: client <adresse_server> <port_server>\
<commande> <hash> [<ip>]\n");
    exit(EXIT_FAILURE);
}



/**************************PUT************************************************/



/**
 * \fn put_t write_put_request(uint16_t * hash, struct in6_addr hash_addr)
 * \brief ecrire un message "put" contenant le hash et l'adresse donnés
 * en parametre
 */
put_t write_put_request(uint16_t * hash, struct in6_addr hash_addr)
{
    int i;

    put_t msg;
    msg.header = PUT_HEADER;
    for(i=0;i<HASH_LENGTH;i++)
        msg.hash[i]=hash[i];
    msg.addr = hash_addr;
    return msg;
}



/**
 * \fn void put(int sockfd, struct addrinfo * server_info, uint16_t * h,
 * struct in6_addr a)
 * \brief envoyer une message put a partir de sa socket, a l'adresse
 * server_info et contenant les hash "h" et l'adresse "a"
 */
void put(int sockfd, struct addrinfo * server_info, uint16_t * h,
        struct in6_addr a)
{
    put_t msg = write_put_request(h,a);

    if (sendto(sockfd, &msg,sizeof(put_t), 0,
                server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("sendto");
        close(sockfd);;
        exit(EXIT_FAILURE);
    }
}



/************************GET**************************************************/


/**
 * \fn get_t write_get_request(uint16_t * hash)
 * \brief ecrire un message "get" contenant le hash donné en parametres
 */
get_t write_get_request(uint16_t * hash)
{
    int i;
    get_t msg;
    msg.header = GET_HEADER;
    for(i=0;i<HASH_LENGTH;i++)
        msg.hash[i]=hash[i];
    return msg;
}

/**
 * \fn void get(int sockfd, struct addrinfo * server_info, uint16_t * h)
 * struct in6_addr a)
 * \brief envoyer une message "get" a partir de sa socket, a l'adresse
 * server_info et contenant le hash "h" puis attendre la réponse
 * (max 1 seconde) et afficher le résultat
 */
void get(int sockfd, struct addrinfo * server_info, uint16_t * h)
{
    bool rcvd = false;
    //send msg

    get_t msg = write_get_request(h);

    if (sendto(sockfd, &msg, sizeof(get_t), 0,
                server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        perror("sendto");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    //receive answer

    struct timeval time_val;
    fd_set read_set;

    FD_ZERO(&read_set);
    FD_SET(sockfd, &read_set);
    time_val.tv_sec = 1;
    time_val.tv_usec = 0;

    response_t * answer = malloc(sizeof(response_t));

    VERB(printf("IP disponibles pour le hash demandé :\n"));
    //attendre une seconde la reception d'un message
    //si au bout de une seconde on ne recoit plus de message, on arrete d'en
    //attendre
    while (select(sockfd+1, &read_set, NULL, NULL, &time_val))
    {
        //tant qu'on recoit des messages on affiche le resultat
        if (recv(sockfd, answer, MSG_MAX_LENGTH, 0) == -1)
        {
            perror("recv:");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        rcvd = true;
        print_address(answer->addr);
    }
    if(!rcvd)
        //si on n'a recu aucun message on le signale
        VERB(printf("aucune adresse disponible\n"));
}


int main(int argc, char ** argv)
{

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

    if( argc-optind+1 < 5 || argc-optind+1 > 6)
        exit_usage_client();

    //récuperation des arguments du programme

    char * server_ip = argv[1+optind-1];
    char *  server_port = argv[2+optind-1];
    char * cmd = argv[3+optind-1];
    uint16_t * hash = char_to_uint16_t_hash(argv[4+optind-1]);
    if(hash==NULL)
        exit_usage_client();
    struct in6_addr * hash_ip;
    if(strncmp(cmd,"put",3)==0)
    {
        if(argv[5+optind-1]==NULL)
            exit_usage_client();
        hash_ip = char_to_in6_addr(argv[5+optind-1]);
        if(hash_ip==NULL)
            exit_usage_client();
    }


    int sockfd;
    int status;
    bool sock_success = false;
    struct addrinfo hints, *server, *server_info;
    memset(&hints, 0, sizeof hints);

    //utilisation d'IPv6 et UDP
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;

    //recuperation des informations sur le server
    if ((status = getaddrinfo(server_ip, server_port,
                    &hints, &server)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    //ouverture d'une socket pour communiquer avec le server
    server_info = server;
    while((server_info != NULL) && !sock_success)
    {
        if ((sockfd = socket(server_info->ai_family, server_info->ai_socktype,
                        server_info->ai_protocol)) == -1)
        {
            perror("socket:");
            sock_success = false;
            server_info = server_info->ai_next;
        }
        else
            sock_success = true;
    }
    if (server_info == NULL)
    {
        fputs("Création de socket impossible", stderr);
        return 2;
    }

    /******programme*****/


    if(strncmp(cmd,"put",3)==0)
    {
        //envoi du message put
        put(sockfd, server_info, hash, *hash_ip);
    }
    else if(strncmp(cmd,"get",3)==0)
    {
        //envoi du message get, attente de réponse, et affichage de résultat
        get(sockfd, server_info, hash);
        //envoi du message get, attente de réponse, et affichage de résultat
    }

    free(hash_ip);
    freeaddrinfo(server);
    close(sockfd);
    exit(EXIT_SUCCESS);
}
