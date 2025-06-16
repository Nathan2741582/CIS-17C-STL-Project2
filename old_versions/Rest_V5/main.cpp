/* 
 * File:   Restaurant.cpp
 * Author: Nathan
 */

#include <iostream>
#include <vector>
#include <list>
//#include <set>
#include <map>
#include <unordered_set>
#include <stack>
#include <queue>
//#include <deque>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <iomanip>
#include <numeric>
#include <sstream>
//things to add for future versions:
//alert queue for multiline alert handling, better gameplay with health inspectors and ratings
#include "namedict.h"

using namespace std;

pair<string, bool> sysMsg = {"", false};

void clrScrn() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void alert(pair<string, bool>& a) {
    if (a.second == true) {
        cout<<"!! "<<a.first<<" !!"<<endl;
        a.second = false;
        a.first = "";
    }
}

void alert(string msg) {
    sysMsg.first = msg;
    sysMsg.second = true;
}

class ResSys;
class SubMenu;
class MenMgr;
class GuestMgr;
class KitchOps;

class SubMenu {
public:
    SubMenu(ResSys& rs);
    virtual ~SubMenu() {}

    virtual void run() {
        bool runMenu = true;
        while (runMenu) {
            dispHead(getTitle());
            dispCont();
            cout<<endl;
            dispFoot();
            alert(sysMsg);
            cout<<">";

            int ch;
            cin>>ch;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                alert("Invalid input. Please enter a number.");
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');


            if (ch == 0) {
                runMenu = false;
            } else {
                if (!handleCh(ch)) {
                    alert("Invalid menu option.");
                }
            }
        }
    }

    virtual string getTitle() = 0;
    virtual void dispCont() = 0;
    virtual bool handleCh(int ch) { return false; }

    virtual void dispHead(const string& title) {
        clrScrn();
        cout<<"--- [ "<<title<<" ] ---"<<endl<<endl;
    }

    virtual void dispFoot() {
        cout<<"--------------------------"<<endl;
        cout<<"0) Return to Main Menu"<<endl;
    }

protected:
    ResSys& resto;
};


class MenMgr : public SubMenu {
public:
    MenMgr(ResSys& rs);

    string getTitle() override { return "Menu Management"; }
    void dispCont() override;
    bool handleCh(int ch) override;
};

class GuestMgr : public SubMenu {
public:
    GuestMgr(ResSys& rs);

    string getTitle() override { return "Guest & Seating Management"; }
    void dispCont() override;
    bool handleCh(int ch) override;
};

class KitchOps : public SubMenu {
public:
     KitchOps(ResSys& rs);

     string getTitle() override { return "Kitchen Operations"; }
     void dispCont() override;
     bool handleCh(int ch) override;
};


class ResSys {
private:
    vector<pair<string, float>> menu;
    map<int, list<string>> seatMap;
    unordered_set<int> inspIdx;
    queue<string> waitLst;
    stack<string> ordHist;
    map<string, int> pantry;

    int capac;
    int occTbls;
    int nextIdx;

    float restCash;
    float restRate;
    bool isEnded;

    const float INITIAL_BALANCE = 1000.0;
    const float CUSTOMER_CHARGE = 12.50;
    const float INSPECTOR_BONUS = 50.0;
    
    const float RATING_STEP = 0.1;

    int day;
    float hour;
    const float MAX_H = 8.0;
    
    void initRest() {
        day = 1;
        hour = 0.0;
        capac = rand() % 20 + 5;
        occTbls = 0;
        nextIdx = 0;
        seatMap.clear();
        while (!waitLst.empty()) waitLst.pop();
        while (!ordHist.empty()) ordHist.pop();
        pantry = { {"Flour", 10}, {"Tomato", 20}, {"Cheese", 15}, {"Salmon", 5} };

        for (int i = 1; i <= capac; ++i) {
            seatMap[i] = list<string>();
        }
        detInsp();

        restCash = INITIAL_BALANCE;
        restRate = 3.0;
        isEnded = false;

        int initPrty = capac > 1 ? rand() % (capac / 2) + 1 : (capac == 1 ? 1 : 0);
        popTbl(initPrty, true);
    }

