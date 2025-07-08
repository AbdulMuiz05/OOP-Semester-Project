#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>   // For sleep functions
#include <chrono>
using namespace std;

enum Difficulty { EASY, MEDIUM, HARD };

// Forward declarations
class Kingdom;
class Population;
class Army;
class Bank;
class Market;
class BuildingSystem;

class Event {
public:
    string description;
    int foodChange, goldChange, happinessChange, armyChange, populationChange;
    Event(string desc, int food, int gold, int happy, int army, int pop)
        : description(desc), foodChange(food), goldChange(gold),
        happinessChange(happy), armyChange(army), populationChange(pop) {}
};

class Leader {
public:
    string name;
    int leadership;
    int corruption;
    Leader(string n, int l, int c) : name(n), leadership(l), corruption(c) {}
    virtual void makeDecision() = 0;
    virtual ~Leader() {}
};

class King : public Leader {
public:
    int taxRate;
    string Commander;
    King(string n, int l = 50, int c = 10) : Leader(n, l, c), taxRate(15) {}
    void makeDecision() override {
        cout << name << " makes a royal decree.\n";
    }
    void setTaxRate(int rate) {
        if (rate < 5 || rate > 50) {
            cout << "Tax rate must be between 5% and 50%\n";
            return;
        }
        taxRate = rate;
        cout << name << " set tax rate to " << taxRate << "%.\n";
    }
    void appointCommander(string ComName) {
        Commander = ComName;
        cout << name << " appointed " << Commander << " as commander.\n";
    }
};

class Commander : public Leader {
public:
    int loyalty;
    Commander(string n, int l = 60, int c = 5) : Leader(n, l, c), loyalty(70) {}
    void makeDecision() override {
        cout << name << " gives military orders.\n";
    }
};

class MerchantLeader : public Leader {
public:
    MerchantLeader(string n, int l = 40, int c = 30) : Leader(n, l, c) {}
    void makeDecision() override {
        cout << name << " negotiates trade deals.\n";
    }
};

template <typename T>
class Inventory {
private:
    T quantity;
public:
    Inventory(T q = 0) : quantity(q) {}
    void add(T amount) {
        quantity += amount;
    }

    void remove(T amount) {
        if (amount > quantity) {
            cout << "Not enough resources\n";
            return;
        }
        quantity -= amount;
    }

    T get() const {return quantity;
    }

    void set(T q) {
        quantity = q;
    }
};

class Population {
private:
    int peasants;
    int merchants;
    int nobility;
    int soldiers;
    int happiness;
    friend class Disasters;
public:

    Population(int p, int m, int n, int s, int h) :
        peasants(p), merchants(m), nobility(n), soldiers(s), happiness(h) {}

    int getTotal() const {
        return peasants + merchants + nobility + soldiers;
    }

    int getHappiness() const {
        return happiness;
    }

    int getPeasants() const {
        return peasants;
    }

    void updateHappiness(int change) {
        happiness += change;
        if (happiness > 100) {
            happiness = 100;
        }
        if (happiness < 0) {
            happiness = 0;
        }
    }

    void addPeasants(int count) {
        peasants += count;
    }

    void removePeasants(int count) {
        peasants -= count; if (peasants < 0) peasants = 0;
    }

    void addSoldiers(int count) {
        soldiers += count;
    }

    void starve(int foodShortage) {
        int deaths = foodShortage / 2;
        peasants -= deaths;
        if (peasants < 0) peasants = 0;
        updateHappiness(-20);
    }

    void plague() {
        int deaths = getTotal() * 0.1;
        peasants -= deaths * 0.7;
        merchants -= deaths * 0.15;
        nobility -= deaths * 0.1;
        soldiers -= deaths * 0.05;
        updateHappiness(-30);
    }
};

class Army {
private:
    int soldiers;
    int weapons;
    int morale;
    bool inWar;
public:
    Army(int s, int w, int m) : soldiers(s), weapons(w), morale(m), inWar(false) {}

