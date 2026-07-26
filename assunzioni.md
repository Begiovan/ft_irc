Qui dentro metto ciò che do per scontato lato client:

c'è un metodo getFd che restituisce l'fd del client 


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