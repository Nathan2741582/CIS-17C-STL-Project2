/*
 * Author: Nathan James Pamintuan
 * Purpose: Lab STL Review (Refactored Restaurant Sim)
 */

// User Libraries
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <stack>
#include <queue>
#include <deque>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <iomanip>
#include <numeric> 

#include "namedict.h" 

using namespace std;


pair<string, bool> sysMsg = {"", false};

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void alert(pair<string, bool>& a) {
    if (a.second == true) {
        cout<<"!! " << a.first << " !!" << endl;
        a.second = false;
        a.first = "";
    }
}

void alert(string msg) {
    sysMsg.first = msg;
    sysMsg.second = true;
}


class RestaurantSystem; 
class subMenu;         
class ManageMenu;       
class GuestManagement; 
class KitchenOps;       

class subMenu {
public:
    subMenu(RestaurantSystem& rs); 
    virtual ~subMenu() {}

    virtual void run() {
        bool runMenu = true;
        while (runMenu) {
            displayHeader(getTitle());
            dispContent();
            cout<<endl;
            displayFooter();
            alert(sysMsg);
            cout<<"> ";

            int ch;
            cin >> ch;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                alert("Invalid input. Please enter a number.");
                continue;
            }

         

            if (ch == 0) {
                runMenu = false;
            } else {
                if (!handleChoice(ch)) {
                    alert("Invalid menu option.");
                }
            }
           
        }
    }

    virtual string getTitle() = 0;
    virtual void dispContent() = 0;
    virtual bool handleChoice(int ch) { return false; }

    virtual void displayHeader(const string& title) {
        clearScreen();
        cout<<"--- [ " << title << " ] ---" << endl << endl;
    }

    virtual void displayFooter() {
        cout<<"--------------------------" << endl;
        cout<<"0) Return to Main Menu" << endl;
    }

protected:
    RestaurantSystem& restaurant; 
};




class ManageMenu : public subMenu {
public:
    ManageMenu(RestaurantSystem& rs); 

    string getTitle() override { return "Menu Management"; }

    void dispContent() override; 

    bool handleChoice(int ch) override;
    };

class GuestManagement : public subMenu {
public:
    GuestManagement(RestaurantSystem& rs); 

    string getTitle() override { return "Guest & Seating Management"; }

    void dispContent() override; 

    bool handleChoice(int ch) override; 
};

class KitchenOps : public subMenu {
public:
     KitchenOps(RestaurantSystem& rs); 

     string getTitle() override { return "Kitchen Operations"; }

     void dispContent() override; 

     bool handleChoice(int ch) override; 
};


class RestaurantSystem {
private:
 
    vector<pair<string, double>> menu;
    map<int, list<string>> seatingChart;
    unordered_set<int> inspectorIndices;
    queue<string> waitlist;
    stack<string> orderHistory;
    map<string, int> pantryStock;

    int capacity;
    int occupiedTables;
    int nextNameIndex;


    void initializeRestaurant() {
        capacity = rand() % 20 + 5; // Tables between 5 and 24
        occupiedTables = 0;
        nextNameIndex = 0;
        seatingChart.clear();
        while (!waitlist.empty()) waitlist.pop(); // Clear queue
        while (!orderHistory.empty()) orderHistory.pop(); // Clear stack
        pantryStock = { {"Flour", 10}, {"Tomato", 20}, {"Cheese", 15}, {"Salmon", 5} };

        for (int i = 1; i <= capacity; ++i) {
            seatingChart[i] = list<string>();
        }
        determineInspectors();
        int initialParties = capacity > 1 ? rand() % (capacity / 2) + 1 : (capacity == 1 ? 1 : 0);
        populateTable(initialParties, true);
    }

    void determineInspectors() {
        inspectorIndices.clear();
        if (totalNames <= 0) return;

        vector<int> indices(totalNames);
        iota(indices.begin(), indices.end(), 0);
        random_shuffle(indices.begin(), indices.end());

        int numInspectors = static_cast<int>(totalNames * 0.4);
        for (int i = 0; i < numInspectors && i < totalNames; ++i) {
            inspectorIndices.insert(indices[i]);
        }
    }

    void addPartyToWaitlist() {
        string partyName = "Party" + to_string(waitlist.size() + occupiedTables + 1); 
        waitlist.push(partyName);
        alert(partyName + " added to waitlist. Current waitlist size: " + to_string(waitlist.size()));
    }


public:

    friend class ManageMenu;
    friend class GuestManagement;
    friend class KitchenOps;
    friend class subMenu; 

