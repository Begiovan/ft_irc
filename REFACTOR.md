# ft_irc — Piano di refactor

> Obiettivo: eliminare la ridondanza **senza cambiare la logica** del programma.
> L'idea di fondo è spostare in pochi punti comuni ciò che oggi è ripetuto in ogni comando:
> invio delle risposte, controlli di validazione, ingresso/uscita dai canali, notifiche.
> Molti bug della todo list (sezione B) spariscono "gratis" come effetto collaterale.

Legenda priorità: 🔴 bug visibile/crash · 🟠 non conforme al protocollo · 🟢 pulizia/refactor

---

## A. Orientamento del refactor

### Da dove nasce la ridondanza

Guardando i comandi, quasi tutti seguono lo stesso schema copiato a mano:

```text
if (params.size() < N)        -> ERR_NEEDMOREPARAMS + "\r\n"
chan = findChannel(params[0]) -> ERR_NOSUCHCHANNEL  + "\r\n"
if (!chan->isMember(client))  -> ERR_NOTONCHANNEL   + "\r\n"
if (!chan->isOperator(...))   -> ERR_CHANOPRIVSNEEDED + "\r\n"
... logica vera (5-10 righe) ...
```

In più ci sono tre "operazioni di dominio" riscritte in più file:

| Operazione                         | Dove è duplicata oggi                         |
|------------------------------------|-----------------------------------------------|
| togliere un client da un canale    | kick.cpp, quit/disconnectClient, (PART futuro) |
| notificare "chi condivide un canale con me" | nick.cpp (con dedup), quit.cpp (senza dedup) |
| inviare topic + lista NAMES         | join.cpp, topic.cpp                            |
| "se ora è registrato manda WELCOME" | pass.cpp, nick.cpp, user.cpp                   |
| cercare un client per nick         | findClient + returnClient                      |

Il refactor consiste nel dare un **nome e una casa** a ciascuno di questi pezzi.

### Architettura obiettivo (stessi file, responsabilità più nette)

```text
main.cpp        -> parsing argomenti, segnali, avvio
Server          -> rete (poll, accept, recv, send) + registro client/canali
                   + operazioni di dominio condivise (join/part/quit/broadcast)
Client          -> stato del client, buffer in/out, estrazione righe, prefix()
Channel         -> stato del canale, regole (canJoin, canChangeTopic), NIENTE I/O
parser          -> riga -> Command { prefix, command (uppercase), params }
ACommand        -> metadati del comando + helper di validazione
Comandi (*.cpp) -> solo la logica specifica del comando
irc_response    -> formati dei messaggi, senza "\r\n"
```

---

### Step 0 — Rete di sicurezza (prima di toccare codice)

Senza test, "non cambiare la logica" è impossibile da verificare.

- [ ] Scrivere `tests/scenario.sh` con `nc` (o un piccolo script Python) che esegue:
      registrazione completa, NICK duplicato, JOIN/PRIVMSG tra 2 client, TOPIC,
      INVITE su canale +i, MODE +k/+l/+o, KICK, QUIT.
- [ ] Salvare l'output di oggi come riferimento e rilanciarlo dopo ogni step.
      Le differenze devono essere solo quelle volute (bug corretti).

### Step 1 — Un solo punto di uscita per i messaggi

**Problema:** `"\r\n"` aggiunto a mano ovunque (già dimenticato in INVITE), sorgente
incoerente (`"*"` vs nick), `"ircserv"` e `"@ircserv"` scritti nelle macro.

- [ ] `Server::sendToClient` (o un nuovo `reply()`) aggiunge **lui** `"\r\n"`.
      Le macro in `irc_response.hpp` non lo contengono e i comandi non lo scrivono più.
- [ ] `Client::nickOrStar()` → nick, oppure `"*"` se ancora vuoto.
      Usarlo come `source` in **tutte** le risposte numeriche.
- [ ] `Client::prefix()` → `nick!user@host`. Le macro "Command Responses"
      prendono un solo parametro `prefix` invece di `(source, user)`.
- [ ] `#define SERVER_NAME "ircserv"` in un solo punto.

*Risolve:* INVITE senza CRLF, JOIN che manda una riga vuota, TOPIC senza prefisso,
sorgente incoerente in PRIVMSG/PASS/NICK, `ERR_NICKNAMEINUSE` malformato.

### Step 2 — Helper di validazione in `ACommand`

**Problema:** la catena di controlli dello schema sopra è copiata in ogni comando.

