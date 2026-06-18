#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cmath>

// ===================================================
// =============== OBSERVER PATTERN ==================
// ===================================================

struct IObserver
{
    virtual ~IObserver() {}
    virtual void OnNotify(const std::string& message) = 0;
};

class Event
{
public:
    void Add(IObserver* obs)
    {
        const auto it = std::find(std::begin(mObservers), std::end(mObservers), obs);
        if (it == std::end(mObservers)) mObservers.push_back(obs);
    }

    void Notify(const std::string& message) const
    {
        for (auto observer : mObservers)
        {
            observer->OnNotify(message);
        }
    }

private:
    std::vector<IObserver*> mObservers;
};

// Suggested concrete observers (already implemented; they will do nothing
// until Event::Add / Notify are implemented).
class DisplayObserver : public IObserver
{
public:
    void OnNotify(const std::string& m) override
    {
        std::cout << "[Display] " << m << "\n";
    }
};

class WaiterObserver : public IObserver
{
public:
    void OnNotify(const std::string& m) override
    {
        std::cout << "[Waiter] " << m << " -> deliver!\n";
    }
};

class HistoryObserver : public IObserver
{
public:
    void OnNotify(const std::string& m) override
    {
        std::cout << "[History] " << m << "\n";
    }
};


// ===================================================
// =============== FLYWEIGHT PATTERN =================
// ===================================================

struct Recipe
{
    std::string name;
    int baseCookTimeMin;

    Recipe(const std::string& n, int t)
        : name(n), baseCookTimeMin(t)
    {
    }
};

class RecipeFactory
{
public:
    std::shared_ptr<Recipe> Get(const std::string& name, int baseTime)
    {
        if (std::shared_ptr<Recipe> recipe = mCache[name]) return recipe;
        mCache[name] = std::make_shared<Recipe>(name, baseTime);
        return mCache.at(name);
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Recipe>> mCache;
};


// ===================================================
// =============== COMMAND PATTERN ===================
// ===================================================

struct ICommand
{
    virtual ~ICommand() {}
    virtual void Execute() = 0;
};

// Receiver (kitchen)
class KitchenReceiver
{
public:
    void Cook(const std::string& dish, int minutes)
    {
        std::cout << "[Cook] Cooking " << dish << " (" << minutes << " min)\n";
    }

    void Serve(const std::string& dish)
    {
        std::cout << "[Serve] Serving " << dish << "\n";
    }
};

// Concrete command: cook
class CookDishCommand : public ICommand
{
public:
    CookDishCommand(KitchenReceiver& pReceiver, std::string pDish, const int pMinutes) :
        mReceiver(pReceiver), mDish(std::move(pDish)), mMinutes(pMinutes)
    {
    }

    void Execute() override
    {
        mReceiver.Cook(mDish, mMinutes);
    }

private:
    KitchenReceiver& mReceiver;
    std::string mDish;
    int mMinutes;
};

// Concrete command: serve
class ServeDishCommand : public ICommand
{
public:
    ServeDishCommand(KitchenReceiver& pReceiver, std::string pDish) :
        mReceiver(pReceiver), mDish(std::move(pDish))
    {
    }

    void Execute() override
    {
        mReceiver.Serve(mDish);
    }

private:
    KitchenReceiver& mReceiver;
    std::string mDish;
};

// Invoker
class WaiterInvoker
{
public:
    void Set(std::shared_ptr<ICommand> cmd)
    {
        mCurrentCommand = cmd;
    }

    void Trigger()
    {
        mCurrentCommand->Execute();
    }

private:
    std::shared_ptr<ICommand> mCurrentCommand;
};


// ===================================================
// =============== STRATEGY PATTERN ==================
// ===================================================

struct ICookStrategy
{
    virtual ~ICookStrategy() {}
    virtual int ComputeCookTime(int base) const = 0;
    virtual const char* Name() const = 0;
};

class FastCookStrategy : public ICookStrategy
{
public:
    int ComputeCookTime(const int base) const override
    {
        return std::max(1, base - 2);
    }

    const char* Name() const override
    {
        return "Fast";
    }
};

class SlowCookStrategy : public ICookStrategy
{
public:
    int ComputeCookTime(const int base) const override
    {
        return base + 2;
    }

