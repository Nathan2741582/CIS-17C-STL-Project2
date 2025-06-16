/* 
 * Author: Nathan James Pamintuan
 * Purpose:  Lab STL Review 
 */

//User Libraries
#include <iostream>
#include <cstring>
#include "namedict.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <limits>
#include <map>
#include <set>
#include <unordered_set>
#include <list>
#include <iomanip>
using namespace std;




//User Libraries




//Global Constants
extern string NAMEDICT[];
extern int totalNames;


  
pair<string, bool> sysMsg = {"", false};
vector<pair<string,double>> menu = {
        {"Margherita Pizza", 8.99},
        {"Caesar Salad", 6.50},
        {"Spaghetti Bolognese", 10.25},
        {"Grilled Salmon", 14.75},
        {"Chocolate Cake", 5.00}
};

map<int, list<string>> seatingChart;

//Function Prototype



void rstInit(int&, int&, const unordered_set<int>&);
void displayMenu(vector<pair<string,double>>&);
void alert(pair<string, bool>&);
void alert(string);
void srtMenuPrice(vector<pair<string,double>>&);
void srtMenuName(vector<pair<string,double>>&);
void ppuTable(int&, int, const unordered_set<int>&);
void dispChart(int&, int&, const unordered_set<int>&);
void detInsp(unordered_set<int>&);

//Execution Begins Here!
int main(int argc, char** argv) {
    srand(static_cast<unsigned int>(time(0)));
    random_shuffle(NAMEDICT, NAMEDICT + totalNames);
    int cap, occTb;
    unordered_set<int> insp;
    
    bool run = true;
    rstInit(cap, occTb, insp);
    detInsp(insp);
    while(run){
    cout<<"[== Bistro Ahri Restaurant Management System ==]"<<endl;
    cout<<"Current table capacity: "<<cap<<endl;
    cout<<"Current occupied tables: "<<occTb<<endl;
    cout<<"=========[Options]==========="<<endl;
    cout<<"1) Manage Menu"<<endl;
    cout<<"2) Guest Management"<<endl;
  //  cout<<"1) View Menu"<<endl;
  //  cout<<"2) Shuffle Specials"<<endl;
  //  cout<<"3) Sort Menu (by price)"<<endl;
  //  cout<<"4) Sort Menu (by name)"<<endl;
 //   cout<<"5) Seat Next Party"<<endl;
 //   cout<<"6) Add to Waitlist"<<endl;
    cout<<"7) Serve an Order"<<endl;
    cout<<"8) Undo Last Serve"<<endl;
    cout<<"9) Check Ingredient Stock"<<endl;
    cout<<"10) Add Ingredient to Pantry"<<endl;
    cout<<"11) Report Cheapest & Priciest Dish"<<endl;
 //   cout<<"12) View Seating Chart"<<endl;
    cout<<"0) Exit"<<endl;
    alert(sysMsg);
    
    cout<<"> ";
    
    int choice;
    cin>>choice;
    switch(choice){
        case 1:
            displayMenu(menu);
            break;
        case 2:
            dispChart(cap, occTb, insp);
            break;
        case 3:
            break;
        case 0: run = false;
        
        break;
        default: alert("Invalid entry");
    }
    
    }
    
    
  
    
    
 

   //Exit Stage Right 
    return 0;
}

//Function Implementations


void rstInit(int& cap, int& occ, const unordered_set<int>& a){
    cap = random()%25+1;
    if(cap <= 5){ //least amount of tables = 4
        cap = 5;
    }
    int initP = random()%cap+1;
    occ = 0;
    
    
    for(int i = 1; i <= cap; ++i) { //set capacity
    seatingChart.insert(pair<int, list<string>>(i, list<string>()));
}
    ppuTable(occ, initP, a);
}

void displayMenu(vector<pair<string,double>>& menu){
    bool m_run = true;
    while(m_run){
        int mnu;
    cout<<"======"<<endl;
    cout<<"Menu :"<<endl;
    cout<<"======"<<endl;
    for(vector<pair<string,double>>::const_iterator dish = menu.begin(); dish != menu.end(); dish++)
    {
        cout<<dish->first <<": $"<<fixed<<setprecision(2)<<dish->second<<endl;
    }
    
    cout<<"1) Shuffle Menu"<<endl;
    cout<<"2) Sort Menu (by price)"<<endl;
    cout<<"3) Sort Menu (by name)"<<endl;
    cout<<"0) Previous"<<endl;
    alert(sysMsg);
    cout<<"> ";    
    cin>>mnu;
    switch(mnu){
        case 1:  
             random_shuffle(menu.begin(), menu.end());
             alert("Menu Shuffled");
             break;
        case 2:
            srtMenuPrice(menu);
            alert("Menu sorted by price.");
            break;
        case 3:
            srtMenuName(menu);
            alert("Menu sorted by name.");
            break;
        case 0: m_run = false;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        break;
        default: alert("Invalid entry");
        
    }

    
    }
        
}

