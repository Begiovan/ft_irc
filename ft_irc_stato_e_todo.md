# ft_irc — Stato attuale e TODO della parte mandatoria

## Legenda

```text
[OK]      presente e già abbozzato/funzionante
[PARZ]    presente, ma incompleto o da integrare
[TODO]    ancora da implementare
[FIX]     presente, ma da correggere prima di proseguire
```

---

# 1. Stato attuale del progetto

## 1.1 Vista generale

```text
                              ft_irc
                                 |
        +------------------------+------------------------+
        |                        |                        |
        v                        v                        v
   main(7).cpp              socket(2).cpp          Modello del dominio
        |                        |                        |
        |                        |             +----------+----------+
        |                        |             |                     |
        v                        v             v                     v
  crea il socket          funzioni libere   Client               Channel
  del server              di networking       |                     |
        |                        |              |                     |
        |                        +--> makePollFd |                     |
        |                        +--> newClient |                     |
        |                        +--> incomingMsg                     |
        |                        +--> makePoll                        |
        |                                                           |
        +---------------------------> makePoll(serverSocket) <-------+


   Bozza separata:

   funzioni_mode.cpp
        |
        +--> handleTopic()   [bozza, non compilante]
        +--> handleKick()    [bozza, non compilante]
```

Al momento il progetto contiene:

- uno scheletro di networking funzionante basato su `socket`, `bind`, `listen`, `accept`, `poll` e `recv`;
- una classe `Client` che conserva fd, registrazione, nickname, username, buffer e canali;
- una classe `Channel` che conserva membri, operatori, invitati, topic e modalità;
- alcune prime idee per la gestione di `TOPIC` e `KICK`;
- nessuna classe `Server` ancora implementata;
- nessun parser IRC ancora implementato;
- nessun sistema completo di risposte numeriche IRC.

---

## 1.2 Flusso di rete attuale

```text
main()
  |
  +--> socket()
  |
  +--> setsockopt(SO_REUSEADDR)
  |
  +--> fcntl(O_NONBLOCK)
  |
  +--> bind()
  |
  +--> listen()
  |
  +--> makePoll(serverSocket)
          |
          +--> crea vector<pollfd>
          |
          +--> inserisce serverSocket
          |
          +--> ciclo poll()
                  |
                  +--> evento sul serverSocket
                  |       |
                  |       +--> accept()
                  |       +--> rende il nuovo fd non bloccante
                  |       +--> inserisce il fd nei pollfd
                  |
                  +--> evento su un client fd
                          |
                          +--> recv()
                          +--> stampa i byte ricevuti
                          +--> chiude ed elimina il fd se disconnesso
```

### Limite attuale

Il nuovo fd viene aggiunto a `poll`, ma non viene ancora creato e conservato un oggetto `Client` associato a quel fd.

```text
pollfd fd=5
    |
    +--> esiste

Client fd=5
    |
    +--> non viene ancora creato dal networking
```

---

## 1.3 Classe `Client`

```text
Client
 |
 +-- int _fd
 +-- int _registrationStatus
 +-- string _username
 +-- string _nickname
 +-- string _buffer
 +-- set<const Channel*> _channels
 |
 +-- get/set fd
 +-- get/set username
 +-- get/set nickname
 +-- add/remove/get channels
 +-- append al buffer tramite setBuffer()
 +-- get/clear buffer
 +-- set dei flag PASS_OK, NICK_OK, USER_OK
 +-- controllo isRegistered()
```

### Stato

```text
[OK]   conserva il file descriptor
[OK]   conserva username e nickname
[OK]   conserva un buffer separato per ogni client
[OK]   usa una bitmask per PASS_OK, NICK_OK e USER_OK
[OK]   conosce i Channel di cui fa parte
[PARZ] API del buffer ancora minima
[PARZ] registrazione priva di controlli e rimozione flag
[OK]  isRegistered() dovrebbe essere const
[OK]  setBuffer() in realtà fa append: nome poco intuitivo
[FIX]  getter stringa restituiscono copie invece di const reference
[FIX]  manca un metodo per estrarre solo un comando completo dal buffer
```

