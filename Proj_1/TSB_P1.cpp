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
int randomAlg(){
    bool showProgress = config["algorithms"]["showProgress"];
    int optimalRes = config["instance"]["optimalRes"];
    auto rng = default_random_engine {};
    Graph* graph = new Graph(gSize);
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    vector<int>  currPath;
    width = config["settings"]["barWidth"];
    long long fraction=maxtime/(width*10);
    bool pathExist = true;
    if(showProgress){
        printBar(0,"\nMetoda losowa\t\t");
    }
    auto startT = chrono::high_resolution_clock::now();//Początek pomiaru czasu
    int res = INT32_MAX;
    resPath = {};
    for(int i=0;i<gSize;i++){
        currPath.insert(currPath.end(),i);
    }
    while(duration.count()<maxtime){
        auto endT = chrono::high_resolution_clock::now();//Koniec pomiaru czasu
        duration = endT - startT;//Czas wykonania zapisany w ns
        shuffle(begin(currPath), end(currPath), rng);
        if(showProgress){
            printBar((double)duration.count()/maxtime,"Metoda losowa\t\t");
        }
        int currRes=0;
        for(int j = 0;j<gSize-1;j++){
            pathExist = graph->pathExist(currPath[j],currPath[j+1]);
            currRes += graph->getMatrixItem(currPath[j],currPath[j+1]);
            if(!pathExist){
                break;
            }   
        }
        if(!pathExist){
            continue;
        }
        pathExist = graph->pathExist(currPath[gSize-1],currPath[0]);
        currRes += graph->getMatrixItem(currPath[gSize-1],currPath[0]);
        if(!pathExist){
            continue;
        }
        if(currRes<res){
            res=currRes;
            resPath.clear();
            resPath.insert(resPath.end(),currPath.begin(),currPath.end()); 
            resPath.insert(resPath.end(),currPath[0]);          
            if(currRes==optimalRes){
                break;
            }
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
int nearest_neighbour(){
    int startVert;
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    Graph* graph = new Graph(gSize);
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    vector<int>  currPath;
    int res=INT_MAX,currRes;
    bool pathExist = true;
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
                    pathExist = graph->pathExist(currPath[i],j);
                    if(!pathExist){
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
        currRes += graph->getMatrixItem(currPath[gSize-1],currPath[0]);
        pathExist = graph->pathExist(currPath[gSize-1],currPath[0]);
        if(!pathExist){
            continue;
        }
        if(currRes<res){
            currPath.insert(currPath.end(),startVert);
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
Graph* bfGraph;
int bfRes,bfOptimalRes;
bool bfShowProgress;
long long factorial,fraction,iterNum = 0;
void heap_alg(int n,vector<int>& currPath, int partialCost,auto startT){
    auto endT = chrono::high_resolution_clock::now();//Koniec pomiaru czasu
    bool pathExist = true; 
    duration = endT - startT;
    if(duration.count()<=maxtime && bfRes != bfOptimalRes){
       if(n!=1){
            heap_alg(n-1,currPath,partialCost,startT);
            for(int i=0;i<n-1;i++){
                if(n%2==0){
                    swap(currPath[i],currPath[n-1]);
                }else{
                    swap(currPath[0],currPath[n-1]);
                }
                heap_alg(n - 1, currPath,partialCost,startT);
            }
        }else{
            int currRes=0;
            for(int j=0;j<gSize-1;j++){
                pathExist = bfGraph->pathExist(currPath[j],currPath[j+1]);
                currRes += bfGraph->getMatrixItem(currPath[j],currPath[j+1]);
                if(!pathExist){
                    break;
                }
            }
            if(!pathExist){
                return;
            }
            pathExist = bfGraph->pathExist(currPath[gSize-1],currPath[0]);
            currRes += bfGraph->getMatrixItem(currPath[gSize-1],currPath[0]);
            if(!pathExist){
                return;
            }
            if(currRes<=bfRes){
                bfRes=currRes;
                resPath.clear();
                resPath.insert(resPath.end(),currPath.begin(),currPath.end());
                resPath.insert(resPath.end(),currPath[0]);
            }
            if(bfShowProgress){
                iterNum++;
                if(iterNum%fraction==0){
                    printBar((double)iterNum/factorial,"Metoda brute-force \t");
                }
            }
        } 
    }   
}
int brute_force(){
    bfOptimalRes = config["instance"]["optimalRes"];
    bfShowProgress = config["algorithms"]["showProgress"];
    int startVert;
    bfGraph = new Graph(gSize);
    resPath.clear();
    bfGraph->readFromFile(config["instance"]["inputFile"]);
    vector<int> currPath;
    bfRes = INT_MAX;
    if(bfShowProgress){
        factorial = 1;
        for(int i=1;i<=gSize;i++){
            factorial *=i;
        }
        fraction = factorial/(width*10);
        printBar(0,"\nMetoda brute-force\t");
    }
    auto startT = chrono::high_resolution_clock::now();//Początek pomiaru czasu
    for(int i=0;i<gSize;i++){
        currPath.insert(currPath.end(),i);
    }
    heap_alg(gSize,currPath,0,startT);
    auto endT = chrono::high_resolution_clock::now();//Koniec pomiaru czasu
    duration = endT - startT;//Czas wykonania zapisany w ns
    if(bfShowProgress){
        clear_input();
    }
    delete bfGraph;
    return bfRes;
}
int bound(int vert,Graph* graph,vector<int>& visited){
    bool pathExist = true;
    int lowerBound = 0;
    for (int i = 0; i < visited.size() - 1; i++) {
        lowerBound += graph->getMatrixItem(visited[i], visited[i + 1]);
        pathExist = graph->pathExist(visited[i], visited[i + 1]);
        if(!pathExist){
            return -1;
        }
    }
    int minOutgoing = INT_MAX;
    for (int i = 0; i < gSize; i++) {
        if (find(visited.begin(), visited.end(), i) == visited.end()) {
            int minEdge = INT_MAX;
            for (int j = 0; j < gSize; j++) {
                if (i != j && find(visited.begin(), visited.end(), j) == visited.end()) {
                    int edgeWeight = graph->getMatrixItem(i, j);
                    if(edgeWeight>0){
                        minEdge = min(minEdge,edgeWeight);
                    }
                }
            }
            if (minEdge < INT_MAX) {
                lowerBound += minEdge;
            }
            int edgeWeight = graph->getMatrixItem(vert, i);
            if(edgeWeight>0){
                minOutgoing = min(minOutgoing, graph->getMatrixItem(vert, i));
            } 
        }
    }
    if (minOutgoing < INT_MAX) {
        lowerBound += minOutgoing;
    }
    return lowerBound;
}
int dfs(){
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    if(showProgress){
        factorial = pow(gSize,gSize-4); 
        fraction = factorial/ (width*20);
        printBar(0,"\nMetoda BNB-bfs\t\t");
    }
    Graph* graph = new Graph(gSize);
    bool pathExist = true; 
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    vector<int> currPath;
    int res,count = 0;
    stack<Node> dfsStack;
    auto startT = chrono::high_resolution_clock::now();//Początek pomiaru czasu
    vector<int> visited = {0};
    res = abs(nearest_neighbour());
    Node root = {visited,0,bound(0,graph,visited)};
    dfsStack.push(root);
    while(!dfsStack.empty() && duration.count()<=maxtime){
        Node currNode = dfsStack.top();
        dfsStack.pop();
        if(currNode.bound>=res){
            continue;
        }
        if(currNode.path.size()==gSize){
            int currRes = currNode.cost + graph->getMatrixItem(currNode.path.back(), currNode.path.front());
            pathExist = graph->pathExist(currNode.path.back(), currNode.path.front());
            if(!pathExist){
                continue;
            }
            if (currRes < res) {
                res = currRes;
                resPath = currNode.path;
                resPath.insert(resPath.end(),currNode.path.at(0));
                if(res==optimalRes){
                    break;
                }
            }
            continue;
        }
        for(int i=0;i<gSize;i++){
            if(find(currNode.path.begin(),currNode.path.end(),i)==currNode.path.end()){
                int newCost = currNode.cost + graph->getMatrixItem(currNode.path.back(), i);
                pathExist = graph->pathExist(currNode.path.back(), i);
                if(!pathExist){
                    continue;
                }
                vector<int> newPath = currNode.path;
                newPath.push_back(i);
                Node child = {newPath, newCost,bound(i,graph,newPath) };
                if (child.bound < res) {
                    dfsStack.push(child);
                }
            }
        }
        auto endT = chrono::high_resolution_clock::now();
        duration = endT - startT;
        if(showProgress){
            count++;
            if(count%fraction==0){
                printBar((double)count/factorial ,"Metoda BNB-bfs\t\t");
            }
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
int bfs(){
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    if(showProgress){
        factorial = pow(gSize,gSize-4); 
        fraction = factorial/ (width*20);
        printBar(0,"\nMetoda BNB-bfs\t\t");
    }
    Graph* graph = new Graph(gSize);
    bool pathExist = true; 
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    vector<int> currPath;
    int res = INT_MAX,count = 0;
    queue<Node> bfsQueue;
    auto startT = chrono::high_resolution_clock::now();//Początek pomiaru czasu
    vector<int> visited = {0};
    Node root = {visited,0,bound(0,graph,visited)};
    bfsQueue.push(root);
    while(!bfsQueue.empty() && duration.count()<=maxtime){
        Node currNode = bfsQueue.front();
        bfsQueue.pop();
        if(currNode.bound>=res){
            continue;
        }
        if(currNode.path.size()==gSize){
            int currRes = currNode.cost + graph->getMatrixItem(currNode.path.back(), currNode.path.front());
            pathExist = graph->pathExist(currNode.path.back(), currNode.path.front());
            if(!pathExist){
                continue;
            }
            if (currRes < res) {
                res = currRes;
                resPath = currNode.path;
                resPath.insert(resPath.end(),currNode.path.at(0));
                if(res==optimalRes){
                    break;
                }
            }
            continue;
        }
        for(int i=0;i<gSize;i++){
            if(find(currNode.path.begin(),currNode.path.end(),i)==currNode.path.end()){
                int newCost = currNode.cost + graph->getMatrixItem(currNode.path.back(), i);
                pathExist = graph->pathExist(currNode.path.back(), i);
                if(!pathExist){
                    continue;
                }
                vector<int> newPath = currNode.path;
                newPath.push_back(i);
                Node child = {newPath, newCost,bound(i,graph,newPath) };
                if (child.bound < res) {
                    bfsQueue.push(child);
                }
            }
        }
        auto endT = chrono::high_resolution_clock::now();
        duration = endT - startT;
        if(showProgress){
            count++;
            if(count%fraction==0){
                printBar((double)count/factorial ,"Metoda BNB-bfs\t\t");
            }
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
int leastCost(){
    int optimalRes = config["instance"]["optimalRes"];
    bool showProgress = config["algorithms"]["showProgress"];
    if(showProgress){
        factorial = pow(gSize,gSize-3); 
        fraction = factorial/ (width*20);
        printBar(0,"\nMetoda BNB-least cost\t");
    }
    int res = INT_MAX,count = 0;
    Graph* graph = new Graph(gSize);
    bool pathExist = true; 
    resPath.clear();
    graph->readFromFile(config["instance"]["inputFile"]);
    vector<int> currPath;
    priority_queue<Node, vector<Node>> leastCostQueue;
    auto startT = chrono::high_resolution_clock::now();//Początek pomiaru czasu
    vector<int> visited = {0};
    Node root = {visited,0,bound(0,graph,visited)};
    leastCostQueue.push(root);
    while (!leastCostQueue.empty()) {
        Node currNode = leastCostQueue.top();
        leastCostQueue.pop();
        if(currNode.bound>=res){
            continue;
        }
        if (currNode.path.size() == gSize) {
            int currRes = currNode.cost + graph->getMatrixItem(currNode.path.back(), 0);
            pathExist = graph->pathExist(currNode.path.back(), 0);
            if(!pathExist){
                continue;
            }
            if(currRes<res){
                res=currRes;
                resPath = currNode.path;
                resPath.insert(resPath.end(),currNode.path.at(0));
                if(res==optimalRes){
                    break;
                }
            }
            continue;
        }
        for (int i = 0; i < gSize; i++) {
            if (find(currNode.path.begin(), currNode.path.end(), i) == currNode.path.end()) {
                int newCost = currNode.cost + graph->getMatrixItem(currNode.path.back(), i);
                pathExist = graph->pathExist(currNode.path.back(), i);
                if(!pathExist){
                    continue;
                }
                vector<int> newPath = currNode.path;
                newPath.push_back(i);
                Node child = {newPath, newCost, bound(i, graph, newPath)};
                if (child.bound < res){
                    leastCostQueue.push(child);
                } 
            }
        }
        auto endT = chrono::high_resolution_clock::now();
        duration = endT - startT;
        if(showProgress){
            count++;
            if(count%fraction==0){
                printBar((double)count/factorial ,"Metoda BNB-bfs\t\t");
            }
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
string optimalPath;
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
        row.append(to_string(res));//Otrzymany wynik
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
void print_rows(int count,int k,int i,int j,int bigGraph){
    system("cls");
    printBar((double)count/5,"Caly program \t\t");
    cout<<"\n";
    printBar((double)k/2,"Symetria \t\t");
    cout<<"\n";
    printBar((double)i/3,"Gestosci \t\t");
    cout<<"\n";
    printBar((double)j/bigGraph,"Rozmiary \t\t");
    cout<<"\n";
}
void testFunc(int count,string funcName,function<int()> func){
    int bigGraph = config["settings"]["bigGraph"];
    int smallGraph = config["settings"]["smallGraph"];
    int iterations = config["settings"]["iterations"];
    int density[] = {30,60,100};
    bool sym = true;
    string fileNameOut = "\\Wyniki\\"+funcName+"resoult.csv";
    config["instance"]["outputFile"] = fileNameOut;
    for(int k=0;k<2;k++){
        for(int i=0;i<3;i++){
            print_rows(count,k,i,0,bigGraph);
            for(int j=smallGraph;j<bigGraph;j++){
                string fileName = "\\Dane\\";
                if(sym){
                    fileName.append("SYM"+to_string(density[i])+"\\");
                }else{
                    fileName.append("ASYM"+to_string(density[i])+"\\");
                }
                fileName.append("GRAF"+to_string(j)+"_"+to_string(density[i])+".txt");
                config["instance"]["inputFile"] = fileName;
                write_header(funcName);
                gSize = j;
                ofstream outputFile(fileNameOut,ios_base::app);
                outputFile<<"Size: "<<gSize<<"\n";
                outputFile.close();
                config["instance"]["optimalRes"]=brute_force();
                for(int l=1;l<=iterations;l++){
                    clear_input();
                    printBar((double)l/iterations,"Iteracje \t\t");
                    duration = chrono::duration<double,nano>::zero();
                    int res = func();
                    iterNum=0;
                    write(res);
                }
                print_rows(count,k,i,j,bigGraph);
            }
            print_rows(count,k,i,0,bigGraph);
        }
        sym=false;
        print_rows(count,k,0,0,bigGraph);
    }
}
void testProc(map<string,function<int()>> funcMap){
    int bigGraph = config["settings"]["bigGraph"];
    width = config["settings"]["barWidth"];
    int count = 0;
    for(const auto& [funcName, func] : funcMap){
        if(funcName == config["algorithms"]["random"]){
            double times[] = {0.01,0.1,1,10,100};
            for(int i=0;i<5;i++){
                maxtime = times[i]*1000000000;
                testFunc(count,funcName, func);
            }
        }else{
            testFunc(count,funcName, func);
        }
        count++;
        print_rows(count,0,0,0,bigGraph);  
    }    
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
        {config["algorithms"]["nearest-neighbour"],nearest_neighbour},
        {config["algorithms"]["brute-force"],brute_force},
        {config["algorithms"]["dfs"],dfs},
        {config["algorithms"]["bfs"],bfs},
        {config["algorithms"]["leastCost"],leastCost},
        {config["algorithms"]["random"],randomAlg}
    };
    //W mapie znajduej odpowiednią funkcje i ją uruchamiam
    if(test){
        testProc(funcMap);
    }
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
            iterNum=0;
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



 