    void detInsp() {
        inspIdx.clear();
        if (totalNames <= 0) return;

        vector<int> indices(totalNames);
        iota(indices.begin(), indices.end(), 0);
        random_shuffle(indices.begin(), indices.end());

        int numInsp = static_cast<int>(totalNames * 0.1);
        for (int i = 0; i < numInsp && i < totalNames; ++i) {
            inspIdx.insert(indices[i]);
        }
    }

    void addWait() {
    string prtyNm = "Party" + to_string(waitLst.size() + occTbls + 1);
        waitLst.push(prtyNm);
        alert(prtyNm + " added to waitlist. Current waitlist size: " + to_string(waitLst.size()));
    }


public:
    friend class MenMgr;
    friend class GuestMgr;
    friend class KitchOps;
    friend class SubMenu;

    ResSys() : capac(0), occTbls(0), nextIdx(0), day(1), hour(0.0) {
        srand(static_cast<unsigned int>(time(0)));
        menu = {
            {"Margherita Pizza", 8.99}, {"Caesar Salad", 6.50},
            {"Spaghetti Bolognese", 10.25}, {"Grilled Salmon", 14.75},
            {"Chocolate Cake", 5.00}
        };
         if (totalNames > 0) {
            random_shuffle(NAMEDICT, NAMEDICT + totalNames);
         }

         restCash = INITIAL_BALANCE;
         restRate = 3.0;
         isEnded = false;

        initRest();
    }

    void popTbl(int numSeat, bool init = false) {
        for (int party = 0; party < numSeat; ++party) {
             if (occTbls >= capac) {
                 if (!init) addWait();
                 break;
             }
            bool seated = false;
            for (auto it = seatMap.begin(); it != seatMap.end(); ++it) {
                if (it->second.empty()) {
                    int tblId = it->first;
                    int prtySz = (rand() % 4) + 1;
                    string prtySumm = "Party of " + to_string(prtySz) + " seated at Table " + to_string(tblId) + ":";

                    for (int j = 0; j < prtySz; ++j) {
                        string gstNm;
                        if (nextIdx < totalNames) {
                            gstNm = NAMEDICT[nextIdx];
                            if (inspIdx.count(nextIdx)) {
                                alert("ALERT: Inspector " + gstNm + " seated at table " + to_string(tblId));
                                improveRating();
                            }
                            nextIdx++;
                        } else {
                            gstNm = "Guest" + to_string(rand() % 1000);
                        }
                        it->second.push_back(gstNm);
                        prtySumm += " " + gstNm;
                    }
                    occTbls++;
                    seated = true;
                    break;
                }
            }
            if (!seated && !init) {
                 addWait();
            } else if (!seated && init) {
                 break;
            }
        }
    }

    void depopTbl(bool msg) {
        
        if (occTbls == 0) {
            if(msg){
                alert("No occupied tables to free.");
            }
            return;
        }
        int numLv = rand() % 3;
        if (numLv == 0) {
            if(msg){
                alert("No parties left this time.");
            }
            return;
        }

        vector<int> currOcc;
        for (const auto& pair : seatMap) {
            if (!pair.second.empty()) {
                currOcc.push_back(pair.first);
            }
        }

        if (currOcc.empty()) {
             occTbls = 0;
             return;
        }

        random_shuffle(currOcc.begin(), currOcc.end());

        int fredCnt = 0;
        for (int i = 0; i < numLv && i < currOcc.size(); ++i) {
            int tblId = currOcc[i];
            auto tblItr = seatMap.find(tblId);
            if (tblItr != seatMap.end() && !tblItr->second.empty()) {
                 for(const string& gstNm : tblItr->second) {
                      for(int inspId : inspIdx) {
                           if (inspId < totalNames && gstNm == NAMEDICT[inspId]) {
                               if(msg) {
                                   alert("ALERT: Inspector " + gstNm + " has left table " + to_string(tblId));
                               }
                                break;
                           }
                      }
                 }
                tblItr->second.clear();
                occTbls--;
                fredCnt++;
               if(msg){
                   alert("Table " + to_string(tblId) + " has been freed.");
               }
            }
        }

        if (fredCnt > 0 && !waitLst.empty()) {
             int canSeat = capac - occTbls;
             int numWait = min((int)waitLst.size(), canSeat);
             if (numWait > 0) {
                if(msg)  {
                    alert("Attempting to seat " + to_string(numWait) + " parties from waitlist...");
                }
                  for(int i = 0; i < numWait; ++i) {
                       if(occTbls >= capac) break;
                       string prtyNm = waitLst.front();
                       waitLst.pop();
                      if(msg){
                          alert("Seating " + prtyNm + " from waitlist...");
                      }
                       popTbl(1);
                  }
             }
        }
    }

