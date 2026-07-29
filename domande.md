**Sul Client:** 

1. Un client "non ancora autenticato" (prima di PASS/NICK/USER) va rappresentato con lo stesso oggetto Client o no? 

connessione server con port e psw, crea una istanza vuota che viene riempita poi con pasw e nick.

2. Come rappresentate lo stato di registrazione — un booleano, un enum con più fasi (es. "connesso", "password ok", "nick ok", "user ok", "registrato")?

ENUM -> dentro classe user
-- connesso al server
-- user ok
-- psw ok
-- nick univoco sul server ok
-- registrato

3. Il buffer di input: accumulate byte grezzi finché non trovate `\r\n`, o pensate a un'altra strategia?
accumuliamo byte grezzi come get line


4. Il buffer di output: come gestite il caso in cui la write() sul socket non riesce a scrivere tutto in una volta (write parziale)?

wait finché non ha completato il messaggio come get. 


5. Un client tiene un elenco dei canali a cui appartiene, o lo si ricava sempre interrogando i Channel?
ogni client ha il suo elenco di canali a cui appartiene

6. Come identificate univocamente un client nel sistema — il fd è sufficiente o serve anche un id logico?

fd 

**Sul Channel:**

7. La lista membri è una semplice lista di puntatori/riferimenti a Client, o una mappa nickname→Client?

mappa perché non ammette dups

8. Come distinguete "membro normale" da "operatore" — due liste separate, o un flag dentro una struttura associata al membro?

flag?

9. Il topic chi può vederlo — solo i membri, o chiunque faccia una query?

chiunque faccia un query

10. La key del canale (mode k) dove la salvate e come la confrontate quando arriva un JOIN?

sul chan la psw del canale, il server quando riceve join f un check sulla lista dei chan e confronta la psw

11. Il limite utenti (mode l) va controllato solo al JOIN o anche altrove?

solo al join 

12. Un canale vuoto (0 membri) va eliminato automaticamente? Chi se ne occupa?

va eliminato automaticamente, se ne occupa il server 

**Sul confine Networking → Logica:**

13. Chi si occupa di splittare comando e parametri — il Networking prima di passare i dati, o la Logica quando li riceve?

la logica (server )

14. Come gestite un comando con sintassi invalida — chi lo intercetta e chi genera l'errore?
il server intercetta e genera

15. Se un client manda più comandi in un solo pacchetto TCP (es. `NICK Mario\r\nUSER ...\r\n` tutto insieme), chi si occupa di splittarli in comandi separati?

server 

16. Se un comando arriva a metà (senza `\r\n` finale) cosa fa il Networking — aspetta il prossimo giro di poll()?

aspetta il giro di poll()

**Sul confine Logica → Networking (le risposte):**

17. La Logica scrive direttamente nel buffer di output del Client, o passa una stringa a una funzione del Networking che se ne occupa?

funzione che scrive su buffer out client // to-do

18. Chi si occupa di aggiungere `\r\n` alla fine di ogni risposta — sempre la stessa funzione centrale, per evitare errori sparsi nel codice?

si, la funzione menzionata prima 

19. Come gestite un messaggio broadcast (es. PRIVMSG a un intero canale) — un ciclo che scrive su ogni client membro, dove sta questa funzione?

ciclo su ogni client membro, metodo nel chan 

20. Serve una funzione helper unica tipo "manda questa stringa a questo client" usata ovunque, così chi scrive comandi non deve pensare al socket?

funzione da definire meglio // to-do

**Sulla gestione degli errori/numeric reply:**

21. Chi decide il formato esatto delle risposte numeriche — le scrivete insieme come "contratto" prima di dividervi, o ognuno le implementa quando serve? 

implementiamo quando serve

22. Serve una funzione centrale tipo "costruisci una risposta numerica dato codice + parametri", condivisa da entrambi?

funzione centrale ( in logica ) prende errno da server e stampa errore

**Sull'architettura generale:**

23. Il Server (classe che gestisce poll()) ha diretto accesso alla mappa di tutti i Channel, o solo il "layer logica" ce l'ha?

server ha mappa di tutti i chan 

24. Dove vive la mappa fd→Client — in una classe Server unica, condivisa da tutto il codice?

classe server private


25. State usando eccezioni C++ o codici di ritorno per segnalare errori interni (non IRC, proprio errori di programma tipo malloc fallita)? 

//to-do

26. Come gestite la disconnessione di un client a metà di un comando che tocca un canale (es. stava per fare KICK) — serve pulizia coordinata?

funzione centrale di pulizia ( da logica? ) // to-do cosa ritorna nello specifico poll() se fd non disponibile