    RestaurantSystem() : capacity(0), occupiedTables(0), nextNameIndex(0) {
        srand(static_cast<unsigned int>(time(0)));
        menu = {
            {"Margherita Pizza", 8.99}, {"Caesar Salad", 6.50},
            {"Spaghetti Bolognese", 10.25}, {"Grilled Salmon", 14.75},
            {"Chocolate Cake", 5.00}
        };
         if (totalNames > 0) {
            random_shuffle(NAMEDICT, NAMEDICT + totalNames);
         }
        initializeRestaurant();
    }


    void populateTable(int numPartiesToSeat, bool initial = false) {
        for (int party = 0; party < numPartiesToSeat; ++party) {
             if (occupiedTables >= capacity) { 
                 if (!initial) addPartyToWaitlist();
                 break; // Stop trying if full
             }
            bool seated = false;
            for (auto it = seatingChart.begin(); it != seatingChart.end(); ++it) {
                if (it->second.empty()) {
                    int tableId = it->first;
                    int partySize = (rand() % 4) + 1;
                    string partySummary = "Party of " + to_string(partySize) + " seated at Table " + to_string(tableId) + ":";

                    for (int j = 0; j < partySize; ++j) {
                        string guestName;
                        int index = nextNameIndex % totalNames;
                        guestName = NAMEDICT[index];
                        if(inspectorIndices.count(index)){
                             alert("ALERT: Inspector " + guestName + " seated at table " + to_string(tableId));
                        }
                        nextNameIndex++;
                      
                        it->second.push_back(guestName);
                        partySummary += " " + guestName;
                        }
                    occupiedTables++;
                    seated = true;
               //     if (!initial) alert(partySummary);
                    break;
                }
            }
            if (!seated && !initial) {
                 addPartyToWaitlist();
                 // break; 
            } else if (!seated && initial) {
                 break;
            }
        }
    }

    void depopulateTable() {
        if (occupiedTables == 0) {
            alert("No occupied tables to free.");
            return;
        }
        int numToLeave = rand() % 3;
        if (numToLeave == 0) {
            alert("No parties left this time.");
            return;
        }
        vector<int> currentlyOccupiedTables;
        for (const auto& pair : seatingChart) {
            if (!pair.second.empty()) {
                currentlyOccupiedTables.push_back(pair.first);
            }
        }
        if (currentlyOccupiedTables.empty()) {
             occupiedTables = 0;
             return;
        }
        random_shuffle(currentlyOccupiedTables.begin(), currentlyOccupiedTables.end());
        int freedCount = 0;
        for (int i = 0; i < numToLeave && i < currentlyOccupiedTables.size(); ++i) {
            int tableId = currentlyOccupiedTables[i];
            auto tableIter = seatingChart.find(tableId);
            if (tableIter != seatingChart.end() && !tableIter->second.empty()) {
                 for(const string& guestName : tableIter->second) {
                      for(int inspIndex : inspectorIndices) {
                           if (inspIndex < totalNames && guestName == NAMEDICT[inspIndex]) {
                                alert("ALERT: Inspector " + guestName + " has left table " + to_string(tableId));
                                break;
                           }
                      }
                 }
                tableIter->second.clear();
                occupiedTables--;
                freedCount++;
                alert("Table " + to_string(tableId) + " has been freed.");
            }
        }
        if (freedCount > 0 && !waitlist.empty()) {
             int canSeatNow = capacity - occupiedTables;
             int numToSeatFromWaitlist = min((int)waitlist.size(), canSeatNow);
             if (numToSeatFromWaitlist > 0) {
                  alert("Attempting to seat " + to_string(numToSeatFromWaitlist) + " parties from waitlist...");
                  for(int i = 0; i < numToSeatFromWaitlist; ++i) {
                       if(occupiedTables >= capacity) break; 
                       string partyName = waitlist.front();
                       waitlist.pop();
                       alert("Seating " + partyName + " from waitlist...");
                       populateTable(1); 
                  }
             }
        }
    }

  
     void runMainLoop() {
        bool run = true;
  
        ManageMenu menuMgr(*this);
        GuestManagement guestMgr(*this);
        KitchenOps kitchenMgr(*this);

        while (run) {
            clearScreen();
            cout<<"[== Bistro Ahri Restaurant Management System ==]" << endl;
            cout<<"Tables: " << occupiedTables << " occupied / " << capacity << " total" << endl;
            cout<<"Waitlist Size: " << waitlist.size() << endl;
            cout<<"Orders Served (for undo): " << orderHistory.size() << endl;
            cout<<"=========[ Main Options ]===========" << endl;
            cout<<"1) Manage Menu" << endl;
            cout<<"2) Guest Management" << endl;
            cout<<"3) Kitchen Operations" << endl;
            cout<<"------------------------------------" << endl;
            cout<<"0) Exit System" << endl;
            cout<<"====================================" << endl;

            alert(sysMsg);
            cout<<"> ";

            int choice;
            cin >> choice;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                alert("Invalid input. Please enter a number.");
                continue;
            }
           
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (choice) {
                case 1: menuMgr.run(); break;
                case 2: guestMgr.run(); break;
                case 3: kitchenMgr.run(); break;
                case 0: run = false; cout<<"Exiting system." << endl; break;
                default: alert("Invalid main menu option."); break;
            }
        }
    }

}; 

