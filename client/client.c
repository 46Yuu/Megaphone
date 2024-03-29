#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>

#include "client.h"
#include "messages_client.h"
#include "../lecture.h"
#include "../fichiers.h"
#include "reponses_serveur.h"
#include "abonnement.h"

void affiche_adresse(struct sockaddr_in6 *adr){
    char adr_buf[INET6_ADDRSTRLEN];
    memset(adr_buf, 0, sizeof(adr_buf));

    inet_ntop(AF_INET6, &(adr->sin6_addr), adr_buf, sizeof(adr_buf));
    printf("adresse serveur : IP: %s port: %d\n", adr_buf, ntohs(adr->sin6_port));
}

int get_server_addr(char* hostname, char* port, int * sock, struct sockaddr_in6* addr, int* addrlen) {
    struct addrinfo hints, *r, *p;
    int ret;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED | AI_ALL;

    if ((ret = getaddrinfo(hostname, port, &hints, &r)) != 0 || NULL == r){
        fprintf(stderr, "erreur getaddrinfo : %s\n", gai_strerror(ret));
        return -1;
    }
  
    *addrlen = sizeof(struct sockaddr_in6);
    p = r;
    while( p != NULL ){
        if((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) > 0){
            if(connect(*sock, p->ai_addr, *addrlen) == 0)
                break;
            close(*sock);
        }
        p = p->ai_next;
    }
    if (NULL == p) 
        return -2;

    *addr = *((struct sockaddr_in6 *)p->ai_addr);
    freeaddrinfo(r);
    return 0;
}

int socket_udp_telechargement(int* sock_udp, int* port){
    *sock_udp = socket(PF_INET6, SOCK_DGRAM, 0);
    if (*sock_udp < 0) {
        return 0;
    }
    struct sockaddr_in6 servadrudp;
    memset(&servadrudp, 0, sizeof(servadrudp));
    servadrudp.sin6_family = AF_INET6;
    servadrudp.sin6_addr = in6addr_any;
    //le système choisira un port
    servadrudp.sin6_port = htons(0);
    if (bind(*sock_udp, (struct sockaddr *)&servadrudp, sizeof(servadrudp)) < 0) {
        return 0;
    }
    //obtenir le port choisi
    socklen_t addrlen = sizeof(servadrudp);
    if(getsockname(*sock_udp,(struct sockaddr*)&servadrudp,&addrlen)==-1){
        return 0;
    }
    *port = ntohs(servadrudp.sin6_port);
    return 1;
}

int get_data(char *data,int taille,int sock){
    buf_t *buf = creer_buf_t(taille);
    if (buf == NULL){
        perror("erreur creation buf");
        return 0;
    }
    int r =read_msg(sock, buf);
    if (r == 0){
        perror("erreur de read");
        free(buf);
        return 0;
    }
    memmove(data, buf->buf, taille);
    free_buf(buf);
    return 1;
}


int demande_dernier_billets(int sock,u_int16_t id_client,uint16_t numfil, uint16_t nb){
    char * mess_dernier_billets = message_dernier_billets(id_client,numfil,nb);
    if(send(sock, mess_dernier_billets,LEN_MESS_CLIENT,0) != LEN_MESS_CLIENT){
        free(mess_dernier_billets);
        return 0;
    } 
    free(mess_dernier_billets);

    //réponse du serveur
    char rep[NB_OCTECS_MESSAGE_SERVEUR];
    memset(rep,0,NB_OCTECS_MESSAGE_SERVEUR);
    if (!get_data(rep,NB_OCTECS_MESSAGE_SERVEUR,sock))
        return 0;

    uint16_t nbb = reponse_derniers_billets(rep);
    printf("Il ya exactement %u messages qui arrivent\n",nbb);

    for(int i = 0; i < nbb; i++){
        char data[NB_OCTECS_DERNIERS_MESSAGE_JUSQUA_DATALEN];
        memset(data,0,NB_OCTECS_DERNIERS_MESSAGE_JUSQUA_DATALEN);
        if (!get_data(data,NB_OCTECS_DERNIERS_MESSAGE_JUSQUA_DATALEN,sock))
            return 0;
        uint8_t datalen = reponse_derniers_billets_datalen(data);

        char message[datalen+1];
        memset(message,0,datalen+1);
        if (!get_data(message,datalen,sock))
            return 0;
        printf("Message %d : %s\n",i+1,message);
        printf("-------------------------------\n");
    }
    return 1;
}

int sabonner_au_fil(struct in6_addr addr_diffus, uint16_t port){
    info_abonn* infos = (info_abonn*)malloc(sizeof(info_abonn));
    if(!infos){
        perror("malloc");
        return 0;
    }
    infos->addr_diffus = addr_diffus;
    infos->port = port;

    pthread_t thread;
    if (pthread_create(&thread, NULL, abonnement, infos) == -1) {
        perror("pthread_create");
        free(infos);
        return 0;
    }  
    return 1;
}

