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

	* **loader_retrieve(char\* key)** :  Function by which a product is retrieved
	from load balancer with a given key. It is done by finding the first position on hashring whose server has a bigger hash then the hash of the key. And then calling
	the function that get a value from a hashmap.

	* **loader_add_server(int server_id)** :  Function that adds a new server to the load balancer with a given id together with the two duplicates. It is done by
	finding for every id the position where to insert the new server (duplicate) in
	the hashring, and then then redistributing the elements from the following server
	(consistent hashing).

	* **loader_remove_server(int server_id)** : Function that removes a given server from the load balancer together with all the duplicates. It is done by
	finding for every id the position from where to erase the server (duplicate) in
	the hashring, and then redistributing the elements from the current server
	to the following one. After the current server is empty (all elements have been redistributed) we erase the server from memory (and from hashring).

	* **server_store(char\* key, char\* value)** : Function that stores a pair <key, value> in a server.

	* **server_retrieve(char\* key)** : Function that retrieves value from a pair <key, value> from a server. 

	* **server_remove(char\* key)** : Function that removes a pair <key, value> from a server.

#