main_client : bin/main_client.o bin/messages_client.o bin/client.o bin/interaction.o bin/lecture.o
	gcc bin/main_client.o bin/client.o bin/messages_client.o bin/interaction.o bin/lecture.o -o main_client 
bin/main_client.o : client/main_client.c client/client.h 
	gcc -c client/main_client.c -o bin/main_client.o
bin/client.o : client/client.c client/client.h client/messages_client.h
	gcc -c client/client.c -o bin/client.o
bin/interaction.o : client/interaction.c client/client.h client/interaction.h
	gcc -c client/interaction.c -o bin/interaction.o
bin/messages_client.o : client/messages_client.c client/messages_client.h
	gcc -c client/messages_client.c -o bin/messages_client.o

main_serveur : bin/main_serveur.o bin/messages_serveur.o bin/serveur.o bin/inscrits.o bin/fils.o bin/lecture.o
	gcc bin/main_serveur.o bin/messages_serveur.o bin/serveur.o bin/inscrits.o bin/fils.o bin/lecture.o -pthread -o main_serveur
bin/main_serveur.o : serveur/main_serveur.c serveur/serveur.h lecture.h 
	gcc -c serveur/main_serveur.c -o bin/main_serveur.o
bin/serveur.o : serveur/serveur.c serveur/serveur.h lecture.h
	gcc -c serveur/serveur.c -o bin/serveur.o
bin/messages_serveur.o : serveur/messages_serveur.c serveur/messages_serveur.h
	gcc -c serveur/messages_serveur.c -o bin/messages_serveur.o
bin/fils.o : serveur/fils.c serveur/fils.h serveur/messages_serveur.h
	gcc -c serveur/fils.c -o bin/fils.o
bin/inscrits.o : serveur/inscrits.c serveur/inscrits.h
	gcc -c serveur/inscrits.c -o bin/inscrits.o

bin/lecture.o : lecture.c lecture.h 
	gcc -c lecture.c -o bin/lecture.o

clean : 
	rm -rf  main_client main_serveur bin/*.o
