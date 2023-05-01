<img align="right" src="https://ocw.cs.pub.ro/courses//res/sigla_sd.png" width="150" heigh="150">

**Name: Vasile Alexandru-Gabriel**  
**Group: 314CA**

## Load Balancer (Homework 2)

### Description:

* The topic consists in the implementation of a **Load Balancer** using **Consistent Hashing**. This is a frequently used mechanism in distributed systems and has the advantage of fulfilling the *minimal disruption constraint*, i.e. minimizing the number of transfers required when a server is stopped or one is started. Specifically, when a server is down, only the objects on that server need to be redistributed to nearby servers. Similarly, when a new server is added, it will only fetch objects from a limited number of servers, the neighboring ones. 

* Description of the implementation of the commands:
	* **loader_store(char \*key, char \*value)** : Function by which a product is stored on one of the available servers. It is done by searching on the hashring the server whose hash is higher than the hash of the key of the object to be
	added (with binary search) and then calling the function that store an object
	into a hashmap.

	* **loader_retrieve(char\* key)** : 

	* **loader_add_server(int server_id)** : 

	* **loader_remove_server(int server_id)** : 

	* **server_store(char\* key, char\* value)** : 

	* **server_retrieve(char\* key)** : 

	* **server_remove(char\* key)** : 

* **Organizarea fisierelor** : 

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