    int getSoldiers() const {
        return soldiers;
    }

    int getMorale() const {
        return morale;
    }

    void recruit(int count, Population& population) {
        if (count > population.getPeasants() / 10) {
            cout << "Cannot recruit more than 10% of peasant population\n";
            return;
        }
        soldiers += count;
        population.removePeasants(count);
        cout << "Recruited " << count << " soldiers.\n";
    }

    void train() {
        cout << "Training soldiers...\n";
        // Simulate delay without thread/sleep
        for (int i = 0; i < 100000000; i++) {} // Simple busy wait
        morale += 10;
        if (morale > 100) morale = 100;
        cout << "Training complete! Morale +10\n";
    }

    void paySoldiers(int amount, Inventory<int>& gold) {
        if (gold.get() < amount) {
            cout << "Not enough gold to pay soldiers\n";
            return;
        }
        gold.remove(amount);
        morale += 5;
        cout << "Soldiers paid. Morale +5\n";
    }

    void battle() {
        if (soldiers == 0) {
            cout << "No soldiers to fight\n";
            return;
        }
        inWar = true;
        int casualties = rand() % (soldiers / 4);
        soldiers -= casualties;
        morale -= 15;
        cout << "Battle fought! Lost " << casualties << " soldiers. Morale -15\n";
    }
};

class Bank {
private:
    int goldReserve;
    float trustRate; // 1.0 = full trust, 0.0 = no trust
public:
    Bank(int reserve = 10000) : goldReserve(reserve), trustRate(1.0f) {}
    void giveLoan(int amount, Inventory<int>& kingdomGold) {
        if (amount > goldReserve) {
            cout << " Bank cannot provide this loan: insufficient reserve.\n";
            return;
        }
        goldReserve -= amount;
        kingdomGold.add(amount);
        cout << " Bank loaned " << amount << " gold to the kingdom.\n";
    }

    void receiveRepayment(int amount, Inventory<int>& kingdomGold) {
        if (kingdomGold.get() < amount) {
            cout << " Kingdom lacks enough gold to repay loan.\n";
            trustRate -= 0.1f;
            if (trustRate < 0.0f) trustRate = 0.0f;
            return;
        }
        kingdomGold.remove(amount);
        goldReserve += amount;
        trustRate += 0.05f;
        if (trustRate > 1.0f) trustRate = 1.0f;
        cout << " Loan of " << amount << " gold repaid. Trust rate increased.\n";
    }

    void audit() const {
        cout << "\n Bank Audit Report:\n";
        cout << "Gold Reserve: " << goldReserve << "\n";
        cout << "Trust Rate: " << trustRate * 100 << "%\n";
    }

    float getTrustRate() const {
        return trustRate;
    }
    int getReserve() const {
        return goldReserve;
    }

};

class Market {
private:
    struct TradeRoute {
        string name;
        int value;
    };
    TradeRoute tradeRoutes[5]; // Fixed size instead of vector
    int routeCount;
    int foodPrice;
    int weaponPrice;
public:
    Market() : foodPrice(1), weaponPrice(5), routeCount(1) {
        tradeRoutes[0] = { "Neighbor Kingdom", 50 };
    }

    void buyFood(int amount, Inventory<int>& food, Inventory<int>& gold) {
        int cost = amount * foodPrice;
        if (gold.get() < cost) {
            cout << "Not enough gold\n";
            return;
        }
        gold.remove(cost);
        food.add(amount);
        cout << "Bought " << amount << " food for " << cost << " gold.\n";
    }

    void sellFood(int amount, Inventory<int>& food, Inventory<int>& gold) {
        if (food.get() < amount) {
            cout << "Not enough food\n";
            return;
        }
        food.remove(amount);
        gold.add(amount * foodPrice * 0.8); // 80% of buy price
        cout << "Sold " << amount << " food for " << amount * foodPrice * 0.8 << " gold.\n";
    }

