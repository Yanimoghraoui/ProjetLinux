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


int affiche_serveur_adresse_socket(int sock)
{
  struct sockaddr_in serveur_adresse;
  socklen_t longueur;
  longueur = sizeof(struct sockaddr_in);
  if (getsockname(sock, (struct sockaddr*)&serveur_adresse, &longueur) < 0)
  {
     fprintf(stderr, "Erreur getsockname");
     return -1;
  }
  printf("IP = %s, Port = %u\n", 
inet_ntoa(serveur_adresse.sin_addr), 
ntohs(serveur_adresse.sin_port));
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
  affiche_serveur_adresse_socket(sock);
  strcpy(bufferW, "Veuillez entrer une phrase : ");
  write(sock, bufferW, strlen(bufferW)+1);
  nb= read(sock, bufferR, BUFFER_SIZE);
  bufferR[nb-2] = '\0';
  printf("L’utilisateur distant a tapé: %s", bufferR);
  sprintf(bufferW, "Merci!");
  write(sock, bufferR, BUFFER_SIZE);
}

int main(void)
{
  int sock_contact; int sock_connectee;
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
      if (pid_fils == 0)  {/* fils */
        close(sock_contact);
        traite_connection(sock_connectee);
        exit(0);
      } else              /* pere */
        close(sock_connectee);
    }
    
return 0; 
}
