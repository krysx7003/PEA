#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include "Graph.hpp"
using namespace std;
/**
 * Wczytuje dane z pliku. Brak połączenia oznaczony jest cyfrą 0
 * @param fileName Nazwa pliku z danymi 
 */
void Graph::readFromFile(string fileName){
        string basePath = filesystem::current_path().string();
        fileName = basePath+fileName;
        ifstream file(fileName);
        string buffer;
        int newWeight;
        int i = 0;
        if(file.is_open()){
            initializeMemory();
            while(!file.eof()){
                for(int j=0;j<vertices;j++){
                    file>>newWeight;
                    graphMatrix[i][j] = newWeight;
                }
                getline(file, buffer);
                i++;
                if(i==vertices){
                    break;
                }                
            }
            file.close();
        }
    }
/**
 * Zapisuje macierz w pliku
 * @param fileName Nazwa pliku w którym zostanie zapisana macierz
 */
void Graph::saveGraph(string fileName){
    string basePath = filesystem::current_path().string();
    fileName = basePath+fileName;
    ofstream file(fileName,ios_base::app);
    for(int i=0;i<vertices;i++){
        for(int j=0;j<vertices;j++){
            file<<graphMatrix[i][j]<<" ";
        }
        file<<"\n";
    }
}
/**
 * Inicializuje macierz sąsiedztwa grafu
 */
void Graph::initializeMemory(){
    graphMatrix =new int*[vertices];
    for (int i = 0; i < vertices; i++) {
        graphMatrix[i] = new int[vertices];
        for (int j = 0; j < vertices; j++) {
            graphMatrix[i][j] = 0;
        }
    }
}
bool Graph::pathExist(int i,int j){
    return graphMatrix[i][j]>0;
}
Graph::~Graph(){
    for(int i=0;i<vertices;i++){
        delete[] graphMatrix[i];
    }
    delete graphMatrix;
}
/**
 * Zwraca wartość ścieżki między dwoma wierzchołkami
 * @param i Pierwszy wierzchołek
 * @param j Drugi wierzchołek
 */
int Graph::getMatrixItem(int i,int j){return graphMatrix[i][j];}
Graph::Graph(){}
/**
 * @param verticeAmmount Liczba wierzchołków w grafie 
 */
Graph::Graph(int verticeAmmount){
    vertices = verticeAmmount;
}