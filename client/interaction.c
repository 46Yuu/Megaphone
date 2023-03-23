#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "client.h"
#include "interaction.h"



int inscrire_client(int sock){
    char pseudo[11];
    memset(pseudo, '#', 10);
    printf("Entrez votre pseudo (entre 1 et 10 caractères :\n");
    scanf("%s", pseudo);
    pseudo[10] = '\0';

    uint16_t id = demande_inscription(sock, pseudo);
    if(id){
        printf("inscription réussie. ID attribué : %d\n", id);
        return 1;
    }
    printf("Echec, le serveur n'a pas pu vous inscrire\n");
    return 0;
}

int poster_billet(int sock){
    int id;
    int numfil;
    uint8_t datalen;
    char texte[256]={0};
    printf("Entrez votre id :\n");
    scanf("%d", &id);
    printf("Entrez le numéro de fil ou 0 pour poster le billet sur un nouveau fil :\n");
    scanf("%d", &numfil);
    printf("Entrez le texte du billet :\n");
    scanf("%s", texte);
    datalen = strlen(texte);
    
    uint16_t res = poster_un_billet(sock, id, numfil, datalen, texte);
    if(res==0){
        fprintf(stderr,"Le billet n'est pas posté\n");
        return 0;
    }
    printf("Le billet est posté sur le fil numéro %u\n", res);
    return 1;
}

int demader_billets(int sock){
    return 0;
}

int choix_client(int sock){
  char offre_inscription[] = "Tapez 1 pour s'inscrire auprès du serveur\n";
  char offre_poster_billet[] = "Tapez 2 pour poster un billet sur un fil\n"; 
  char offre_demande_billets[] = "Tapez 3 pour demander la liste des derniers billets sur un fil\n";
  printf("%s%s%s", offre_inscription, offre_poster_billet, offre_demande_billets);
  int rep_client;
  scanf("%d", &rep_client);
  switch (rep_client){
    case 1:
      return inscrire_client(sock);
    case 2 :
      return poster_billet(sock);
    case 3 : 
      return demader_billets(sock);
    default:
      fprintf(stderr,"Opération pas prise en charge\n");
      break;
  }
}