    void chgDaily() {
        if (isEnded) {
            return;
        }
        int dayExp;
        dayExp = 300 + rand()%100 + 1;
        restCash -= dayExp;
        
        alert("Charged $" + to_string(dayExp) + " for daily expenses.");
        if (restCash <= 0) {
            restCash = 0;
            isEnded = true;
            alert("GAME OVER: Your restaurant has gone bankrupt!");
        }
    }
    
    void endDay(){
        
        int tblFreed = 0;
        for(auto& pair : seatMap){
            if(!pair.second.empty()){
                int tblId = pair.first;
            
            for (const string& gstNm : pair.second) { 
                for (int inspId : inspIdx) {
                    if (inspId < totalNames && gstNm == NAMEDICT[inspId]) {
                        alert("ALERT: Inspector " + gstNm + " has left table " + to_string(tblId));
                        break;
                        }
                    }
                }
            }
            pair.second.clear();
            tblFreed++;
        }
        occTbls = 0;
        alert("End of Day: "+to_string(tblFreed)+" table(s) have been cleared. EOD daily expenses charged.");
        chgDaily();
    }
    void earnFromOrder(const string& dish) {
        if (isEnded) return;

        float price = 0.0;
        for (const auto& menuItem : menu) {
            if (menuItem.first == dish) {
                price = menuItem.second;
                break;
            }
        }

        float earnings = CUSTOMER_CHARGE + price;

        float ratingBonus = 1.0 + (restRate - 3.0) / 10.0;
        if (ratingBonus < 0.8){
            ratingBonus = 0.8;
        }
        if (ratingBonus > 1.2){
            ratingBonus = 1.2;
        }

        float finalEarnings = earnings * ratingBonus;
        restCash += finalEarnings;

        stringstream ss;
        ss<<fixed<<setprecision(2)<<finalEarnings;
        alert("Earned $" + ss.str() + " from serving " + dish + " (Rating Bonus Applied)");
    }

    void improveRating() {
        if (isEnded) return;

        restRate += RATING_STEP;
        if (restRate > 5.0) {
            restRate = 5.0;
        }

        stringstream ss;
        ss<<fixed<<setprecision(1)<<restRate;
    }

    bool isGameEnded() const {
        return isEnded;
    }

    
    void advTime(bool rnd = true) {
        float timeAdv = 0.1; 
        
        if (rnd) {
            timeAdv = 0.25 + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 0.5;
        }
        
        hour += timeAdv;
        
        if (hour >= MAX_H) {
            if (!isEnded) {
                alert("It's closing time (" + to_string(MAX_H) + ":00)! Ending the day...");
                endDay();
                day++;
                hour = 0.0;
            }
        }
    }
    
    string getFormattedTime() const {
        int hourVal = static_cast<int>(this->hour);
        int minutes = static_cast<int>((this->hour - hourVal) * 60);
        
        stringstream ss;
        ss<<hourVal<<":"<<setw(2)<<setfill('0')<<minutes;
        return ss.str();
    }