### Flusso previsto del buffer

```text
recv(fd)
   |
   v
Client::_buffer += bytes
   |
   +--> nessun "\r\n" trovato
   |       |
   |       +--> attende altri byte
   |
   +--> trovato "\r\n"
           |
           +--> estrazione di una riga completa
           +--> la parte rimanente resta nel buffer
```

---

## 1.4 Classe `Channel`

```text
Channel
 |
 +-- string _name
 +-- string _topic
 +-- string _key
 |
 +-- set<const Client*> _members
 +-- set<int> _operators
 +-- set<int> _invited
 |
 +-- bool _inviteOnly       (+i)
 +-- bool _topicRestricted  (+t)
 +-- int  _userLimit        (+l)
 |
 +-- add/remove member
 +-- add/remove operator
 +-- invite/remove invite
 +-- canJoin()
 +-- canChangeTopic()
 +-- set/clear key
 +-- set/clear user limit
 +-- toggle invite-only
 +-- toggle topic restriction
```

### Stato

```text
[OK]   nome, topic e key
[OK]   membri del canale
[OK]   operatori
[OK]   lista invitati
[OK]   modalità i, t, k, o, l abbozzate
[OK]   primo membro promosso automaticamente operatore
[OK]   controllo base per accesso con invite, key e limit
[PARZ] funzioni restituiscono spesso bool/void, senza codici errore IRC
[PARZ] relazione Client <-> Channel non viene ancora sincronizzata
[FIX]  copy constructor e operator= non copiano membri/operatori/invitati
[FIX]  uso misto di Client* e fd rende la struttura poco uniforme
[FIX]  include di client.hpp e forward declaration sono ridondanti
[FIX]  addMember() aggiunge il membro al Channel ma non il Channel al Client
[FIX]  removeMember() rimuove dal Channel ma non dal Client
[FIX]  dopo la rimozione dell'ultimo operatore manca una politica di promozione
```

---

## 1.5 Bozza gestione comandi

```text
funzioni_mode.cpp
 |
 +-- handleTopic()
 |
 +-- handleKick()
```

### Stato

```text
[PARZ] chiarisce l'idea di separare il comando dalla classe Channel
[FIX]  `Class` deve essere `class`
[FIX]  viene usato `this` fuori da una classe
[FIX]  confronto stringa con NULL non valido
[FIX]  _nickName non è disponibile nel contesto
[FIX]  KICK rimuove solo il ruolo operatore, non il membro dal canale
[TODO] invio delle risposte al client tramite send()
[TODO] broadcast agli altri membri del canale
[TODO] codici numerici di errore IRC
```

---

# 2. Architettura verso cui portare il progetto

```text
                              main()
                                 |
                                 v
                    Server(port, password)
                                 |
                              run()
                                 |
               +-----------------+-----------------+
               |                                   |
               v                                   v
          Networking                           Stato server
               |                                   |
     +---------+---------+              +----------+----------+
     |         |         |              |                     |
     v         v         v              v                     v
  accept()   recv()    send()      map<int, Client>   map<string, Channel>
     |         |
     |         +--> Client::_buffer
     |                 |
     |                 +--> estrazione righe IRC complete
     |                              |
     +--> Client(fd)                 v
                              Parser / Command
                                      |
                                      v
                                CommandHandler
                                      |
                   +------------------+------------------+
                   |                  |                  |
                   v                  v                  v
                 Client             Channel           Server
             registrazione       membri/mode       lookup/broadcast
```

Separazione delle responsabilità:

```text
Server         = rete, ownership, lookup, ciclo poll, invio e disconnessioni
Client         = stato della singola connessione e registrazione
Channel        = stato e regole interne del canale
Parser         = testo IRC -> comando + parametri
CommandHandler = validazione ed esecuzione dei comandi IRC
Replies        = costruzione delle risposte numeriche e dei messaggi IRC
```