subMenu::subMenu(RestaurantSystem& rs) : restaurant(rs) {}


ManageMenu::ManageMenu(RestaurantSystem& rs) : subMenu(rs) {}


bool ManageMenu::handleChoice(int ch) {
    switch (ch) {
        case 1: // Sort Price
            sort(restaurant.menu.begin(), restaurant.menu.end(),
                 [](const auto& a, const auto& b) { return a.second < b.second; });
            alert("Menu sorted by price.");
            return true;
        case 2: // Sort Name
            sort(restaurant.menu.begin(), restaurant.menu.end(),
                 [](const auto& a, const auto& b) { return a.first < b.first; });
            alert("Menu sorted by name.");
            return true;
        case 3: // Shuffle
            random_shuffle(restaurant.menu.begin(), restaurant.menu.end());
            alert("Menu shuffled.");
            return true;
        case 4: // Min/Max
            { // Start scope for case 4
                if(restaurant.menu.empty()) {
                    alert("Menu is empty.");
                    return true;
                }
                auto minIt = min_element(restaurant.menu.begin(), restaurant.menu.end(),
                                       [](const auto& a, const auto& b){ return a.second < b.second; });
                auto maxIt = max_element(restaurant.menu.begin(), restaurant.menu.end(),
                                       [](const auto& a, const auto& b){ return a.second < b.second; });

             
                stringstream ss_min, ss_max;
                ss_min << fixed << setprecision(2) << minIt->second;
                ss_max << fixed << setprecision(2) << maxIt->second;

                alert("Cheapest: " + minIt->first + "($" + ss_min.str() + ")" +
                    " / Priciest: " + maxIt->first + "($" + ss_max.str() + ")");
            } 
            return true;
        default:
            return false;
    }
}

void ManageMenu::dispContent() {
    cout<<"Current Menu:" << endl;
    cout<<"-------------" << endl;
    if (restaurant.menu.empty()) {
        cout<<"(Menu is empty)" << endl;
    } else {
        int count = 1;
        cout<<fixed << setprecision(2); 
        for (const auto& item : restaurant.menu) {
            cout<<setw(2) << count++ << ") " << left << setw(25) << item.first
                 << " $" << item.second << endl;
        }
    }
    cout<<"-------------" << endl;
    cout<<"1) Sort by Price (Asc)" << endl;
    cout<<"2) Sort by Name (Asc)" << endl;
    cout<<"3) Shuffle Menu" << endl;
    cout<<"4) Show Cheapest/Priciest Item" << endl;
}




GuestManagement::GuestManagement(RestaurantSystem& rs) : subMenu(rs) {}

void GuestManagement::dispContent() {
    cout<<"Seating Chart (Tables: " << restaurant.occupiedTables << "/" << restaurant.capacity << "):" << endl;
    cout<<"--------------------------" << endl;
    for (const auto& pair : restaurant.seatingChart) {
        cout<<"Table " << setw(2) << pair.first << ": ";
        if (pair.second.empty()) {
            cout<<"(Empty)" << endl;
        } else {
            string guests;
            for (const string& name : pair.second) {
                guests += name + ", ";
            }
             if (!guests.empty()) guests = guests.substr(0, guests.length() - 2);
            cout<<guests << endl;
        }
    }
     cout<<"--------------------------" << endl;
     cout<<"Waitlist (" << restaurant.waitlist.size() << " parties): ";
     if (restaurant.waitlist.empty()) {
         cout<<"(Empty)" << endl;
     } else {
         queue<string> tempQ = restaurant.waitlist;
         string qStr;
         int count = 0;
         while (!tempQ.empty() && count < 5) { 
             qStr += tempQ.front() + ", ";
             tempQ.pop();
             count++;
         }
         if (!qStr.empty()) qStr = qStr.substr(0, qStr.length() - 2);
         if (restaurant.waitlist.size() > 5) qStr += " ...";
         cout<<qStr << endl;
     }
     cout<<"--------------------------" << endl;
    cout<<"1) Seat Next Party (if space)" << endl;
    cout<<"2) Add Party to Waitlist (Manual)" << endl;
    cout<<"3) Simulate Parties Leaving" << endl;
    cout<<"4) Seat from Waitlist (if space)" << endl;
}

