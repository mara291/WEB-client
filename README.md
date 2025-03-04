Am inceput implementarea temei prin crearea in main a unui meniu in care
utilizatorul poate introduce comenzi. Pentru fiecare comanda am creat o
functie. Am declarat global cookie-ul si token-ul, acestea putand fi usor
accesate de functiile care le modifica. Pana la intoducerea comenzii exit,
utilizatorul poate apela oricare dintre fucntiile din enunt, sau poate
introduce o comanda gresita, care ii va da din nou sansa de a introduce o
comanda.

**register** -> Utilizatorul introduce username si parola. Se face un POST
request, se trimite mesajul si se verifica raspunsul. Daca acesta contine
cuvantul "error", se afiseaza un mesaj custom cu detaliile erorii, in caz
contrar, se afiseaza un mesaj tip "SUCCESS".

**login** -> Utilizatorul introduce username si parola. Se face un POST request
si se verifica raspunsul. In cazul in care nu sunt erori, se salveaza valoarea
cookie-ului in variabila cookie declarata global.

**enter_library** -> Se face un GET request si se verifica raspunsul. In cazul
in care nu sunt erori, se salveaza valoarea token-ului in variabila token
declarata global.

**get_books** -> Se face un GET request, iar din raspuns se extrag datele cartilor
si se afiseaza pentru fiecare ID-ul si titlul.

**get_book** -> Utilizatorul introduce ID-ul cartii. Se face un GET request, iar
din raspuns se extrag detaliile cartii si se afiseaza in ordine.

**add_book** -> Utilizatorul introduce detaliile cartii pe care vrea sa o adauge,
apoi se face un POST request catre server.

**delete_book** -> Utilizatorul introduce ID-ul cartii pe care vrea sa o stearga,
apoi se face un DELETE request.

**logout** -> Se face un GET request, iar dupa delogare cookie-ul si token-ul
devin NULL.