     void runMain() {
        bool run = true;
        MenMgr menuMgr(*this);
        GuestMgr gstMgr(*this);
        KitchOps kitMgr(*this);

        while (run) {
            clrScrn();
        cout<<"[== Bistro Ahri Restaurant Management System ==]"<<endl;
        cout<<"Day: "<<day<<" | Time: "<<getFormattedTime()<<" / "<<MAX_H<<":00"<<endl;
            stringstream ss_money, ss_rating;
            ss_money<<fixed<<setprecision(2)<<restCash;
            ss_rating<<fixed<<setprecision(1)<<restRate;
            
        cout<<"Bank Balance: $"<<ss_money.str()<<" | Rating: "<<ss_rating.str()<<" stars"<<endl;

                    if (isEnded) {
        cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
        cout<<"!! GAME OVER: RESTAURANT BANKRUPT !!"<<endl;
        cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
                    }

        cout<<"Tables: "<<occTbls<<" occupied / "<<capac<<" total"<<endl;
        cout<<"Waitlist Size: "<<waitLst.size()<<endl;
        cout<<"Orders Served (for undo): "<<ordHist.size()<<endl;
        cout<<"=========[ Main Options ]==========="<<endl;
        cout<<"1) Manage Menu"<<endl;
        cout<<"2) Guest Management"<<endl;
        cout<<"3) Kitchen Operations"<<endl;
        cout<<"4) End Current Day"<<endl;
        cout<<"------------------------------------"<<endl;
                    if (isEnded) {
        cout<<"9) Start New Game After Bankruptcy"<<endl;
                    }
        cout<<"0) Exit System"<<endl;
        cout<<"===================================="<<endl;

                    alert(sysMsg);
        cout<<"> ";

            int choice;
            cin>>choice;

            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                alert("Invalid input. Please enter a number.");
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');


            switch (choice) {
                case 1: menuMgr.run(); break;
                case 2: gstMgr.run(); break;
                case 3: kitMgr.run(); break;
                case 4:
                    if(!isEnded){
                        alert("Manually ending day...");
                        endDay();
                        day++;
                        hour = 0.0;
                    }
                    else{
                        alert("Cannot end day while bankrupt.");
                    }
                    break;
                case 9:
                    if (isEnded) {
                        initRest();
                        alert("Starting fresh with a new restaurant!");
                    } else {
                        alert("You can only restart after going bankrupt.");
                    }
                    break;
                case 0: run = false; cout<<"Exiting system."<<endl; break;
                default: alert("Invalid main menu option."); break;
            }
        }
    }

};

SubMenu::SubMenu(ResSys& rs) : resto(rs) {}

MenMgr::MenMgr(ResSys& rs) : SubMenu(rs) {}

bool MenMgr::handleCh(int ch) {
    if (resto.isGameEnded() && ch != 0) {
         alert("Game Over! Cannot manage menu when bankrupt.");
         return true;
    }

    switch (ch) {
        case 1:
            sort(resto.menu.begin(), resto.menu.end(),
                 [](const auto& a, const auto& b) { return a.second < b.second; });
            alert("Menu sorted by price (Ascending).");
            return true;
        case 2:
            sort(resto.menu.begin(), resto.menu.end(),
                 [](const auto& a, const auto& b) { return a.first < b.first; });
            alert("Menu sorted by name (Alphabetical).");
            return true;
        case 3:
            random_shuffle(resto.menu.begin(), resto.menu.end());
            alert("Menu shuffled randomly.");
            return true;
        case 4:
            {
                if(resto.menu.empty()) {
                    alert("Menu is empty, cannot find min/max.");
                    return true;
                }
                auto minIt = min_element(resto.menu.begin(), resto.menu.end(),
                                       [](const auto& a, const auto& b){ return a.second < b.second; });
                auto maxIt = max_element(resto.menu.begin(), resto.menu.end(),
                                       [](const auto& a, const auto& b){ return a.second < b.second; });

                stringstream ss_min, ss_max;
                ss_min<<fixed<<setprecision(2)<<minIt->second;
                ss_max<<fixed<<setprecision(2)<<maxIt->second;

                alert("Cheapest: " + minIt->first + "($" + ss_min.str() + ")" +
                    " / Priciest: " + maxIt->first + "($" + ss_max.str() + ")");
            }
            return true;
        default:
            return false;
    }
}

