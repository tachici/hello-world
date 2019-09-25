//Tachici Andrei-Alexandru 331CB

TEMA2 APD:

	Database.java :
Incapsuleaza: - un ExecutorService de tipul ThreadPoolExecutor.
			  - HashMap<String,Table> unde K = numele tabelei
			  si V = o referinta catre un Table care extinde
			  de fapt ArrayList<ArrayList<Object>>.

Implementare metode:

initDb(numWorkerThreads):
	Creeaza un ThreadPoolExecutor a carui numar de thread-uri
	active este egal cu numarul de thread-uri maxime specificate
	in apelul metodei.

createTable(tableName, columnNames, columnTypes):
	- adauga un Table in hashMap-ul din Database
	- un tabel este un ArrayList<ArrayList<Object>> care contine
	linii ale tabelului.
	- numele unei coloane este mapat la un anumit indice folosind
	un hashMap unde K = nume coloana V = indice al coloanei.

select(tableName, operations, condition):
	- Intoarce o lista de coloane cate sunt in lista de
	operations. Elementele coloanelor apartin intrarilor care
	satisfac condition.
	- metoda select paralelizeaza initial gasirea acelor intrari
	in tabel care satisfac conditia data.
	- Creeaza dataBase.nrThreads obiecte de tipul ConditionCheckRequest
	care implementeaza interfata Callable<ArrayList<Integer>>. Primesc
	in construcor un indice de start si un indice de end. Intre acestia
	trebuie sa verifice daca intrarile din tabel de la pozitiile [start,end),
	satisfac conditia data.
	- In select aceste obiecte sunt trimise la ExecutorService, iar 
	rezultatul este concatenat.
	- ArrayList-ul obtinut contine indicii intrarilor din tabela
	care satisfac conditia data.
	- Acest vector va fi impartit la numThread-uri pentru a 
	executa operatiile trimise in array-ul operations.
	- Pentru fiecare: [op1, op2, .. opn] creeaza database.nrThreads
	obiecte de tipul OperationExecutor care primesc un constructor
	vectorul de indici la care se afla intrari valide, un indice de
	start, end si operatia care trebuie executata.
	- Trimite la ExecutorService si concateneaza rezultatele.

update(tableName, values, condition):
	- Paralelizeaza verificarea intrarilor ce trebuiesc
	updatate. Creeaza numThreads UpdateReq-uri care contin
	un indice de start si end si verifica fiecare intre
	[Start,End) daca o intrare in tabel respecta o conditie si
	daca da o inlocuieste.

insert(tableName, values):
	- insereaza mereu la sfarsitul tabelei.

Sincronizare:

- Fiecare tabela contine un ReentrantLock tableAccess;
  Acesta este locked daca a inceput o Tranzactie 
  asupra tabelei respective.
  Fiecare thread al utilizatorilor care doreste sa acceseze o tabela
  trebuie sa poate achizitiona acest lock.
  Este necesar ca acest lock sa fie de tipul Reentrant pentru
  a permite accesul nerestrictionat asupra tabelei a utilizatorului
  care a pornit tranzactia.

- Sincronizarea la nivelul tabelei se reduce la problema
  Cititor-Scriitor unde Select este Cititor iar update si
  insert este Scriitor.
  Astfel fiecare tabela contine doua semafoare initializate
  la 1:
  	- resourceAccess: care trebuie achizitionat pentru
  	a putea modifica/citi o tabela
  	-readCountAccess: care trebuie achizitionat de un
  	cititor, un select, pentru a incrementa sau decrementa
  	numarul curent de cititori.

- Un update este defapt o mini-Tranzactie deoarece metoda
  update va creea alte obiecte de tip UpdateReq care se
  ocupa de parti diferite ale tabelei, care vor fi executate
  de thread-urile din ExecutorService dar va restrictiona
  accesul altor thread-uri ale utilizatorilor.

Mentionari:

- daca pentru testul: Select/Insert Consistency
  in primul for din metoda run() al clasei ConsistencyReaderThread
  reduc numarul de iteratii de la 100_000 la 10_000 toate
  testele imi trec in 6 min pe masina proprie: i5-7200U

Rezultate timpi pe masina proprie:

run:
[[0, 1, 2, 20, 4, 5, 6, 7, 8, 9], [Ion0, Ion1, Ion2, Ion20, Ion4, Ion5, Ion6, Ion7, Ion8, Ion9], [0, 1, 2, 20, 4, 5, 6, 7, 8, 9], [false, true, false, false, false, true, false, true, false, true], [0, 1, 2, 20, 4, 5, 6, 7, 8, 9]]
[[62], [0], [20], [6], [10], [0]]
Select/Insert Consistency PASS
Select/Update Consistency PASS
Transactions Consistency PASS
There are now 1 Threads
[[1065788928]]
Insert time 6751
Update time 6094
Select time 26183
There are now 2 Threads
[[1065788928]]
Insert time 5094
Update time 3520
Select time 19483
There are now 4 Threads
Insert time 5235
[[1065788928]]
Update time 2703
Select time 17497
BUILD SUCCESSFUL (total time: 6 minutes 5 seconds)