---

# 3. TODO mandatori suddivisi per area

## 3.1 `main`

- [ ] Accettare esattamente i parametri richiesti:

```text
./ircserv <port> <password>
```

- [ ] Verificare numero degli argomenti.
- [ ] Validare la porta:
  - solo numerica;
  - nel range valido;
  - conversione sicura senza overflow.
- [ ] Conservare la password del server.
- [ ] Creare una sola istanza di `Server`.
- [ ] Avviare il server tramite `server.run()`.
- [ ] Gestire eccezioni/errori di inizializzazione e uscita pulita.

---

## 3.2 Classe `Server` — proprietà e ciclo di vita

### Attributi principali

- [ ] Creare `Server.hpp` e `Server.cpp`.
- [ ] Aggiungere almeno:

```cpp
int _serverSocket;
int _port;
std::string _password;
std::vector<pollfd> _pollfds;
std::map<int, Client> _clients;
std::map<std::string, Channel> _channels;
```

- [ ] Valutare una struttura per i nickname, oppure una funzione di ricerca nella mappa client.
- [ ] Impedire copie accidentali del server, oppure implementarne correttamente la forma canonica.

### Inizializzazione

- [ ] Spostare da `main.cpp` dentro `Server`:
  - `socket()`;
  - `setsockopt()`;
  - `fcntl()`;
  - `bind()`;
  - `listen()`.
- [ ] Inizializzare completamente `sockaddr_in` prima dell'uso.
- [ ] Aggiungere il socket del server a `_pollfds`.
- [ ] Controllare ogni valore di ritorno.
- [ ] Chiudere correttamente il socket se una fase fallisce.

### Event loop

- [ ] Trasformare `makePoll()` in `Server::run()`.
- [ ] Controllare il valore restituito da `poll()`.
- [ ] Gestire `EINTR` se necessario.
- [ ] Controllare `revents` per:
  - `POLLIN`;
  - `POLLHUP`;
  - `POLLERR`;
  - `POLLNVAL`.
- [ ] Evitare errori sugli indici quando un elemento viene eliminato dal vector.
- [ ] Garantire che ogni operazione I/O resti non bloccante.

### Accettazione client

- [ ] Trasformare `newClient()` in `Server::acceptClient()`.
- [ ] Accettare tutte le connessioni in attesa finché `accept()` non restituisce `EAGAIN/EWOULDBLOCK`.
- [ ] Rendere ogni nuovo socket non bloccante.
- [ ] Creare subito `Client(newFd)`.
- [ ] Inserire il client in `_clients` usando il fd come chiave.
- [ ] Inserire il fd in `_pollfds`.
- [ ] Gestire il fallimento parziale senza lasciare fd aperti.

### Ricezione dati

- [ ] Trasformare `incomingMsg()` in `Server::receiveFromClient()`.
- [ ] Trovare il `Client` tramite il fd del `pollfd`.
- [ ] Aggiungere i byte ricevuti al buffer di quel client.
- [ ] Gestire correttamente:
  - `recv() > 0`;
  - `recv() == 0`;
  - `recv() == -1` con `EAGAIN/EWOULDBLOCK`;
  - altri errori.
- [ ] Estrarre tutte le righe IRC complete presenti dopo una singola `recv()`.
- [ ] Lasciare nel buffer solamente l'eventuale comando incompleto.
- [ ] Gestire il limite massimo previsto per un messaggio IRC.

### Invio dati

- [ ] Creare una funzione centralizzata, per esempio:

```cpp
void sendToClient(Client& client, const std::string& message);
```

- [ ] Gestire invii parziali di `send()`.
- [ ] Terminare correttamente i messaggi con `\r\n`.
- [ ] Non usare `std::cout` come risposta al client IRC.
- [ ] Implementare broadcast a tutti i membri di un canale.
- [ ] Implementare broadcast con esclusione opzionale del mittente.

