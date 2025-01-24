#include <iostream>
#include <fstream>
#include <map>
#include <chrono>
#include <vector>
#include <random>
#include <cmath> 
#include <stdlib.h> 
#include <filesystem>
#include <climits>
#include <queue>
#include <filesystem>
#include <stack>
//Biblioteka nlohmann/json służąca do obsługi pilków json
//https://github.com/nlohmann/json
#include "json.hpp"
#include "Graph.hpp"
#include "Node.hpp"

using json = nlohmann::json;
using namespace std;

int width;
void printBar(double progress,string label){
    int done = width*progress;
    cout<<label<<"||"; 
    cout.flush();      
    for(int j=0;j<width;j++){
        if(j<done){
            cout<<"#";
        }else{
            cout<<"-";
        }
    }
    cout<<"|| "<< fixed << setprecision(2)<<progress*100<<"%\r";
    cout.flush(); 
}
void clear_input(){
    for(int i=0;i<60;i++){
            cout<<" ";
    }
    cout<<"\r";
}
json config;
vector<int> resPath;
vector<string> searchMethods;
chrono::duration<double, nano> duration;
int gSize;
long long maxtime;
string optimalPath;
int pathValue(vector<int> path,Graph *graph){
    int value = 0;
    for(int i = 0;i<path.size();i++){
        int currentVert = path.at(i);
        int nextVert = path.at((i+1)%path.size());
        int cost = graph->getMatrixItem(currentVert,nextVert);
        if(cost<0){
            return -1;
        }  
        value += cost;       
    }
    return value;
}
int nearest_neighbour(){
    int startVert;
    bool showProgress = config["algorithms"]["showProgress"];
    Graph* graph = new Graph(gSize);
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    vector<int>  currPath;
    int res=INT_MAX,currRes;
    if(showProgress){
        printBar(0,"\nMetoda nearest neighbour");
    }
    auto startT = chrono::high_resolution_clock::now();//Początek pomiaru czasu
    startVert = -1;
    while(startVert<gSize-1){
        startVert++;
        currRes=0;
        currPath.insert(currPath.end(),startVert);
        for(int i=0;i<gSize-1;i++){
            int currVal = INT_MAX;
            int currVert;
            for(int j=0;j<gSize;j++){
                if(count(currPath.begin(), currPath.end(), j)==0){
                    int val = graph->getMatrixItem(currPath[i],j);
                    if(val<0){
                        continue;
                    }
                    if(val<currVal){
                        currVal = val;
                        currVert = j;
                    }
                }
            }
            currRes +=currVal;
            currPath.insert(currPath.end(),currVert);
        }
        int val = graph->getMatrixItem(currPath[gSize-1],currPath[0]);
        if(val<0){
            continue;
        }
        currRes += val;
        if(currRes<res){
            resPath.clear();
            res=currRes;
            resPath.insert(resPath.end(),currPath.begin(),currPath.end());
        }
        currPath.clear();
        if(showProgress){
            printBar((double)startVert/gSize,"Metoda nearest neighbour");
        }
    } 
    auto endT = chrono::high_resolution_clock::now();//Koniec pomiaru czasu
    duration = endT - startT;//Czas wykonania zapisany w ns
    if(showProgress){
        clear_input();
    }
    delete graph;
    return res;
}
bool isInTabu(pair<int,int> move,vector<pair<int,int>> tabuList){
    for(pair<int,int> tabuMove:tabuList){
        if(move == tabuMove){
            return true;
        }
    }
    return false;
}
int tabuSearch(){
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    int tabuLength = gSize * (int)config["algorithms"]["lenMultiplier"];
    int improvementCount = config["algorithms"]["improvementCount"];
    string startPath = config["algorithms"]["startPath"];
    string newPathMethod = config["algorithms"]["newPathMethod"];
    long long estimatedCount = maxtime,fraction = 0;
    auto rng = default_random_engine {chrono::system_clock::now().time_since_epoch().count()};
    if(showProgress){
        fraction = 1000000000;
        printBar(0,"\nMetoda Tabu Search\t");
    }
    int res = INT_MAX;
    int noImprovement = 0;
    Graph* graph = new Graph(gSize);
    resPath.clear();
    vector<int> currPath;int currVal = INT_MAX;
    vector<vector<int>> neighbors; 
    vector<pair<int,int>> tabuList;
    vector<pair<int,int>> moves;  
    chrono::duration<double, nano> currTime;
    graph->readFromFile(config["instance"]["inputFile"]);
    for(int i = 0;i<gSize;i++){
        currPath.push_back(i);
    }
    if (startPath == "random"){
        shuffle(begin(currPath), end(currPath), rng);
    }
    currVal = pathValue(currPath,graph);
    if(startPath == "nn"){
        currVal = nearest_neighbour();
        currPath = resPath;
        resPath = currPath; res = currVal;
    }
    auto startT = chrono::high_resolution_clock::now();
    while(currTime.count()<maxtime){
        if(newPathMethod == "swap"){
            for(int i = 0;i<gSize;i++){
                for(int j = i+1;j<gSize;j++){
                    vector<int> neighbor = currPath;
                    swap(neighbor[i],neighbor[j]);
                    neighbors.push_back(neighbor);
                    moves.push_back({i,j});
                }
            }         
        }else if(newPathMethod == "insert"){
            for(int it = 0;it<(gSize*gSize-1)/2;it++){
                int i = rand()%gSize;
                int j = rand()%gSize;
                vector<int> neighbor = currPath;
                int tmp = neighbor[i];
                neighbor.erase(neighbor.begin() + i);
                neighbor.insert(neighbor.begin() + j, tmp); 
                neighbors.push_back(neighbor);
                moves.push_back({i,j});
            }       
        }
        vector<int> bestPath;int bestVal = INT_MAX;pair<int,int> bestMove;
        for(int i =0;i<neighbors.size();i++){
            vector<int>neighbor = neighbors[i];
            int val = pathValue(neighbor,graph);
            if(val<0){
                continue;
            }
            if(!isInTabu(moves[i],tabuList)){
                if(val<bestVal){
                    bestPath = neighbor;bestVal = val;bestMove = moves[i];
                    noImprovement = 0;
                }else{
                    noImprovement++;
                }  
            }else if(val<res){
                bestPath = neighbor;bestVal = val;bestMove = moves[i];
                noImprovement = 0;
            }
        }
        moves.clear();
        neighbors.clear();
        if(noImprovement>improvementCount){
            shuffle(begin(currPath), end(currPath), rng);
            noImprovement = 0;
        }else{
            currPath = bestPath;currVal = bestVal;
            tabuList.push_back(bestMove);
        }
        if(bestVal<res){
            resPath = bestPath;res = bestVal;
        } 
        if(tabuList.size()>tabuLength){
            tabuList.erase(tabuList.begin());
        }
        auto endT = chrono::high_resolution_clock::now();
        currTime = endT - startT;
        if(showProgress){
            if(currTime.count()>=fraction){
                double tmp = (currTime.count()/1000)/(estimatedCount/1000); 
                printBar(tmp,"Metoda Tabu Search\t");
                fraction += 1000000000;
            }
        }
    }
    auto endT = chrono::high_resolution_clock::now();//Koniec pomiaru czasu
    duration = endT - startT;
    if(showProgress){
        clear_input();
    }
    delete graph;
    return res;
}
bool accept(int dVal,float currTemp){
    if(dVal<0){
        return true;
    }else{
        float r = (float)(rand())/((float)(RAND_MAX)+1);            
        if(r<exp(dVal/currTemp)){
            return true;
        }else{
            return false;
        }
    }
}
int simulatedAnealing(){
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    float minTemp = config["algorithms"]["minTemp"];
    float alpha = config["algorithms"]["alpha"];
    float currTemp = config["algorithms"]["currTemp"];
    int improvementCount = config["algorithms"]["improvementCount"];
    string startPath = config["algorithms"]["startPath"];
    string newPathMethod = config["algorithms"]["newPathMethod"];
    int epochLen = config["algorithms"]["epochLen"];
    chrono::duration<double, nano> currTime;
    int estimatedCount = 0,fraction = 0;
    auto rng = default_random_engine {};
    if(showProgress){
        estimatedCount = currTemp / log(alpha);
        fraction = estimatedCount/(width*20);
        printBar(0,"\nMetoda Wyzarzania\t\t");
    }
    int res,count = 0;
    int noImprovement = 0;
    vector<int> currPath;int currVal;
    Graph* graph = new Graph(gSize);
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    for(int i = 0;i<gSize;i++){
        currPath.push_back(i);
    }
    if (startPath == "random"){
        shuffle(begin(currPath), end(currPath), rng);
    }
    currVal = pathValue(currPath,graph);
    if(startPath == "nn"){
        currVal = nearest_neighbour();
        currPath = resPath;
    }
    auto startT = chrono::high_resolution_clock::now();
    while(currTemp>minTemp && noImprovement < improvementCount && currTime.count()<maxtime){
        for(int i = 0;i<epochLen;i++){
            vector<int> newPath = currPath;
            int firstItem = rand()%gSize;
            int secondItem = rand()%gSize;
            if(newPathMethod == "swap"){
                swap(newPath[firstItem],newPath[secondItem]);
            }else if(newPathMethod == "insert"){
                int vert = newPath[firstItem];
                newPath.erase(newPath.begin()+firstItem);
                newPath.insert(newPath.begin()+secondItem,vert);
            }
            int val = pathValue(newPath,graph);
            if(val<0){
                continue;
            }
            int dVal = val - currVal;
            if(accept(dVal,currTemp)){
                currVal = val;
                currPath = newPath;
                noImprovement = 0;
            }
        }
        noImprovement++;
        currTemp *= alpha;
        if(showProgress){
            count++;
            if(count%fraction==0){
                printBar((double)count/estimatedCount ,"Metoda Wyzarzania\t\t");
            }
        }
        auto endT = chrono::high_resolution_clock::now();
        currTime = endT - startT;
    }
    auto endT = chrono::high_resolution_clock::now();//Koniec pomiaru czasu
    duration = endT - startT;
    res = currVal;
    resPath = currPath;
    if(showProgress){
        clear_input();
    }
    delete graph;
    return res;
}
vector<vector<float>> pheromoneLevels;
int betaA,alfa;
int choseVert(int currentVert,vector<int> visited,Graph* graph){
    vector<double> probs; 
    double sum = 0.0f;
    bool entered = false;
    for(int i = 0;i<gSize;i++){
        if(find(visited.begin(), visited.end(), i) == visited.end()){
            // cout<<i<<"\r";
            entered = true;
            double eta = pow(1.0/graph->getMatrixItem(currentVert, i),betaA);
            double tau = pow(pheromoneLevels[currentVert][i],alfa);
            double prob = tau * eta;
            probs.push_back(prob);
            sum += prob; 
        } else {
            probs.push_back(0.0f); 
        }
    }
    double random = ((double)rand() / (RAND_MAX + 1.0f));
    random += 1e-9;
    double cumulative = 0.0f;
    for (int i = 0; i < gSize; i++) {
        cumulative += probs[i]/sum;
        if (random <= cumulative) {
            return i;
        }
    }  
    return -1;
}
int antColony(){
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    int antsNumber = gSize;
    float rho = config["algorithms"]["rho"];
    alfa = config["algorithms"]["alfa"];
    betaA = config["algorithms"]["beta"];
    string updatePheromones = config["algorithms"]["updatePheromones"];
    int improvementCount = config["algorithms"]["improvementCount"];
    long long estimatedCount = maxtime,fraction = 0;
    int noImprovement = 0;
    if(showProgress){
        fraction = 1000000000;
        printBar(0,"\nAlgorytm mrowkowy\t");
    }
    int res,count = 0;
    vector<int> bestPath;int bestVal = INT_MAX;
    pheromoneLevels.clear();
    vector<vector<int>> allPaths;
    Graph* graph = new Graph(gSize);
    resPath.clear();
    chrono::duration<double, nano> currTime;
    graph->readFromFile(config["instance"]["inputFile"]);
    auto startT = chrono::high_resolution_clock::now();
    bestVal = nearest_neighbour();
    bestPath = resPath;
    float cnn = (float)gSize/(bestVal/100);
    pheromoneLevels.resize(gSize, vector<float>(gSize,cnn));
    while(currTime.count()<maxtime){
        vector<int> currPath;int currVal;
        for(int i=0;i<antsNumber;i++){
            currPath.clear();currVal = INT_MAX;
            int currVert = rand()%gSize;
            currPath.push_back(currVert);
            while(currPath.size()<gSize){
                int nextVert = choseVert(currVert,currPath,graph);
                currPath.push_back(nextVert);
                currVert = nextVert;
                if(updatePheromones == "QAS"){
                    int prevVert = currPath[currPath.size()-2], lastVert = currPath[currPath.size()-1];
                    pheromoneLevels[prevVert][lastVert] += (float)gSize/graph->getMatrixItem(prevVert,lastVert);
                }
            }
            if(updatePheromones == "QAS"){
                int prevVert = currPath[currPath.size()-1], lastVert = currPath[0];
                pheromoneLevels[prevVert][lastVert] += (float)gSize/graph->getMatrixItem(prevVert,lastVert);
            }
            currVal = pathValue(currPath,graph);
            if(updatePheromones == "CAS"){
                for(int j=0;j<currPath.size();j++){
                    pheromoneLevels[currPath[j]][currPath[(j+1)%gSize]] += (float)gSize/(currVal/100); 
                }  
            }
        }
        if(currVal<bestVal){
            bestPath = currPath;bestVal = currVal;
        }else{
            noImprovement++;
        }
        if(noImprovement<improvementCount){
            break;
        }
        for(int i=0;i<gSize;i++){
            for(int j=0;j<gSize;j++){
                float tmp = pheromoneLevels[i][j] * rho;
                if(tmp==0){
                    tmp = 1e-12;
                }
                pheromoneLevels[i][j] = tmp;
            }
        }
        auto endT = chrono::high_resolution_clock::now();
        currTime = endT - startT;
        if(showProgress){
            if(count%fraction==0){
                if(currTime.count()>=fraction){
                    double tmp = (currTime.count()/1000)/(estimatedCount/1000); 
                    printBar(tmp,"Algorytm mrowkowy\t");
                    fraction += 1000000000;
                }
            }
        } 
    }
    auto endT = chrono::high_resolution_clock::now();//Koniec pomiaru czasu
    duration = endT - startT;
    res = bestVal;
    resPath = bestPath;
    if(showProgress){
        clear_input();
    }
    delete graph;
    return res;
}
string bool_to_string(bool convert){
    if(convert){
        return "true";
    }else{
        return "false";
    }
}
void print(int res){
    int maxDisplaySize = config["settings"]["maxDisplaySize"];
    cout<<"========================================================\n";
    string fileName = config["instance"]["inputFile"];
    cout<<"Plik wejsciowy: "<<fileName<<"\n";
    string funcName = config["instance"]["algorithName"];
    cout<<"Uzyty algorytm: "<<funcName<<"\n";
    cout<<"Pokazywanie paska postepu: "<<bool_to_string(config["algorithms"]["showProgress"])<<"\n";
    int optimalRes = config["instance"]["optimalRes"];
    cout<<"Optymalny wynik: "<<optimalRes<<"\n";
    if(gSize<maxDisplaySize){
        cout<<"Optymalna sciezka: "<<optimalPath<<"\n";
    }
    cout<<"Otrzymany wynik: "<<res<<"\n";
    if(gSize<maxDisplaySize){
        cout<<"Otrzymana sciezka: ";
        for (int number : resPath) {
            cout << number+1 << " ";
        }
        cout<<"\n";
    }
    cout<<"Czas wykonania: "<<duration.count()<<" ns, "<<duration.count()/(1e9)<<" s"<<"\n";
    double absError = abs(res-optimalRes);
    double error = absError/optimalRes;
    cout<<"Blad bezwzgledny: "<<absError<<"\n";
    cout<<"Blad wzgledny (liczbowo): "<<error<<"\n";
    cout<<"Blad wzgledny (procentowo): "<<error*100<<"%"<<"\n";
}
void write(int res){
    int optimalRes = config["instance"]["optimalRes"];
    string fileName = config["instance"]["outputFile"];
    string filePath = filesystem::current_path().string()+fileName;
    ofstream outputFile(filePath,ios_base::app);
    if(outputFile.is_open()){
        string row;
        row.append(to_string((int)optimalRes));//Optymalny wynik
        row.append(",");
        if(abs(res)<=2000000000){
            row.append(to_string(res));//Otrzymany wynik
        }
        row.append(",");
        row.append(to_string(duration.count()/60000000000));//Czas wykonania
        row.append(",");
        double absError = abs(res-optimalRes);
        double error = absError/optimalRes;
        row.append(to_string(error*100));//"Blad wzgledny (procentowo)
        row.append("%\n");
        outputFile<<row;
        outputFile.close();
    }
}
void write_header(string funcName){
    string fileNameOut = config["instance"]["outputFile"];
    string fileNameIn = config["instance"]["inputFile"];

    string filePath = filesystem::current_path().string()+fileNameOut;
    ofstream outputFile(filePath,ios_base::app);
    if (filesystem::is_empty(filePath)){
        string title = config["csvTitleString"];
        outputFile<<title<<"\n";
    }
    outputFile<<fileNameIn<<"\n ";
    outputFile.close();
}
void test(function<int()> testFunc){
    string asymFiles[] = {"ftv33.txt","ftv64.txt","kro100p.txt","ftv170.txt"}; 
    int asymSizes[] = {33,64,100,170};
    int asymOptValues[] = {1286,1839,36230,2755};
    string symFiles[] = {"rat99.txt","pr152.txt","ts225.txt","pr264.txt","lin318.txt","pr439.txt"};
    int symSizes[] = {99,152,225,264,318,439,318,439};
    int symOptValues[] = {1211,73682,126643,49135,42029,107217};
    int iterations = config["settings"]["iterations"]; 
    cout<<"\n"; 
    for(int i=0;i<6;i++){
        printBar((double)i/6,"Symetryczne \t\t");
        string fileName ="\\Dane\\TSP\\"+symFiles[i];
        config["instance"]["inputFile"] = fileName;
        write_header("");
        gSize = symSizes[i];
        config["instance"]["optimalRes"] = symOptValues[i];
        for(int j=0;j<iterations;j++){
            int res = testFunc();
            write(res);
        }
    }
    for(int i=0;i<4;i++){
        printBar((double)i/6,"Asymetryczne \t\t");
        string fileName ="\\Dane\\ATSP\\"+asymFiles[i];
        config["instance"]["inputFile"] = fileName;
        write_header("");
        gSize = asymSizes[i];
        config["instance"]["optimalRes"] = asymOptValues[i];
        for(int j=0;j<iterations;j++){
            int res = testFunc();
            write(res);
        }
    }
}
void setupTests(){
    width = config["settings"]["barWidth"];
    int i = 0,iterations = 3;
    
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["startPath"] = "random";
    // config["instance"]["outputFile"] = "//Wyniki//TsRandom.csv";
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["startPath"] = "nn";
    // config["instance"]["outputFile"] = "//Wyniki//TsNN.csv";
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["startPath"] = "nn";

    // config["algorithms"]["newPathMethod"] = "swap";
    // config["instance"]["outputFile"] = "//Wyniki//TsSwap.csv";
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["newPathMethod"] = "insert";
    // config["instance"]["outputFile"] = "//Wyniki//TsInsert.csv";
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["newPathMethod"] = "insert";
    // config["algorithms"]["lenMultiplier"] = 0.5;
    // config["instance"]["outputFile"] = "//Wyniki//TsTabuLen.csv";
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["lenMultiplier"] = 0.4;
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["lenMultiplier"] = 0.3;
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["lenMultiplier"] = 0.2;
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["lenMultiplier"] = 0.1;
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["improvementCount"] = 25;
    // config["instance"]["outputFile"] = "//Wyniki//TsCount.csv";
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["improvementCount"] = 50;
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["improvementCount"] = 75;
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["improvementCount"] = 100;
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");
    // config["algorithms"]["improvementCount"] = 125;
    // test(tabuSearch);i++;
    // printBar((double)i/iterations,"Caly program \t\t");

    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["updatePheromones"] = "QAS";
    config["instance"]["outputFile"] = "//Wyniki//AoFeroMet.csv";
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["updatePheromones"] = "CAS";
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    
    config["algorithms"]["rho"] = 0.1;
    config["instance"]["outputFile"] = "//Wyniki//AoRho.csv";
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["rho"] = 0.2;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["rho"] = 0.3;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["rho"] = 0.4;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["rho"] = 0.5;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");

    config["instance"]["outputFile"] = "//Wyniki//AoAB.csv";
    config["algorithms"]["alfa"] = 1;
    config["algorithms"]["beta"] = 3;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["alfa"] = 2;
    config["algorithms"]["beta"] = 3;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["alfa"] = 3;
    config["algorithms"]["beta"] = 3;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["alfa"] = 3;
    config["algorithms"]["beta"] = 2;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
    config["algorithms"]["alfa"] = 3;
    config["algorithms"]["beta"] = 1;
    test(antColony);i++;
    printBar((double)i/iterations,"Caly program \t\t");
}
int main(){
    ifstream file("configP2.json");
    if(file.is_open()){
        file>>config;
    }else{
        cout<<"Could not open config file!\n";
    }
    srand(time(0));
    bool showProgress = config["settings"]["showProgress"];
    bool printToConsol = config["settings"]["printToConsol"];
    bool test = config["settings"]["test"];
    optimalPath = config["instance"]["optimalPath"];
    gSize = config["instance"]["size"]; 
    int iterations = config["settings"]["iterations"];   
    string funcName = config["instance"]["algorithName"];
    float timeMin = config["settings"]["time"];
    maxtime = 60000000000 * timeMin;//czas podany w minutach
    //Mapuje napis z configa do odpowiadającedj mu funkcji
    map<string,function<int()>> funcMap = {
        {config["algorithms"]["tabuSearch"],tabuSearch},
        {config["algorithms"]["simulatedAnealing"],simulatedAnealing},
        {config["algorithms"]["antColony"],antColony}
    };
    setupTests();
     //W mapie znajduej odpowiednią funkcje i ją uruchamiam
    if(funcMap.find(funcName)!=funcMap.end()){
        write_header(funcName);
        width = config["settings"]["barWidth"];
        if(showProgress==true){
            printBar(0,"Caly program \t\t");
        }
        string fileNameOut = config["instance"]["outputFile"];
        ofstream outputFile(fileNameOut,ios_base::app);
        outputFile<<"Size: "<<gSize<<"\n";
        outputFile.close();
        for(int i=1;i<=iterations;i++){
            int res = funcMap[funcName]();
            if(printToConsol==true){
                print(res);
            }
            if(showProgress==true){
                printBar((double)i/iterations,"Caly program \t\t");
            }
            write(res);
            duration = chrono::duration<double,nano>::zero();
        }
        cout<<"\n";
    }else{
        cout<<"Nie znaleziono funkcji\n";
    }
    cout<<"Program zakonczony\n";
    cout<<"Press ENTER to continue...\n";
    cin.get();

    return 0;
}
//Badania do wykonania
//Błąd od rozmiaru
//Czas od rozmiaru 
//Wszystko na małych
//Dla mrówków
//Instancje ~500 ~700 