    const char* Name() const override
    {
        return "Slow";
    }
};

class SteamCookStrategy : public ICookStrategy
{
public:
    int ComputeCookTime(const int base) const override
    {
        return base;
    }

    const char* Name() const override
    {
        return "Steam";
    }
};


// ===================================================
// ================== STATE PATTERN ==================
// ===================================================

struct IRestaurantState
{
    virtual ~IRestaurantState() {}
    virtual const char* Name() const = 0;
    virtual bool CanAcceptOrder() const = 0;
};

class OpenState : public IRestaurantState
{
public:
    const char* Name() const override { return "Open"; }
    bool CanAcceptOrder() const override { return true; }
};

class PausedState : public IRestaurantState
{
public:
    const char* Name() const override { return "Paused"; }
    bool CanAcceptOrder() const override { return false; }
};

class ClosedState : public IRestaurantState
{
public:
    const char* Name() const override { return "Closed"; }
    bool CanAcceptOrder() const override { return false; }
};

class RestaurantContext
{
public:
    RestaurantContext()
        : current_(&open_)
    {
    }

    void SetOpen()
    {
        current_ = &open_;
    }

    void SetPaused()
    {
        current_ = &paused_;
    }

    void SetClosed()
    {
        current_ = &closed_;
    }

    const IRestaurantState& State() const
    {
        return *current_;
    }

private:
    OpenState open_;
    PausedState paused_;
    ClosedState closed_;
    IRestaurantState* current_;
};


// ===================================================
// ============= ABSTRACT FACTORY PATTERN ============
// ===================================================

class Dish; // forward

struct IKitchenFactory
{
    virtual ~IKitchenFactory() {}
    virtual std::unique_ptr<Dish> CreateMain(RecipeFactory&, const ICookStrategy&) = 0;
    virtual std::unique_ptr<Dish> CreateSide(RecipeFactory&, const ICookStrategy&) = 0;
};


// ===================================================
// ========== PROTOTYPE + DISH (COMBINED) ============
// ===================================================

class Dish
{
public:
    Dish(std::string n, std::shared_ptr<Recipe> r, const ICookStrategy* s)
        : name_(std::move(n)), recipe_(std::move(r)), strategy_(s)
    {
    }

    // PROTOTYPE
    std::unique_ptr<Dish> Clone() const
    {
        return std::make_unique<Dish>(name_, std::make_shared<Recipe>(*recipe_), strategy_);
    }

    void SetName(const std::string& n)
    {
        name_ = n;
    }

    const std::string& Name() const
    {
        return name_;
    }

    const Recipe* TryGetRecipe() const
    {
        // helper to avoid crashes until Flyweight is implemented
        return recipe_.get();
    }

    int CookTime() const
    {
        if (!strategy_ || !recipe_) return 0;
        return strategy_->ComputeCookTime(recipe_->baseCookTimeMin);
    }

    const char* StrategyName() const
    {
        return strategy_ ? strategy_->Name() : "None";
    }

private:
    std::string name_;
    std::shared_ptr<Recipe> recipe_;   // Flyweight (shared)
    const ICookStrategy* strategy_;    // Strategy (behavior)
};


// ===================================================
// ========== ABSTRACT FACTORY IMPLEMENTATIONS =======
// ===================================================

class ItalianKitchenFactory : public IKitchenFactory
{
public:
    std::unique_ptr<Dish> CreateMain(RecipeFactory& rf, const ICookStrategy& s) override
    {
        return std::make_unique<Dish>("Pasta", rf.Get("Pasta", 5), &s);
    }

    std::unique_ptr<Dish> CreateSide(RecipeFactory& rf, const ICookStrategy& s) override
    {
        return std::make_unique<Dish>("Risotto", rf.Get("Risotto", 6), &s);
    }
};

class JapaneseKitchenFactory : public IKitchenFactory
{
public:
    std::unique_ptr<Dish> CreateMain(RecipeFactory& rf, const ICookStrategy& s) override
    {
        return std::make_unique<Dish>("Ramen", rf.Get("Ramen", 5), &s);
    }

