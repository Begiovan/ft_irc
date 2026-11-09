# ft_irc — Piano di refactor

> Obiettivo: eliminare la ridondanza **senza cambiare la logica** del programma.
> L'idea di fondo è spostare in pochi punti comuni ciò che oggi è ripetuto in ogni comando:
> invio delle risposte, controlli di validazione, ingresso/uscita dai canali, notifiche.
> Molti bug della todo list spariscono "gratis" come effetto collaterale.

Legenda priorità: 🔴 bug che si vede o rompe qualcosa · 🟠 comportamento sbagliato rispetto al protocollo IRC · 🟢 refactor e pulizia, che non cambiano il comportamento.
Ogni voce riporta tra parentesi la tematica di origine (Robustezza, Parser, Registrazione, Canali, Modello dati, Risposte IRC, Header e build, main, Pulizia).

---

## A. Orientamento del refactor

### In sintesi

La ridondanza nasce dal fatto che ogni comando riscrive a mano tre cose che dovrebbero stare in un posto solo.

1. **Come si manda una risposta.** Oggi ogni chiamata aggiunge `"\r\n"` a mano e sceglie da sé `"*"` o il nick. Se lo fanno `sendToClient`, `Client::nickOrStar()` e `Client::prefix()` (che produce `nick!user@host`), spariscono diversi bug: INVITE senza `\r\n`, JOIN che manda una riga vuota, TOPIC senza prefisso.
2. **Come si valida una richiesta.** Quasi tutti i comandi ripetono la stessa catena: parametri sufficienti, canale esistente, utente membro, utente operatore. Spostata in pochi helper di `ACommand` (`needParams`, `needChannel`, `needMember`, `needOperator`, `needNick`), che mandano l'errore e ritornano false, KICK, INVITE, TOPIC e MODE si accorciano di circa metà. Lo stesso vale per `dispatch`: al posto delle due catene di `if` basta una mappa di comandi creata una volta sola. Così si risolvono anche il doppio errore per chi non è registrato e i comandi in minuscolo.
3. **Cosa succede nel modello.** Entrare o uscire da un canale, avvisare chi condivide un canale con te e completare la registrazione sono sequenze copiate in più file. Le differenze tra le copie sono proprio i bug: il QUIT duplicato, il KICK ridondante, il WELCOME scritto tre volte. Diventano funzioni di `Server` come `joinChannel`, `partChannel`, `broadcastToNeighbours` e `tryCompleteRegistration`.

Gli altri due punti chiave:

- **Modello dati.** Oggi `Channel` salva i membri come puntatori ma operatori e invitati come fd. Usando `Client*` ovunque spariscono i `const_cast` e anche il bug degli inviti ereditati da un nuovo client che riceve lo stesso fd.
- **Disconnessioni.** Con lo schema "mark & sweep" il client viene solo segnato come da disconnettere e chiuso a fine giro di `poll`. Questo elimina il `i--` in `run()`, il QUIT che perde i messaggi in coda e la disconnessione brusca che non avvisa i canali.

Prima di tutto c'è lo **step 0**: uno script di test con `nc` da rilanciare dopo ogni step. È l'unico modo per verificare che la logica non sia cambiata.

Gli step 1-3 toccano soprattutto i comandi, gli step 4-6 soprattutto `Server`, quindi due persone del gruppo possono lavorarci in parallelo. Ogni step si può chiudere in un commit a sé.

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
| cercare un client per nick         | findClientByNick + getClientByNick                      |

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

**Voci della todo list risolte da questo step:**

- [ ] *(Pulizia e repository)* 🟢 Mancano dei test anche minimi, per esempio uno script `nc` con le sequenze di registrazione, JOIN, MODE e KICK, da rilanciare dopo il refactor.

### Step 1 — Un solo punto di uscita per i messaggi

**Problema:** `"\r\n"` aggiunto a mano ovunque (già dimenticato in INVITE), sorgente
incoerente (`"*"` vs nick), `"ircserv"` e `"@ircserv"` scritti nelle macro.

- [x] `Server::sendToClient` (o un nuovo `reply()`) aggiunge **lui** `"\r\n"`.
      Le macro in `irc_response.hpp` non lo contengono e i comandi non lo scrivono più.
- [x] `Client::nickOrStar()` → nick, oppure `"*"` se ancora vuoto.
      Usarlo come `source` in **tutte** le risposte numeriche.
- [x] `Client::prefix()` → `nick!user@host`, dove `host` è la costante `SERVER_NAME` (resta fisso a `ircserv` come oggi: **nessun campo hostname nel `Client`**). Le macro "Command Responses"
      prendono un solo parametro `prefix` invece di `(source, user)`.
