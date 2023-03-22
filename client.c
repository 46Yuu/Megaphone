#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "client.h"
#include "messages.h"

void test(){
    printf("TEST\n");
}

void affiche_adresse(struct sockaddr_in6 *adr){
    char adr_buf[INET6_ADDRSTRLEN];
    memset(adr_buf, 0, sizeof(adr_buf));
    
    inet_ntop(AF_INET6, &(adr->sin6_addr), adr_buf, sizeof(adr_buf));
    printf("adresse serveur : IP: %s port: %d\n", adr_buf, ntohs(adr->sin6_port));
}

int get_server_addr(char* hostname, char* port, int * sock, struct sockaddr_in6** addr, int* addrlen) {
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

  if (NULL == p) return -2;

  //on stocke l'adresse de connexion
  *addr = (struct sockaddr_in6 *) p->ai_addr;

  //on libère la mémoire allouée par getaddrinfo 
  freeaddrinfo(r);
    
  return 0;
}

int demande_inscription( int fd_sock, char * pseudo){
  
  char * mess_inscr = message_inscription_client(pseudo);

  if(send(fd_sock, mess_inscr,LEN_MESS_INSCR,0) != LEN_MESS_INSCR){
    return -1;
  } 
  free(mess_inscr);
 
  //réponse du serveur
  u_int16_t rep[3];
  if(recv(fd_sock,rep, sizeof(rep) ,0) != sizeof(rep)){
    return -1;
  }

  uint16_t id = reponse_inscription(rep);
  return id;
}

int demande_dernier_billets(int fd_sock,u_int16_t id_client,uint16_t numfil, uint16_t nb){
  printf("Demande derniers billets\n");

  char * mess_dernier_billets = message_dernier_billets(id_client,numfil,nb);
  printf("Avant send\n");
  if(send(fd_sock, mess_dernier_billets,LEN_MESS_DMD_BILLETS,0) != LEN_MESS_DMD_BILLETS){
    free(mess_dernier_billets);
    return -1;
  } 
  printf("Apres send\n");

  free(mess_dernier_billets);
 
  //réponse du serveur
  u_int16_t rep[3];
  int taille = 0;
  if((taille = recv(fd_sock,rep, sizeof(rep) ,0)) != sizeof(rep)){
    printf("Taille est %d\n",taille);
    // printf("rep est est %s\n",rep);
    return -1;
  }

  printf("La taille est %d\n",taille);
  return 1;

}

int demande_dernier_billets_tous_les_fils(int fd_sock,u_int16_t id_client){
  uint16_t numfil = 0;
  uint16_t nb = 4; // Pour l'instant je teste avec la demande des 4 derniers billets de tous les fils
  int result = demande_dernier_billets(fd_sock,id_client,numfil,nb);
  return result;

}