Aggiungere a `ACommand` dei metodi `protected` che **inviano l'errore e ritornano false/NULL**:

```cpp
bool     needParams(Client &c, const std::vector<std::string> &p, size_t n);
Channel *needChannel(Client &c, const std::string &name);      // 403
bool     needMember(Client &c, Channel &ch);                   // 442
bool     needOperator(Client &c, Channel &ch);                 // 482
Client  *needNick(Client &c, const std::string &nick);         // 401
```

Un comando diventa:

```cpp
void Kick::execute(Client &c, const std::vector<std::string> &p) {
    if (!needParams(c, p, 2)) return;
    Channel *ch = needChannel(c, p[0]);
    if (!ch || !needMember(c, *ch) || !needOperator(c, *ch)) return;
    Client *target = needNick(c, p[1]);
    if (!target) return;
    ...logica specifica...
}
```

- [ ] Dare ad `ACommand` anche il **nome** del comando e i **parametri minimi**
      (`ACommand(Server&, "KICK", 2, true)`), così `needParams` sa cosa scrivere
      in `ERR_NEEDMOREPARAMS` senza ripeterlo.
- [ ] Cambiare la firma: `execute(Client &, const std::vector<std::string> &)`
      (niente copie, niente puntatore che può essere NULL).

*Effetto:* KICK, INVITE, TOPIC, MODE, PRIVMSG, NOTICE, JOIN si accorciano di metà
e i messaggi d'errore diventano coerenti per costruzione.

### Step 3 — Registro dei comandi al posto di `dispatch`

**Problema:** due catene di `if` quasi identiche, `isAuth &&` ridondante,
`new`/`delete` a ogni riga, doppio errore per i non registrati.

- [ ] `std::map<std::string, ACommand*> _commands` creata **una volta** nel costruttore
      di `Server` e liberata nel distruttore.
- [ ] Ogni `ACommand` sa se richiede la registrazione (`requiresRegistration()`).
- [ ] Nuovo dispatch in un solo punto:

```text
riga vuota                  -> ignora
comando non in mappa        -> 421 ERR_UNKNOWNCOMMAND
richiede reg. e non reg.    -> 451 ERR_NOTREGISTERED
altrimenti                  -> cmd->execute(...)
```

- [ ] Il nome del comando va convertito in maiuscolo nel parser.

*Risolve:* doppio errore 451+421, comandi case-sensitive, righe vuote,
PASS/USER dopo la registrazione (saranno sempre raggiungibili e
risponderanno 462 grazie ai loro controlli interni).

### Step 4 — Operazioni di dominio condivise in `Server`

**Problema:** stesse sequenze riscritte in più comandi, con piccole differenze
(che sono proprio i bug).

- [ ] `void joinChannel(Client&, Channel&)` → addMember + addChannel + rimozione invito.
- [ ] `void partChannel(Client&, Channel&)` → removeMember + removeChannel + `removeEmptyChan`.
      Usato da KICK, QUIT, disconnect (e PART se lo aggiungete).