### Lookup e ownership

- [ ] Cercare un client per fd.
- [ ] Cercare un client per nickname.
- [ ] Verificare unicità dei nickname.
- [ ] Cercare un canale per nome.
- [ ] Creare un canale al primo `JOIN` se non esiste.
- [ ] Eliminare un canale quando non ha più membri.

### Disconnessione

- [ ] Creare `Server::disconnectClient()`.
- [ ] Rimuovere il client da tutti i Channel.
- [ ] Informare gli altri utenti con un messaggio `QUIT` quando necessario.
- [ ] Rimuovere il client da `_clients`.
- [ ] Rimuovere il fd da `_pollfds`.
- [ ] Chiudere il fd una sola volta.
- [ ] Eliminare eventuali canali rimasti vuoti.

---

## 3.3 Classe `Client`

### Stato della connessione

- [ ] Conservare il fd ricevuto da `accept()`.
- [ ] Inizializzare `_registrationStatus` a `0`.
- [ ] Inizializzare esplicitamente stringhe e container, anche se lo fanno già di default.
- [ ] Aggiungere eventuale stato di disconnessione solo se realmente utile.

### Registrazione IRC

- [ ] Mantenere i flag:
  - `PASS_OK`;
  - `NICK_OK`;
  - `USER_OK`.
- [ ] Rendere `isRegistered()` const.
- [ ] Aggiungere `hasRegistrationFlag()`.
- [ ] Decidere come impedire comandi non consentiti prima della registrazione.
- [ ] Impedire modifiche illegali a `USER` dopo la registrazione.
- [ ] Gestire cambi di nickname dopo la registrazione.

### Identità

- [ ] Validare nickname e username nel command handler.
- [ ] Restituire `const std::string&` dai getter per evitare copie inutili.
- [ ] Uniformare il nome dell'attributo `_nickname` in tutto il progetto.
- [ ] Valutare se aggiungere il real name/trailing del comando `USER` se richiesto dalla vostra interpretazione del protocollo.

### Buffer

- [ ] Rinominare `setBuffer()` in `appendBuffer()`.
- [ ] Restituire il buffer tramite `const std::string&`.
- [ ] Aggiungere una funzione per rimuovere solo la parte già processata.
- [ ] Non cancellare tutto il buffer se contiene anche un comando incompleto successivo.
- [ ] Gestire più comandi arrivati nella stessa `recv()`.
- [ ] Gestire un comando diviso tra più `recv()`.

### Relazione con i canali

- [ ] Decidere definitivamente tra `Channel*` e `const Channel*`.
- [ ] Aggiungere `isInChannel()`.
- [ ] Aggiornare `_channels` durante `JOIN`, `PART`, `KICK` e `QUIT`.
- [ ] Evitare puntatori rimasti verso Channel già distrutti.

---

## 3.4 Classe `Channel`

### Membri e operatori

- [ ] Uniformare il tipo usato per membri, operatori e invitati:
  - tutti `Client*`, oppure
  - membri `Client*` e lookup coerente per fd.
- [ ] Aggiungere `isMember()`.
- [ ] Impedire duplicati durante `JOIN`.
- [ ] Aggiornare anche il `Client` durante add/remove.
- [ ] Rimuovere un utente da membri, operatori e invitati durante `PART/KICK/QUIT`.
- [ ] Stabilire cosa succede se non restano operatori ma il canale è ancora popolato.
- [ ] Eliminare il canale dal server quando diventa vuoto.

### Copia e assegnazione

- [ ] Decidere se i `Channel` devono essere copiabili.
- [ ] Se sì, copiare anche:
  - `_members`;
  - `_operators`;
  - `_invited`.
- [ ] Se no, impedire la copia invece di lasciarla parziale.

### Topic

