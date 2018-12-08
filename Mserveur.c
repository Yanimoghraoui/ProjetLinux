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
#define JMAX 2

//variables globles
char *adresse_shm;	//adresse shm

int cree_socket_serveur(){

	/* 
	Creation du processus serveur
	*/
	
	socklen_t lg;			//taille socket
	int socket_serveur;		//numéro de socket
	struct sockaddr_in adresse;	//adresse de socket
	
	// Creation socket serveur
	if ((socket_serveur = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Serveur : cree_socket_serveur : Erreur socket ");
		exit(0);
	}
	//..
	memset(&adresse, 0, sizeof(struct sockaddr_in));
	adresse.sin_family = PF_INET; 			// Protocol IP
	adresse.sin_port = htons(33016);		// port 33016
	inet_aton("127.0.0.1", &adresse.sin_addr);	// IP localhost
	//..
	if (bind(socket_serveur, (struct sockaddr*) &adresse, sizeof(struct sockaddr_in)) < 0){
		close(socket_serveur);
		perror("Serveur : cree_socket_serveur : Erreur bind ");
		exit(0);
	}
	// Affichage console
	lg = sizeof(struct sockaddr_in);
	if (getsockname(socket_serveur,(struct sockaddr*)&adresse, &lg) == -1){
		perror("Serveur : cree_socket_serveur : Erreur getsockname ");
		exit(0);
	}
	printf("Serveur : IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));

	return socket_serveur;
	
}

int init(){

	/* 
	Initialise le plateau de jeu zone grise et zone libre
	Crée le segment de mémoire partagée pour stocker le plateau
	OUTPUT : int clef : la clef pour permettre dans la suite du programme
	d’accéder et de modifier la mémoire partagée 
	*/
	
	int i;			//incrément
	char map[SHM_SIZE];	//plateau de jeu
	int p; 			//nb aléatoire en 0 et 100
	int pos_O;		//'O'
	int clef;		//clé d’accès shm
	int id;			//id shmget
	
	/* Initialisation MAP */
	srand(time(NULL));
	//'X'
	for(i=0;i<SHM_SIZE;i++){
		// Bord haut + gauche + droit + bas ‘X’
		if( (i<N+2)||(i%(N+2)==0)||((i+1)%(N+2)==0)||(i>SHM_SIZE-(N+3)) ){
			map[i]='X';
		}
		// Intérieur du plateau 30% ‘X’ else ‘ ‘
		else{
			p=rand()%(100);
			if(p<=30){
				map[i]='X';
			}
			else{
				map[i]=' ';
			}
		}
	}
	//'O'
	// Cherche une place
	do{
		pos_O=rand()%((N+2)*(N+2));
	}while(map[pos_O]!=' ');
	// PLace 'O'
	map[pos_O]='O';
	
	/* Initialisation SHM */
	// Creation clef SHM entre processus père-fils (IPC_PRIVATE)
	clef = IPC_PRIVATE;
	if(clef == -1){
	 	perror("Serveur : init_shm : Erreur génération clef ");
	 	exit(0);
	 }
	// Creation SHM
	id = shmget(clef,SHM_SIZE,IPC_CREAT| IPC_EXCL | 0666);
	while(id==-1){
		printf("ID shm = -1\n");
		if(errno==EEXIST){
			//la mémoire partagée existe déjà on la ferme 
			id = shmget(clef,SHM_SIZE,IPC_CREAT| 0666);
			printf("ID Clef : %d\n",id);
			shmctl(id, IPC_RMID, NULL);
			//puis on en recréer un nouveau
			id = shmget(clef,SHM_SIZE,IPC_CREAT| IPC_EXCL | 0666);
			printf("ID Clef : %d\n",id);
		}
		else{
			perror("Serveur : Echec de la création du segment de mémoire patgée ");
			exit(0);
		}
	}
	// Attachement du segment mémoire adresse_shm variable globale du programme
	adresse_shm = shmat(id, NULL, 0);
	
	if(adresse_shm==(void*)-1){
		perror("Serveur : inti_shm : Impossible d'attacher le segment mémoire ");
		exit(0);
	}
	
	// Ecriture du plateau dans shm
	strcpy(adresse_shm, map);
	
	return clef;
}

int connexion(int socket_serveur){

	/*
	Acceptation de nouvelle connexion sur le serveur
	*/
	
	socklen_t lg;			//taille socket
	int socket_joueur;		//numéro de socket
	struct sockaddr_in adresse;	//adresse de socket
	
	lg = sizeof(struct sockaddr_in);
	
	socket_joueur = accept(socket_serveur, (struct sockaddr*)&adresse, &lg);
	if (socket_joueur < 0){
		perror("Serveur : connexion : Erreur accept ");
		exit(0);
	}
	
	return socket_joueur;
}

void affiche_IP_joueur(int socket_joueur){

	socklen_t lg;			//taille socket
	struct sockaddr_in adresse;	//adresse de socket
	
	lg = sizeof(struct sockaddr_in);
	
	if (getpeername(socket_joueur, (struct sockaddr*)&adresse, &lg) < 0){
		perror("Serveur : affiche_IP_joueur : Erreur getpeername ");
		exit(0);
	}
	printf("IP joueur = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
}

int nouveau_j( int clef,int joueur){
	
	/*
	Place aléatoirement le joueur sur le plateau
	OUTPUT : position du joeur
	*/
	
	int pos_init;		//position du joueur au moment de son apparition sur le plateau
	
	srand(time(NULL));
	// Accès mémoire partagée	
	if((shmget(clef,SHM_SIZE,0444))==-1){
		perror("Serveur : init_nouveau_j : shmget ");
		exit(0);
	}
	
	// Cherche une place pour le joueur sur le plateau
	/* On vérifie que la position n'est pas déjà occupé */
	do{
		pos_init=rand()%((N+2)*(N+2));
	}while(adresse_shm[pos_init]!=' ');
	// Place le joueur sur le plateau
	adresse_shm[pos_init]=joueur +'0';
	
	return pos_init;
}

void envoyer_carte( int clef, int socket_joueur){
		
	/*
	Recupere le plateau dans la mémoire partagée
	Envoie le plateau au joueur
	*/
	
	int i;				//incrément
	char Buffer_SHM[BUFFER_SIZE-1];	//pour stocker la map 
	char Buffer_SOCK[BUFFER_SIZE];	//pour écrire dans la socket
	struct sockaddr_in adresse;	//adresse de socket
	socklen_t lg;			//taille socket		
	
	/* Récupère la plateau dans SHM */
	// Accès SHM
	if((shmget(clef,SHM_SIZE,0444))<0){
		perror("Serveur : envoyer_carte : shmget ");
		exit(0);
	}
	// Copie le contenu de la SHM
	strcpy(Buffer_SHM, adresse_shm);
	
	for(i=1;i<BUFFER_SIZE;i++){
	/* On commence à i=1 car BUFFER_SOCK[0] réservé pour la lecture de la commande 
	dans la fonction traitement_commande */
		Buffer_SOCK[i]=Buffer_SHM[i-1];
	}
	
	/* Envoie du plateau */
	lg = sizeof(struct sockaddr_in);
	// Acces socket_joueur
	if (getpeername(socket_joueur, (struct sockaddr*) &adresse, &lg) < 0) {
		perror("Serveur : envoyer_carte : Erreur getpeername ");
		exit(0);
	}
	// Ecriture dans socket_joueur
	write(socket_joueur, Buffer_SOCK, BUFFER_SIZE);
}

void nouveau_O(char* Buffer_SHM){

	/*
	Place un nouveau 'O' sur le plateau de jeu 
	*/
	
	int position;
	
	srand(time(NULL));
	// Cherche nouvelle place pour 'O'
	do{
		position=rand()%((N+2)*(N+2));
	}while(Buffer_SHM[position]!=' ');
	// Place 'O'
	Buffer_SHM[position]='O';
}

int deplacement(int position, char commande){
	
	int nouvelle_position;
	
	switch (commande) {
		case 'e' : //ESC
		nouvelle_position=-1;
		break;
		case 'q': //gauche			
		nouvelle_position=position-1;
		break;
		case 's': //bas
		nouvelle_position=position+N+2;
		break;
		case 'd': //droite
		nouvelle_position=position+1;
		break;
		case 'z': //haut
		nouvelle_position=position-N-2;
		break;
	}
	
	return nouvelle_position;
}

void traitement_commande(int clef, int socket_joueur,int *joueur){

	/*
	Lit la commande envoyé depuis socket_joeur
	En fonction de la commande
		- Déplace le joueur
		- Quitte le jeu
	OUTPUT : int position
		nouvelle position du joueur après le déplacement
		si -1 le joueur quitte le jeu
	*/

	struct sockaddr_in adresse;		//adresse socket
	socklen_t lg;				//taille socket
	char commande;				//commande
	char Buffer_SOCK[BUFFER_SIZE];		//pour lire socket_joueur
	char Buffer_SHM[BUFFER_SIZE];		//pour lire et ecrire shm
	int id_joueur = joueur[0];
	int position = joueur[1];
	int score = joueur[2];
	int nouvelle_position = position;
	
	/* Récupère la commande de socket joueur */
	lg = sizeof(struct sockaddr_in);
	// Accès socket_joueur
	if (getpeername(socket_joueur, (struct sockaddr*) &adresse, &lg) < 0) {
		perror("Serveur : traitement_commande : Erreur getpeername ");
		exit(0);
	}
	// Lit la commande envoyée par socket_joueur
	read(socket_joueur, Buffer_SOCK , BUFFER_SIZE);
	commande=Buffer_SOCK[0];
	
	/* Récupère le plateau dans shm */
	// Accès shm
	if((shmget(clef,SHM_SIZE,0444))<0){
		perror("Serveur : traitement_commande : shmget\n");
		exit(0);
	}
	// Copie la map
	strcpy(Buffer_SHM, adresse_shm);
	
	/* Gestion de la commande */
	// Clear ancienne position
	Buffer_SHM[position]=' ';
	
	//calcule la nouvelle position
	nouvelle_position = deplacement(position,commande);
	// Si le joueur ne quitte pas le jeu
	if(nouvelle_position != -1){
		// Verifie que le déplacement est possible
		// Si ' ' le joueur se déplace
		if(Buffer_SHM[nouvelle_position]==' '){
			Buffer_SHM[nouvelle_position]=joueur[0] + '0';
		}
		// Si 'O' le joueur gagne un point
		else if(Buffer_SHM[nouvelle_position]=='O'){
			score ++;
			Buffer_SHM[nouvelle_position]=joueur[0] + '0';
			nouveau_O(Buffer_SHM);
		}
		//Si 'X' ou autre joueur le joueur perd
		else{
			printf("J%d PERDU\n",joueur[0]);
			nouvelle_position=-1;
		}
	}
	// Mise à jour de la map
	//recopier la nouvelle map dans shm
	strcpy(adresse_shm,Buffer_SHM);
	// Si le joueur quitte la partie ou a perdu
	if(nouvelle_position == -1){
		printf("J%d quitte la partie, score : %d\n",joueur[0],score);
	}
	
	joueur[1]=nouvelle_position;
	joueur[2]=score;
	
}

void message_perdu(int socket_joueur, int score){

	/*
	Envoie un message avec le score au joueur qui perd/quitte le jeu
	*/
	
	int i=0;			//incrément 
	char Buffer_SOCK[BUFFER_SIZE];	//pour écrire dans la socket
	struct sockaddr_in adresse;	//adresse de socket
	socklen_t lg;			//taille socket	
	char* message;
	
	//Message
	sprintf(message," Au revoir SCORE : %d", score);
	while(message[i]!='\0'){
		Buffer_SOCK[i]=message[i];
		i++;
	}
	
	lg = sizeof(struct sockaddr_in);
	// Acces socket_joueur
	if (getpeername(socket_joueur, (struct sockaddr*) &adresse, &lg) < 0) {
		perror("Serveur : envoyer_carte : Erreur getpeername ");
		exit(0);
	}
	// Ecriture dans socket_joueur
	write(socket_joueur, Buffer_SOCK, BUFFER_SIZE);
}


int main(){
	
	int clef;
	int socket_serveur;
	int socket_joueur;
	pid_t pid; 
	int nb_joueur=0;
	int joueur[3];
	char commande;
	
	// creation socket serveur:
	socket_serveur = cree_socket_serveur();
	
	//initialisation de la carte
	clef=init();

	printf(" ********************* \n ** NOUVELLE PARTIE **\n *********************\n\n");
	listen(socket_serveur, JMAX);
	
	while(1){
		
		// acceptation de connexion:
		socket_joueur = connexion( socket_serveur );
		nb_joueur++;
		
		// affichage machine distante:
		printf("joueur %d a rejoint la partie!\n", nb_joueur);
		affiche_IP_joueur(socket_joueur);
		pid = fork();
			
		if (pid==-1){
			perror("Serveur : main : Erreur fork \n");
			exit(0);
		}
		
		//fils -> on joue
		else if (pid==0){ 
			close(socket_serveur);
			joueur[0]=nb_joueur;
			joueur[1]=nouveau_j(clef,joueur[0]);
			joueur[2]=0;
			while(joueur[1]!=-1){
				envoyer_carte(clef, socket_joueur);
				traitement_commande(clef,socket_joueur,joueur);
			}
			message_perdu(socket_joueur,joueur[2]);
			close(socket_joueur);
			exit(0);
		}
		//père -> on attend d'autres connections
		else { 
			close(socket_joueur);
		}

	}
	return 0;
}