    void updatePrices() {
        foodPrice += rand() % 3 - 1; // -1 to +1 change
        weaponPrice += rand() % 5 - 2; // -2 to +2 change
        if (foodPrice < 1) foodPrice = 1;
        if (weaponPrice < 3) weaponPrice = 3;
    }

    void showPrices() const {
        cout << "Market Prices:\n";
        cout << "Food: " << foodPrice << " gold per unit\n";
        cout << "Weapons: " << weaponPrice << " gold per unit\n";
    }
};

class BuildingSystem {
private:
    int farms;
    int barracks;
    int mines;
    int blacksmiths;
public:
    BuildingSystem() : farms(1), barracks(1), mines(1), blacksmiths(1) {}

    int getFarms() const { return farms; }
    int getBarracks() const { return barracks; }

    void buildFarm(Inventory<int>& wood, Inventory<int>& stone) {
        if (wood.get() < 50 || stone.get() < 30) {
            cout << "Not enough resources\n";
            return;
        }
        wood.remove(50);
        stone.remove(30);
        farms++;
        cout << "Built a new farm. Total farms: " << farms << "\n";
    }

    void buildBarracks(Inventory<int>& wood, Inventory<int>& stone) {
        if (wood.get() < 80 || stone.get() < 50) {
            cout << "Not enough resources\n";
            return;
        }
        wood.remove(80);
        stone.remove(50);
        barracks++;
        cout << "Built a new barracks. Total barracks: " << barracks << "\n";
    }

    void produceResources(Inventory<int>& food, Inventory<int>& iron) {
        food.add(farms * 100);
        iron.add(mines * 20);
    }
};

class Kingdom {
public:
    int turn;
    Difficulty difficulty;
    King* currentKing;
    Population population;
    Army army;
    Bank bank;
    Market market;
    BuildingSystem buildings;
    Inventory<int> food;
    Inventory<int> gold;
    Inventory<int> wood;
    Inventory<int> stone;
    Inventory<int> iron;
    Inventory<int> weapons;
    int lastDisasterTurn;
    int lastWarTurn;
    int lastElectionTurn;
    bool gameOver;

    void randomEvent() {
        int event = rand() % 10;
        switch (event) {
        case 0: {
            int plagueDeaths = population.getTotal() * 0.1;
            cout << "A plague has killed " << plagueDeaths << " people!\n";
            population.plague();
            break;
        }
        case 1: {
            int goldFound = 100 + rand() % 200;
            cout << "Miners found a gold vein! +" << goldFound << " gold.\n";
            gold.add(goldFound);
            break;
        }
        case 2: {
            cout << "Merchants report increased trade! Happiness +5\n";
            population.updateHappiness(5);
            break;
        }
        case 3: {
            cout << "Bandits attacked a trade route! Gold -50\n";
            gold.remove(50);
            break;
        }
        case 4: {
            cout << "Good harvest this season! Food +200\n";
            food.add(200);
            break;
        }
        }
    }

    void checkElection() {
        if (turn - lastElectionTurn >= 5) {
            cout << "\n=== ELECTION TIME ===\n";
            int approval = population.getHappiness() / 2 + rand() % 30;

            if (approval < 40) {
                cout << "The people are unhappy! " << currentKing->name << " has been overthrown!\n";
                delete currentKing;
                currentKing = new King("King_" + to_string(turn));
                population.updateHappiness(20); // New king happiness boost
            }
            else {
                cout << currentKing->name << " remains in power with " << approval << "% approval.\n";
            }

            lastElectionTurn = turn;
        }
    }

