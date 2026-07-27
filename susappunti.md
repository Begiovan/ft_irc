# Aggiornamento 26 Luglio

La classe channel è abbastanza intuitiva quindi non ho aggiunto commenti a riguardo.

## Ipotetico flusso join channel

il server crea il channel se non esiste e aggiunge il membro richiesto con addmember ( metodo pubblico del chan )
il primo membro è anche op ( addOperator )


flusso join

JOIN #general
        │
        ▼
Server trova il canale?
        │
 ┌──────┴──────┐
 │             │
NO            SI
 │             │
 ▼             ▼
crea       usa quello
Channel
 │
 ▼
checkJoin()
 │
 ▼
addMember()
 │
 ▼
broadcast JOIN
 │
 ▼
NAMES
 │
 ▼
TOPIC

# Aggiornamento 27 Luglio 

Client: 

enum per le flag di registrazione l'ho creato come bitmask ( suggerimento di chat ) in modo da usare '|' e '&' bitwise

quindi 
1 = 001
2 = 010
4 = 100

quando poi andremo a verificare lo stato completo di registrazione il risultato sarà

111

_isOnline si salverà lo stato del client