int demande_abonnement(int sock,u_int16_t id_client,uint16_t numfil){
    char * mess_abonnement_fil = message_abonnement_fil(id_client,numfil);
    if(send(sock, mess_abonnement_fil,LEN_MESS_CLIENT,0) != LEN_MESS_CLIENT){
        free(mess_abonnement_fil);
        return 0;
    } 
    free(mess_abonnement_fil);
    char data[NB_OCTECS_REPONSES_ABONNEMENT];
    memset(data,0,NB_OCTECS_REPONSES_ABONNEMENT);
    if (!get_data(data,NB_OCTECS_REPONSES_ABONNEMENT,sock))
        return 0;
    struct in6_addr addr_diffus;
    uint16_t port;
    if(!reponse_abonnement(data, &addr_diffus, &port, id_client, numfil))
        return 0;
    return sabonner_au_fil(addr_diffus, port);
}


uint16_t demande_inscription(int fd_sock, char * pseudo){
    char * mess_inscr = message_inscription_client(pseudo);

    if(send(fd_sock, mess_inscr,LEN_MESS_INSCR,0) != LEN_MESS_INSCR){
        return 0;
    } 
    free(mess_inscr);

    //réponse du serveur
    char rep[NB_OCTECS_MESSAGE_SERVEUR];
    memset(rep,0,NB_OCTECS_MESSAGE_SERVEUR);
    if (!get_data(rep,NB_OCTECS_MESSAGE_SERVEUR,fd_sock))
        return 0;
    uint16_t id = reponse_inscription(rep);
    return id;
}



uint16_t poster_un_billet(int sock,uint16_t id, uint16_t num_fil, uint8_t datalen, char * data){
    char * mess = message_client(CODE_REQ_P0STER_BILLET,id,num_fil,0,datalen,data);

    if(send(sock,mess,7+datalen,0)!=7+datalen){
        free(mess);
        return 0;
    }
    free(mess);

    //réponse du serveur
    char rep[NB_OCTECS_MESSAGE_SERVEUR];
    memset(rep,0,NB_OCTECS_MESSAGE_SERVEUR);
    if (!get_data(rep,NB_OCTECS_MESSAGE_SERVEUR,sock))
        return 0;
    return reponse_poster_billet(rep);
}

uint16_t ajouter_un_fichier(int sock, uint16_t id, uint16_t num_fil, uint8_t taille_nom_fichier, char * nom_fichier,char * hostname, char * file_path){
    char * mess_demande_ajout_fichier = message_client(CODE_REQ_AJOUT_FICHIER,id,num_fil,0,taille_nom_fichier,nom_fichier);
    if(!mess_demande_ajout_fichier){
        perror("mess_demande_ajout_fichier");
        return 0;
    }
    if(send(sock,mess_demande_ajout_fichier,LEN_MESS_CLIENT+taille_nom_fichier,0) != LEN_MESS_CLIENT+taille_nom_fichier){
        free(mess_demande_ajout_fichier);
        return 0;
    }
    free(mess_demande_ajout_fichier);
    char data[NB_OCTECS_MESSAGE_SERVEUR];
    memset(data,0,NB_OCTECS_MESSAGE_SERVEUR);
    if (!get_data(data,NB_OCTECS_MESSAGE_SERVEUR,sock))
        return 0;
    uint16_t port = reponse_ajout_fichier(data,id);
    if(!port)
        return 0;
    close(sock);
    struct in6_addr addr_inutile;
    if(!envoi_fichier(id,port,file_path,hostname,1,addr_inutile)){
        return 0;
    }
    return 1;
}

uint16_t telecharger_un_fichier(int sock, uint16_t id, uint16_t num_fil, uint8_t taille_nom_fichier, char * nom_fichier){
    int port_udp;
    int * sock_udp = (int*)malloc(sizeof(int));
    if(!sock_udp){
        perror("malloc");
        return 0;
    }
    if(!socket_udp_telechargement(sock_udp,&port_udp))
        return 0;
    char * mess_telechargement_fichier = message_client(CODE_REQ_TELECHARGER_FICHIER,id,num_fil,port_udp,taille_nom_fichier,nom_fichier);
    if(send(sock,mess_telechargement_fichier,LEN_MESS_CLIENT+taille_nom_fichier,0) != LEN_MESS_CLIENT+taille_nom_fichier){
        free(mess_telechargement_fichier);
        return 0;
    }
    char data[NB_OCTECS_MESSAGE_SERVEUR];
    memset(data,0,NB_OCTECS_MESSAGE_SERVEUR);
    if (!get_data(data,NB_OCTECS_MESSAGE_SERVEUR,sock))
        return 0;
    if(!reponse_telechargement_fichier(data,id,num_fil,port_udp))
        return 0;
    close(sock);
    if(reception_par_paquets_de_512(*sock_udp, nom_fichier, id, num_fil,1) <= 0)
        return 0;
    return 1;
}