    void checkGameOver() {
        if (population.getTotal() <= 0) {
            cout << "GAME OVER: Everyone has died. The kingdom has fallen.\n";
            gameOver = true;
            return;
        }
        if (food.get() <= 0) {
            cout << "GAME OVER: No food left. The kingdom has starved.\n";
            gameOver = true;
            return;
        }
        if (gold.get() < -1000) {
            cout << "GAME OVER: The kingdom is bankrupt.\n";
            gameOver = true;
            return;
        }
        if (population.getHappiness() <= 10) {
            cout << "GAME OVER: The people have revolted and overthrown the kingdom.\n";
            gameOver = true;
            return;
        }
        if (army.getSoldiers() == 0 && rand() % 10 == 0) {
            cout << "GAME OVER: Enemy kingdom attacked and conquered your defenseless land.\n";
            gameOver = true;
            return;
        }
        if (turn >= 20) {
            cout << "\n\n=== YOU WIN! ===\n";
            cout << "Your kingdom has survived 20 turns and proven its stability!\n";
            cout << "Final Stats:\n";
            showStatus();
            gameOver = true;
            return;
        }
    }

public:
    Kingdom(Difficulty diff) :
        difficulty(diff),
        turn(1),
        currentKing(new King("King_1")),
        population(100, 20, 10, 30, 70),
        army(30, 50, 60),
        lastDisasterTurn(-5),
        lastWarTurn(-5),
        lastElectionTurn(0),
        gameOver(false) {

        switch (diff) {
        case EASY:
            food.set(1000);
            gold.set(1000);
            wood.set(200);
            stone.set(200);
            iron.set(150);
            weapons.set(100);
            break;
        case MEDIUM:
            food.set(700);
            gold.set(700);
            wood.set(150);
            stone.set(150);
            iron.set(100);
            weapons.set(70);
            break;
        case HARD:
            food.set(500);
            gold.set(500);
            wood.set(100);
            stone.set(100);
            iron.set(70);
            weapons.set(50);
            break;
        }
    }

    ~Kingdom() {
        delete currentKing;
    }

    void showStatus() const {
        cout << "\n--------------------------\n";
        cout << "Loading kingdom status...\n";
        cout << "--------------------------\n";

        this_thread::sleep_for(std::chrono::seconds(2));
        system("cls");

        cout << "\n=== KINGDOM STATUS (Turn " << turn << ") ===\n";
        cout << "King: " << currentKing->name << endl;
        cout << "Tax: " << currentKing->taxRate << "%\n";
        cout << "Population: " << population.getTotal() << endl;
        cout << "Happiness: " << population.getHappiness() << "%\n";
        cout << "Food: " << food.get() << endl;
        cout << "Gold: " << gold.get() << "\n";
        cout << "Resources: Wood=" << wood.get() << " Stone=" << stone.get()
            << " Iron=" << iron.get() << " Weapons=" << weapons.get() << "\n";
        cout << "Army: " << army.getSoldiers() << " soldiers (Morale: " << army.getMorale() << "%)\n";
        cout << "Buildings: Farms=" << buildings.getFarms() << " Barracks=" << buildings.getBarracks() << "\n";
        cout << "------------------------------------\n";
    
    }

    void nextTurn() {
        if (gameOver) return;

        turn++;
        cout << "\n=== TURN " << turn << " BEGINS ===\n";

        // Consume food
        int foodConsumption = population.getTotal() / 2;
        food.remove(foodConsumption);
        cout << "Consumed " << foodConsumption << " food.\n";

        // Pay soldiers
        army.paySoldiers(army.getSoldiers() * 2, gold);

        // Produce resources
        buildings.produceResources(food, iron);

        // Random events
        if (rand() % 4 == 0) {
            randomEvent();
        }

        // Check for elections
        checkElection();

        // Check for disasters
        int disasterInterval = (difficulty == EASY) ? 3 : 2;
        if (turn - lastDisasterTurn >= disasterInterval) {
            string disasters[] = { "Earthquake", "Famine", "Flood" };
            //Disasters::applyDisaster(*this, disasters[rand() % 3]);
            lastDisasterTurn = turn;
        }

        // Update market prices
        market.updatePrices();

        // Check game over conditions
        checkGameOver();
    }

