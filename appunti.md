# Aggiornamento 22/07 9:44

**syqe iki**

## Spiegazione creazione socket

1. **Creare un socket usando la funzione `socket()`**, questo ritorna un fd.
   I parametri passati alla funzione sono:
   - **Domain**: quindi la famiglia di indirizzi (in questo caso IPv4) `AF_INET`.
     Se invece si voleva IPv6 allora era `AF_INET6`.
   - **Type**: in questo caso TCP (`SOCK_STREAM`), questo perché crea un flusso stabile
     e ordinato. Se avessimo usato `SOCK_DGRAM` sarebbe stato UDP, questo protocollo
     non da garanzia di ordine/consegna e sarebbe stato un macello.
   - **Protocol**: in questo caso 0 così il kernel sceglie quello che vuole (ovviamente TCP).

2. **Preparare la struttura dell'indirizzo**
   Prima di passare a `bind()` i parametri, serve una struttura definita che specifica quale
   indirizzo e porta deve ascoltare.
   - `sin_family` = `AF_INET` (deve essere uguale a quella del socket)
   - `sin_port` = la porta sulla quale deve ascoltare, si usa `htons` perché deve essere convertita in byte
   - `sin_addr.s_addr` = quale interfaccia deve usare, si usa `INADDR_ANY` (0.0.0.0) così da avere tutte le connessioni.
     Nel caso si potrebbe usare una a piacere, ma poi sarebbe un macello.

3. **Creare il `bind()`**
   Una volta impostati tutti i parametri possiamo passarli a `bind()`. `bind()` si occupa di legare indirizzo/porta specifici
   così che il kernel sa dove far arrivare i pacchetti.
   - `serverSocket`, che sarebbe fd creato con `socket()`
   - `serverAddress`, che sarebbe il puntatore alla struttura creata prima.
     Va castata a `struct sockaddr*` perché è l'interfaccia generica che il kernel si aspetta.
   - la dimensione della struttura, così il kernel sa quanti byte leggere.

4. **Impostare un `listen()`**
   Semplicemente prende il socket creato e quante connessioni il kernel può mettere in coda.
   > **ATTENZIONE**: non sarebbe il numero massimo di client, quello deve essere gestito dopo con `poll()`.

**ps.**
Ho messo uno `sleep()` per testare la connessione aprendo un altro terminale e facendo eseguire:

```
lsof -i :<porta>   OPPURE   netstat -tlnp | grep <porta>
```

La porta sarebbe quella messa dentro `htons()`.
Senza lo `sleep()` il programma si chiuderebbe subito perché non va in loop, per quello serve `accept()` che sto studiando adesso.

---

# Aggiornamento 22/07 20:08

**syqe iki**

Adesso ci sono una marea di informazioni nuove:

Abbiamo creato il poll di socket e funziona benissimo, puoi tranquillamente aprire più terminali
e funziona benissimo, tutti i messaggi arrivano al server.

Nel caso volessi provare fai `nc 127.0.0.1 8080` e ti apre un client nuovo, puoi fare lo stesso comando su più terminali e funziona.

## Spiegazione di poll() fatta bene

Cosa andiamo a fare nella nuova versione del codice: semplicemente andiamo a creare un vector di fd.

Dentro questo vector andiamo ad impostare tutti i parametri che servono a poll tra cui l'evento `POLLIN` (il più importante).

Questo evento sarebbe ogni volta che lui riceve qualcosa (nel nostro caso sia una nuova connessione che un messaggio).

Poi usiamo un ciclo infinito per monitorare tutti i nostri fd. Dentro questo noi creiamo un nuovo `poll()` che punta al primo oggetto di vector.
Il terzo parametro di `poll()` sarebbe il timeout o una cosa simile, impostato a `-1` gli dici di controllare quando succede qualcosa, sennò `0` finisce subito e numeri positivi lo usa come timeout.

Poi gli diciamo di controllare tutti gli fd e vedere se succede `POLLIN`, nel caso lui vede se si tratta di una nuova connessione allora fa `accept` e aggiunge fd al vector, altrimenti `recv` e scrive il messaggio ricevuto.