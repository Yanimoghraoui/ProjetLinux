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



int cree_socket_tcp_ip_serveur()
{
   int socket_serveur;
   struct sockaddr_in serveur_adresse;
   if ((socket_serveur = socket(PF_INET, SOCK_STREAM, 0)) < 0) {   // IP et TCP
      fprintf(stderr, "Erreur socket.");
      return -1; 
   }
   memset(&serveur_adresse, 0, sizeof(struct sockaddr_in));      // initialisation à 0
   serveur_adresse.sin_family = PF_INET;                         // Protocol IP
   serveur_adresse.sin_port = htons(33016);                      // port 33016
   inet_aton("127.0.0.1", &serveur_adresse.sin_addr);            // IP localhost
   if (bind(socket_serveur, (struct sockaddr*) &serveur_adresse, sizeof(struct sockaddr_in)) < 0)
   {
      close(socket_serveur);
      fprintf(stderr, "Erreur bind");
      return -1;
   }
  return socket_serveur;
}

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


int connexion (int socket_contact){
    int sock_connectee;
    struct sockaddr_in adress;
    socklen_t lg;
    listen(socket_contact, 1);
    lg = sizeof(struct sockaddr_in);
    sock_connectee = accept(socket_contact, (struct sockaddr*)&adress, &lg);
    if (sock_connectee < 0){
        fprintf(stderr, "erreur accept ");
        return -1;
    }

    close(socket_contact);
    return sock_connectee;
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



void traite_connection(int sock)
{
  struct sockaddr_in serveur_adresse;
  socklen_t  lg;
  char bufferR[BUFFER_SIZE];
  char bufferW[BUFFER_SIZE];
  int nb;
  lg = sizeof(struct sockaddr_in);
  if (getpeername(sock, (struct sockaddr*) &serveur_adresse, &lg) < 0) {
    fprintf(stderr, "Erreur getpeername");
    return;
  }
  printf("Connexion : locale (sock_connectee)");    
  affiche_adresse_distant(sock);
  strcpy(bufferW, "Veuillez entrer une phrase : ");
  write(sock, bufferW, strlen(bufferW)+1);
  nb= read(sock, bufferR, BUFFER_SIZE);
  bufferR[nb-2] = '\0';
  printf("L’utilisateur distant a tapé: %s", bufferR);
  sprintf(bufferW, "Merci!");
  write(sock, bufferR, BUFFER_SIZE);
}

/*
void traitement( int socket_connectee ) {
char buffer[BUFFER_SIZE];
int nb;
// envoi du message d'accueil:
strcpy( buffer, "Bonjour, je suis la machine locale." );
write( socket_connectee, buffer, strlen(buffer)+1 );
// reception de la reponse de la machine distante:
nb= read(socket_connectee, buffer, BUFFER_SIZE);
buffer[nb-2] = '\0';
// affichage:
printf( "Texte recu de la machine distante: %s\n", buffer );
}
*/

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


/*
int main(void)
{
	printf("Bonjour Je suis le serveur!\n");
    int socket_serveur;// creation socket de serveur pour les connections: 
    int socket_client;  //pour relier la machine distante à une socket "client"

    printf("Au revoir!\n");        // termine:
	
  int sock_contact; int sock_connectee;
  int socket_serveur;// creation socket de serveur pour les connections: 
  int socket_client;  //pour relier la machine distante à une socket "client"
  struct sockaddr_in serveur_adresse;
  socklen_t lg;
  pid_t pid_fils;
  if  (sock_contact = cree_socket_tcp_ip_serveur() < 0)
    return -1;
  listen(sock_contact, 10);
  while (1) {
      lg = sizeof(struct sockaddr_in);
      sock_connectee = accept(sock_contact,(struct sockaddr*)&serveur_adresse,&lg);
      if (sock_connectee < 0) {
        fprintf(stderr, "Erreur accept\n");
        return -1;
      }
      if ( (pid_fils = fork())==-1) {
        fprintf(stderr, "Erreur fork\n");
        return -1;
      }
      if (pid_fils == 0)  {// fils 
        close(sock_contact);
        traite_connection(sock_connectee);
        exit(0);
      } else              // pere 
        close(sock_connectee);
    }
    
return 0; 
}
*/



//Code main du serveur de base sans boucle while

int main() {
    printf("Bonjour Je suis le client!\n");
    int socket_serveur;// creation socket de serveur pour les connections: 
    int socket_client;  //pour relier la machine distante à une socket "client"
    socket_serveur = cree_socket_tcp_ip_serveur(); 
    affiche_adresse_socket(socket_serveur);// affichage adresse socket serveur locale:
    socket_client = connexion( socket_serveur ); // accepte co sur serveur et liaison à socket "client"
    traite_connection(socket_client);
	close(socket_serveur);      // on continue avec la socket connectee:  
    affiche_adresse_distant(socket_client);		
    communication(socket_client); 
    printf("Au revoir!\n");        // termine:
}