bool GuestManagement::handleChoice(int ch) {
    switch (ch) {
        case 1: // Seat Next
            if (restaurant.occupiedTables < restaurant.capacity) {
                alert("Attempting to seat 1 new party...");
                restaurant.populateTable(1);
            } else {
                alert("Restaurant is full. Add party to waitlist?");
            }
            return true;
         case 2: // Add Waitlist
             restaurant.addPartyToWaitlist();
             return true;
        case 3: // Parties Leave
            alert("Simulating parties leaving...");
            restaurant.depopulateTable();
            return true;
         case 4: // Seat from Waitlist
             if (restaurant.waitlist.empty()) {
                  alert("Waitlist is empty.");
             } else if (restaurant.occupiedTables >= restaurant.capacity) {
                  alert("Restaurant is full, cannot seat from waitlist now.");
             } else {
                  string partyName = restaurant.waitlist.front();
                  restaurant.waitlist.pop();
                  alert("Attempting to seat " + partyName + " from waitlist...");
                  restaurant.populateTable(1); // Try to seat one party
             }
             return true;
        default:
            return false;
    }
}



KitchenOps::KitchenOps(RestaurantSystem& rs) : subMenu(rs) {}



void KitchenOps::dispContent() {
      cout<<"Pantry Stock:" << endl;
      cout<<"-------------" << endl;
      if (restaurant.pantryStock.empty()) {
           cout<<"(Pantry is empty)" << endl;
      } else {
           for (const auto& item : restaurant.pantryStock) {
                cout<<"- " << left << setw(20) << item.first << ": " << item.second << endl;
           }
      }
      cout<<"-------------" << endl;
      cout<<"Order History (for undo): " << restaurant.orderHistory.size() << " items" << endl;
      if (!restaurant.orderHistory.empty()) {
           cout<<"Last Order: " << restaurant.orderHistory.top() << endl;
      }
      cout<<"-------------" << endl;
      cout<<"1) Serve Random Order" << endl;
      cout<<"2) Undo Last Served Order" << endl;
      cout<<"3) Check Ingredient Stock" << endl;
      cout<<"4) Add Ingredient to Pantry" << endl;
 }

bool KitchenOps::handleChoice(int ch) {
     switch (ch) {
           case 1: { //Serve Order
                if (restaurant.menu.empty()) {
                     alert("No menu items to serve!");
                     return true;
                }
                int randIndex = rand() % restaurant.menu.size();
                string orderDesc = restaurant.menu[randIndex].first;
               
                bool canMake = true;
                if (orderDesc == "Margherita Pizza" && (restaurant.pantryStock["Tomato"] <= 0 || restaurant.pantryStock["Cheese"] <= 0)) canMake = false;
                if (orderDesc == "Grilled Salmon" && restaurant.pantryStock["Salmon"] <= 0) canMake = false;
          

                if (canMake) {
                     restaurant.orderHistory.push("Served: " + orderDesc);
                     alert("Served " + orderDesc + ". Added to history.");
                    
                } else {
                     alert("Cannot serve " + orderDesc + ", insufficient ingredients!");
                }
                return true;
           }
           case 2: { // Undo Serve
                if (restaurant.orderHistory.empty()) {
                     alert("No orders in history to undo.");
                } else {
                     string lastOrder = restaurant.orderHistory.top();
                     restaurant.orderHistory.pop();
                     alert("Undid last action: " + lastOrder);
                     
                }
                return true;
           }
           case 3: { // Check Stock
                string ingredient;
                cout<<"Enter ingredient name to check: ";
                getline(cin, ingredient); 

                auto it = restaurant.pantryStock.find(ingredient);
                if (it != restaurant.pantryStock.end()) {
                     alert("Stock for " + ingredient + ": " + to_string(it->second));
                } else {
                     alert(ingredient + " not found in pantry stock.");
                }
                return true;
           }
           case 4: { // Add Stock
                string ingredient;
                int quantity;
                cout<<"Enter ingredient name to add/update: ";
                getline(cin, ingredient);
                cout<<"Enter quantity to add: ";
                cin >> quantity;
                if (cin.fail() || quantity < 0) {
                     cin.clear();
                     cin.ignore(numeric_limits<streamsize>::max(), '\n');
                     alert("Invalid quantity.");
                     return true;
                }
                 cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

                restaurant.pantryStock[ingredient] += quantity;
                alert("Added " + to_string(quantity) + " to " + ingredient + ". New total: " + to_string(restaurant.pantryStock[ingredient]));
                return true;
           }
           default:
                return false;
      }
}


int main(int argc, char** argv) {

    RestaurantSystem bistro;
    bistro.runMainLoop();

    return 0;
}