/*
 * Author: Nathan James Pamintuan
 * Purpose:  Lab Slope Intercept & STL Demonstration
 * Features: pair, vector, list, stack, queue, set, map
 *            shuffle, sort, find, min_element, max_element
 *            iterators, health-inspector check (runtime)
 */

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <utility>
#include <string>
#include "namedict.h"   // provides NAMEDICT[] and totalNames

using namespace std;

// Function prototypes
void displayMenu(const vector<pair<string,double>>& menu);
void shuffleMenu(vector<pair<string,double>>& menu);
void sortMenuByPrice(vector<pair<string,double>>& menu);
void sortMenuByName(vector<pair<string,double>>& menu);
pair<string,double> findDish(const vector<pair<string,double>>& menu, const string& name);
pair<string,double> getMinPrice(const vector<pair<string,double>>& menu);
pair<string,double> getMaxPrice(const vector<pair<string,double>>& menu);

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    // 1) Table setup using original random capacity
    int cap   = rand() % 50 + 1;    // max tables
    int occTb = rand() % cap  + 1;  // occupied tables

    cout << "[== Bistro Ahri Guest & Order Management ==]\n";
    cout << "Table capacity: " << cap << "\n";
    cout << "Currently occupied tables: " << occTb << "\n\n";

    // 2) Prepare STL containers
    vector<pair<string,double>> menu = {
        {"Margherita Pizza", 8.99},
        {"Caesar Salad", 6.50},
        {"Spaghetti Bolognese", 10.25},
        {"Grilled Salmon", 14.75},
        {"Chocolate Cake", 5.00}
    };
    set<string> pantry    = {"Tomato", "Lettuce", "Basil", "Salmon"};
    set<string> vipGuests;  // can add dynamically

    set<string> healthInspectors;
    const int numInspectors = 600;  // choose how many inspectors
    set<int> inspectorIndices;
    while(inspectorIndices.size() < numInspectors) {
        int idx = rand() % totalNames;
        inspectorIndices.insert(idx);
    }
    for(int idx : inspectorIndices) {
        healthInspectors.insert(NAMEDICT[idx]);
    }

    map<int, list<string>> seatingChart;
    queue<string> waitList;
    stack<string> servedHistory;

    // Initialize tables in seating chart (empty lists)
    for(int tbl = 1; tbl <= occTb; ++tbl) {
        seatingChart.insert(make_pair(tbl, list<string>()));
    }

    // Seed waitlist with shuffled guest names
    random_shuffle(NAMEDICT, NAMEDICT + totalNames);
    for(int i = 0; i < totalNames; ++i) {
        waitList.push(NAMEDICT[i]);
    }

    int nextTable = 1;  
    bool running = true;

    while(running) {
        cout << "\n=== Main Menu ===\n";
        cout << "1) View Menu\n";
        cout << "2) Shuffle Specials\n";
        cout << "3) Sort Menu (by price)\n";
        cout << "4) Sort Menu (by name)\n";
        cout << "5) Seat Next Party\n";
        cout << "6) Add to Waitlist\n";
        cout << "7) Serve an Order\n";
        cout << "8) Undo Last Serve\n";
        cout << "9) Check Ingredient Stock\n";
        cout << "10) Add Ingredient to Pantry\n";
        cout << "11) Report Cheapest & Priciest Dish\n";
        cout << "12) View Seating Chart\n";
        cout << "0) Exit\n";
        cout << "> ";
        int choice;
        cin >> choice;
        cin.ignore(10000,'\n');

        switch(choice) {
            case 1:
                displayMenu(menu);
                break;
            case 2:
                shuffleMenu(menu);
                cout << "Menu shuffled!" << endl;
                break;
            case 3:
                sortMenuByPrice(menu);
                cout << "Menu sorted by price." << endl;
                break;
            case 4:
                sortMenuByName(menu);
                cout << "Menu sorted by name." << endl;
                break;
            case 5: {
                if(waitList.empty()) {
                    cout << "No parties on the waitlist." << endl;
                } else {
                    string guest = waitList.front();
                    waitList.pop();
                    seatingChart[nextTable].push_back(guest);
                    cout << guest << " seated at table " << nextTable << "." << endl;
                    // Check if guest is a health inspector
                    if(healthInspectors.find(guest) != healthInspectors.end()) {
                        cout << "*** Alert: Health Inspector " << guest
                             << " has been seated! ***" << endl;
                    }
                    nextTable = (nextTable % occTb) + 1;
                }
                break;
            }
            case 6: {
                string name;
                cout << "Enter guest name to add: ";
                getline(cin, name);
                waitList.push(name);
                cout << name << " added to waitlist." << endl;
                break;
            }
            case 7: {
                displayMenu(menu);
                string dish;
                cout << "Enter dish name to serve: ";
                getline(cin, dish);
                auto found = findDish(menu, dish);
                if(found.first.empty()) {
                    cout << "Dish not on menu." << endl;
                } else {
                    servedHistory.push(found.first);
                    cout << "Served " << found.first << "." << endl;
                }
                break;
            }
            case 8:
                if(servedHistory.empty()) {
                    cout << "No served orders to undo." << endl;
                } else {
                    string last = servedHistory.top();
                    servedHistory.pop();
                    cout << "Undo serve: " << last << " returned to kitchen." << endl;
                }
                break;
            case 9: {
                string ing;
                cout << "Ingredient name: ";
                getline(cin, ing);
                if(pantry.find(ing) != pantry.end())
                    cout << ing << " is in stock." << endl;
                else
                    cout << ing << " is NOT in stock." << endl;
                break;
            }
            case 10: {
                string ing;
                cout << "Ingredient name to add: ";
                getline(cin, ing);
                pantry.insert(ing);
                cout << ing << " added to pantry." << endl;
                break;
            }
            case 11: {
                auto cheap = getMinPrice(menu);
                auto pricey = getMaxPrice(menu);
                cout << "Cheapest: " << cheap.first << " ($" << cheap.second << ")" << endl;
                cout << "Priciest: " << pricey.first << " ($" << pricey.second << ")" << endl;
                break;
            }
            case 12:
                for(auto tblIt = seatingChart.begin(); tblIt != seatingChart.end(); ++tblIt) {
                    cout << "Table " << tblIt->first << ":\n";
                    for(auto it = tblIt->second.begin(); it != tblIt->second.end(); ++it)
                        cout << "  - " << *it << '\n';
                }
                break;
            case 0:
                running = false;
                break;
            default:
                cout << "Invalid choice." << endl;
        }
    }

    cout << "Goodbye!" << endl;
    return 0;
}

