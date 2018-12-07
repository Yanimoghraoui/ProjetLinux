#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BUFFER_SIZE 256

	//socket client
int cree_socket_tcp_client()
{
  struct sockaddr_in serveur_adresse;
  int socket_client;
  char bufferR[BUFFER_SIZE];
  char bufferW[BUFFER_SIZE];
  int nb;
  if ((socket_client = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "Erreur socket \n");
    return -1; 
  }
  memset(&serveur_adresse, 0, sizeof(struct sockaddr_in));
  serveur_adresse.sin_family = AF_INET;
  serveur_adresse.sin_port = htons(33016);	// quelle port en deuxième argument ?
  inet_aton("127.0.0.1", &serveur_adresse.sin_addr);	// quelle adresse en premier argument ?
  if (connect(socket_client, (struct sockaddr*) &serveur_adresse, sizeof(serveur_adresse)) < 0)
  {
    close(socket_client);
    fprintf(stderr, "Erreur connect \n");
    return -1;
  }

  nb= read(socket_client, bufferR, BUFFER_SIZE);
  printf("%s \n",bufferR);
 
  return socket_client;
}


	//Affiche
int affiche_adresse_socket(int sock)
{
  struct sockaddr_in adresse;
  socklen_t longueur;
  longueur = sizeof(struct sockaddr_in);
  if (getsockname(sock, (struct sockaddr*)&adresse, &longueur) < 0)
  {
     fprintf(stderr, "Erreur getsockname");
     return -1;
  }
  printf("IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
  return 0;
}

	

int affiche_adresse_distant(int sock)
{
  struct sockaddr_in adresse;
  socklen_t longueur;
  longueur = sizeof(struct sockaddr_in);
  if (getpeername(sock, (struct sockaddr*)&adresse, &longueur) < 0)
  {
     fprintf(stderr, "Erreur getpeername");
     return -1;
  }
  printf("IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
  return 0;
}

void communication( int sock){
  struct sockaddr_in adress;
  socklen_t lg;
  char bufferR[BUFFER_SIZE];
  char bufferW[BUFFER_SIZE];
  int nb;
  lg = sizeof(struct sockaddr_in);
  if (getpeername(sock, (struct sockaddr*) &adress, &lg) < 0) {
    fprintf(stderr, "Erreur getpeername");
    return;
  }
  printf("Connexion : locale (sock_connectee)");
  affiche_adresse_socket(sock);
  strcpy(bufferW, "Veuillez entrer une phrase : ");
  write(sock, bufferW, strlen(bufferW)+1);
  nb= read(sock, bufferR, BUFFER_SIZE);
  bufferR[nb-2] = '\0';
  printf("L’utilisateur distant a tapé: %s", bufferR);
  sprintf(bufferW, "Merci!");
  write(sock, bufferR, BUFFER_SIZE);

}

int main() {
    printf("Bonjour Je suis le client!\n");
    int socket_client;// creation socket locale client pour la connection machine distante: 
    socket_client = cree_socket_tcp_client(); 
    affiche_adresse_socket(socket_client);// affichage adresse socket serveur locale:	
    printf("Au revoir!\n");        // termine:
}

