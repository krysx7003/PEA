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
int tabuSearch(){
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    int maxIterations = gSize * (int)config["algorithms"]["maxMultiplier"];
    int tabuLength = gSize * (int)config["algorithms"]["lenMultiplier"];
    int improvementCount = config["algorithms"]["improvementCount"];
    string startPath = config["algorithms"]["startPath"];
    int estimatedCount = maxIterations,fraction = 0;
    auto rng = default_random_engine {};
    if(showProgress){
        fraction = estimatedCount/(width*20);
        printBar(0,"\nMetoda Tabu Search\t\t");
    }
    int res = INT_MAX;
    int noImprovement = 0;
    Graph* graph = new Graph(gSize);
    resPath.clear();
    vector<int> currPath;int currVal = INT_MAX;
    vector<vector<int>> neighbors; 
    vector<vector<int>> tabuList; 
    graph->readFromFile(config["instance"]["inputFile"]);
    auto startT = chrono::high_resolution_clock::now();
    for(int i = 0;i<gSize;i++){
        currPath.push_back(i);
    }
    if (startPath == "random"){
        shuffle(begin(currPath), end(currPath), rng);
    }
    currVal = pathValue(currPath,graph);
    for(int count = 0;count < maxIterations;count++){
        // for(int i = 0;i<gSize;i++){
        //     for(int j = 0;j<gSize;j++){
        //         vector<int> neighbor = currPath;
        //         swap(neighbor[i],neighbor[j]);
        //         neighbors.push_back(neighbor);
        //     }
        // }
        for (int i = 0; i < gSize; i++) {
            int x = rand() % gSize;
            int y = rand() % gSize;
            while (x == y) y = rand() % gSize;
            vector<int> neighbor = currPath;
            swap(neighbor[x], neighbor[y]);
            neighbors.push_back(neighbor);
        }
        vector<int> bestPath;int bestVal = INT_MAX;
        for(vector<int>neighbor:neighbors){
            int val = pathValue(neighbor,graph);
            if(val<0){
                continue;
            }
            if(find(tabuList.begin(), tabuList.end(), neighbor) == tabuList.end() || noImprovement>improvementCount){
                if(val<bestVal){
                    bestPath = neighbor;bestVal = val;
                }
                noImprovement = 0;
            }else{
                noImprovement++;
            }   
        }
        currPath = bestPath;currVal = bestVal;
        if(bestVal<res){
            resPath = bestPath;res = bestVal;
        }
        tabuList.push_back(bestPath);
        if(tabuList.size()>tabuLength){
            tabuList.erase(tabuList.begin());
        }
        if(showProgress){
            if(count%fraction==0){
                printBar((double)count/estimatedCount ,"Metoda Wyzarzania\t\t");
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
    int estimatedCount = 0,fraction = 0;
    auto rng = default_random_engine {};
    if(showProgress){
        estimatedCount = log(minTemp / currTemp) / log(alpha);
        fraction = estimatedCount/(width*20);
        printBar(0,"\nMetoda Wyzarzania\t\t");
    }
    int res,count = 0;
    int noImprovement = 0;
    vector<int> currPath;int currVal;
    Graph* graph = new Graph(gSize);
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    auto startT = chrono::high_resolution_clock::now();
    currVal = nearest_neighbour();
    for(int i = 0;i<gSize;i++){
        currPath.push_back(i);
    }
    if (startPath == "random"){
        shuffle(begin(currPath), end(currPath), rng);
    }
    while(currTemp>minTemp && noImprovement > improvementCount){
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
int choseVert(int currentVert,vector<int> visited,Graph* graph){
    vector<float> probs;
    float sum = 0.0f;
    for(int i = 0;i<gSize;i++){
        if(find(visited.begin(), visited.end(), i) == visited.end()){
            float heuristic = graph->getMatrixItem(currentVert, i);
            float pheromone = pheromoneLevels[currentVert][i];
            float prob = pheromone * heuristic;
            probs.push_back(prob);
            sum += prob;
        }else {
            probs.push_back(0.0f); 
        }
    }
    if(sum == 0){
        return -1;
    }
    for (int i = 0; i < probs.size(); i++) {
        probs[i] /= sum;
    }
    float random = ((float)rand() / RAND_MAX);
    float cumulative = 0.0f;
    for (int i = 0; i < gSize; i++) {
        if (find(visited.begin(), visited.end(), i) == visited.end()) {
            cumulative += probs[i];
            if (random < cumulative) {
                return i;
            }
        }
    }
    return -1; 
}
int antColony(){
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    int antsNumber = config["algorithms"]["antsNumber"];
    int iterations = config["algorithms"]["iterations"];
    float rho = config["algorithms"]["rho"];
    int estimatedCount = 0,fraction = 0;
    if(showProgress){
        estimatedCount = iterations;
        fraction = estimatedCount/(width*20);
        printBar(0,"\nAlgorytm mrowkowy\t\t");
    }
    int res,count = 0;
    vector<int> currPath;
    vector<int> bestPath;int bestVal = INT_MAX;
    vector<vector<int>> allPaths;
    Graph* graph = new Graph(gSize);
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    auto startT = chrono::high_resolution_clock::now();
    pheromoneLevels.resize(gSize, vector<float>(gSize, 1.0f));
    for(int i = 0;i<iterations;i++){
        for(int j = 0;j<antsNumber;j++){
            currPath.clear();
            int currVert = rand()%gSize;
            vector<int> visited;
            visited.push_back(currVert);
            currPath.push_back(currVert);
            while(visited.size()<gSize){
                int nextVert = choseVert(currVert,visited,graph);
                if(nextVert == -1){
                    break;
                }
                visited.push_back(nextVert);
                currPath.push_back(nextVert);
                currVert = nextVert;
            }
            int val = pathValue(currPath,graph);
            if(val<0){
                continue;
            }
            if(val<bestVal){
                bestVal = val;
                bestPath = currPath;
            }
            allPaths.push_back(currPath);
        }
        for (int j = 0; j< gSize; j++) {
            for (int k = 0; k < gSize; k++) {
                pheromoneLevels[j][k] *= (1 - rho);
            }
        }
        for(vector<int> path:allPaths){
            int val = pathValue(path, graph);
            float pheromone = 1.0f/val;
            for(int j = 0;j<gSize-1;j++){
                pheromoneLevels[path[j]][path[j+1]] += pheromone;
            }
        }
        if(showProgress){
            count++;
            if(count%fraction==0){
                printBar((double)count/estimatedCount ,"Algorytm mrowkowy\t\t");
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
        //Optymalna scieżka
        row.append(optimalPath);
        row.append(",");
        if(abs(res)<=2000000000){
            row.append(to_string(res));//Otrzymany wynik
        }
        row.append(",");
        //Otrzymana scieżka
        for (int number : resPath) {
            row.append(to_string(number+1));
            row.append("-");
        }
        row.pop_back();//Usuwa nadmiarowy -
        row.append(",");
        row.append(to_string(duration.count()));//Czas wykonania
        row.append(",");
        double absError = abs(res-optimalRes);
        double error = absError/optimalRes;
        row.append(to_string(absError));//Blad bezwzgledny
        row.append(",");
        row.append(to_string(error));//Blad wzgledny (liczbowo)
        row.append(",");
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
    outputFile<<fileNameIn<<", "<<funcName;
    string parameters = "";
    if(funcName=="random" || funcName=="brute-force"){
        parameters.append(",");
        parameters.append(to_string(config["settings"]["time"]));
        parameters.append("min");  
    }
    parameters.append(",");
    parameters.append(bool_to_string(config["algorithms"]["showProgress"]));
    outputFile<<parameters<<"\n";
    outputFile.close();
}
int main(){
    ifstream file("configP2.json");
    if(file.is_open()){
        file>>config;
    }else{
        cout<<"Could not open config file!\n";
    }
    srand(time(NULL));
    bool showProgress = config["settings"]["showProgress"];
    bool printToConsol = config["settings"]["printToConsol"];
    bool test = config["settings"]["test"];
    optimalPath = config["instance"]["optimalPath"];
    gSize = config["instance"]["size"]; 
    int iterations = config["settings"]["iterations"];   
    string funcName = config["instance"]["algorithName"];
    maxtime = config["settings"]["time"];//czas podany w minutach
    maxtime *= 60000000000;//1 minuta to 6*10^9 ns
    //Mapuje napis z configa do odpowiadającedj mu funkcji
    map<string,function<int()>> funcMap = {
        {config["algorithms"]["tabuSearch"],tabuSearch},
        {config["algorithms"]["simulatedAnealing"],simulatedAnealing},
        {config["algorithms"]["antColony"],antColony}
    };
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