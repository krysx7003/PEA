#ifndef ANTCOLONY_H
#define ANTCOLONY_H

#include "Graf.h"
#include <cmath>
#include <vector>
#include <limits>
#include <random>
#include <iostream>
#include <chrono>

using namespace std;

class AntColonyOptimization {
public:
    Graf* graf;
    int iloscMrowek;
    int maxCzasSekundy;
    double alfa;
    double beta;
    double rho;
    double Q;
    chrono::high_resolution_clock::time_point startTime;

    vector<vector<double>> feromony;
    vector<int> najlepszaSciezka;
    double najlepszyKoszt;
    enum AlgorithmType { CAS, QAS };  // Enum for selecting between CAS and QAS
    AlgorithmType algType;  // Variable to store the selected algorithm type
    double costThreshold;  // New member variable to store the stopping cost threshold

    AntColonyOptimization(Graf* graf, int iloscMrowek, int maxCzasSekundy, double alfa, double beta, double rho, double Q, AlgorithmType algType, double costThreshold)
        : graf(graf), iloscMrowek(iloscMrowek), maxCzasSekundy(maxCzasSekundy), alfa(alfa), beta(beta), rho(rho), Q(Q), najlepszyKoszt(numeric_limits<double>::max()),
          algType(algType), costThreshold(costThreshold) {
        int iloscWierzcholkow = graf->maciezSasiedztwa.size();
        feromony.assign(iloscWierzcholkow, vector<double>(iloscWierzcholkow, 1.0));  // Initialize pheromones to 1.0
    }

    vector<int> wykonajMrowke(int start) const {
        const size_t iloscWierzcholkow = graf->maciezSasiedztwa.size();
        vector<int> sciezka;
        vector<bool> odwiedzone(iloscWierzcholkow, false);  // Tracks visited vertices
        sciezka.reserve(iloscWierzcholkow + 1);

        sciezka.push_back(start);  // Start the path with the starting vertex
        odwiedzone[start] = true;

        while (sciezka.size() < iloscWierzcholkow) {
            int ostatni = sciezka.back();
            int nastepny = wybierzNastepnyWierzcholek(ostatni, odwiedzone);

            // If no valid next vertex is found, break out
            if (nastepny == -1) break;

            // Add the next vertex to the path and mark it as visited
            sciezka.push_back(nastepny);
            odwiedzone[nastepny] = true;  // Mark it as visited
        }

        // Ensure the cycle is complete (i.e., return to the starting vertex)
        // Allow the starting vertex to be revisited at the end only
        if (sciezka.size() == iloscWierzcholkow) {
            sciezka.push_back(start);  // Complete the cycle
        }

        return sciezka;
    }

    int wybierzNastepnyWierzcholek(int obecny, const vector<bool>& odwiedzone) const {
        int iloscWierzcholkow = graf->maciezSasiedztwa.size();
        vector<double> prawdopodobienstwa(iloscWierzcholkow, 0.0);
        double suma = 0.0;

        // CAS and QAS logic
        for (int i = 0; i < iloscWierzcholkow; ++i) {
            // Exclude paths with weight -1, revisited vertices, or no path
            if (!odwiedzone[i] && graf->maciezSasiedztwa[obecny][i] > 0 && graf->maciezSasiedztwa[obecny][i] != -1) {
                double tau_eta = pow(feromony[obecny][i], alfa) * pow(1.0 / graf->maciezSasiedztwa[obecny][i], beta);
                prawdopodobienstwa[i] = tau_eta;
                suma += tau_eta;
            }
        }

        // Avoid division by zero
        if (suma == 0.0) return -1;

        double losowa = static_cast<double>(rand()) / RAND_MAX;
        double akumulator = 0.0;
        for (int i = 0; i < iloscWierzcholkow; ++i) {
            if (prawdopodobienstwa[i] > 0) {  // Only consider vertices with non-zero probability
                akumulator += prawdopodobienstwa[i] / suma;
                if (losowa <= akumulator) return i;
            }
        }
        return -1;
    }

    double obliczKosztSciezki(const vector<int>& sciezka) const {
        double koszt = 0.0;
        for (size_t i = 0; i < sciezka.size() - 1; ++i) {
            int current = sciezka[i];
            int next = sciezka[i + 1];
            double path_cost = graf->maciezSasiedztwa[current][next];

            // Ensure forbidden or invalid paths (including asymmetric cases) are not considered
            if (path_cost == -1) {
                return numeric_limits<double>::infinity();  // Invalid path, return infinite cost
            }

            koszt += path_cost;
        }
        return koszt;
    }

    void aktualizujFeromony(const vector<vector<int>>& wszystkieSciezki, const vector<double>& koszty) {
        // Evaporation of pheromones
        for (auto& rzad : feromony) {
            for (auto& wartosc : rzad) {
                wartosc *= (1.0 - rho);  // Evaporation factor
            }
        }

        // Update pheromones based on the algorithm type (CAS or QAS)
        for (size_t k = 0; k < wszystkieSciezki.size(); ++k) {
            double delta = 0.0;

            if (algType == CAS) {
                // CAS: Reinforce cyclic behavior (favoring complete cycles)
                if (wszystkieSciezki[k].size() == graf->maciezSasiedztwa.size() + 1) {
                    delta = Q / koszty[k];  // Pheromone update is inversely proportional to the path cost
                } else {
                    delta = (Q / koszty[k]) * 0.5;  // Reduced pheromone deposit for non-complete cycles
                }
            }
            else if (algType == QAS) {
                // QAS: Focus on quantity, might depend on the path length or cost
                delta = Q / koszty[k];  // Standard update proportional to the path cost
            }

            // Update pheromones on the path
            for (size_t i = 0; i < wszystkieSciezki[k].size() - 1; ++i) {
                int u = wszystkieSciezki[k][i];
                int v = wszystkieSciezki[k][i + 1];
                // Ensure that pheromone updates respect directionality
                if (graf->maciezSasiedztwa[u][v] != -1) {
                    feromony[u][v] += delta;
                }
            }
        }
    }

    double checkElapsedTime() const {
        return chrono::duration<double>(chrono::high_resolution_clock::now() - startTime).count();
    }

    void znajdzOptymalnaSciezke() {
        startTime = chrono::high_resolution_clock::now();
        random_device rd;
        mt19937 gen(rd());

        while (checkElapsedTime() < maxCzasSekundy && najlepszyKoszt > costThreshold) {
            vector<vector<int>> wszystkieSciezki;
            vector<double> koszty;

            for (int i = 0; i < iloscMrowek; ++i) {
                int start = gen() % graf->maciezSasiedztwa.size();
                vector<int> sciezka = wykonajMrowke(start);
                if (sciezka.size() == graf->maciezSasiedztwa.size() + 1) {
                    double koszt = obliczKosztSciezki(sciezka);
                    wszystkieSciezki.push_back(sciezka);
                    koszty.push_back(koszt);
                    // Ensure the algorithm doesn't select over-optimized solutions
                    if (koszt < najlepszyKoszt) {
                        najlepszyKoszt = koszt;
                        najlepszaSciezka = sciezka;
                    }
                }
            }
            if (!wszystkieSciezki.empty()) {
                aktualizujFeromony(wszystkieSciezki, koszty);
            }
        }
    }

    void wypiszNajlepszaSciezke() const {
        cout << "\nNajlepsza sciezka: ";
        for (int wierzcholek : najlepszaSciezka) {
            cout << wierzcholek << " ";
        }
        cout << "\nKoszt: " << najlepszyKoszt << endl;
    }
};

#endif