- [x] `#define SERVER_NAME "ircserv"` in un solo punto.

**Voci della todo list risolte da questo step:**

- [x] *(Registrazione)* 🟠 `ERR_NICKNAMEINUSE` usa `getNickname()`, che prima della registrazione è vuoto, e la riga esce malformata. Va usato `*`.
- [x] *(Canali e comandi di canale)* 🔴 **JOIN su un canale in cui sei già** (`join.cpp:40`): il messaggio d'errore resta vuoto e al client arriva solo `"\r\n"`.
- [x] *(Canali e comandi di canale)* 🔴 **INVITE, `ERR_USERONCHANNEL` senza `"\r\n"`** (`invite.cpp:38`): il messaggio si attacca al successivo.
- [x] *(Canali e comandi di canale)* 🟠 Il broadcast di TOPIC non ha il prefisso `nick!user@host`, a differenza degli altri messaggi. Conviene creare una macro `RPL_TOPICCHANGE`.
- [x] *(Risposte IRC)* 🟢 **`"\r\n"` aggiunto a mano in ogni chiamata**, ed è già stato dimenticato una volta (INVITE). Conviene che lo aggiunga `sendToClient`, oppure un helper `reply()`.
- [x] *(Risposte IRC)* 🟢 Il nick sorgente nelle risposte non è coerente: a volte `"*"`, a volte `getNickname()` (vedi PRIVMSG e PASS). Serve un helper `nickOrStar(client)`.
- [x] *(Risposte IRC)* 🟢 Il nome del server `ircserv` è scritto dentro ogni macro. Si può raccogliere in una costante, oppure passare dalle macro a funzioni inline.
- [ ] *(Risposte IRC)* 🟠 PONG dovrebbe essere `:ircserv PONG ircserv :token`.
- [ ] *(Risposte IRC)* 🟢 `RPL_MODE` con argomento vuoto lascia uno spazio in coda alla riga.s

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

**Voci della todo list risolte da questo step:**

- [ ] *(Registrazione)* 🟠 `USER` controlla un solo parametro, ma il protocollo ne richiede 4: con meno parametri va risposto `461 ERR_NEEDMOREPARAMS`. Il realname (4° parametro) non va salvato: non lo usa nessun comando richiesto dal subject.
- [ ] *(Modello dati)* 🟢 Molti metodi ricevono `std::string` per valore (`setNickname`, `appendBuffer`, `findClientByNick`…) e `execute` riceve un `vector` per valore. Meglio passare `const &`. *(completato nello Step 5)*

### Step 3 — Registro dei comandi al posto di `dispatch`

**Problema:** due catene di `if` quasi identiche, `isAuth &&` ridondante,
`new`/`delete` a ogni riga, doppio errore per i non registrati.

- [x] `std::map<std::string, ACommand*> _commands` creata **una volta** nel costruttore
      di `Server` e liberata nel distruttore.
- [x] Ogni `ACommand` sa se richiede la registrazione (`requiresRegistration()`).
- [x] Nuovo dispatch in un solo punto:

```text
riga vuota                  -> ignora
comando non in mappa        -> 421 ERR_UNKNOWNCOMMAND
richiede reg. e non reg.    -> 451 ERR_NOTREGISTERED
altrimenti                  -> cmd->execute(...)
```

- [x] Il nome del comando va convertito in maiuscolo nel parser.

**Voci della todo list risolte da questo step:**

- [x] *(Parser e dispatch dei comandi)* 🔴 **Errore doppio per i client non registrati.** `dispatch` invia già `ERR_NOTREGISTERED` e ritorna `NULL`, poi `receiveClient` aggiunge anche `ERR_UNKNOWNCOMMAND`.
- [x] *(Parser e dispatch dei comandi)* 🟠 I comandi sono case-sensitive: `join` non viene riconosciuto.
- [ ] *(Parser e dispatch dei comandi)* 🟠 Una riga vuota riceve `ERR_UNKNOWNCOMMAND`; andrebbe ignorata in silenzio.
- [x] *(Parser e dispatch dei comandi)* 🟠 `PASS` e `USER` dopo la registrazione rispondono "unknown command" invece di `462 ERR_ALREADYREGISTERED`, perché non compaiono nella lista dei comandi per utenti autenticati.
- [x] *(Parser e dispatch dei comandi)* 🟢 `dispatch` contiene due liste quasi uguali e un `isAuth &&` ripetuto. Si può sostituire con una `std::map<std::string, ACommand*>` creata una volta nel costruttore, più un set dei comandi ammessi prima della registrazione. Si elimina anche il `new`/`delete` a ogni riga.