void srtMenuPrice(vector<pair<string,double>>& a){
     sort(a.begin(), a.end(), [](auto &a, auto &b){ return a.second < b.second; });
}


void srtMenuName(vector<pair<string,double>>& a) {
    sort(a.begin(), a.end(), [](auto &a, auto &b){ return a.first < b.first; });
}

void detInsp(unordered_set<int>& a){ //shuffle names and only take the first 40% of elements, pass indicies of names to unordered set
    int insDet[totalNames];
    int insNum = 0;
    //iota(insDet, insDet + totalNames, 0);
    for(int i = 0; i < totalNames; i++){
        insDet[i] = i;
    }
    random_shuffle(insDet, insDet + totalNames);
    
    insNum = static_cast<int>(totalNames * 0.4);
    for(int i = 0; i < insNum; i++){
        a.insert(insDet[i]);
    }
    
   // for(auto j = a.begin(); j != a.end(); j++){ //debug
   //     cout<<*j<<" ";
   // }
    
    
}

void ppuTable(int& occ, int pty, const unordered_set<int>& insp){
    static int idxN = 0;      
    static bool init = true;  

    for(int party = 0; party < pty; ++party){
        bool seated = false;
      

        
        for(auto i = seatingChart.begin(); i != seatingChart.end(); i++){
            if(i->second.empty()){
                int rnd = rand() % 100 + 1;
                int size = (rnd <= 50 ? 1
                          : rnd <= 70 ? 2
                          : rnd <= 85 ? 3
                          : 4);
             
                for(int j = 0; j < size; ++j){
                    if(idxN < totalNames){
                      
                        if(!init && insp.count(idxN)){
                            alert("Inspector " + NAMEDICT[idxN] +
                                  " seated at table " + to_string(i->first));
                        }
                        i->second.push_back(NAMEDICT[idxN++]);
                    }
                    else {
                        i->second.push_back("Guest");
                    }
                }
              
                occ++;
                seated = true;
                break;  
            }
        }
        if(!seated && !init){
            alert("No tables free, party added to waitlist");
            break; 
        }
    }
    init = false;
}

void dpuTable(int& a, const unordered_set<int>& b){
    //a = occupied tables, b = inspector set
    int toLeave = rand()%3; 
    if(toLeave == 0){
        alert("No tables freed");
        return;
    }
    
    //randomize tables to leave
    vector<int> occTables;
    for(auto j = seatingChart.begin(); j != seatingChart.end(); j++){
        if(!j->second.empty()){
            occTables.push_back(j->first);
        }
    }
    
    random_shuffle(occTables.begin(), occTables.end());
    
    int freedT = 0;
    
    for(auto k = occTables.begin(); k != occTables.end() && freedT < toLeave; k++){
        int tableId = *k;
        auto tableIter = seatingChart.find(tableId);
        
        for(auto& guestName : tableIter->second){
            for(int iIdx : b){
                if(iIdx < totalNames && guestName == NAMEDICT[iIdx]){
                    alert("Inspector " + guestName + " has left table " + to_string(tableId));
                    break;
                }
            }
        }
        tableIter->second.clear();
        freedT++;
        a--;
        alert("Table "+to_string(tableId)+" has been freed");
    }
}

void dispChart(int& a, int& b, const unordered_set<int>& c){
    bool c_run = true;
    while(c_run){
        int mnu;
    for(auto i = seatingChart.begin(); i != seatingChart.end(); ++i) { //display each table
        cout<<"Table "<<i->first<<":"<<endl;
        for(auto it = i->second.begin(); it != i->second.end(); ++it){
            cout << "  - " << *it << '\n';
        } //display each member
                        
         }
        cout<<"1) Seat Next Party"<<endl;
        cout<<"2) Wait for Free Tables"<<endl;
        cout<<"0) Previous"<<endl;
        alert(sysMsg);
        cout<<"> ";
        cin>>mnu;
        switch(mnu){
            case 1:
                 ppuTable(a, 1, c);
                break;
            case 2:
                dpuTable(a, c);
                break;
            case 0:
                c_run = false;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            default: alert("Invalid entry");
                
        }
    }
}
    



void alert(pair<string, bool>&a){
    if(a.second == true){
        cout<<"!! "<<a.first<<" !!"<<endl;
        a.second = false;
        a.first = "";
    }
}

void alert(string a){
    sysMsg.first = a;
    sysMsg.second = true;
}


