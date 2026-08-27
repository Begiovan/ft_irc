<div align="center">

# ft_irc

### A lightweight IRC server built from scratch in C++98

No external IRC libraries. Just raw sockets, `poll()`, and a lot of RFC reading.

[![C++](https://img.shields.io/badge/C%2B%2B-98-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)](https://en.wikipedia.org/wiki/C%2B%2B98)
[![42](https://img.shields.io/badge/42-project-000000?style=for-the-badge&logo=42&logoColor=white)](https://42.fr)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey?style=for-the-badge)](#)

</div>

---

## Indice

- [Descrizione](#descrizione)
- [Funzionalità](#funzionalità)
- [Architettura](#architettura)
- [Struttura del progetto](#struttura-del-progetto)
- [Comandi supportati](#comandi-supportati)
- [Requisiti](#requisiti)
- [Compilazione](#compilazione)
- [Utilizzo](#utilizzo)
- [Esempio di sessione](#esempio-di-sessione)
- [Testare il server](#testare-il-server)
- [Dettagli tecnici](#dettagli-tecnici)
- [Limitazioni note](#limitazioni-note)
- [Autori](#autori)

---

## Descrizione

**ft_irc** è un server IRC (Internet Relay Chat) sviluppato interamente in **C++98**, senza l'uso di librerie IRC esterne. Il progetto implementa da zero:

- gestione di connessioni multiple non bloccanti tramite `poll()`
- il protocollo IRC (parsing dei messaggi, codici di risposta numerici conformi a RFC 1459/2812)
- autenticazione client (`PASS` → `NICK` → `USER`)
- gestione completa di canali, operatori, inviti e modalità
- comunicazione client-client e client-canale (`PRIVMSG`, `NOTICE`)

L'obiettivo è poter usare un client IRC reale (KVIrc, irssi, WeeChat, HexChat...) per connettersi al server, creare canali, invitare utenti, impostare topic e permessi, esattamente come su un server IRC vero.

---

## Funzionalità

- Connessioni multiple gestite in modo non bloccante con un unico `poll()`
- Autenticazione con password del server
- Gestione nickname e username con validazione
- Creazione e gestione canali (pubblici, con password, a numero limitato, invite-only)
- Sistema di operatori di canale con privilegi dedicati
- Messaggistica privata e di canale (`PRIVMSG`, `NOTICE`)
- Gestione `TOPIC` con restrizioni configurabili
- `KICK`, `INVITE`, `MODE` completamente funzionanti
- Disconnessioni gestite correttamente, sia `QUIT` esplicito che disconnessioni impreviste

---

## Architettura

Il cuore del server è un event loop singolo basato su `poll()`:

```
Client TCP
   |
   v
Server::acceptClient()
   |
   v
poll() su tutti i fd
   |
   +-- POLLIN  --> Server::receiveClient()
   |                   |
   |                   v
   |               parseCommand()  (Command: nome + parametri)
   |                   |
   |                   v
   |               Server::dispatch()
   |                   |
   |                   v
   |               ACommand concreto (Nick, Join, Privmsg, ...)
   |                   |
   |                   v
   |               Client::appendSendBuffer()
   |
   +-- POLLOUT --> Server::flushClient()  --> send() al client
```

Passaggi principali:

1. ogni ciclo aggiorna gli eventi da monitorare (`POLLIN` sempre, `POLLOUT` solo se c'è qualcosa da inviare)
2. per ogni fd pronto in lettura, i byte vengono accumulati in un buffer per client finché non si individua una riga completa (`\n`)
3. la riga viene parsata in un `Command` (nome comando + parametri)
4. `Server::dispatch()` istanzia il comando concreto (pattern Command, tramite `ACommand`)
5. l'output non viene scritto subito con `send()`, ma accodato in un buffer di invio e scaricato quando il socket è pronto in scrittura, evitando blocchi su client lenti

---

## Struttura del progetto

```
ft_irc/
├── main.cpp              entry point, parsing argv, avvio server
├── server.cpp/.hpp       event loop, poll(), accept, dispatch dei comandi
├── client.cpp/.hpp       stato del singolo client (nick, buffer, registrazione)
├── channel.cpp/.hpp      stato di un canale (membri, operatori, modalità)
├── parser.cpp/.hpp       parsing delle righe IRC in Command
├── utils.cpp             validazioni (nickname, interi positivi)
├── irc_response.hpp      macro per i codici di risposta/errore IRC
├── ACommand.hpp          interfaccia comando + dichiarazioni comandi concreti
└── *.cpp                 un file per ciascun comando (nick, join, kick, mode...)
```

---

## Comandi supportati

| Comando | Descrizione | Richiede auth |
|---|---|---|
| `PASS <password>` | Autentica il client con la password del server | No |
| `NICK <nickname>` | Imposta/cambia il nickname | No |
| `USER <username>` | Imposta lo username, completa la registrazione | No |
| `JOIN <channel> [key]` | Entra in un canale (lo crea se non esiste) | Sì |
| `PRIVMSG <target> <msg>` | Messaggio privato o di canale | Sì |
| `NOTICE <target> <msg>` | Notifica privata o di canale | Sì |
| `TOPIC <channel> [:topic]` | Visualizza o imposta il topic | Sì |
| `KICK <channel> <nick> [reason]` | Espelle un utente dal canale (solo operator) | Sì |
| `INVITE <nick> <channel>` | Invita un utente in un canale (solo operator) | Sì |
| `MODE <channel> [+/-modes] [args]` | Gestisce modalità canale (`i`, `t`, `k`, `l`, `o`) | Sì |
| `PING <token>` | Keep-alive, risponde con `PONG` | Sì |
| `QUIT [:message]` | Disconnette il client dal server | Sì |
| `CAP LS / REQ` | Negoziazione capability (compatibilità client) | No |

### Modalità canale (MODE)

| Flag | Effetto |
|---|---|
| `i` | canale invite-only |
| `t` | topic modificabile solo dagli operatori |
| `k` | canale protetto da password |
| `l` | limite massimo di utenti |
| `o` | assegna/rimuove lo status di operatore |

---

## Requisiti

- Compilatore C++ con supporto C++98 (`g++`, `clang++`)
- Sistema POSIX (Linux o macOS): usa `socket()`, `poll()`, `fcntl()`
- `make`

---

## Compilazione

```bash
git clone <url-del-tuo-repo>
cd ft_irc
make
```

Se non hai ancora un `Makefile`, i sorgenti si compilano manualmente con:

```bash
c++ -Wall -Wextra -Werror -std=c++98 *.cpp -o ircserv
```

---

## Utilizzo

```bash
./ircserv <port> <password>
```

- `port` deve essere un intero valido maggiore di 1024
- `password` è la password richiesta ai client per autenticarsi (`PASS`)

Esempio:

```bash
./ircserv 6667 supersecret
```

---

## Esempio di sessione

Connessione manuale via `nc` (utile per debug, un client IRC vero gestisce tutto in automatico):

```bash
nc localhost 6667
PASS supersecret
NICK gigi
USER gigi 0 * :Gigi Bianchi
JOIN #general
PRIVMSG #general :ciao a tutti!
```

---

## Testare il server

Il server può essere testato con:

- `nc` (netcat): utile per inviare comandi grezzi e debug
- client IRC reali: [KVIrc](https://kvirc.net/), [irssi](https://irssi.org/), [WeeChat](https://weechat.org/), [HexChat](https://hexchat.github.io/)

Esempio di connessione con irssi:

```bash
irssi -c localhost -p 6667 -w supersecret
```

---

## Dettagli tecnici

- **I/O non bloccante**: tutti i socket (server e client) sono impostati con `fcntl(fd, F_SETFL, O_NONBLOCK)`.
- **Un solo `poll()`** monitora sia il socket di ascolto che tutti i client connessi: nessun thread, nessun processo figlio.
- **Buffering dei messaggi**: sia in ricezione che in invio, per gestire correttamente TCP che non garantisce l'arrivo dei dati "a riga intera" in un'unica `recv()`.
- **Pattern Command**: ogni comando IRC è una classe che eredita da `ACommand`, istanziata dinamicamente in `Server::dispatch()` in base al nome del comando ricevuto.
- **Gestione disconnessioni**: sia `QUIT` esplicito che disconnessioni impreviste (chiusura brusca del socket, crash del client) notificano correttamente gli altri membri dei canali con un messaggio `QUIT`, mantenendo lo stato dei canali sempre coerente.

---

## Limitazioni note

- Non implementa TLS/SSL (connessioni in chiaro)
- Non implementa tutti i comandi opzionali del protocollo IRC (es. `WHOIS`, `LIST`, `PART` dedicato: un membro esce dal canale solo tramite `KICK`/`QUIT`)
- Pensato per un singolo processo/singolo server, senza federazione tra server (`SERVER`/`CONNECT`)

---

## Autori

- **Begiovan** — [GitHub](https://github.com/Begiovan)
- **SusannaKay** — [GitHub](https://github.com/SusannaKay)
