#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>

/* syqe iki
    spiegazione creazione socket:
    
    1. Creare un socket usando la funzione socket(), questo ritorna un fd.
        i parametri passati alla funzione sono:
           I. Domain, quindi la famiglia di indirizzi ( in questo caso ipv4 ) AF_INET.
                se invece si voleva ipv6 allora era AF_INET6

          II. Type, in questo caso TCP ( SOCK_STREAM ), questo perche' crea un flusso stabile
                e ordinato. Se avessimo usato SOCK_DGRAM sarebbe stato UDP, questo protocollo
                non da garanzia di ordine/consegna e sarebbe stato un macello
            
         III. Protocol, in questo caso 0 cosi il kernel sceglie quello che vuole ( ovviamente TCP )

    2. Preparare la struttura dell'indirizzo
        Prima di passare a bind() i parametri, serve una struttura definita che specifica quale
        indirizzo e porta deve ascoltare

        I.  sin_family = AF_INET ( deve essere uguale a quella del socket )

       II.  sin_port = la porta sulla quale deve ascoltare, si usa htons perche' deve essere convertita in byte

      III.  sin_addr.s_addr = quale interfaccia deve usare, si usa INADDR_ANY ( 0.0.0.0 ) cosi da avere tutte le connessioni.
                                nel caso si potrebbe usare una a piacere, ma poi sarebbe un macello

    3. Creare il bind()
        Una volta impostati tutti i parametri possiamo passarli a bind(). bind() si occupa di legare indirizzo/porta specifici
        cosi' che il kernel sa dove far arrivare i pacchetti

        I. serverSocket, che sarebbe fd creato con socket()

       II. serverAddress, che sarebbe il puntatore alla struttura creata prima .
            Va castata a struct sockaddr* perché è l'interfaccia generica che il kernel si aspetta

      III.  la dimensione della struttura, cosi il kernel sa quanti byte leggere

    4. impostare un listen()
        semplicemente prende il socket creato e quante connessioni il kernel puo' mettere in coda.
        ATTENZIONE: non sarebbe il numero massimo di client, quello deve essere gestito dopo con poll()

    ps.
        ho messo uno sleep() per testare la connessione aprendo un altro terminale e facendo eseguire:

            lsof -i :<porta> OPPURE netstat -tlnp | grep <porta>

            la porta sarebbe quella messa dentro htons()

        senza lo sleep() il programma si chiuderebbe subito perche' non va in loop, per quello serve accept() che sto studiando adesso
*/

int main()
{

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == -1)
        std::cout<<"errore di creazione socket"<< std::endl;

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1 )
        std::cout<<"errore di bind"<< std::endl;

    listen(serverSocket, 5);

    sleep(50);
}