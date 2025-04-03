# Projektowanie efektywnych algorytmów

Do projektu załączona była instrukcja [PEA projekt-6.pdf](<PEA projekt-6.pdf>).

## 1. \Proj_1

* Implementacja algorytmów do rozwiązywania TSP
     1. Losowy
     2. NN
     3. Brute-Force
     4. BFS (BNB)
     5. DFS (BNB)
     6. LC (BNB)
* Własna implementacja grafu (Nie obowiązkowe)
* Użyto biblioteki nlohmann/json do czytania i obsługi plików *.json
* Badania czasu wykonania poszczególnych algorytmów
* Instancje losowe (Własne)

## 2. \Proj_2

* Implementacja algorytmów do rozwiązywania TSP
     1. Tabu search
     2. Simulated annealing
     3. Mrówkowy
* Własna implementacja grafu (Nie obowiązkowe)
* Użyto biblioteki nlohmann/json do czytania i obsługi plików *.json
* Badania czasu wykonania i jakości rozwiązania (Procent błędu względnego) poszczególnych algorytmów
* Instancje ze strony <http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/>
* Instancje przechowywane w \TspLib\SYM lub \TspLib\ASYM
* Do wygenerowania plików *.txt (Potrzebnych do poprawnego funkcjonowania programu) wykożystano odpowiedniego konwertera w \TspLib
* Instancja kro124p.atsp zawiera 100 wierzchołków. Więc plik .txt wygenerowany z tej instancji nazywa się kro100p.txt (Nie kro124.txt) \

## 4. AlgorytmTS.cpp

* Przykład algorytmu Tabu Search

## 5. AntColony.h i tmp.py

* Przykład algorytmu Mrówkowego

## Obsługa

1. Program przygotowany na systemy Windows
2. Do całego projektu załączony jest CMakeLists.txt.
3. Aby zkompilować kod do pliku wykonywalnego należ użyć komend:

        cmake -S . -B build/release -G "Unix Makefiles"
        cmake --build build/release

4. Pliki wykonywalne zostaną umieszczone w katalogu build
5. By programy działały poprawnie w katalogu w którym się znajdują muszą być
   1. config(Odpowiednio P1 i P2).json
   2. Katalog Dane(Z jego obecną zawartością)
   3. Katalog Wyniki