void MenMgr::dispCont() {
    
        cout<<"Current Menu:"<<endl;
        cout<<"-------------"<<endl;
            if (resto.menu.empty()) {
        cout<<"(Menu is empty)"<<endl;
            } else {
                int count = 1;
        cout<<fixed<<setprecision(2);
                for (const auto& item : resto.menu) {
        cout<<setw(2)<<count++<<") "<<left<<setw(25)<<item.first
                        <<" $"<<item.second<<endl;
                }
            }
        cout<<"-------------"<<endl;
        cout<<"1) Sort by Price (Asc)"<<endl;
        cout<<"2) Sort by Name (Asc)"<<endl;
        cout<<"3) Shuffle Menu"<<endl;
        cout<<"4) Show Cheapest/Priciest Item"<<endl;

}

GuestMgr::GuestMgr(ResSys& rs) : SubMenu(rs) {}

void GuestMgr::dispCont() {
    
    cout<<"Seating Chart (Tables: "<<resto.occTbls<<"/"<<resto.capac<<"):"<<endl;
    cout<<"--------------------------"<<endl;

    for (const auto& pair : resto.seatMap) {
        
        cout<<"Table "<<setw(2)<<pair.first<<": ";
       
        if (pair.second.empty()) {
        cout<<"(Empty)"<<endl;
        } 

        else {
            string guests;
            for (const string& name : pair.second) {
                guests += name + ", ";
            }
             if (!guests.empty()) guests = guests.substr(0, guests.length() - 2);
cout<<guests<<endl;
        }
    }
cout<<"--------------------------"<<endl;
cout<<"Waitlist ("<<resto.waitLst.size()<<" parties): ";

     if (resto.waitLst.empty()) {
        cout<<"(Empty)"<<endl;
     } 
     
     else {
         queue<string> tempQ = resto.waitLst;
         string qStr;
         int count = 0;
         
         
         while (!tempQ.empty() && count < 5) {
             qStr += tempQ.front() + ", ";
             tempQ.pop();
             count++;
         }
         
         if (!qStr.empty()){
             qStr = qStr.substr(0, qStr.length() - 2);
         }
         
         if (resto.waitLst.size() > 5) {
             qStr += " ...";
         }
        cout<<qStr<<endl;
            }
       cout<<"--------------------------"<<endl;
       cout<<"1) Seat Next Party (if space)"<<endl;
       cout<<"2) Add Party to Waitlist (Manual)"<<endl;
       cout<<"3) Wait for tables to free"<<endl;
       cout<<"4) Seat from Waitlist (if space)"<<endl;
       }

bool GuestMgr::handleCh(int ch) {
    if (resto.isGameEnded() && ch != 0) {
         alert("Game Over! Cannot manage guests when bankrupt.");
         return true;
    }

    switch (ch) {
        case 1:
            if (resto.occTbls < resto.capac) {
                alert("Attempting to seat 1 new party...");
                resto.popTbl(1);
                resto.advTime(false);
            } else {
                alert("Restaurant is full. Add party to waitlist?");
            }
            return true;
         case 2:
             resto.addWait();
             return true;
        case 3:
            alert("Waiting for parties to leave...");
            resto.depopTbl(true);
            resto.advTime(true);
            return true;
         case 4:
             if (resto.waitLst.empty()) {
                  alert("Waitlist is empty.");
             } 
             
             else if (resto.occTbls >= resto.capac) {
                  alert("Restaurant is full, cannot seat from waitlist now.");
             } 
             
             else {
                  string prtyNm = resto.waitLst.front();
                  resto.waitLst.pop();
                  alert("Attempting to seat " + prtyNm + " from waitlist...");
                  resto.popTbl(1);
             }
             return true;
             
        default:
            return false;
    }
}

KitchOps::KitchOps(ResSys& rs) : SubMenu(rs) {}

