#include "AlgorytmTS.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <chrono>
#include <random>

AlgorytmTS::AlgorytmTS()
    : najlepszyKoszt(std::numeric_limits<double>::max()) {}

void AlgorytmTS::wykonaj(const std::vector<std::vector<double>> &graf, const std::vector<int> &sciezkaStartowa,
                         double czasStopu, int rozmiarListyTabu, int maksIteracjeBezPoprawy) {
    auto start = std::chrono::steady_clock::now();
    auto czasNajlepszegoRozwiazania = start; // Czas znalezienia najlepszego rozwiązania

    // Inicjalizacja zmiennych
    obecnaSciezka = sciezkaStartowa;
    najlepszaSciezka = sciezkaStartowa;
    najlepszyKoszt = obliczKoszt(graf, sciezkaStartowa);
    listaTabu.clear();

    int iteracjeBezPoprawy = 0;
    std::mt19937 generator(std::random_device{}()); // Generator losowy

    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(static_cast<int>(czasStopu))) {
        // Generowanie sąsiedztwa
        auto sasiedztwo = generujSasiedztwo(obecnaSciezka);
        auto najlepszySasiad = znajdzNajlepszegoSasiada(sasiedztwo, graf);

        // Sprawdzenie poprawy najlepszego kosztu
        double kosztSasiada = obliczKoszt(graf, najlepszySasiad);
        if (kosztSasiada < najlepszyKoszt) {
            najlepszyKoszt = kosztSasiada;
            najlepszaSciezka = najlepszySasiad;
            iteracjeBezPoprawy = 0;
            czasNajlepszegoRozwiazania = std::chrono::steady_clock::now(); // Zapis czasu
        } else {
            iteracjeBezPoprawy++;
        }

        // Dywersyfikacja
        if (iteracjeBezPoprawy > maksIteracjeBezPoprawy) {
            losowoZmienSciezke(obecnaSciezka, generator);
            iteracjeBezPoprawy = 0;
            std::cout << "Dywersyfikacja uruchomiona. Nowa losowa sciezka." << std::endl;
        } else {
            obecnaSciezka = najlepszySasiad;
        }

        // Aktualizacja listy Tabu
        aktualizujListeTabu(najlepszySasiad, rozmiarListyTabu);
    }

    // Wyświetlanie najlepszego wyniku
    std::cout << "Najlepszy koszt: " << najlepszyKoszt << std::endl;
    std::cout << "Najlepsza sciezka: ";
    for (int wierzcholek : najlepszaSciezka) {
        std::cout << wierzcholek << " -> ";
    }
    std::cout << std::endl;

    // Wyświetlanie czasu znalezienia najlepszego rozwiązania
    auto czasTrwania = std::chrono::duration_cast<std::chrono::milliseconds>(czasNajlepszegoRozwiazania - start).count();
    std::cout << "Czas znalezienia najlepszego rozwiazania: " << czasTrwania / 1000.0 << " sekund" << std::endl;
}

std::vector<std::vector<int>> AlgorytmTS::generujSasiedztwo(const std::vector<int> &sciezka) {
    std::vector<std::vector<int>> sasiedztwo;

    for (size_t i = 1; i < sciezka.size() - 1; ++i) {
        for (size_t j = i + 1; j < sciezka.size() - 1; ++j) {
            std::vector<int> nowaSciezka = sciezka;
            std::swap(nowaSciezka[i], nowaSciezka[j]);
            sasiedztwo.push_back(nowaSciezka);
        }
    }

    return sasiedztwo;
}

std::vector<int> AlgorytmTS::znajdzNajlepszegoSasiada(const std::vector<std::vector<int>> &sasiedztwo,
                                                      const std::vector<std::vector<double>> &graf) {
    std::vector<int> najlepszySasiad;
    double najlepszyKosztSasiada = std::numeric_limits<double>::max();

    for (const auto &sasiad : sasiedztwo) {
        double koszt = obliczKoszt(graf, sasiad);
        if (koszt < najlepszyKosztSasiada && !jestNaLiscieTabu(sasiad)) {
            najlepszySasiad = sasiad;
            najlepszyKosztSasiada = koszt;
        }
    }

    return najlepszySasiad;
}

void AlgorytmTS::losowoZmienSciezke(std::vector<int> &sciezka, std::mt19937 &generator) {
    std::uniform_int_distribution<int> dystrybucja(1, sciezka.size() - 2);

    for (int i = 0; i < 3; ++i) { // Zamiana trzech losowych par wierzchołków
        int a = dystrybucja(generator);
        int b = dystrybucja(generator);
        std::swap(sciezka[a], sciezka[b]);
    }
}

void AlgorytmTS::aktualizujListeTabu(const std::vector<int> &rozwiazanie, int rozmiarListyTabu) {
    listaTabu.push_back(rozwiazanie);
    if (listaTabu.size() > static_cast<size_t>(rozmiarListyTabu)) {
        listaTabu.pop_front();
    }
}

bool AlgorytmTS::jestNaLiscieTabu(const std::vector<int> &rozwiazanie) {
    return std::find(listaTabu.begin(), listaTabu.end(), rozwiazanie) != listaTabu.end();
}

double AlgorytmTS::obliczKoszt(const std::vector<std::vector<double>> &graf, const std::vector<int> &sciezka) {
    double koszt = 0.0;
    for (size_t i = 0; i < sciezka.size() - 1; ++i) {
        koszt += graf[sciezka[i]][sciezka[i + 1]];
    }
    return koszt;
}
#include <fstream>
#include <sstream>

bool AlgorytmTS::zapiszSciezkeDoPliku(const std::string &nazwaPliku) {
    std::ofstream plik(nazwaPliku);
    if (!plik.is_open()) {
        return false;
    }

    // Zapisujemy ilość wierzchołków
    plik << najlepszaSciezka.size() << std::endl;

    // Zapisujemy ścieżkę
    for (size_t i = 0; i < najlepszaSciezka.size(); ++i) {
        plik << najlepszaSciezka[i];
        if (i < najlepszaSciezka.size() - 1) {
            plik << " -> ";
        }
    }

    plik.close();
    return true;
}

std::vector<int> AlgorytmTS::wczytajSciezkeZPliku(const std::string &nazwaPliku) {
    std::ifstream plik(nazwaPliku);
    if (!plik.is_open()) {
        return {};
    }

    int liczbaWierzcholkow;
    plik >> liczbaWierzcholkow;

    std::string linia;
    std::getline(plik, linia); // Odczytujemy resztę wiersza po liczbie wierzchołków
    std::getline(plik, linia);

    std::vector<int> sciezka;
    std::istringstream iss(linia);
    std::string token;

    // Parsujemy ścieżkę "1 -> 2 -> ..."
    while (std::getline(iss, token, '-')) {
        int wierzcholek = std::stoi(token);
        sciezka.push_back(wierzcholek);
        iss.get(); // Usuwamy znak '>'
    }

    plik.close();
    return sciezka;
}
const std::vector<int>& AlgorytmTS::getNajlepszaSciezka() const {
    return najlepszaSciezka;
}