### Step 4 — Operazioni di dominio condivise in `Server`

**Problema:** stesse sequenze riscritte in più comandi, con piccole differenze
(che sono proprio i bug).

- [x] `void joinChannel(Client&, Channel&)` → addMember + addChannel + rimozione invito.
- [x] `void partChannel(Client&, Channel&)` → removeMember + removeChannel + `removeEmptyChan`.
      Usato da KICK, QUIT, disconnect (e PART se lo aggiungete).
- [x] `void broadcastToNeighbours(Client&, const std::string&, bool includeSelf)`
      → raccoglie i membri di tutti i canali in un `set` (il pattern che c'è già in NICK)
      e invia una volta sola. Usato da NICK e QUIT.
- [x] `void sendTopic(Client&, Channel&)` e `void sendNames(Client&, Channel&)`
      → usati da JOIN e TOPIC.
- [x] `void tryCompleteRegistration(Client&)` → manda RPL_WELCOME se la registrazione
      si è appena completata. Usato da PASS, NICK, USER.
- [x] `Client *getClientByNick(const std::string&)` al posto di `findClientByNick` + `getClientByNick`
      (confronto case-insensitive).

**Voci della todo list risolte da questo step:**

- [ ] *(Robustezza del server)* 🟢 `removeEmptyChan`: il controllo `!channel` arriva dopo che il puntatore è già stato usato (`channel->empty()`). Inoltre cerca il canale con un ciclo invece di usare `_channels.erase(name)`.
- [ ] *(Registrazione)* 🟠 Il controllo del nick è case-sensitive: `Sus` e `sus` possono coesistere. Inoltre `NICK` con il proprio nick attuale risponde "already in use".
- [ ] *(Registrazione)* 🟢 Il blocco "se ora è registrato manda `RPL_WELCOME`" è ripetuto in pass, nick e user. Si può estrarre in `Server::tryCompleteRegistration(Client*)`.
- [ ] *(Canali e comandi di canale)* 🟠 QUIT manda il messaggio una volta per ogni canale: chi condivide due canali con te lo riceve due volte. NICK invece raccoglie i destinatari in un set, ed è il pattern giusto da riusare.
- [ ] *(Canali e comandi di canale)* 🟢 KICK chiama `removeOperator` e `removeInvite` prima di `removeMember`, che fa già entrambe le cose.
- [ ] *(Modello dati)* 🟢 `findClientByNick` e `getClientByNick` sono duplicati: ne basta uno solo, `Client* getClientByNick(const std::string&)`. In più si può valutare una mappa nick → Client.

### Step 5 — Modello dati coerente (togliere i `const_cast`)

- [OK] `Channel`: `_members`, `_operators`, `_invited` tutti come `std::set<Client*>`
      (oggi membri = puntatori, operatori/invitati = fd).
- [OK] `Client`: `std::set<Channel*>`, getter `const` (`getNickname() const` ecc.).
- [OK] Rimuovere ogni `const_cast` (broadcast, nick, join, disconnect).
- [OK] `Channel` senza I/O: `setUserLimit` ritorna `bool` invece di stampare.
- [OK] Rimuovere i metodi inutilizzati (`setFd`, `clearBuffer`, `clearSendBuffer`),
      rinominare `topicRes()`/`inviteRes()` → `isTopicRestricted()`/`isInviteOnly()`.

**Voci della todo list risolte da questo step:**

- [OK] *(Canali e comandi di canale)* 🔴 **Inviti e operatori sono salvati per fd.** Se un utente invitato si disconnette senza essere mai entrato, il prossimo client che riceve lo stesso fd eredita l'invito. Conviene usare `Client*` come per `_members` (vedi sezione 5).
- [OK] *(Canali e comandi di canale)* 🟢 `Channel::setUserLimit` stampa su `cout`. Il modello non dovrebbe fare I/O: meglio che ritorni un `bool`.
- [OK] *(Modello dati)* 🟢 **Puntatori `const` usati ovunque e poi forzati con `const_cast`.** `Channel` tiene `set<const Client*>`, `Client` tiene `set<const Channel*>`, e così servono `const_cast` in broadcast, nick, join e disconnect. Si risolve salvando `Client*` e `Channel*` non const e rendendo const i getter (`getNickname()`, `getUsername()`, `isInChannel()`).
- [OK] *(Modello dati)* 🟢 `Channel` usa una rappresentazione mista: i membri sono puntatori, operatori e invitati sono fd. Conviene uniformare tutto a `Client*`.
- [OK] *(Modello dati)* 🟢 Il codice morto (`setFd`, `clearBuffer`, `clearSendBuffer`) è da togliere o da usare. I nomi `topicRes()` e `inviteRes()` sono poco chiari: meglio `isTopicRestricted()` e `isInviteOnly()`.
- [OK] *(Modello dati)* 🟠 **Send buffer del bot che cresce all'infinito.** `chanbot` è membro di ogni canale, quindi `broadcast` gli accoda ogni messaggio, ma il bot non è in `_fds` e il suo buffer non viene mai svuotato. Basta che `broadcast`/`sendToClient` saltino i client con `isBot()`.

### Step 6 — Loop di rete più semplice e sicuro

**Problema:** `run()` modifica `_fds` mentre lo scorre, `i--` con controlli successivi
sull'indice sbagliato, `QUIT` chiude il fd mentre il comando è ancora in esecuzione.

- [x] Pattern **"mark & sweep"**: `Client::markForDisconnect()` invece di chiudere subito;
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

**Voci della todo list risolte da questo step:**

- [ ] *(Robustezza del server)* 🔴 **SIGPIPE non gestito.** Se un client chiude la connessione mentre ha dati in coda, `send()` genera SIGPIPE e il server si chiude. Va ignorato con `signal(SIGPIPE, SIG_IGN)` nel `main`, oppure con `SO_NOSIGPIPE` sul socket (su macOS).
- [ ] *(Robustezza del server)* 🔴 **`run()`, indice sbagliato dopo una disconnessione** (`server.cpp:266-272`). Dopo `i--`, il controllo `POLLOUT` successivo usa `_fds[i]`, cioè il fd precedente. Basta un `continue` dopo `i--`.
- [ ] *(Robustezza del server)* 🔴 **`POLLHUP`/`POLLERR` non gestiti.** Se `revents` contiene solo `POLLHUP`, il client non viene mai rimosso e `poll` continua a ritornare subito (busy loop).
- [ ] *(Robustezza del server)* 🟠 Il valore di ritorno di `poll()` non è controllato (`-1`, `EINTR`).
- [ ] *(Robustezza del server)* 🟠 `flushClient`: se `send` fallisce con un errore diverso da `EAGAIN` fa solo `perror`. Il client andrebbe disconnesso.
- [ ] *(Robustezza del server)* 🟠 Nessun limite al buffer di input: un client che manda dati senza mai inviare `\n` fa crescere la memoria all'infinito. Il protocollo IRC fissa le righe a 512 byte, quindi conviene troncare o disconnettere.
- [ ] *(Robustezza del server)* 🟠 `disconnectClient` (chiusura brusca, senza `QUIT`) toglie il client dai canali ma non avvisa gli altri membri con un messaggio `QUIT`.
- [ ] *(Robustezza del server)* 🟠 `QUIT` chiude subito il fd: tutto quello che è ancora nel send buffer va perso.
- [ ] *(Robustezza del server)* 🟢 `~Server()` non libera `_clients` e `_channels` e non chiude i socket. Manca anche uno shutdown pulito con SIGINT.
- [ ] *(Parser e dispatch dei comandi)* 🟢 `receiveClient` cerca il client due volte (`it` e `clientIt`). Conviene estrarre `processLine(Client*, line)`.

### Step 7 — Header e build

- [ ] Forward declaration negli header, include veri nei `.cpp` → niente cicli
      `server.hpp` ↔ `ACommand.hpp`, `parser.hpp` ↔ `ft_irc.hpp`.
- [ ] Uniformare a include guard (oggi misto con `#pragma once`).
- [ ] Makefile: `-MMD -MP` + `-include $(OBJS:.o=.d)`, oggetti in `obj/`.
- [ ] `.gitignore`: `.o` → `*.o` (oggi ignora solo un file chiamato esattamente `.o`), aggiungere `*.d`.

**Voci della todo list risolte da questo step:**

- [ ] *(Header e build)* 🟢 Include circolari: `server.hpp` include `ACommand.hpp`, che include `server.hpp`; `parser.hpp` e `ft_irc.hpp` si includono a vicenda. Basta usare forward declaration negli header e spostare gli include nei `.cpp`.
- [ ] *(Header e build)* 🟢 Lo stile delle protezioni degli header è misto: `#pragma once` in alcuni file, include guard in altri. Meglio uniformare con le include guard.
- [ ] *(Header e build)* 🟢 `ft_irc.hpp` è un header "contenitore" che include tutto: conviene ridurlo.
- [ ] *(Header e build)* 🟢 Il Makefile non tiene traccia delle dipendenze dagli header: se modifichi un `.hpp`, i `.o` non vengono ricompilati. Si risolve con `-MMD -MP` e `-include $(OBJS:.o=.d)`, eventualmente mettendo gli oggetti in una cartella `obj/`.
- [ ] *(Header e build)* 🟢 Nel `.gitignore` c'è `.o` invece di `*.o`: ignora solo un file chiamato esattamente `.o`, quindi i file oggetto non vengono ignorati. Per ora nel repo non ne sono finiti.

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

---

## B. Fix puntuali (fuori dal refactor)

Correzioni che non dipendono dagli step: si possono fare in qualsiasi momento, anche prima del refactor.

- [ ] *(Parser e dispatch dei comandi)* 🔴 **Parser con righe vuote o spazi in fondo.** `"JOIN "` produce un parametro vuoto e crea un canale senza nome. `nline[0]` su una stringa vuota è comportamento indefinito, e lo stesso vale per `params[0][0]` in PRIVMSG e NOTICE.
- [ ] *(Parser e dispatch dei comandi)* 🟠 Spazi multipli generano parametri vuoti.
- [ ] *(Parser e dispatch dei comandi)* 🟠 Il prefisso iniziale (`:nick CMD ...`) non è gestito.
- [ ] *(Registrazione)* 🔴 **`isValidNickname` rifiuta `_` e `|`.** Molti client, irssi compreso, aggiungono `_` quando il nick è occupato, e qui ricevono `432 ERR_ERRONEUSNICKNAME`. Il `-` invece non dovrebbe essere ammesso come primo carattere.
- [ ] *(Registrazione)* 🟠 L'ordine non è imposto: NICK e USER sono accettati prima di PASS. Con la password sbagliata il client resta connesso e può riprovare all'infinito.
- [ ] *(Canali e comandi di canale)* 🟠 JOIN non valida il nome del canale: `JOIN pippo` crea un canale senza `#`. Non supporta nemmeno le liste separate da virgola (`#a,#b chiave1,chiave2`).
- [ ] *(Canali e comandi di canale)* 🟠 **MODE**: *(meglio farlo dopo lo Step 2, con gli helper di validazione)*
  - con `+o` fa un broadcast subito, poi alla fine ne fa un altro con l'intera stringa `params[1]`, senza argomenti;
  - se c'è un errore a metà, le modifiche precedenti restano applicate ma nessuno viene avvisato;
  - `+l 0` stampa "invalid" su stdout ma fa comunque il broadcast come se fosse cambiato qualcosa;
  - `MODE #canale` (interrogazione) dovrebbe rispondere con `324 RPL_CHANNELMODEIS`;
  - `MODE <nick> +i`, che molti client inviano dopo la registrazione, riceve `ERR_NOSUCHCHANNEL`.
- [ ] *(Canali e comandi di canale)* 🟠 INVITE richiede di essere operatore anche se il canale non è `+i`.
- [ ] *(main e configurazione)* 🟠 Con un numero sbagliato di argomenti il programma esce con `0` senza nessun messaggio. Serve un messaggio d'uso e il ritorno `1`.
- [ ] *(main e configurazione)* 🟠 La porta non ha limite superiore: `99999` passa il controllo e poi viene troncata da `htons`.
- [ ] *(main e configurazione)* 🟢 `setupSocket(int _port)` ha un parametro con lo stesso nome del membro e lo nasconde. In più il backlog di `listen` è 5: meglio `SOMAXCONN`.
- [ ] *(Pulizia e repository)* 🟢 I log sono misti tra italiano e inglese e spesso doppi (`cout` + `perror`). Anche il log di disconnessione stampa l'indice nel vettore invece del fd.
- [ ] *(Pulizia e repository)* 🟢 Gli appunti (`domande.md`, `appunti.md`, `ft_irc_stato_e_todo.md`) sono nella root: si possono spostare in `docs/`. Anche `ft_irc_stato_e_todo.md` andrebbe aggiornato, perché alcune voci non corrispondono più al codice.

---

## C. Ordine consigliato

Prima i 🔴: SIGPIPE e il ciclo di `run()` (Step 6), il parser e i caratteri del nick (sezione B) sono quelli che si vedono durante una valutazione. I 🔴 della sezione B si possono sistemare subito; quelli che stanno dentro uno step conviene affrontarli insieme allo step.

Poi lo Step 5 (modello dati) e lo Step 1 (risposte IRC): uniformare i puntatori e aggiungere `"\r\n"` in automatico semplifica quasi tutti i comandi senza cambiarne la logica.