// Function implementations

void displayMenu(const vector<pair<string,double>>& menu) {
    cout << "\n-- Menu --\n";
    for(auto it = menu.begin(); it != menu.end(); ++it) {
        cout << it->first << ": $" << it->second << '\n';
    }
}

void shuffleMenu(vector<pair<string,double>>& menu) {
    random_shuffle(menu.begin(), menu.end());
}

void sortMenuByPrice(vector<pair<string,double>>& menu) {
    sort(menu.begin(), menu.end(), [](auto &a, auto &b){ return a.second < b.second; });
}

void sortMenuByName(vector<pair<string,double>>& menu) {
    sort(menu.begin(), menu.end(), [](auto &a, auto &b){ return a.first < b.first; });
}

pair<string,double> findDish(const vector<pair<string,double>>& menu, const string& name) {
    auto it = find_if(menu.begin(), menu.end(), [&](auto &p){ return p.first == name; });
    if(it != menu.end()) return *it;
    return make_pair(string(), 0.0);
}

pair<string,double> getMinPrice(const vector<pair<string,double>>& menu) {
    return *min_element(menu.begin(), menu.end(), [](auto &a, auto &b){ return a.second < b.second; });
}

pair<string,double> getMaxPrice(const vector<pair<string,double>>& menu) {
    return *max_element(menu.begin(), menu.end(), [](auto &a, auto &b){ return a.second < b.second; });
}
