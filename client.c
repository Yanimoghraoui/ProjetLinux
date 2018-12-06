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

int connexion(int socket_contact) {
    struct sockaddr_in adresse;
    int socket_connectee;
    socklen_t longueur;
    socket_connectee = accept(socket_contact,
    (struct sockaddr*)&adresse,&longueur);
    if (socket_connectee < 0){
        perror( "accept" );
        close(socket_contact);
        exit(11);
    }
    return socket_connectee;
}


int affiche_adresse_socket(int sock){
    struct sockaddr_in adresse;
    socklen_t longueur;
    longueur = sizeof(struct sockaddr_in);
    if (getsockname(sock, (struct sockaddr*)&adresse, &longueur) < 0){
        close(sock);
        perror("Erreur getsockname");
        exit(3);
    }
    printf("IP = %s, Port = %u\n", inet_ntoa(adresse.sin_addr), ntohs(adresse.sin_port));
    return 0;
}


int main()
{
  if(cree_socket_tcp_client() == -1) 
  {
	fprintf(stderr, "Erreur création client \n");
	return -1; 
  }

}
  %contour associe chaque point à une valeur 
