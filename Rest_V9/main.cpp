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


#include "namedict.h"

using namespace std;

pair<string, bool> sysMsg = {"", false};

void clrScrn() {

    for (int k = 0; k < 10; k++)
    {
        cout<<endl;
    }

    /* DEPRECATED: does not play nice with certain IDEs, works fine with netbeans though
     *
     *
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
*/
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

class RecDep;
class OrdPrior;
class GuestHashTable;
class DelivOps;

struct GstOrd {
    string gstNm;
    string dish;
    int tblId;
    bool isVip;
    float ordTime;
    int prior;
};

class RecDep {
public:
    struct RecNode {
        string dish;
        map<string, int> dirIngr;
        vector<pair<RecNode*, int>> subRec;
        float prepT;


        float calcTotTime() {
            float maxSub = 0;
            for(auto& dep : subRec) {
                maxSub = max(maxSub, dep.first->calcTotTime());
            }
            return prepT + maxSub;
        }


        bool chkMake(map<string, int>& pntry, map<string, int>& used) {

            for(auto& ing : dirIngr) {
                if(pntry[ing.first] - used[ing.first] < ing.second) {
                    return false;
                }
            }


            for(auto& ing : dirIngr) {
                used[ing.first] += ing.second;
            }


            for(auto& dep : subRec) {
                for(int i = 0; i < dep.second; ++i) {
                    if(!dep.first->chkMake(pntry, used)) {

                        for(auto& ing : dirIngr) {
                            used[ing.first] -= ing.second;
                        }
                        return false;
                    }
                }
            }

            return true;
        }


        void consIngr(map<string, int>& pntry, vector<string>& log) {

            for(auto& dep : subRec) {
                for(int i = 0; i < dep.second; ++i) {
                    dep.first->consIngr(pntry, log);
                }
            }


            for(auto& ing : dirIngr) {
                pntry[ing.first] -= ing.second;
                log.push_back("Used " + to_string(ing.second) + " " + ing.first + " for " + dish);
            }
        }


        void dispTree(int indent = 0) {
            string indStr(indent * 2, ' ');
            cout<<indStr<<"- "<<dish<<" (Prep: "<<prepT<<" min)"<<endl;

            if(!dirIngr.empty()) {
                cout<<indStr<<"  Direct: ";
                bool first = true;
                for(auto& ing : dirIngr) {
                    if(!first) cout<<", ";
                    cout<<ing.first<<"("<<ing.second<<")";
                    first = false;
                }
                cout<<endl;
            }

            for(auto& dep : subRec) {
                cout<<indStr<<"  Requires "<<dep.second<<"x:"<<endl;
                dep.first->dispTree(indent + 2);
            }
        }
    };

    map<string, RecNode*> recMap;

    ~RecDep() {
        for(auto& pair : recMap) {
            delete pair.second;
        }
    }

    void addRec(string name, map<string, int> ingr, float prep) {
        if(recMap.find(name) != recMap.end()) {
            delete recMap[name];
        }
        RecNode* node = new RecNode();
        node->dish = name;
        node->dirIngr = ingr;
        node->prepT = prep;
        recMap[name] = node;
    }

    bool addDep(string parent, string child, int qty) {
        if(recMap.find(parent) == recMap.end() || recMap.find(child) == recMap.end()) {
            return false;
        }


        if(hasCycle(parent, child)) {
            return false;
        }

        recMap[parent]->subRec.push_back({recMap[child], qty});
        return true;
    }

    bool hasCycle(string parent, string potChild) {

        if(parent == potChild) return true;

        RecNode* childNode = recMap[potChild];
        for(auto& dep : childNode->subRec) {
            if(hasCycle(parent, dep.first->dish)) {
                return true;
            }
        }
        return false;
    }

    RecNode* getRec(string name) {
        auto it = recMap.find(name);
        return (it != recMap.end()) ? it->second : nullptr;
    }
};



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

class DelivOps : public SubMenu {
public:
    DelivOps(ResSys& rs);

    string getTitle() override { return "Delivery Operations"; }
    void dispCont() override;
    bool handleCh(int ch) override;
};

class OrdPrior : public SubMenu {
public:
    OrdPrior(ResSys& rs);

    string getTitle() override { return "Order Priority Management"; }
    void dispCont() override;
    bool handleCh(int ch) override;

private:
    void qckSortOrds(vector<GstOrd>& ords, int low, int high);
    int partOrds(vector<GstOrd>& ords, int low, int high);
    void procTopOrd();
    void addRandOrds(int cnt);
    void takeOrdFromTbl();
    void viewAllOrds();
};


class GuestHashTable {
private:
    struct GuestRec {
        string name;
        int visCnt;
        float totSpnt;
        bool isVIP;
        GuestRec(const string& n) : name(n), visCnt(0), totSpnt(0.0), isVIP(false) {}
    };

    vector<list<GuestRec>> table;
    int tblSz;
    int numGuests;

    const float VIP_THRESHOLD = 100.0;
    const int VIP_VISITS = 5;

    int hash1(const string& name) {
        int hash = 0;
        for (char c : name) {
            hash = (hash * 31 + c) % tblSz;
        }
        return hash;
    }

    int hash2(const string& name) {
        int hash = 0;
        for (int i = 0; i < name.length(); ++i) {
            hash = (hash + (i + 1) * name[i]) % tblSz;
        }
        return hash;
    }

public:
    GuestHashTable(int size = 97) : tblSz(size), numGuests(0) {
        table.resize(tblSz);
    }

    void addGuest(const string& name) {
        int idx = hash1(name);

        for (auto& guest : table[idx]) {
            if (guest.name == name) {
                guest.visCnt++;
                checkVIPStatus(guest);
                return;
            }
        }

        GuestRec newGuest(name);
        newGuest.visCnt = 1;
        table[idx].push_back(newGuest);
        numGuests++;
    }

    void recOrd(const string& name, float amount) {
        int idx = hash1(name);

        for (auto& guest : table[idx]) {
            if (guest.name == name) {
                guest.totSpnt += amount;
                checkVIPStatus(guest);
                return;
            }
        }


        GuestRec newGuest(name);
        newGuest.totSpnt = amount;
        table[idx].push_back(newGuest);
        numGuests++;
    }

    GuestRec* findGuest(const string& name) {
        int idx = hash1(name);

        for (auto& guest : table[idx]) {
            if (guest.name == name) {
                return &guest;
            }
        }
        return nullptr;
    }

    void checkVIPStatus(GuestRec& guest) {
        if (!guest.isVIP && (guest.totSpnt >= VIP_THRESHOLD || guest.visCnt >= VIP_VISITS)) {
            guest.isVIP = true;
            alert("Guest " + guest.name + " is now a VIP!");
        }
    }

    vector<pair<string, float>> getTopSpenders(int num = 5) {
        vector<pair<string, float>> allGuests;

        for (const auto& bucket : table) {
            for (const auto& guest : bucket) {
                allGuests.push_back({guest.name, guest.totSpnt});
            }
        }

        sort(allGuests.begin(), allGuests.end(),
             [](const auto& a, const auto& b) { return a.second > b.second; });

        if (allGuests.size() > num) {
            allGuests.resize(num);
        }

        return allGuests;
    }

    vector<string> getVIPs() {
        vector<string> vips;

        for (const auto& bucket : table) {
            for (const auto& guest : bucket) {
                if (guest.isVIP) {
                    vips.push_back(guest.name);
                }
            }
        }

        return vips;
    }

    void dispStats() {
        cout<<"Guest Database Statistics:"<<endl;
        cout<<"Total Unique Guests: "<<numGuests<<endl;
        cout<<"VIP Guests: "<<getVIPs().size()<<endl;

        float totRev = 0.0;
        int totVis = 0;

        for (const auto& bucket : table) {
            for (const auto& guest : bucket) {
                totRev += guest.totSpnt;
                totVis += guest.visCnt;
            }
        }

        if (numGuests > 0) {
            cout<<fixed<<setprecision(2);
            cout<<"Average Spending per Guest: $"<<(totRev / numGuests)<<endl;
            cout<<"Average Visits per Guest: "<<setprecision(1)<<(static_cast<float>(totVis) / numGuests)<<endl;
        }
    }
};

class ResSys {
private:
    vector<pair<string, float>> menu;
    map<int, list<string>> seatMap;
    unordered_set<int> inspIdx;
    queue<string> waitLst;
    stack<string> ordHist;
    map<string, int> pantry;
    vector<GstOrd> pendOrds;
    unordered_set<string> vipGsts;
    GuestHashTable guestDB;

    RecDep recDeps;

    map<int, vector<pair<int, float>>> cityGrph;
    map<int, string> areaNames;
    queue<pair<string, int>> delivQue;
    map<int, int> activeDeliv;

    int capac;
    int occTbls;
    int nextIdx;

    float restCash;
    float restRate;
    bool isEnded;

    const float INITIAL_BALANCE = 1000.0;
    const float CUSTOMER_CHARGE = 12.50;
    const float INSPECTOR_BONUS = 50.0;
    const float DELIV_FEE = 5.0;
    const float DELIV_TIME_MULT = 0.1;

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
        pantry = { {"Flour", 10}, {"Tomato", 20}, {"Cheese", 15}, {"Salmon", 5}, {"Water", 20}, {"Meat", 8}, {"Oil", 15}, {"Lettuce", 10}, {"Chocolate", 12}};


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

    void initRecipes() {

        recDeps.addRec("Pizza Dough", {{"Flour", 2}, {"Water", 1}}, 1.5);
        recDeps.addRec("Tomato Sauce", {{"Tomato", 3}}, 0.5);
        recDeps.addRec("Cheese Mix", {{"Cheese", 2}}, 0.2);


        recDeps.addRec("Margherita Pizza", {{"Cheese", 1}}, 2.0);
        recDeps.addDep("Margherita Pizza", "Pizza Dough", 1);
        recDeps.addDep("Margherita Pizza", "Tomato Sauce", 1);


        recDeps.addRec("Pasta", {{"Flour", 1}, {"Water", 1}}, 1.0);
        recDeps.addRec("Meat Sauce", {{"Tomato", 2}, {"Meat", 1}}, 1.5);
        recDeps.addRec("Spaghetti Bolognese", {}, 0.5);
        recDeps.addDep("Spaghetti Bolognese", "Pasta", 1);
        recDeps.addDep("Spaghetti Bolognese", "Meat Sauce", 1);

        recDeps.addRec("Grilled Salmon", {{"Salmon", 1}, {"Oil", 1}}, 3.0);
        recDeps.addRec("Caesar Salad", {{"Lettuce", 2}, {"Cheese", 1}}, 1.0);
        recDeps.addRec("Chocolate Cake", {{"Flour", 2}, {"Chocolate", 3}}, 4.0);
    }

    bool canMakeWithDeps(const string& dish) {
        RecDep::RecNode* rec = recDeps.getRec(dish);
        if(!rec) {

            return true;
        }

        map<string, int> used;
        return rec->chkMake(pantry, used);
    }

    void serveWithDeps(const string& dish) {
        RecDep::RecNode* rec = recDeps.getRec(dish);
        if(!rec) {

            return;
        }

        vector<string> log;
        rec->consIngr(pantry, log);


        string logMsg = "Recipe prepared: ";
        for(const auto& entry : log) {
            logMsg += "\n  " + entry;
        }
        alert(logMsg);
    }

     void initDelivMap() {

        cityGrph.clear();
        areaNames.clear();


        areaNames[0] = "Restaurant";
        areaNames[1] = "Downtown";
        areaNames[2] = "Westside";
        areaNames[3] = "Eastside";
        areaNames[4] = "Northgate";
        areaNames[5] = "Southpark";
        areaNames[6] = "Riverside";
        areaNames[7] = "Hillcrest";
        areaNames[8] = "Lakefront";
        areaNames[9] = "Midtown";
        areaNames[10] = "Airport";



        addEdge(0, 1, 2.5);
        addEdge(0, 9, 3.0);


        addEdge(1, 2, 4.0);
        addEdge(1, 3, 3.5);
        addEdge(1, 9, 2.0);


        addEdge(2, 4, 5.0);
        addEdge(2, 7, 3.0);
        addEdge(3, 5, 4.5);
        addEdge(3, 8, 3.5);
        addEdge(4, 7, 2.5);
        addEdge(5, 8, 3.0);
        addEdge(5, 10, 6.0);
        addEdge(6, 7, 2.0);
        addEdge(6, 8, 2.5);
        addEdge(6, 9, 3.5);
        addEdge(9, 10, 7.0);
    }

    void addEdge(int u, int v, float dist) {
        cityGrph[u].push_back({v, dist});
        cityGrph[v].push_back({u, dist});
    }

    vector<int> findDelivRoute(int dest) {

        const float INF = numeric_limits<float>::infinity();
        map<int, float> dist;
        map<int, int> parent;
        priority_queue<pair<float, int>, vector<pair<float, int>>, greater<>> pq;


        for (const auto& node : areaNames) {
            dist[node.first] = INF;
            parent[node.first] = -1;
        }

        dist[0] = 0;
        pq.push({0, 0});

        while (!pq.empty()) {
            float d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            if (d > dist[u]) continue;

            for (const auto& edge : cityGrph[u]) {
                int v = edge.first;
                float w = edge.second;

                if (dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }


        vector<int> path;
        if (parent[dest] == -1 && dest != 0) {
            return path;
        }

        int curr = dest;
        while (curr != -1) {
            path.push_back(curr);
            curr = parent[curr];
        }

        reverse(path.begin(), path.end());
        return path;
    }

    float getDelivDist(int dest) {
        vector<int> route = findDelivRoute(dest);
        if (route.empty()) return -1.0;

        float totalDist = 0.0;
        for (int i = 1; i < route.size(); ++i) {
            int u = route[i-1];
            int v = route[i];

            for (const auto& edge : cityGrph[u]) {
                if (edge.first == v) {
                    totalDist += edge.second;
                    break;
                }
            }
        }

        return totalDist;
    }

    void procDeliv() {
        if (delivQue.empty()) return;

        int numProc = min(3, (int)delivQue.size());
        string delivSum = "Deliveries completed: ";

        for (int i = 0; i < numProc; ++i) {
            auto deliv = delivQue.front();
            delivQue.pop();

            float dist = getDelivDist(deliv.second);
            float earnings = DELIV_FEE + (dist * 0.5);

            restCash += earnings;
            activeDeliv[deliv.second]--;

            stringstream ss;
            ss << fixed << setprecision(2) << earnings;
            delivSum += areaNames[deliv.second] + "($" + ss.str() + ") ";
        }

        alert(delivSum);
    }


public:
    friend class MenMgr;
    friend class GuestMgr;
    friend class KitchOps;
    friend class SubMenu;
    friend class OrdPrior;
    friend class DelivOps;

    ResSys() : capac(0), occTbls(0), nextIdx(0), day(1), hour(0.0), guestDB(97) {
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
        initDelivMap();
        initRecipes();
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


                            guestDB.addGuest(gstNm);


                            auto guestRec = guestDB.findGuest(gstNm);
                            if (guestRec && guestRec->isVIP) {
                                alert("VIP Guest " + gstNm + " has arrived!");
                            }

                            if (inspIdx.count(nextIdx)) {
                                alert("ALERT: Inspector " + gstNm + " seated at table " + to_string(tblId));
                                improveRating();
                            }
                            nextIdx++;
                        } else {
                            gstNm = "Guest" + to_string(rand() % 1000);
                            guestDB.addGuest(gstNm);
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
    void earnFromOrder(const string& dish, int tblId = -1) {
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
        cout<<"4) Order Priority Management"<<endl;
        cout<<"5) Delivery Operations"<<endl;
        cout<<"6) End Current Day"<<endl;
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
                case 4: {
                static OrdPrior ordMgr(*this);
                ordMgr.run();
                break;
                }
                case 5:
                    if (!isEnded) {
                        DelivOps delivMgr(*this);
                        delivMgr.run();
                    } else {
                        alert("Cannot manage deliveries when bankrupt.");
                    }
                break;
                case 6:
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
       cout<<"5) View Guest Database Stats"<<endl;
       cout<<"6) View Top Spenders"<<endl;
       cout<<"7) View VIP List"<<endl;

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
        case 5:
            resto.guestDB.dispStats();
            cout<<endl<<"Press Enter to continue...";
            cin.get();
            return true;

        case 6: {
                auto topSpend = resto.guestDB.getTopSpenders();
                cout<<"Top Spenders:"<<endl;
                cout<<"-------------"<<endl;
                int rank = 1;
                for (const auto& guest : topSpend) {
                    cout<<rank++<<". "<<left<<setw(20)<<guest.first
                        <<" $"<<fixed<<setprecision(2)<<guest.second<<endl;
                }
                cout<<endl<<"Press Enter to continue...";
                cin.get();
                return true;
        }

        case 7: {
                auto vips = resto.guestDB.getVIPs();
                cout<<"VIP Guests ("<<vips.size()<<"):"<<endl;
                cout<<"-------------"<<endl;
                if (vips.empty()) {
                    cout<<"No VIP guests yet."<<endl;
                } else {
                    for (const auto& vip : vips) {
                        cout<<"★ "<<vip<<endl;
                    }
                }
                cout<<endl<<"Press Enter to continue...";
                cin.get();
                return true;
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
cout<<"5) View Recipe Dependencies"<<endl;
cout<<"6) Check Recipe Availability"<<endl;
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
                    if (canMake) {
                        RecDep::RecNode* rec = resto.recDeps.getRec(ordDesc);
                        if(rec) {

                            resto.serveWithDeps(ordDesc);
                        } else {

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
                        }

                        resto.ordHist.push("Served: " + ordDesc);
                        vector<int> occTbls;
                        for (const auto& pair : resto.seatMap) {
                            if (!pair.second.empty()) {
                                occTbls.push_back(pair.first);
                            }
                        }
                        int serveTbl = occTbls.empty() ? -1 : occTbls[rand() % occTbls.size()];
                        resto.advTime(true);
                        resto.earnFromOrder(ordDesc, serveTbl);
                        resto.depopTbl(false);
                    }

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
                 case 5: {
            string dish;
            cout<<"Enter dish name to view recipe tree: ";
            getline(cin, dish);

            RecDep::RecNode* rec = resto.recDeps.getRec(dish);
            if(rec) {
                cout<<endl<<"Recipe Tree for "<<dish<<":"<<endl;
                cout<<"Total prep time: "<<fixed<<setprecision(1)<<rec->calcTotTime()<<" minutes"<<endl;
                cout<<"--------------------"<<endl;
                rec->dispTree();
                cout<<"--------------------"<<endl;
                cout<<"Press Enter to continue...";
                cin.get();
            } else {
                alert("Recipe not found in dependency system.");
            }
            return true;
        }

        case 6: {
            cout<<endl<<"Recipe Availability Check:"<<endl;
            cout<<"-------------------------"<<endl;

            for(const auto& menuItem : resto.menu) {
                string status;
                RecDep::RecNode* rec = resto.recDeps.getRec(menuItem.first);

                if(rec) {
                    map<string, int> tempUsed;
                    if(rec->chkMake(resto.pantry, tempUsed)) {
                        status = "Available";
                    } else {
                        status = "Missing ingredients";
                    }
                } else {

                    bool canMake = true;
                    if (menuItem.first == "Margherita Pizza" &&
                        (resto.pantry["Tomato"] <= 0 || resto.pantry["Cheese"] <= 0 || resto.pantry["Flour"] <= 0)) {
                        canMake = false;
                    }
                    if (menuItem.first == "Grilled Salmon" && resto.pantry["Salmon"] <= 0){
                        canMake = false;
                    }
                    if (menuItem.first == "Spaghetti Bolognese" && resto.pantry["Tomato"] <= 0){
                        canMake = false;
                    }
                    if (menuItem.first == "Caesar Salad" && resto.pantry["Cheese"] <= 0){
                        canMake = false;
                    }
                    status = canMake ? "Available (legacy)" : "Missing ingredients";
                }

                cout<<setw(25)<<left<<menuItem.first<<" : "<<status<<endl;
            }
            cout<<"-------------------------"<<endl;
            cout<<"Press Enter to continue...";
            cin.get();
            return true;
        }
           default:
                return false;
      }
}

OrdPrior::OrdPrior(ResSys& rs) : SubMenu(rs) {}

void OrdPrior::dispCont() {
    cout<<"Pending Orders ("<<resto.pendOrds.size()<<" total):"<<endl;
    cout<<"-------------------------------"<<endl;

    if (resto.pendOrds.empty()) {
        cout<<"(No pending orders)"<<endl;
    } else {
        int dispCnt = min(10, (int)resto.pendOrds.size());
        for (int i = 0; i < dispCnt; ++i) {
            auto& ord = resto.pendOrds[i];
            string priorStr = ord.prior == 2 ? "[INSP]" : (ord.prior == 1 ? "[VIP]" : "[REG]");
            cout<<setw(2)<<(i+1)<<") "<<priorStr<<" Table "<<ord.tblId
                <<": "<<left<<setw(15)<<ord.gstNm
                <<" - "<<ord.dish<<endl;
        }
        if (resto.pendOrds.size() > 10) {
            cout<<"   ... and "<<(resto.pendOrds.size()-10)<<" more orders"<<endl;
        }
    }

    cout<<"-------------------------------"<<endl;
    cout<<"VIP Guests: "<<resto.vipGsts.size()<<endl;
    cout<<"-------------------------------"<<endl;
    cout<<"1) Take Order from Table"<<endl;
    cout<<"2) Sort Orders by Priority"<<endl;
    cout<<"3) Process Top Priority Order"<<endl;
    cout<<"4) Add Random Orders (Simulation)"<<endl;
    cout<<"5) Mark Guest as VIP"<<endl;
    cout<<"6) Show All VIP Guests"<<endl;
    cout<<"7) View All Pending Orders"<<endl;
}
bool OrdPrior::handleCh(int ch) {
    if (resto.isGameEnded() && ch != 0) {
        alert("Game Over! Cannot manage orders when bankrupt.");
        return true;
    }

    switch (ch) {
        case 1: {
            takeOrdFromTbl();
            return true;
        }
        case 2: {
            if (resto.pendOrds.empty()) {
                alert("No orders to sort.");
            } else {
                qckSortOrds(resto.pendOrds, 0, resto.pendOrds.size() - 1);
                alert("Orders sorted by priority (Inspector > VIP > Regular).");
            }
            return true;
        }
        case 3: {
            procTopOrd();
            return true;
        }
        case 4: {
            if (resto.occTbls == 0) {
                alert("No occupied tables to generate orders from.");
            } else {
                int cnt = min(5, resto.occTbls);
                addRandOrds(cnt);
                alert("Added " + to_string(cnt) + " random orders.");
            }
            return true;
        }
        case 5: {
            string gstNm;
            cout<<"Enter guest name to mark as VIP: ";
            getline(cin, gstNm);

            if (gstNm.empty()) {
                alert("Invalid guest name.");
            } else {
                resto.vipGsts.insert(gstNm);

                for (auto& ord : resto.pendOrds) {
                    if (ord.gstNm == gstNm && ord.prior < 2) {
                        ord.isVip = true;
                        ord.prior = 1;
                    }
                }
                alert(gstNm + " marked as VIP guest.");
            }
            return true;
        }
        case 6: {
            if (resto.vipGsts.empty()) {
                alert("No VIP guests registered.");
            } else {
                string vipList = "VIP Guests: ";
                int cnt = 0;
                for (const auto& vip : resto.vipGsts) {
                    if (cnt++ > 0) vipList += ", ";
                    vipList += vip;
                    if (cnt >= 5) {
                        vipList += " ...(" + to_string(resto.vipGsts.size() - 5) + " more)";
                        break;
                    }
                }
                alert(vipList);
            }
            return true;
        }
        case 7: {
            viewAllOrds();
            return true;
        }
        default:
            return false;
    }
}

void OrdPrior::qckSortOrds(vector<GstOrd>& ords, int low, int high) {
    if (low < high) {
        int pi = partOrds(ords, low, high);
        qckSortOrds(ords, low, pi - 1);
        qckSortOrds(ords, pi + 1, high);
    }
}

int OrdPrior::partOrds(vector<GstOrd>& ords, int low, int high) {
    GstOrd pivot = ords[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {

        bool shouldSwap = false;
        if (ords[j].prior > pivot.prior) {
            shouldSwap = true;
        } else if (ords[j].prior == pivot.prior && ords[j].ordTime < pivot.ordTime) {
            shouldSwap = true;
        }

        if (shouldSwap) {
            i++;
            swap(ords[i], ords[j]);
        }
    }

    swap(ords[i + 1], ords[high]);
    return i + 1;
}

void OrdPrior::procTopOrd() {
    if (resto.pendOrds.empty()) {
        alert("No pending orders to process.");
        return;
    }


    qckSortOrds(resto.pendOrds, 0, resto.pendOrds.size() - 1);

    GstOrd topOrd = resto.pendOrds[0];
    resto.pendOrds.erase(resto.pendOrds.begin());


    bool canMake = true;
    if (topOrd.dish == "Margherita Pizza" &&
        (resto.pantry["Tomato"] <= 0 || resto.pantry["Cheese"] <= 0 || resto.pantry["Flour"] <= 0)) {
        canMake = false;
    }
    if (topOrd.dish == "Grilled Salmon" && resto.pantry["Salmon"] <= 0) {
        canMake = false;
    }
    if (topOrd.dish == "Spaghetti Bolognese" && resto.pantry["Tomato"] <= 0) {
        canMake = false;
    }
    if (topOrd.dish == "Caesar Salad" && resto.pantry["Cheese"] <= 0) {
        canMake = false;
    }

    if (canMake) {

        if (topOrd.dish == "Margherita Pizza") {
            resto.pantry["Tomato"]--; resto.pantry["Cheese"]--; resto.pantry["Flour"]--;
        }
        if (topOrd.dish == "Grilled Salmon") {
            resto.pantry["Salmon"]--;
        }
        if (topOrd.dish == "Spaghetti Bolognese") {
            resto.pantry["Tomato"]--;
        }
        if (topOrd.dish == "Caesar Salad") {
            resto.pantry["Cheese"]--;
        }

        string priorStr = topOrd.prior == 2 ? "Inspector" : (topOrd.prior == 1 ? "VIP" : "Regular");
        resto.ordHist.push("Priority Order: " + priorStr + " - " + topOrd.dish + " for " + topOrd.gstNm);

        resto.advTime(true);
        resto.earnFromOrder(topOrd.dish);

        if (topOrd.prior == 2) {
            resto.improveRating();
            alert("Served " + topOrd.dish + " to Inspector " + topOrd.gstNm + " (Rating improved!)");
        } else {
            alert("Served " + topOrd.dish + " to " + priorStr + " guest " + topOrd.gstNm);
        }
    } else {
        alert("Cannot serve " + topOrd.dish + ", insufficient ingredients! Order cancelled.");
    }
}

void OrdPrior::addRandOrds(int cnt) {
    vector<int> occTbls;
    for (const auto& pair : resto.seatMap) {
        if (!pair.second.empty()) {
            occTbls.push_back(pair.first);
        }
    }

    if (occTbls.empty()) return;

    for (int i = 0; i < cnt && i < occTbls.size(); ++i) {
        int tblIdx = rand() % occTbls.size();
        int tblId = occTbls[tblIdx];

        auto& gsts = resto.seatMap[tblId];
        if (!gsts.empty()) {

            auto it = gsts.begin();
            advance(it, rand() % gsts.size());
            string gstNm = *it;


            if (!resto.menu.empty()) {
                int dishIdx = rand() % resto.menu.size();
                string dish = resto.menu[dishIdx].first;

                GstOrd newOrd;
                newOrd.gstNm = gstNm;
                newOrd.dish = dish;
                newOrd.tblId = tblId;
                newOrd.ordTime = resto.hour;


                bool isInsp = false;
                for (int inspId : resto.inspIdx) {
                    if (inspId < totalNames && gstNm == NAMEDICT[inspId]) {
                        isInsp = true;
                        break;
                    }
                }

                if (isInsp) {
                    newOrd.prior = 2;
                    newOrd.isVip = false;
                } else if (resto.vipGsts.count(gstNm)) {
                    newOrd.prior = 1;
                    newOrd.isVip = true;
                } else {
                    newOrd.prior = 0;
                    newOrd.isVip = false;
                }

                resto.pendOrds.push_back(newOrd);
            }
        }
    }
}

void OrdPrior::takeOrdFromTbl() {

    vector<int> occTbls;
    for (const auto& pair : resto.seatMap) {
        if (!pair.second.empty()) {
            occTbls.push_back(pair.first);
        }
    }

    if (occTbls.empty()) {
        alert("No occupied tables to take orders from.");
        return;
    }

    cout<<endl<<"Occupied Tables:"<<endl;
    for (int tblId : occTbls) {
        cout<<"Table "<<tblId<<": ";
        int gstCnt = 0;
        for (const auto& gst : resto.seatMap[tblId]) {
            if (gstCnt++ > 0) cout<<", ";
            cout<<gst;
        }
        cout<<endl;
    }


    int tblId;
    cout<<endl<<"Enter table number: ";
    cin>>tblId;

    if (cin.fail() || resto.seatMap[tblId].empty()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        alert("Invalid table selection.");
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');


    cout<<endl<<"Guests at Table "<<tblId<<":"<<endl;
    vector<string> gstLst(resto.seatMap[tblId].begin(), resto.seatMap[tblId].end());
    for (int i = 0; i < gstLst.size(); ++i) {
        cout<<(i+1)<<") "<<gstLst[i]<<endl;
    }


    int gstChoice;
    cout<<endl<<"Select guest (1-"<<gstLst.size()<<"): ";
    cin>>gstChoice;

    if (cin.fail() || gstChoice < 1 || gstChoice > gstLst.size()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        alert("Invalid guest selection.");
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string selGst = gstLst[gstChoice - 1];


    cout<<endl<<"Menu Items:"<<endl;
    for (int i = 0; i < resto.menu.size(); ++i) {
        cout<<(i+1)<<") "<<resto.menu[i].first<<" - $"<<fixed<<setprecision(2)<<resto.menu[i].second<<endl;
    }


    int dishChoice;
    cout<<endl<<"Select dish (1-"<<resto.menu.size()<<"): ";
    cin>>dishChoice;

    if (cin.fail() || dishChoice < 1 || dishChoice > resto.menu.size()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        alert("Invalid dish selection.");
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string selDish = resto.menu[dishChoice - 1].first;


    GstOrd newOrd;
    newOrd.gstNm = selGst;
    newOrd.dish = selDish;
    newOrd.tblId = tblId;
    newOrd.ordTime = resto.hour;


    bool isInsp = false;
    for (int inspId : resto.inspIdx) {
        if (inspId < totalNames && selGst == NAMEDICT[inspId]) {
            isInsp = true;
            break;
        }
    }

    if (isInsp) {
        newOrd.prior = 2;
        newOrd.isVip = false;
    } else if (resto.vipGsts.count(selGst)) {
        newOrd.prior = 1;
        newOrd.isVip = true;
    } else {
        newOrd.prior = 0;
        newOrd.isVip = false;
    }

    resto.pendOrds.push_back(newOrd);

    string priorStr = newOrd.prior == 2 ? "[INSPECTOR]" : (newOrd.prior == 1 ? "[VIP]" : "");
    alert("Order added: " + selDish + " for " + selGst + " " + priorStr + " at Table " + to_string(tblId));
}

void OrdPrior::viewAllOrds() {
    if (resto.pendOrds.empty()) {
        alert("No pending orders to display.");
        return;
    }

    cout<<endl<<"All Pending Orders ("<<resto.pendOrds.size()<<" total):"<<endl;
    cout<<"==========================================="<<endl;

    for (int i = 0; i < resto.pendOrds.size(); ++i) {
        auto& ord = resto.pendOrds[i];
        string priorStr = ord.prior == 2 ? "[INSP]" : (ord.prior == 1 ? "[VIP]" : "[REG]");
        cout<<setw(3)<<(i+1)<<") "<<priorStr<<" Table "<<setw(2)<<ord.tblId
            <<": "<<left<<setw(15)<<ord.gstNm
            <<" -> "<<ord.dish<<endl;
    }
    cout<<"==========================================="<<endl;
    cout<<"Press Enter to continue...";
    cin.ignore();
}

DelivOps::DelivOps(ResSys& rs) : SubMenu(rs) {}

void DelivOps::dispCont() {
    cout<<"Delivery Map Areas:"<<endl;
    cout<<"------------------"<<endl;


    for (const auto& area : resto.areaNames) {
        if (area.first != 0) {
            cout<<setw(2)<<area.first<<") "<<area.second;


            float dist = resto.getDelivDist(area.first);
            if (dist > 0) {
                cout<<" (Distance: "<<fixed<<setprecision(1)<<dist<<" km)";
            }


            if (resto.activeDeliv[area.first] > 0) {
                cout<<" [Active: "<<resto.activeDeliv[area.first]<<"]";
            }

            cout<<endl;
        }
    }

    cout<<"------------------"<<endl;
    cout<<"Pending Deliveries: "<<resto.delivQue.size()<<endl;
    cout<<"Delivery Fee: $"<<fixed<<setprecision(2)<<resto.DELIV_FEE<<" + distance charges"<<endl;
    cout<<"------------------"<<endl;
    cout<<"1) Place Delivery Order"<<endl;
    cout<<"2) Show Route to Area"<<endl;
    cout<<"3) Process Deliveries"<<endl;
    cout<<"4) Show Map Connections"<<endl;
}

bool DelivOps::handleCh(int ch) {
    if (resto.isGameEnded() && ch != 0) {
        alert("Game Over! Cannot manage deliveries when bankrupt.");
        return true;
    }

    switch (ch) {
        case 1: {
            if (resto.menu.empty()) {
                alert("No menu items available for delivery!");
                return true;
            }

            cout<<"Enter destination area number (1-10): ";
            int dest;
            cin>>dest;

            if (cin.fail() || dest < 1 || dest > 10) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                alert("Invalid area number.");
                return true;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');


            int randIdx = rand() % resto.menu.size();
            string ordItem = resto.menu[randIdx].first;

            resto.delivQue.push({ordItem, dest});
            resto.activeDeliv[dest]++;

            alert("Delivery order placed: " + ordItem + " to " + resto.areaNames[dest]);
            resto.advTime(false);
            return true;
        }

        case 2: {
            cout<<"Enter destination area number (1-10): ";
            int dest;
            cin>>dest;

            if (cin.fail() || dest < 1 || dest > 10) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                alert("Invalid area number.");
                return true;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            vector<int> route = resto.findDelivRoute(dest);
            if (route.empty()) {
                alert("No route found to " + resto.areaNames[dest]);
            } else {
                string routeStr = "Route: ";
                for (int i = 0; i < route.size(); ++i) {
                    routeStr += resto.areaNames[route[i]];
                    if (i < route.size() - 1) routeStr += " -> ";
                }

                float dist = resto.getDelivDist(dest);
                float time = dist * resto.DELIV_TIME_MULT;

                stringstream ss;
                ss<<fixed<<setprecision(1)<<dist<<" km, Time: "<<setprecision(1)<<time<<" hrs";

                alert(routeStr + " (Distance: " + ss.str() + ")");
            }
            return true;
        }

        case 3: {
            if (resto.delivQue.empty()) {
                alert("No pending deliveries to process.");
            } else {
                resto.procDeliv();
                resto.advTime(true);
            }
            return true;
        }

        case 4: {
            cout<<endl<<"City Map Connections:"<<endl;
            cout<<"--------------------"<<endl;

            for (const auto& node : resto.cityGrph) {
                cout<<resto.areaNames[node.first]<<" connects to: ";

                bool first = true;
                for (const auto& edge : node.second) {
                    if (!first) cout<<", ";
                    cout<<resto.areaNames[edge.first]<<" ("<<edge.second<<" km)";
                    first = false;
                }
                cout<<endl;
            }

            cout<<endl<<"Press Enter to continue...";
            cin.ignore();
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