- [ ] Permettere lettura del topic.
- [ ] Permettere modifica rispettando `+t`.
- [ ] Distinguere topic assente da topic vuoto, se necessario per le risposte.
- [ ] Inviare il topic aggiornato a tutti i membri.

### Modalità obbligatorie

- [ ] `i` — invite-only:
  - attivazione;
  - disattivazione;
  - controllo invito in `JOIN`.
- [ ] `t` — topic riservato agli operatori:
  - attivazione;
  - disattivazione;
  - controllo in `TOPIC`.
- [ ] `k` — chiave del canale:
  - impostazione;
  - rimozione;
  - confronto durante `JOIN`.
- [ ] `o` — privilegio operatore:
  - promozione di un membro;
  - rimozione del privilegio;
  - rifiuto se il target non è nel canale.
- [ ] `l` — limite utenti:
  - impostazione con parametro numerico valido;
  - rimozione;
  - controllo durante `JOIN`.
- [ ] Produrre una rappresentazione corrente delle mode per le risposte a `MODE`.
- [ ] Non lasciare alla classe `Channel` il compito di inviare errori o messaggi di protocollo.

### Inviti

- [ ] Aggiungere un utente alla lista invitati con `INVITE`.
- [ ] Controllare che chi invita abbia i permessi richiesti.
- [ ] Consumare/rimuovere l'invito dopo il `JOIN`, secondo il comportamento scelto.
- [ ] Pulire gli inviti quando un client si disconnette.

---

## 3.5 Parser IRC

- [ ] Creare una rappresentazione del comando, per esempio:

```cpp
struct Command
{
    std::string name;
    std::vector<std::string> params;
    std::string trailing;
};
```

- [ ] Separare una riga completa dal buffer.
- [ ] Rimuovere `\r\n` dalla riga prima del parsing.
- [ ] Separare il nome del comando dai parametri.
- [ ] Normalizzare il nome del comando in maiuscolo.
- [ ] Gestire il parametro trailing introdotto da `:`.
- [ ] Gestire spazi multipli senza generare parametri vuoti inutili.
- [ ] Rilevare comando vuoto o malformato.
- [ ] Non eseguire direttamente la logica applicativa dentro il parser.

Esempio:

```text
Input:
    PRIVMSG #42 :ciao a tutti

Output:
    name     = "PRIVMSG"
    params   = ["#42"]
    trailing = "ciao a tutti"
```

---

## 3.6 Dispatcher / Command Handler

- [ ] Creare un punto centrale che riceva:

```cpp
Client& sender
Command command
```

- [ ] Associare ogni nome comando alla relativa funzione.
- [ ] Verificare quali comandi sono consentiti prima della registrazione.
- [ ] Verificare numero minimo/massimo dei parametri.
- [ ] Delegare a `Server`, `Client` e `Channel` senza duplicare ownership.
- [ ] Costruire e inviare risposte numeriche corrette.
- [ ] Non inserire tutta la logica in una sola funzione enorme.

---

## 3.7 Comandi di registrazione

### `PASS`

- [ ] Verificare presenza del parametro.
- [ ] Confrontare con la password del server.
- [ ] Rifiutare password errata.
- [ ] Impostare `PASS_OK` solo se corretta.
- [ ] Gestire `PASS` inviato quando il client è già registrato.

### `NICK`

- [ ] Verificare presenza del nickname.
- [ ] Validare sintassi del nickname.
- [ ] Verificare che non sia già usato.
- [ ] Impostare `NICK_OK`.
- [ ] Gestire cambio nickname dopo la registrazione.
- [ ] Notificare il cambio agli utenti che condividono canali.

### `USER`

- [ ] Verificare parametri obbligatori.
- [ ] Impostare username.
- [ ] Impostare `USER_OK`.
- [ ] Rifiutare una seconda registrazione `USER`.

### Completamento registrazione

- [ ] Quando PASS, NICK e USER sono validi, inviare il welcome sequence.
- [ ] Assicurarsi che venga inviata una sola volta.

---