- [ ] `void broadcastToNeighbours(Client&, const std::string&, bool includeSelf)`
      → raccoglie i membri di tutti i canali in un `set` (il pattern che c'è già in NICK)
      e invia una volta sola. Usato da NICK e QUIT.
- [ ] `void sendTopic(Client&, Channel&)` e `void sendNames(Client&, Channel&)`
      → usati da JOIN e TOPIC.
- [ ] `void tryCompleteRegistration(Client&)` → manda RPL_WELCOME se la registrazione
      si è appena completata. Usato da PASS, NICK, USER.
- [ ] `Client *getClientByNick(const std::string&)` al posto di `findClient` + `returnClient`
      (confronto case-insensitive).

*Risolve:* QUIT duplicato a chi condivide più canali, KICK ridondante,
nick case-sensitive, codice WELCOME triplicato.

### Step 5 — Modello dati coerente (togliere i `const_cast`)

- [ ] `Channel`: `_members`, `_operators`, `_invited` tutti come `std::set<Client*>`
      (oggi membri = puntatori, operatori/invitati = fd).
- [ ] `Client`: `std::set<Channel*>`, getter `const` (`getNickname() const` ecc.).
- [ ] Rimuovere ogni `const_cast` (broadcast, nick, join, disconnect).
- [ ] `Channel` senza I/O: `setUserLimit` ritorna `bool` invece di stampare.
- [ ] Rimuovere i metodi inutilizzati (`setFd`, `clearBuffer`, `clearSendBuffer`),
      rinominare `topicRes()`/`inviteRes()` → `isTopicRestricted()`/`isInviteOnly()`.
- [ ] Aggiungere `_hostname` e `_realname` al `Client`.

*Risolve:* inviti "ereditati" da un nuovo client con lo stesso fd.

### Step 6 — Loop di rete più semplice e sicuro

**Problema:** `run()` modifica `_fds` mentre lo scorre, `i--` con controlli successivi
sull'indice sbagliato, `QUIT` chiude il fd mentre il comando è ancora in esecuzione.

- [ ] Pattern **"mark & sweep"**: `Client::markForDisconnect()` invece di chiudere subito;
      a fine giro di `poll` si chiudono tutti i client marcati (dopo aver provato a
      svuotare il send buffer). Niente più `i--` né controlli "il client esiste ancora?".
- [ ] `Client::popLine(std::string &out)` → estrae una riga completa dal buffer
      (togliendo `\r`), con limite di 512 byte.
- [ ] Spezzare `run()` in `handleEvents(pollfd&)`, `readFrom(Client&)`,
      `processLine(Client&, const std::string&)`, `writeTo(Client&)`.
- [ ] Gestire `POLLHUP | POLLERR`, controllare il ritorno di `poll`.
- [ ] `signal(SIGPIPE, SIG_IGN)` + SIGINT per uno shutdown pulito
      (il distruttore libera client, canali, comandi e chiude i socket).
- [ ] La disconnessione brusca passa dalla stessa funzione di QUIT
      (`broadcastToNeighbours` + `partChannel` su tutti i canali).

*Risolve:* crash da SIGPIPE, indice sbagliato dopo una disconnessione,
busy loop su POLLHUP, QUIT che perde i messaggi in coda, leak alla chiusura.

### Step 7 — Header e build

- [ ] Forward declaration negli header, include veri nei `.cpp` → niente cicli
      `server.hpp` ↔ `ACommand.hpp`, `parser.hpp` ↔ `ft_irc.hpp`.
- [ ] Uniformare a include guard (oggi misto con `#pragma once`).
- [ ] Makefile: `-MMD -MP` + `-include $(OBJS:.o=.d)`, oggetti in `obj/`.
- [ ] `.gitignore`: `.o` → `*.o` (oggi ignora solo un file chiamato esattamente `.o`), aggiungere `*.d`.

### Ordine e dipendenze

```text
Step 0 (test)
   |
Step 1 (output) ──> Step 2 (helper validazione) ──> Step 3 (registro comandi)
   |
Step 5 (modello dati) ──> Step 4 (operazioni condivise) ──> Step 6 (loop rete)
   |
Step 7 (header/build) — in qualsiasi momento, meglio presto
```

Ogni step è un commit/PR a sé: piccolo, testabile, facile da dividere nel gruppo.
Gli step 1-3 toccano soprattutto i comandi; gli step 4-6 soprattutto `Server`:
due persone possono lavorarci in parallelo.

---

## B. Todo list completa (dalla prima analisi)

Tra parentesi lo step del refactor che la risolve; "—" = fix puntuale.

### 1. Robustezza del server
- [ ] 🔴 SIGPIPE non gestito: client che chiude con dati in coda → server terminato (6)
- [ ] 🔴 `run()`: dopo `i--` il controllo POLLOUT usa il fd sbagliato (`server.cpp:266-272`) (6)
- [ ] 🔴 `POLLHUP`/`POLLERR` non gestiti → client mai rimosso, busy loop (6)
- [ ] 🟠 ritorno di `poll()` non controllato (6)
- [ ] 🟠 `flushClient`: errori diversi da EAGAIN → solo perror, client non disconnesso (6)
- [ ] 🟠 nessun limite al buffer di input (512 byte) (6)
- [ ] 🟠 disconnessione brusca: gli altri membri non ricevono QUIT (6)
- [ ] 🟠 QUIT chiude subito il fd, il send buffer va perso (6)
- [ ] 🟢 `~Server()` non libera client/canali né chiude i socket (6)
- [ ] 🟢 `removeEmptyChan`: null check dopo la dereferenziazione, ricerca lineare (4)

### 2. Parser e dispatch
- [ ] 🔴 doppio errore 451 + 421 per i non registrati (3)
- [ ] 🔴 `"JOIN "` crea un canale senza nome; `nline[0]` / `params[0][0]` su stringa vuota (parser)
- [ ] 🟠 spazi multipli → parametri vuoti (parser)
- [ ] 🟠 prefisso `:nick CMD` non gestito (parser)
- [ ] 🟠 comandi case-sensitive (3)
- [ ] 🟠 riga vuota → 421 invece di essere ignorata (3)
- [ ] 🟠 PASS/USER dopo la registrazione → 421 invece di 462 (3)
- [ ] 🟢 `dispatch` duplicato, `new`/`delete` per riga (3)
- [ ] 🟢 `receiveClient` cerca il client due volte (6)

### 3. Registrazione
- [ ] 🔴 `isValidNickname` rifiuta `_` e `|`, accetta `-` come primo carattere (—)
- [ ] 🟠 nick case-sensitive; NICK con il proprio nick → "already in use" (4)
- [ ] 🟠 `ERR_NICKNAMEINUSE` con nick vuoto → riga malformata (1)
- [ ] 🟠 USER: richiesti 4 parametri, realname ignorato (2, 5)
- [ ] 🟠 password sbagliata: il client resta connesso e può riprovare all'infinito (—)
- [ ] 🟢 blocco WELCOME triplicato in pass/nick/user (4)

### 4. Canali
- [ ] 🔴 inviti/operatori salvati per fd → ereditati da un nuovo client con lo stesso fd (5)
- [ ] 🔴 JOIN su canale in cui si è già → manda una riga vuota (1)
- [ ] 🔴 INVITE: `ERR_USERONCHANNEL` senza `"\r\n"` (`invite.cpp:38`) (1)
- [ ] 🟠 JOIN senza validazione del nome canale (`#`), niente liste con virgole (—)
- [ ] 🟠 MODE: doppio broadcast su `+o`, broadcast senza argomenti, modifiche parziali
      non notificate, `+l 0` notificato come cambiato, query senza 324,
      `MODE <nick> +i` → 403 (—, dopo lo step 2)
- [ ] 🟠 TOPIC: broadcast senza prefisso `nick!user@host` (1)
- [ ] 🟠 INVITE richiede operatore anche se il canale non è +i (—)
- [ ] 🟠 QUIT duplicato a chi condivide più canali (4)
- [ ] 🟢 KICK: `removeOperator`/`removeInvite` ridondanti prima di `removeMember` (4)
- [ ] 🟢 `Channel::setUserLimit` stampa su cout (5)

### 5. Modello dati
- [ ] 🟢 `set<const Client*>` / `set<const Channel*>` + `const_cast` ovunque (5)
- [ ] 🟢 membri come puntatori, operatori/invitati come fd (5)
- [ ] 🟢 `findClient` / `returnClient` duplicati (4)
- [ ] 🟢 stringhe e vector passati per valore (2, 5)
- [ ] 🟢 metodi morti, nomi poco chiari (`topicRes`, `inviteRes`) (5)
- [ ] 🟢 manca hostname/realname nel Client (5)

### 6. Risposte IRC
- [ ] 🟢 `"\r\n"` manuale in ogni chiamata (1)
- [ ] 🟢 sorgente incoerente `"*"` / nick (1)
- [ ] 🟢 `"ircserv"` hardcoded in ogni macro (1)
- [ ] 🟠 PONG: formato corretto `:ircserv PONG ircserv :token` (1)
- [ ] 🟢 `RPL_MODE` con args vuoto lascia uno spazio finale (1)

### 7. Header e build
- [ ] 🟢 include circolari (7)
- [ ] 🟢 `#pragma once` misto a include guard (7)
- [ ] 🟢 `ft_irc.hpp` header "contenitore" (7)
- [ ] 🟢 Makefile senza dipendenze dagli header (7)
- [ ] 🟢 `.gitignore`: `.o` invece di `*.o` (7)

### 8. main e configurazione
- [ ] 🟠 argc sbagliato → esce con 0 senza messaggio (—)
- [ ] 🟠 porta senza limite superiore (> 65535) (—)
- [ ] 🟢 `setupSocket(int _port)` nasconde il membro; backlog 5 → `SOMAXCONN` (—)

### 9. Pulizia e repository
- [ ] 🟢 log misti italiano/inglese, doppi (`cout` + `perror`), log disconnessione stampa l'indice e non il fd (—)
- [ ] 🟢 appunti `.md` nella root → `docs/`; aggiornare `ft_irc_stato_e_todo.md` (—)
- [ ] 🟢 nessun test automatico (0)