void KitchOps::dispCont() {
cout<<"Pantry Stock:"<<endl;
cout<<"-------------"<<endl;
      if (resto.pantry.empty()) {
cout<<"(Pantry is empty)"<<endl;
      } else {
           for (const auto& item : resto.pantry) {
cout<<"- "<<left<<setw(20)<<item.first<<": "<<item.second<<endl;
           }
      }
cout<<"-------------"<<endl;
cout<<"Order History (for undo): "<<resto.ordHist.size()<<" items"<<endl;
      if (!resto.ordHist.empty()) {
cout<<"Last Order: "<<resto.ordHist.top()<<endl;
      }
cout<<"-------------"<<endl;
cout<<"1) Serve Random Order"<<endl;
cout<<"2) Undo Last Served Order"<<endl;
cout<<"3) Check Ingredient Stock"<<endl;
cout<<"4) Add Ingredient to Pantry"<<endl;
 }

bool KitchOps::handleCh(int ch) {
    if (resto.isGameEnded() && ch != 0 && ch != 2) {
         alert("Game Over! Cannot operate kitchen when bankrupt.");
         return true;
    }

     switch (ch) {
           case 1: {
                if(resto.occTbls <= 0){
                    alert("No customers to serve to!");
                    return true;
                }
                if (resto.menu.empty()) {
                    alert("No menu items to serve!");
                    return true;
                }
                if (resto.isGameEnded()) {
                    alert("Game Over! Cannot serve orders when bankrupt.");
                    return true;
                }

                int randIdx = rand() % resto.menu.size();
                string ordDesc = resto.menu[randIdx].first;
                bool canMake = true;

                if (ordDesc == "Margherita Pizza" && (resto.pantry["Tomato"] <= 0 || resto.pantry["Cheese"] <= 0 || resto.pantry["Flour"] <= 0)) {
                    canMake = false;
                }
                if (ordDesc == "Grilled Salmon" && resto.pantry["Salmon"] <= 0){
                    canMake = false;
                }
                if (ordDesc == "Spaghetti Bolognese" && resto.pantry["Tomato"] <= 0){
                    canMake = false;
                }
                if (ordDesc == "Caesar Salad" && resto.pantry["Cheese"] <= 0){
                    canMake = false;
                }

                if (canMake) {
                    if (ordDesc == "Margherita Pizza") { 
                        resto.pantry["Tomato"]--; resto.pantry["Cheese"]--; resto.pantry["Flour"]--;
                    }
                    if (ordDesc == "Grilled Salmon") {
                        resto.pantry["Salmon"]--; 
                    }
                    if (ordDesc == "Spaghetti Bolognese") { 
                        resto.pantry["Tomato"]--;
                    }
                    if (ordDesc == "Caesar Salad") { 
                        resto.pantry["Cheese"]--;
                    }

                    resto.ordHist.push("Served: " + ordDesc);
                    
                    resto.advTime(true);
                    resto.earnFromOrder(ordDesc);
                    resto.depopTbl(false);

                } else {
                    alert("Cannot serve " + ordDesc + ", insufficient ingredients!");
                }
                return true;
            }
           case 2: {
                if (resto.ordHist.empty()) {
                     alert("No orders in history to undo.");
                } else {
                     string lastOrd = resto.ordHist.top();
                     resto.ordHist.pop();
                     alert("Removed last action from history: " + lastOrd + " (Money/Rating NOT reverted)");
                }
                return true;
           }
           case 3: {
                string ingred;
                cout<<"Enter ingredient name to check: ";
                getline(cin, ingred);

                auto it = resto.pantry.find(ingred);
                if (it != resto.pantry.end()) {
                     alert("Stock for " + ingred + ": " + to_string(it->second));
                } else {
                     alert(ingred + " not found in pantry stock.");
                }
                return true;
           }
           case 4: {
                string ingred;
                int qty;
                cout<<"Enter ingredient name to add/update: ";
                getline(cin, ingred);

                cout<<"Enter quantity to add: ";
                cin>>qty;

                if (cin.fail() || qty < 0) {
                     cin.clear();
                     cin.ignore(numeric_limits<streamsize>::max(), '\n');
                     alert("Invalid quantity.");
                     return true;
                }
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                resto.pantry[ingred] += qty;
                alert("Added " + to_string(qty) + " to " + ingred + ". New total: " + to_string(resto.pantry[ingred]));
                return true;
           }
           default:
                return false;
      }
}

int main(int argc, char** argv) {

    ResSys bistro;
    bistro.runMain();

    return 0;
}