valutiamo come controllare gli fd stile monitor philo 

**Sui dettagli pratici che spesso si dimenticano:**

27. Il nome del server (quello che appare come `:ircserv` nelle risposte) è fisso o configurabile? 



28. Come gestite maiuscole/minuscole nei nickname e nei nomi canale (IRC di solito è case-insensitive per questi)?

chi siamo noi per cambiare l immutabile legge insensitive di irc

29. Avete già deciso i limiti **Sul Client:** 

1. Un client "non ancora autenticato" (prima di PASS/NICK/USER) va rappresentato con lo stesso oggetto Client o no? 

connessione server con port e psw, crea una istanza vuota che viene riempita poi con pasw e nick.

2. Come rappresentate lo stato di registrazione — un booleano, un enum con più fasi (es. "connesso", "password ok", "nick ok", "user ok", "registrato")?

ENUM -> dentro classe user
-- connesso al server
-- user ok
-- psw ok
-- nick univoco sul server ok
-- registrato

3. Il buffer di input: accumulate byte grezzi finché non trovate `\r\n`, o pensate a un'altra strategia?
accumuliamo byte grezzi come get line


4. Il buffer di output: come gestite il caso in cui la write() sul socket non riesce a scrivere tutto in una volta (write parziale)?

wait finché non ha completato il messaggio come get. 


5. Un client tiene un elenco dei canali a cui appartiene, o lo si ricava sempre interrogando i Channel?
ogni client ha il suo elenco di canali a cui appartiene

6. Come identificate univocamente un client nel sistema — il fd è sufficiente o serve anche un id logico?

fd 

**Sul Channel:**

7. La lista membri è una semplice lista di puntatori/riferimenti a Client, o una mappa nickname→Client?

mappa perché non ammette dups

8. Come distinguete "membro normale" da "operatore" — due liste separate, o un flag dentro una struttura associata al membro?

flag?

9. Il topic chi può vederlo — solo i membri, o chiunque faccia una query?

chiunque faccia un query

10. La key del canale (mode k) dove la salvate e come la confrontate quando arriva un JOIN?

sul chan la psw del canale, il server quando riceve join f un check sulla lista dei chan e confronta la psw

11. Il limite utenti (mode l) va controllato solo al JOIN o anche altrove?

solo al join 

12. Un canale vuoto (0 membri) va eliminato automaticamente? Chi se ne occupa?

va eliminato automaticamente, se ne occupa il server 

**Sul confine Networking → Logica:**

13. Chi si occupa di splittare comando e parametri — il Networking prima di passare i dati, o la Logica quando li riceve?

la logica (server )

14. Come gestite un comando con sintassi invalida — chi lo intercetta e chi genera l'errore?
il server intercetta e genera

15. Se un client manda più comandi in un solo pacchetto TCP (es. `NICK Mario\r\nUSER ...\r\n` tutto insieme), chi si occupa di splittarli in comandi separati?

server 

16. Se un comando arriva a metà (senza `\r\n` finale) cosa fa il Networking — aspetta il prossimo giro di poll()?

aspetta il giro di poll()

**Sul confine Logica → Networking (le risposte):**

17. La Logica scrive direttamente nel buffer di output del Client, o passa una stringa a una funzione del Networking che se ne occupa?

funzione che scrive su buffer out client // to-do

18. Chi si occupa di aggiungere `\r\n` alla fine di ogni risposta — sempre la stessa funzione centrale, per evitare errori sparsi nel codice?

si, la funzione menzionata prima 

19. Come gestite un messaggio broadcast (es. PRIVMSG a un intero canale) — un ciclo che scrive su ogni client membro, dove sta questa funzione?

ciclo su ogni client membro, metodo nel chan 

20. Serve una funzione helper unica tipo "manda questa stringa a questo client" usata ovunque, così chi scrive comandi non deve pensare al socket?

funzione da definire meglio // to-do

**Sulla gestione degli errori/numeric reply:**

21. Chi decide il formato esatto delle risposte numeriche — le scrivete insieme come "contratto" prima di dividervi, o ognuno le implementa quando serve? 

implementiamo quando serve

22. Serve una funzione centrale tipo "costruisci una risposta numerica dato codice + parametri", condivisa da entrambi?

funzione centrale ( in logica ) prende errno da server e stampa errore

**Sull'architettura generale:**

23. Il Server (classe che gestisce poll()) ha diretto accesso alla mappa di tutti i Channel, o solo il "layer logica" ce l'ha?

server ha mappa di tutti i chan 

