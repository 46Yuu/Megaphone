#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


#include "inscrits.h"
#include "fils.h"
#include "serveur.h"
#include "messages_serveur.h"
#include "../lecture.h"

inscrits_t * inscrits;
fils_t * fils;

void *serve(void *arg) {
    int sock = *((int *) arg);
    u_int16_t entete =lire_entete(sock);
    if(entete == 0){
        perror("erreur réception du message");
        return NULL;
    }

    int *ret = (int *)malloc(sizeof(int));
    int rep=0;
    /*recupération du codereq et verification du type de demande*/
    switch(get_code_req(entete)){
        /*demande d'inscription*/
        case 1: 
            rep = inscrire_client(sock, inscrits);
            break;
        /*poster un billet*/
        case 2:
            rep = poster_un_billet(sock,inscrits,fils, get_id_requete(entete));
            break;
        case 3 :
            rep = 0;
            break;
    }
    if(rep){//succès
        *ret = 1;
        close(sock);
        pthread_exit(ret);
    }
    //envoyer le message d'erreur
    envoie_message_erreur(sock);
    *ret=0;
    close(sock);
    pthread_exit(ret);
}

int main(int argc, char *argv[]){
    /*Creation liste d'inscrits*/
    inscrits = creer_inscrits_t();
    /*Création liste de fils*/
    fils = creer_list_fils();

    //*** creation de l'adresse du destinataire (serveur) ***
    struct sockaddr_in6 address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(atoi(argv[1]));
    address_sock.sin6_addr = in6addr_any;

    //*** creation de la socket ***
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if(sock < 0){
        perror("creation socket");
        exit(1);
    }
    int optval = 0;
    int r = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
    if (r < 0) 
        perror("erreur connexion IPv4 impossible");

    //*** le numero de port peut etre utilise en parallele ***
    optval = 1;
    r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (r < 0) 
        perror("erreur réutilisation de port impossible");

    //*** on lie la socket au port ***
    r = bind(sock, (struct sockaddr *) &address_sock, sizeof(address_sock));
    if (r < 0) {
        perror("erreur bind");
        exit(2);
    }

    //*** Le serveur est pret a ecouter les connexions sur le port ***
    r = listen(sock, 0);
    if (r < 0) {
        perror("erreur listen");
        exit(2);
    }
    
    while(1){
        struct sockaddr_in6 addrclient;
        socklen_t size=sizeof(addrclient);
        
        //*** on crée la varaiable sur le tas ***
        int *sock_client = (int*)malloc(sizeof(int));

        //*** le serveur accepte une connexion et initialise la socket de communication avec le client ***
        *sock_client = accept(sock, (struct sockaddr *) &addrclient, &size);

        if (*sock_client >= 0) {
            pthread_t thread;
            //*** le serveur cree un thread et passe un pointeur sur socket client à la fonction serve ***
            if (pthread_create(&thread, NULL, serve, sock_client) == -1) {
                perror("pthread_create");
                continue;
            }  
            //*** affichage de l'adresse du client ***
            char nom_dst[INET6_ADDRSTRLEN];
            printf("client connecte : %s %d\n", inet_ntop(AF_INET6,&addrclient.sin6_addr,nom_dst,sizeof(nom_dst)), htons(addrclient.sin6_port));
        }
    }
    //*** fermeture socket serveur ***
    close(sock);
    return 0;
}  