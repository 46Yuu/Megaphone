#!/bin/bash

# inscription: 1,identifiant
# envoie: 2,identifiant,numfil,data
# recenvoir: 3,identifiant,numfil,nb_billet

echo "2\n3\n0\nmessage 1 de client 3\n" | ../main_client localhost 7777 
printf "2\n3\n0\nmessage 2 de client 3\n" | ../main_client localhost 7777 
printf "2\n3\n0\nmessage 3 de client 3\n" | ../main_client localhost 7777 
printf "2\n3\n0\nmessage 4 de client 3\n" | ../main_client localhost 7777 
printf "2\n3\n1\nmessage 5 de client 3\n" | ../main_client localhost 7777 
printf "3\n3\n0\n4\n" | ../main_client localhost 7777 > client1.txt
printf "3\n3\n0\n4\n" | ../main_client localhost 7777 > client1.txt
printf "3\n3\n0\n4\n" | ../main_client localhost 7777 > client1.txt
printf "3\n3\n0\n4\n" | ../main_client localhost 7777 > client1.txt