24. Dove vive la mappa fd→Client — in una classe Server unica, condivisa da tutto il codice?

classe server private


25. State usando eccezioni C++ o codici di ritorno per segnalare errori interni (non IRC, proprio errori di programma tipo malloc fallita)? 

//to-do

26. Come gestite la disconnessione di un client a metà di un comando che tocca un canale (es. stava per fare KICK) — serve pulizia coordinata?

funzione centrale di pulizia ( da logica? ) // to-do cosa ritorna nello specifico poll() se fd non disponibile

valutiamo come controllare gli fd stile monitor philo 

**Sui dettagli pratici che spesso si dimenticano:**

27. Il nome del server (quello che appare come `:ircserv` nelle risposte) è fisso o configurabile? 



28. Come gestite maiuscole/minuscole nei nickname e nei nomi canale (IRC di solito è case-insensitive per questi)?

chi siamo noi per cambiare l immutabile legge insensitive di irc

29. Avete già deciso i limiti pratici (lunghezza massima nickname, lunghezza massima messaggio) o li mettete quando serve? 

lo gestirá classe user 

30. Fate un file separato tipo "protocollo.md" o "interfacce.md" dove scrivete tutte queste decisioni, così non si perdono con il tempo? 

che cazzo dillo prima peró....

Divisione compiti:

Persona 1 — Connessione e chat di base (end-to-end)
Socket, bind, listen, accept, poll() loop, fd non bloccanti
Buffering e riassemblaggio dei comandi spezzettati (nc -C test)
Parser dei messaggi IRC (comando + parametri)
PASS, NICK, USER — tutta la fase di autenticazione/registrazione
PING/PONG, QUIT, gestione disconnessioni
PRIVMSG (privato e su canale — la parte di "invio", non la struttura Channel)
JOIN, PART (la parte "un client entra/esce", non la struttura interna del canale)


Persona 2 — Canali e amministrazione (dominio Channel completo)
Progettazione e implementazione della classe Channel (membri, operatori, topic, modes)
La logica di "chi può fare cosa" (operatore vs utente normale)
KICK — Eject a client from the channel
INVITE — Invite a client to a channel
OK - TOPIC — Change or view the channel topic
MODE — con tutte le sotto-opzioni: i, t, k, o, l (questo comando da solo è corposo, ha 5 comportamenti diversi al suo interno)

┌───────────┐
│   main    │
└─────┬─────┘
      │ crea
      ▼
┌───────────────────────────────────────────────────────────┐
│                         Server                            │
│                                                           │
│  ┌─────────────────┐       ┌───────────────────────────┐  │
│  │ Event loop      │       │ Command dispatcher        │  │
│  │                 │       │                           │  │
│  │ poll            │──────▶│ PASS / NICK / USER        │  │
│  │ accept          │       │ JOIN / PART               │  │
│  │ recv            │       │ PRIVMSG / QUIT            │  │
│  │ send            │       │ KICK / INVITE             │  │
│  └─────────────────┘       │ TOPIC / MODE              │  │
│                            └─────────────┬─────────────┘  │
│                                          │                │
│  ┌────────────────────┐    ┌─────────────▼─────────────┐  │
│  │ map fd → Client    │    │ map name → Channel       │  │
│  └─────────┬──────────┘    └─────────────┬─────────────┘  │
└────────────┼─────────────────────────────┼────────────────┘
             │                             │
             ▼                             ▼
┌────────────────────────┐     ┌──────────────────────────┐
│ Client                 │     │ Channel                  │
│                        │     │                          │
│ fd                     │◀───▶│ members                  │
│ identity               │     │ operators                │
│ registration flags     │     │ invited                  │
│ input buffer           │     │ topic                    │
│ output buffer          │     │ modes i/t/k/o/l          │
│ joined channels        │     │ key / limit              │
└────────────────────────┘     └──────────────────────────┘

## Classe channel

std::string Nome
Topic
Lista user (mappa fd - &user)
Lista op (&user)
mode attivi ( lista di bool )
pws chan 

bool hastopic
bool inviteOnly
bool haspsw
bool userlimit

std::string key;
User operator[]
user members[]
std::string topic;


/kick
/invite
/topic
/mode:
-- i : set/remove invite only chan OK
-- t: set/remove restriction of the topic command to chan op OK
-- k: set/remove chan key ( pwd ) OK 
-- o: give take chan op privilage OK
-- l: set/remove the user limit to chan( il cap di utenti ) 

/topic : se c'e' una stringa dopo --> set topic, altrimenti get topic check mode chan pratici (lunghezza massima nickname, lunghezza massima messaggio) o li mettete quando serve? 