## 3.8 Comandi canale mandatori

### `JOIN`

- [ ] Verificare registrazione del client.
- [ ] Validare nome del canale.
- [ ] Creare il Channel se non esiste.
- [ ] Controllare `+i`, `+k` e `+l`.
- [ ] Aggiungere il client su entrambi i lati della relazione.
- [ ] Rendere operatore il primo membro.
- [ ] Inviare il `JOIN` a tutti i membri.
- [ ] Inviare topic e lista nomi.

### `PART`

- [ ] Verificare esistenza del canale.
- [ ] Verificare appartenenza del client.
- [ ] Inviare il messaggio `PART` ai membri.
- [ ] Rimuovere il client su entrambi i lati.
- [ ] Eliminare il canale se vuoto.

### `TOPIC`

- [ ] Senza nuovo topic: restituire il topic attuale.
- [ ] Con nuovo topic: verificare membership e `+t`.
- [ ] Salvare il nuovo topic.
- [ ] Comunicare il cambiamento a tutti i membri.

### `INVITE`

- [ ] Verificare target e canale.
- [ ] Verificare appartenenza e permessi dell'invitante.
- [ ] Verificare che il target non sia già nel canale.
- [ ] Salvare l'invito.
- [ ] Inviare conferma e messaggio al target.

### `KICK`

- [ ] Verificare esistenza del canale.
- [ ] Verificare che il mittente sia operatore.
- [ ] Verificare che il target sia membro.
- [ ] Inviare `KICK` prima della rimozione.
- [ ] Rimuovere completamente il target dal canale.
- [ ] Aggiornare anche il set dei canali del target.
- [ ] Eliminare il canale se vuoto.

### `MODE`

- [ ] Senza modifiche: mostrare le mode correnti del canale.
- [ ] Parsare sequenze `+` e `-`.
- [ ] Gestire i parametri richiesti da `k`, `o`, `l`.
- [ ] Verificare che chi modifica sia operatore.
- [ ] Applicare `i`, `t`, `k`, `o`, `l`.
- [ ] Inviare la modifica a tutti i membri.
- [ ] Generare errori corretti per mode sconosciute o parametri mancanti.

---

## 3.9 Messaggistica

### `PRIVMSG`

- [ ] Verificare presenza di destinatario e testo.
- [ ] Supportare destinatario utente.
- [ ] Supportare destinatario canale.
- [ ] Verificare esistenza del destinatario.
- [ ] Per i canali, verificare le regole di invio richieste.
- [ ] Non reinviare normalmente il messaggio al mittente.

### `NOTICE`

- [ ] Implementare comportamento simile a `PRIVMSG`.
- [ ] Evitare risposte automatiche di errore a `NOTICE`, secondo il protocollo.

---

## 3.10 Disconnessione e `QUIT`

- [ ] Gestire `QUIT` esplicito.
- [ ] Gestire chiusura TCP improvvisa.
- [ ] Raccogliere gli utenti che condividono almeno un canale col client.
- [ ] Evitare di inviare più volte lo stesso `QUIT` allo stesso destinatario.
- [ ] Rimuovere il client da tutti i canali.
- [ ] Rimuovere ruoli e inviti.
- [ ] Eliminare i canali vuoti.
- [ ] Cancellare il client dal server.
- [ ] Chiudere il fd.

---

## 3.11 Risposte IRC ed errori numerici

- [ ] Creare funzioni/helper per formattare messaggi IRC.
- [ ] Aggiungere il prefisso server quando necessario.
- [ ] Aggiungere nickname del destinatario ai numerici.
- [ ] Implementare almeno tutti i numerici richiesti dai flussi mandatori.
- [ ] Gestire, tra gli altri, errori per:
  - comando sconosciuto;
  - parametri insufficienti;
  - client non registrato;
  - nickname già in uso;
  - password errata;
  - canale inesistente;
  - utente inesistente;
  - utente non nel canale;
  - mittente non nel canale;
  - permessi operatore mancanti;
  - invite-only;
  - chiave errata;
  - canale pieno;
  - mode sconosciuta.
