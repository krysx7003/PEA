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
    int maxIterations = gSize*gSize;
    int tabuLength = gSize/3;
    int estimatedCount = maxIterations,fraction = 0;
    if(showProgress){
        fraction = estimatedCount/(width*20);
        printBar(0,"\nMetoda Tabu Search\t\t");
    }
    int res = INT_MAX;
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
    currVal = pathValue(currPath,graph);
    for(int count = 0;count < maxIterations;count++){
        for(int i = 0;i<gSize;i++){
            for(int j = 0;j<gSize;j++){
                vector<int> neighbor = currPath;
                swap(neighbor[i],neighbor[j]);
                neighbors.insert(neighbors.end(),neighbor);
            }
        }
        vector<int> bestPath;int bestVal = INT_MAX;
        for(vector<int>neighbor:neighbors){
            int val = pathValue(neighbor,graph);
            if(val<0){
                continue;
            }
            if(find(tabuList.begin(), tabuList.end(), neighbor) == tabuList.end() && val<bestVal){
                bestPath = neighbor;bestVal = val;
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
    int estimatedCount = 0,fraction = 0;
    if(showProgress){
        fraction = estimatedCount/(width*20);
        printBar(0,"\nMetoda Wyzarzania\t\t");
    }
    int res = INT_MAX,count = 0;
    vector<int> currPath;int currVal;
    float currTemp,minTemp = 0.0001;
    float alpha = 0.9;
    Graph* graph = new Graph(gSize);
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    auto startT = chrono::high_resolution_clock::now();
    currVal = nearest_neighbour();
    currPath = resPath;
    currTemp = 1;
    while(currTemp>minTemp && currVal >optimalRes){
        vector<int> newPath = currPath;
        swap(newPath[rand()%gSize],newPath[rand()%gSize]);
        int val = pathValue(newPath,graph);
        if(val<0){
            continue;
        }
        int dVal = val - currVal;
        if(accept(dVal,currTemp)){
            currVal = val;
            currPath = newPath;
        }
        currTemp *= alpha;
        //TODO - pasek progresu 
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
    ifstream file("config.json");
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
        {config["algorithms"]["simulatedAnealing"],simulatedAnealing}
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
    system("pause");
    return 0;
}