    std::unique_ptr<Dish> CreateSide(RecipeFactory& rf, const ICookStrategy& s) override
    {
        return std::make_unique<Dish>("Sushi", rf.Get("Sushi", 3), &s);
    }
};


// ===================================================
// ======================= MAIN ======================
// ===================================================

int main()
{
    std::cout << "=== RESTAURANT MINI-PROJECT (7 PATTERNS) ===\n\n";

    // ---------- Observer setup ----------
    Event dishReady;
    DisplayObserver display;
    WaiterObserver waiter;
    HistoryObserver history;
    
    dishReady.Add(&display);
    dishReady.Add(&waiter);
    dishReady.Add(&history);

    // ---------- Strategy instances ----------
    FastCookStrategy fast;
    SlowCookStrategy slow;
    SteamCookStrategy steam;

    // ---------- Flyweight / AF setup ----------
    RecipeFactory recipes;
    ItalianKitchenFactory it;
    JapaneseKitchenFactory jp;

    // ---------- State context ----------
    RestaurantContext ctx;
    ctx.SetOpen(); // You can test SetPaused() / SetClosed() after implementation

    // ---------- Create dishes via Abstract Factory ----------
    // NOTE: factories return nullptr until you implement them.
    auto d1 = it.CreateMain(recipes, slow);   // Pasta (Slow) expected
    auto d2 = it.CreateSide(recipes, fast);   // Risotto (Fast) expected
    auto d3 = jp.CreateMain(recipes, steam);  // Ramen (Steam) expected

    // ---------- Strategy demo (safe prints) ----------
    std::cout << "=== COOK TIMES (after Strategy & AF are implemented) ===\n";
    if (d1) std::cout << d1->Name() << " (" << d1->StrategyName() << ") : " << d1->CookTime() << " min\n";
    if (d2) std::cout << d2->Name() << " (" << d2->StrategyName() << ") : " << d2->CookTime() << " min\n";
    if (d3) std::cout << d3->Name() << " (" << d3->StrategyName() << ") : " << d3->CookTime() << " min\n";
    std::cout << "\n";

    // ---------- Prototype: clone configured dish ----------
    std::unique_ptr<Dish> d1Clone;
    if (d1)
    {
        d1Clone = d1->Clone(); // currently returns nullptr (stub) → implement Prototype
        if (d1Clone)
        {
            d1Clone->SetName("Pasta (Table 7 clone)");
        }
    }

    // ---------- Command invocations (no-op until implemented) ----------
    KitchenReceiver kitchen;
    std::shared_ptr<ICommand> cmdCook = std::make_shared<CookDishCommand>(kitchen, d1->Name(), d1->CookTime());
    std::shared_ptr<ICommand> cmdServe = std::make_shared<ServeDishCommand>(kitchen, d1->Name());

    WaiterInvoker inv;
    inv.Set(cmdCook);  inv.Trigger();
    inv.Set(cmdServe); inv.Trigger();

    // ---------- Notify observers when dish is ready ----------
    dishReady.Notify("Dish ready: " + d1->Name());

    // ---------- Flyweight sanity (optional pointer display) ----------
    // After implementing flyweight cache, the two addresses below should match:
    auto rA = recipes.Get("Pasta", 5);
    auto rB = recipes.Get("Pasta", 5);
    std::cout << "\n[Flyweight] Recipe 'Pasta' pointers (after cache): "
              << rA.get() << " / " << rB.get() << "  (should be SAME)\n";

    // ---------- Prototype demo print ----------
    std::cout << "\n=== PROTOTYPE CLONE ===\n";
    if (d1Clone)
    {
        const Recipe* rec = d1Clone->TryGetRecipe();
        std::cout << "Clone: " << d1Clone->Name()
                  << " (Strategy: " << d1Clone->StrategyName()
                  << ", BaseTime: " << (rec ? rec->baseCookTimeMin : -1) << ")\n";
    }
    else
    {
        std::cout << "(Clone not created yet -- implement Prototype)\n";
    }

    // ---------- State gate demo ----------
    std::cout << "\n=== RESTAURANT STATE: " << ctx.State().Name() << " ===\n";
    if (!ctx.State().CanAcceptOrder())
    {
        std::cout << "[State] Orders are blocked in this mode.\n";
    }
    else
    {
        std::cout << "[State] Orders accepted.\n";
    }

    std::cout << "\n=== END (complete all TODOs to see full behavior) ===\n";
    return 0;
}