- [ ] Centralizzare la formattazione per evitare stringhe duplicate sparse nel codice.

---

## 3.12 Build e organizzazione dei file

- [ ] Creare un `Makefile` con:
  - `NAME = ircserv`;
  - `all`;
  - `clean`;
  - `fclean`;
  - `re`.
- [ ] Compilare con lo standard richiesto dal subject.
- [ ] Aggiungere warning flags richieste.
- [ ] Separare header e sorgenti in cartelle coerenti.
- [ ] Rimuovere o integrare i prototipi temporanei quando `Server` sarà pronto.
- [ ] Correggere `funzioni_mode.cpp` oppure sostituirlo con veri handler.
- [ ] Evitare include circolari usando forward declaration dove bastano puntatori/reference.
- [ ] Verificare che ogni header abbia include guard.
- [ ] Non lasciare funzioni non dichiarate o dichiarazioni non implementate.

---

## 3.13 Test minimi

### Networking

- [ ] Più client connessi contemporaneamente.
- [ ] Un client si disconnette senza bloccare gli altri.
- [ ] `recv()` con comando spezzato in più pacchetti.
- [ ] Più comandi nella stessa `recv()`.
- [ ] Client molto lento o che non invia dati.

### Registrazione

- [ ] Password corretta e errata.
- [ ] Ordini diversi di `PASS`, `NICK`, `USER` se consentiti.
- [ ] Nickname duplicato.
- [ ] Comando protetto prima della registrazione.

### Canali

- [ ] Primo membro diventa operatore.
- [ ] `JOIN` e `PART` multipli.
- [ ] Rimozione dell'ultimo membro.
- [ ] `KICK` da operatore e non operatore.
- [ ] `INVITE` con e senza `+i`.
- [ ] `TOPIC` con `+t` attivo e disattivo.
- [ ] `MODE +i/-i`, `+t/-t`, `+k/-k`, `+o/-o`, `+l/-l`.

### Memoria e stabilità

- [ ] Nessun fd lasciato aperto.
- [ ] Nessun puntatore a Client distrutto dentro un Channel.
- [ ] Nessun puntatore a Channel distrutto dentro un Client.
- [ ] Nessun invalid iterator durante erase nei container.
- [ ] Nessuna perdita di memoria.

---

# 4. Ordine di implementazione consigliato

```text
1. Creare Server
        |
2. Spostare dentro Server il networking già esistente
        |
3. Creare Client(fd) durante accept()
        |
4. Collegare recv() al buffer del Client
        |
5. Estrarre righe complete terminate da \r\n
        |
6. Implementare Parser e struttura Command
        |
7. Implementare PASS, NICK, USER e welcome
        |
8. Implementare lookup Client/Channel e send helpers
        |
9. Implementare JOIN e PART
        |
10. Implementare PRIVMSG e NOTICE
        |
11. Implementare TOPIC, INVITE e KICK
        |
12. Implementare MODE i, t, k, o, l
        |
13. Implementare QUIT e cleanup completo
        |
14. Completare numerici, errori e test
```

---

# 5. Prossimo obiettivo concreto

Il prossimo passo non dovrebbe essere aggiungere altra logica alle funzioni globali di `socket(2).cpp`.

Il passo consigliato è:

```text
[PROSSIMO STEP]

Creare la classe Server
        |
        +--> trasferire setup socket da main
        +--> trasferire makePoll() in Server::run()
        +--> trasferire newClient() in Server::acceptClient()
        +--> trasferire incomingMsg() in Server::receiveFromClient()
        +--> aggiungere map<int, Client> _clients
        +--> creare Client(fd) subito dopo accept()
```

Una volta completato questo passaggio, il prototipo di rete diventerà la base reale del progetto e sarà possibile collegare correttamente buffer, parser e comandi IRC.
