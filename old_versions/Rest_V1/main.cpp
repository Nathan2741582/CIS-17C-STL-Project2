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
         


using namespace std;



/*

=======================================

 

 STD CONCEPTS DEMONSTRATED HERE:

 

=======================================

 */



//User Libraries





//Global Constants

extern string NAMEDICT[];

extern int totalNames;


vector<pair<string,double>> menu = {

        {"Margherita Pizza", 8.99},

        {"Caesar Salad", 6.50},

        {"Spaghetti Bolognese", 10.25},

        {"Grilled Salmon", 14.75},

        {"Chocolate Cake", 5.00}

};


//Function Prototype


void rstInit(int&, int&);

void gstAssign(int&);

void displayMenu(const vector<pair<string,double>>& menu);

//Execution Begins Here!

int main(int argc, char** argv) {

     

    srand(static_cast<unsigned int>(time(0)));

    

    int cap, occTb;

    bool run = true;

    rstInit(cap, occTb);

    

    while(run){

    cout<<"[== Bistro Ahri Guest Management System ==]"<<endl;

    cout<<"Current table capacity: "<<cap<<endl;

    cout<<"Current occupied tables: "<<occTb<<endl;

    cout<<"=========[Options]==========="<<endl;

    

    cout<<"1) View Menu"<<endl;

    cout<<"2) Shuffle Specials"<<endl;

    cout<<"3) Sort Menu (by price)"<<endl;

    cout<<"4) Sort Menu (by name)"<<endl;

    cout<<"5) Seat Next Party"<<endl;

    cout<<"6) Add to Waitlist"<<endl;

    cout<<"7) Serve an Order"<<endl;

    cout<<"8) Undo Last Serve"<<endl;

    cout<<"9) Check Ingredient Stock"<<endl;

    cout<<"10) Add Ingredient to Pantry"<<endl;

    cout<<"11) Report Cheapest & Priciest Dish"<<endl;

    cout<<"12) View Seating Chart"<<endl;

    cout<<"0) Exit"<<endl;

    cout<<"> ";

    int choice;


    cin>>choice;

    switch(choice){

        case 1:


            displayMenu(menu);

            break;

        case 0: run = false;

        break;

        default: cout<<endl;

    }

    

    }

    

    

    random_shuffle(NAMEDICT, NAMEDICT + totalNames);

    

    

  for(int i = 0; i < 50; i++) { //name debug

    cout<<NAMEDICT[i]<<" "; // Print the name with a space

    

    // After every 10th name, print a newline

    if((i + 1) % 10 == 0) {

        cout<<endl;

    }

  }

    pair<int, int> p = make_pair(cap, occTb);

    


   //Exit Stage Right

    return 0;

}


//Function Implementations

void gstAssign(int& occ){

    

}


void rstInit(int& cap, int& occ){

    cap = random()%25+1;

    if(cap <= 5){ //least amount of tables = 4

        cap = 5;

    }

    occ = random()%cap+1;

}


void displayMenu(const vector<pair<string,double>>& menu){

    cout<<"Menu :"<<endl;

    for(vector<pair<string,double>>::const_iterator dish = menu.begin(); dish != menu.end(); dish++)

    {

        cout<<dish->first <<": $"<<dish->second<<endl;

    }

    cout<<"0) Return"<<endl;

    //cin>> return to main menu

} 