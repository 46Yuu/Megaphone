#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "inscrits.h"



inscrits_t * creer_inscrits_t (){
    inscrits_t * res =(inscrits_t *) malloc(sizeof(inscrits_t));
    if(res==NULL){
        perror("malloc");
        return NULL;
    }
    memset(res,0,sizeof(inscrits_t));
    res->capacite = LONG_INIT;
    res->inscrits = (inscrit_t*)malloc(LONG_INIT * (sizeof(inscrit_t))); 
    if(res->inscrits==NULL){
        perror("malloc");
        free(res);
        return NULL;
    }
    return res;
}

int add_user(inscrits_t * ins,char * pseudo){
    if(ins->nb_inscrits == 2047){
        fprintf(stderr,"nombre max d'inscrits atteint");
        return 0;
    }
    //vérifier si le tableau est rempli
    if(ins->capacite == ins->nb_inscrits){
        inscrit_t * tmp = (inscrit_t *)realloc(ins->inscrits, 2*ins->capacite);
        if(tmp==NULL){
            perror("realloc");
            return -1;
        }
        ins->inscrits = tmp;
        ins->capacite*=2;
    }
    inscrit_t client;
    client.id = ins->nb_inscrits+1;
    memmove(client.pseudo, pseudo, LEN_PSEUDO * sizeof(char));
    client.pseudo[LEN_PSEUDO] = '\0';

    ins->inscrits[ins->nb_inscrits] = client;
    ins->nb_inscrits+=1;

    return client.id;
}


int est_inscrit(inscrits_t * ins, u_int16_t id_client, char * pseu){
    for(int i=0; i<ins->nb_inscrits; i++){
        if(ins->inscrits[i].id == id_client){
            memmove(pseu, ins->inscrits[i].pseudo, LEN_PSEUDO);
            pseu[LEN_PSEUDO]='\0';
            return TRUE;
        }
    }
    return FALSE;
}

void free_list_inscrits(inscrits_t * ins){
    free(ins->inscrits);
    free(ins);
}