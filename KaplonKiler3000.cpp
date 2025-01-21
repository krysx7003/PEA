#include <windows.h>
#include <iostream>
#include <iomanip> 
using namespace std;

int width;
void printBar(double progress,string label){
    int done = width*progress;
    cout<<label<<"||";      
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
void clear(){
    for(int j=0;j<60;j++){
        cout<<" ";
    }
    cout<<"\r";
}
int main(){
    width = 20;
    int freeme =0;
    int iterations,count = 1;
    cout<<"Podaj ilosc iteracji: ";
    cin>>iterations;
    do{
        cout<<"Iteracja nr "<<count<<"\n";
        for(int i =0;i<=width;i++){
            printBar((double)i/width,"Ladowanie menu...\t");
            Sleep(500);
            if(i==width-1 && freeme !=1){
                clear();
                for(int j=0;j<width-1;j++){
                    i--;
                    printBar((double)i/width,"Backtracking\t\t");
                    Sleep(500);
                }
                clear();
                freeme++;
            }   
        }
        clear();
        count++;
    }while(count<=iterations);
    cout<<"\n";
    cout<<"Tak jakby dziala\n";
    cout<<"Milego dnia\n";
    system("pause");
    return 0;
}