    void playerTurn() {
        if (gameOver) return;

        showStatus();

        cout << "\n=== ACTIONS ===\n";
        cout << "1. Collect Taxes\n";
        cout << "2. Change Tax Rate\n";
        cout << "3. Recruit Soldiers\n";
        cout << "4. Train Soldiers\n";
        cout << "5. Buy/Sell Food\n";
        cout << "6. Build Farm\n";
        cout << "7. Build Barracks\n";
        cout << "8. Take Loan\n";
        cout << "0. Quit Game\n";

        int choice;
        cout << "\nChoose action: ";
        cin >> choice;
        cout << "--------------------------\n";
        switch (choice) {
        case 1: {
            int taxAmount = (population.getTotal() * currentKing->taxRate) / 100;
            gold.add(taxAmount);
            population.updateHappiness(-5);
            cout << "Collected " << taxAmount << " gold in taxes. Happiness -5.\n";
            void produceResources(Inventory<int>&food, Inventory<int>&iron);
            turn++;
            break;
        }
        case 2: {
            int newRate;
            cout << "Enter new tax rate (5-50): ";
            cin >> newRate;
            currentKing->setTaxRate(newRate);
            void produceResources(Inventory<int>&food, Inventory<int>&iron);
            turn++;
            break;
        }
        case 3: {
            int recruits;
            cout << "Enter number of soldiers you want to recruit: ";
            cin >> recruits;
            army.recruit(recruits, population);
            void produceResources(Inventory<int>&food, Inventory<int>&iron);
            turn++;
            break;
        }
        case 4: {
            army.train();
            void produceResources(Inventory<int>&food, Inventory<int>&iron);
            turn++;
            break;
        }
        case 5: {
            cout << "1. Buy Food\n2. Sell Food\n";
            int subchoice;
            cin >> subchoice;
            if (subchoice == 1) {
                int amount;
                cout << "How much food to buy: ";
                cin >> amount;
                market.buyFood(amount, food, gold);
            }
            else {
                int amount;
                cout << "How much food to sell: ";
                cin >> amount;
                market.sellFood(amount, food, gold);
            }
            void produceResources(Inventory<int>&food, Inventory<int>&iron);
            turn++;
            break;
        }
        case 6: {
            buildings.buildFarm(wood, stone);
            void produceResources(Inventory<int>&food, Inventory<int>&iron);
            turn++;
            break;
        }
        case 7: {
            buildings.buildBarracks(wood, stone);
            void produceResources(Inventory<int>&food, Inventory<int>&iron);
            turn++;
            this_thread::sleep_for(std::chrono::seconds(2));
            cout << "---------------------------\n";
            randomEvent();
            break;
        }
        case 8: {
            int amount, term;
            cout << "Enter loan amount: ";
            cin >> amount;
            cout << "Repayment term (turns): ";
            cin >> term;
            bank.giveLoan(amount, gold);
            cout << "--------------------------\n";
            void produceResources(Inventory<int>&food, Inventory<int>&iron);
            turn++;
            break;
        }
        case 0: {
            int ch;
            gameOver = true;
            cout << "Do you want save game?\n";
            cout << "1. Yes\n0. No\n";
            cout << "Enter your choice: ";
            cin >> ch;
            if (ch == 1) {
                cout << "Game saved and exited.\n";
            }
            else if (ch == 0) {
                cout << "Game not saved and exited.\n";
            }
            
            break;
        }
        default: {
            cout << "Invalid choice.\n";
            break;
        }
        }
    }

    bool isGameOver() const {
        return gameOver;
    }

