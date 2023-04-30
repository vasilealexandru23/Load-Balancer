<img align="right" src="https://ocw.cs.pub.ro/courses//res/sigla_sd.png" width="150" heigh="150">

**Nume: Vasile Alexandru-Gabriel**  
**Grupă: 314CA**

## Virtual Memory Allocator (Tema 1)

### Descriere:

* Tema propune implementarea unui alocator de memorie care are rolul
de a rezerva memorie, la nivel de biblioteca, traditional prin apeluri
de memorie precum malloc() sau calloc(). Acestea marchează ca fiind folosite anumite zone de memorie dintr-un pool de bytes prealocat, numit arenă. Deasemenea, alocatorul de memorie se ocupă și cu eliberarea zonelor rezervate, apelul de bibliotecă aferent fiind free().

* Descrierea implementarii comenzilor: 
	* **ALLOC_ARENA** : Alocam dinamic memoria folosita pentru structura de arena
	si pentru lista dubla inlantuita in care vom pastra nodurile, care vor contine
	structuri de tip block_t.

	* **DEALLOC_ARENA** : Eliberam absolut toate resursele folosite de la inceputul
	programului. Parcurgem fiecare lista in parte si eliberam, pe rand, fiecare
	nod impreuna cu datele din el.

	* **ALLOC_BLOCK** : Marcam o noua zona de memorie in arena, cu datele extrase
	din inputul dat de utilizator. Verificam daca datele extrase sunt valide, iar
	in caz contrar afisam un mesaj de eroare. Mai departe, creez structura de block
	impreuna cu cea de miniblock, din interiorul ei. Cum nodurile din lista de
	blockuri sunt ordonate (dupa adresa de start), trebuie sa aflam pozitia pe care
	vom insera nodul in lista de blockuri. Mai departe, verificam tangenta la stanga
	si dreapta. Pentru tangenta la dreapta, stergem blockul respectiv din lista de
	blockuri si facem conexiunile manual dintre listele de miniblockuri. Adaugam
	noul block in lista de blockuri. Pentru tangenta la stanga, sterem blockul abia
	adaugat si facem conexiunile necesare intre miniblockuri. In ambele cazuri de
	tangenta avem grija si dam update la sizeurile blockurilor si listelor de
	miniblockuri.

	* **FREE BLOCK** : In aceasta functie cautam blockul care contine miniblockul
	cu adresa data de utilizator. In cazul in care cautarea s - a terminat si blockul nu a fost gasit afisam un mesaj de eroare. Dupa ce am gasit blockul care contine miniblockul corespunzator, distingem doua cazuri. Primul il constituie cel in care miniblockul se afla fie la inceputul fie la sfarsitul 
	blockului curent, caz in care il eliminam din lista de miniblockuri, actualizam dimensiunea blockului din care provine si verifiam daca blockul a ramas gol sau nu, caz pentru care, daca a ramas gol, il stergem din lista de blockuri si
	eliberam memoria alocata. Pentru al doilea caz, avem miniblockul in mijlocul listei de miniblockuri. Construim un nou block, in care adaugam manual lista de miniblockuri corespunzatoare (cea care incepe de la nodul urmator celui sters)
	si il adaugam in lista de blockuri.

	* **CHECK_VALID_ZONE** : Verificam daca miniblockul de la care incepe adresa data de utilizator este valid(are permisiuni de citire) si este suficient de "mare", adica acopera intreg intervalul dat de utilizator pentru afisare (in caz contrarafisam mesajul de overflow).

	* **READ** : Verificam daca miniblockul de la care incepe adresa data de
	utilizator este valid(prin functia descrisa anterior). Parcurgem blockurile
	pana la cel care contine miniblockul dat de utilizator. Urmeaza parcurgerea
	miniblockurilor si afisarea caracter cu caracter a datelor din rw_buffer.

	* **WRITE** : Verificam daca miniblockul de la care incepe adresa data de
	utilizator este valid(prin functia descrisa anterior). Parcurgem blockurile
	pana la cel care contine miniblockul dat de utilizator. Urmeaza parcurgerea
	miniblockurilor si introducerea caracter cu caracter a datelor in rw_buffer.

	* **PMAP** : Functie prin care afisam toate informatiile despre arena incarcata
	in memorie. Parcurgem toate blockurile, afisam informatiile cerute de utilizator
	impreuna cu miniblockurile in formatul impus.

	* **MPROTECT** : Functie prin care atribuim noi permisiuni unui anumit miniblock. Incepem prin a cauta miniblockul cu adresa impusa de utilizator
	prin parcurgerea blockurilor, apoi pentru fiecare parcurgem lista de miniblockuri, iar in cazul gasirii miniblockului respectiv, schimbam permisiunile. Daca miniblockul nu a fost gasit, afisam la final un mesaj de eroare.

* **Organizarea fisierelor** : In organizarea fisierelor, am pastrat strict comenzile descrise in tema in fisierul original (vma.c). Restul comenzilor ajutatoare functiilor din tema se afla in fisiere diferite (arena_utils.c,
block_utils.c).


### Comentarii asupra temei:

* Crezi că ai fi putut realiza o implementare mai bună?
	* Se pot face diverse optimizari :
		* Adaugarea in structura de lista a unui pointer la sfarsitul listei
		(dll_node_t *tail), pentru a evita parcurgerile in scopul aflarii ultimului
		nod dintr-o lista.
		* Se puteau gasi formule pentru adaugarea in rw_buffer, dar crestea
		ambiguitatea programului si riscul la eroarea unor case-uri imprevizibile.
* Ce ai invățat din realizarea acestei teme?
	* Cum functioneaza un alocator de memorie virtual.
* Alte comentarii
	* Descrierea temei este sumara. Exista teste, in care utilizatorul poate
	da comenzi imprevizibile (incorecte), despre care nu se vorbeste in tema.
	(Nu este specificat cat de mult poate gresi un utilizator in folosirea
	programului).

### (Opțional) Resurse / Bibliografie:
