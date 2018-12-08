#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>

#define BUFFER_SIZE 256
#define SHM_SIZE sizeof(char)*((N+2)*(N+2))
#define N 8

char *adresse_shm;	//adresse shm

int cree_socket_joueur(char *adrip, int port)
{
	int socket_joueur;
	struct sockaddr_in adresse;
	
	if ((socket_joueur = socket(AF_INET, SOCK_STREAM, 0)) < 0) {		// IP et TCP
		perror("Joueur : cree_socket_joueur : Erreur socket \n");
		exit(0);
	}
	
	memset(&adresse, 0, sizeof(struct sockaddr_in));
	
	adresse.sin_family = AF_INET; 		// Protocol IP
	adresse.sin_port = htons(port);		// port 33016
	inet_aton(adrip, &adresse.sin_addr);	// IP localhost
	
	
	if (connect(socket_joueur, (struct sockaddr*) &adresse, sizeof(struct sockaddr_in)) < 0)
	{
		close(socket_joueur);
		perror("Joueur : cree_socket_joueur : Erreur connect \n");
		exit(0);
	}
	return socket_joueur;
}

int affiche_adresse_socket(int sock)
{
	struct sockaddr_in adresse;
	socklen_t longueur;
	
	longueur = sizeof(struct sockaddr_in);
	
	if (getsockname(sock, (struct sockaddr*)&adresse, &longueur) < 0)
	{
		perror("Joueur : affiche_adresse_socket : Erreur getsockname \n");
		exit(0);
	}
	printf("IP joueur = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
	
	return 0;
}

int connexion(int socket_serveur){
	socklen_t lg;
	int socket_joueur;
	struct sockaddr_in adresse;
	
	lg = sizeof(struct sockaddr_in);
	socket_joueur = accept(socket_serveur, (struct sockaddr*)&adresse, &lg);
	
	if (socket_joueur < 0){
		perror("Joueur : connexion : Erreur accept\n");
		exit(0);
	}
	
	return socket_joueur;
}

void affiche_adresse_distante(int sock)
{
	struct sockaddr_in adresse;
	socklen_t longueur;
	
	longueur = sizeof(struct sockaddr_in);
	
	if (getpeername(sock, (struct sockaddr*)&adresse, &longueur) < 0)
	{
		perror("Joueur : affiche_adresse_distante : Erreur getpeername \n");
		exit(0);
	}
	printf("IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
	
	return;
}

int init_shm_joueur(){
	
	int clef, id;
	
	//clé SHM
	clef = IPC_PRIVATE;
	if(clef == -1){
	 	perror("Serveur : init_shm : Erreur génération clef ");
	 	exit(0);
	 }
	 //tentative d'accès en création
	id = shmget(clef,sizeof(char),IPC_CREAT| IPC_EXCL | 0666);
	while(id==-1){
		printf("ID shm = -1\n");
		if(errno==EEXIST){
			//la mémoire partagée existe déjà on la ferme 
			id = shmget(clef,sizeof(char),IPC_CREAT| 0666);
			printf("ID Clef : %d\n",id);
			shmctl(id, IPC_RMID, NULL);
			//puis on en recréer un nouveau
			id = shmget(clef,sizeof(char),IPC_CREAT| IPC_EXCL | 0666);
			printf("ID Clef : %d\n",id);
		}
		else{
			perror("Serveur : Echec de la création du segment de mémoire patgée ");
			exit(0);
		}
	}
	// Attachement du segment mémoire
	adresse_shm = shmat(id, NULL, 0);
	
	if(adresse_shm==(void*)-1){
		perror("Serveur : inti_shm : Impossible d'attacher le segment mémoire ");
		exit(0);
	}
	
	//'o' ok 'e' fin du jeu
	strcpy(adresse_shm,"o");
	
	return clef;
}

char affiche_map(int sock){

	int i;
	struct sockaddr_in adresse;
	socklen_t lg;
	int nb;
	char bufferR[BUFFER_SIZE];
	char map[SHM_SIZE];
    print("fonction affiche map");
	//copymap
	lg = sizeof(struct sockaddr_in);
	
	if (getpeername(sock, (struct sockaddr*) &adresse, &lg) < 0) {
		perror("Joueur : affiche_map : Erreur getpeername\n");
		exit(0);
	}

	read(sock, bufferR, BUFFER_SIZE);
	if(bufferR[0]==' '){
		printf("%s\n",map);
		return 'e';
		exit(0);
	}
	
	
	else{
		for(i=0;i<SHM_SIZE;i++){
			map[i]=bufferR[i+1];
		}
		//affichemap
		for(i=0;i<SHM_SIZE;i++){
			printf("%c", map[i]);
			if(((i+1)%(N+2))==0){
				printf("\n");
			}
		}
		printf("\n");
		return 'o';
	}

}

char lire_commande(clef){
	char* c;
	
	if((shmget(clef,sizeof(char),0444))<0){
		perror("Serveur : init_nouveau_j : shmget ");
		exit(1);
	}
	strcpy(c, adresse_shm);

	return c[0];
}

char commande(int clef,int socket_joueur){

	struct sockaddr_in adresse;
	socklen_t lg;
	char c;
	int nb;
	int id;
	char bufferW[BUFFER_SIZE];
	char bufferSHM[BUFFER_SIZE];
		
	//récupérer la saisie clavier
	system ("/bin/stty raw");
        c = getchar();        
        system ("/bin/stty cooked");
        
        //copier dans shm
        if((shmget(clef,sizeof(char),0444))<0){
		perror("Joueur : commande : shmget ");
		exit(1);
	}
	bufferSHM[0]=c;
	strcpy(adresse_shm, bufferSHM);
	
        //envoie au serveur
	lg = sizeof(struct sockaddr_in);
	
	if (getpeername(socket_joueur, (struct sockaddr*) &adresse, &lg) < 0) {
		perror("Joueur : commande : Erreur getpeername\n");
		return 'e';
	}
	bufferW[0]=c;
   	write(socket_joueur, bufferW, BUFFER_SIZE);
	printf("\n");
	// Le joueur quitte le jeu
	if(c=='e'){
		//attend la fin du fils pour quitter
		wait(NULL);
		//ferme socket_joueur
		close(socket_joueur);
		//ferme SHM_joueur
		if(shmdt(adresse_shm)==-1){
			perror("Joueur : commande : Erreur shmdt ");
		}
		printf("Fin du jeu\n");
	}
	
	
	return c;
}

int main(int argc, char *argv[]){

	int socket_joueur;
	int clef;
	int i ;
	char c,e; // si c = 'e' fin du programme
	pid_t pid_client;
	
	// creation socket de contact:
	socket_joueur = cree_socket_joueur(argv[1], atoi(argv[2]));
	
	// affichage adresse locale:
	affiche_adresse_socket(socket_joueur);
	
	printf(" *** Bienvenue ! ***\n\n");
	
	clef = init_shm_joueur();
	
	pid_client = fork();
			
	if (pid_client==-1 ){
		perror("Joueur : main : Erreur fork \n");
		return -1;
	}
	
	if(pid_client == 0){
		while((c!='e')&&(e!='e')){
			printf("\e[1;1H\e[2J \n nouvel affichage");
			e=affiche_map(socket_joueur);
			printf("q -> gauche\nd -> droite\nz -> haut\ns -> bas\ne -> exit\n");
			c=lire_commande(clef);
			}
	}
	else{
	
	while(c!='e'){
			c=commande(clef,socket_joueur);
		}
	}

	return 0;

}