    void saveGame() const {
        ofstream saveFile("stronghold_save.txt");
        saveFile << turn << "\n";
        saveFile << static_cast<int>(difficulty) << "\n";
        saveFile << currentKing->name << "\n";
        saveFile << currentKing->taxRate << "\n";
        saveFile << food.get() << "\n";
        saveFile << gold.get() << "\n";
        saveFile << wood.get() << "\n";
        saveFile << stone.get() << "\n";
        saveFile << iron.get() << "\n";
        saveFile << weapons.get() << "\n";
        saveFile << population.getHappiness() << "\n";
        saveFile << population.getPeasants() << "\n";
        saveFile << population.getTotal() << "\n";
        saveFile.close();
        cout << "Game saved.\n";
    }

    void loadGame() {
        ifstream saveFile("stronghold_save.txt");
        if (saveFile) {
            int diff;
            string kingName;
            int taxRate, f, g, w, s, i, wp, happiness, peasants, totalPop;
            saveFile >> turn;
            saveFile >> diff;
            difficulty = static_cast<Difficulty>(diff);
            saveFile.ignore();
            getline(saveFile, kingName);
            saveFile >> taxRate;
            saveFile >> f >> g >> w >> s >> i >> wp >> happiness >> peasants >> totalPop;
            delete currentKing;
            currentKing = new King(kingName);
            currentKing->setTaxRate(taxRate);
            int merchants = totalPop * 0.2;  // Adjust these ratios as needed
            int nobility = totalPop * 0.1;
            int soldiers = totalPop - peasants - merchants - nobility;

            population = Population(peasants, merchants, nobility, soldiers, happiness);
            food.set(f);
            gold.set(g);
            wood.set(w);
            stone.set(s);
            iron.set(i);
            weapons.set(wp);

            cout << "Game loaded.\n";
        }
        else {
            cout << "No save file found.\n";
        }
    }
};

class Disasters {
public:
    static void applyDisaster(Kingdom& kingdom, const string& disasterName);
};

void Disasters::applyDisaster(Kingdom& kingdom, const string& disasterName) {
    cout << "DISASTER: " << disasterName << " has struck the kingdom!\n";

    int totalPop = kingdom.population.getTotal();
    int peasantsLoss = static_cast<int>(totalPop * 0.2 * 0.7);
    int merchantsLoss = static_cast<int>(totalPop * 0.2 * 0.15);
    int nobilityLoss = static_cast<int>(totalPop * 0.2 * 0.1);
    int soldiersLoss = static_cast<int>(totalPop * 0.2 * 0.05);

    kingdom.population.removePeasants(peasantsLoss);

    kingdom.food.set(static_cast<int>(kingdom.food.get() * 0.8));
    kingdom.gold.set(static_cast<int>(kingdom.gold.get() * 0.8));
    kingdom.wood.set(static_cast<int>(kingdom.wood.get() * 0.8));
    kingdom.stone.set(static_cast<int>(kingdom.stone.get() * 0.8));
    kingdom.iron.set(static_cast<int>(kingdom.iron.get() * 0.8));
    kingdom.weapons.set(static_cast<int>(kingdom.weapons.get() * 0.8));
    kingdom.population.updateHappiness(-20);

    if (disasterName == "Earthquake") { 
        cout << "Buildings damaged! Resources lost!\n";
    }
    else if (disasterName == "Famine") {
        cout << "Crops failed! Food halved!\n";
    }
    else if (disasterName == "Flood") {
        cout << "Floods destroyed resources!\n";
    }
    cout << "20% of resources and some population lost due to the disaster.\n";
}

int main() {
    srand(time(0));

    cout << " ===== WELCOME TO STRONGHOLD KINGDOM SIMULATOR =====\n\n";
    cout << "Choose difficulty level:\n";
    cout << "1. Easy\n2. Medium\n3. Hard\n";
    cout << "Enter choice: ";
    int diffChoice;
    cin >> diffChoice;
    Difficulty diff = static_cast<Difficulty>(diffChoice - 1);

    Kingdom game(diff);

    while (!game.isGameOver()) {
        game.playerTurn();
    }

    cout << "Thanks for playing!\n";
    system("pause>0");
    return 0;
}