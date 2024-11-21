#pragma once
#include <string>

class Graph{
    private:
    int** graphMatrix; 
    int vertices = 0;
    int edges = 0;
    float density = 0.0f;
    void initializeMemory();
    public:
    void readFromFile(std::string fileName);
    void saveGraph(std::string fileName);
    int getMatrixItem(int i,int j);
    bool pathExist(int i,int j);
    void generate();
    Graph();
    Graph(int verticeAmmount);
    Graph(int verticeAmmount,float targetDensity);
    ~Graph();
};