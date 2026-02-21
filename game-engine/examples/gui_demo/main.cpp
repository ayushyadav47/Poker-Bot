#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "core/Deck.h"
#include "core/GameState.h"
#include "core/Card.h"
#include "engine/PokerEngine.h"
#include "engine/RuleEngine.h"
#include "interfaces/IActionProvider.h"
#include "interfaces/IRandomGenerator.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <deque>

using namespace poker::core;
using namespace poker::engine;

// --- Shared State for Rendering ---
struct RenderState {
    GameState gs;
    std::string lastEvent;
    std::deque<std::string> messages;
    bool waitingForAction = false;
    std::vector<Action> legalActions;
    size_t activePlayerId = 0;
};

std::mutex stateMutex;
RenderState renderState;
std::condition_variable actionCV;
std::atomic<bool> gameRunning{true};
Action userAction;
bool actionReady = false;

// --- Helper Functions ---
std::string formatMoney(int64_t amount) {
    return "$" + std::to_string(amount);
}

// --- Action Provider ---
class GuiActionProvider : public poker::interfaces::IActionProvider {
public:
    Action getAction(size_t playerId, const GameState &state,
                     const std::vector<Action> &legalActions) override {
        if (!gameRunning) return Action(ActionType::Fold, 0, playerId);

        if (playerId == 0) { // Human Player
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                renderState.gs = state; // Sync latest state
                renderState.waitingForAction = true;
                renderState.legalActions = legalActions;
                renderState.activePlayerId = playerId;
                actionReady = false;
            }

            std::unique_lock<std::mutex> lock(stateMutex);
            actionCV.wait(lock, []{ return actionReady || !gameRunning; });

            {
                renderState.waitingForAction = false;
                renderState.legalActions.clear();
            }

            if (!gameRunning) return Action(ActionType::Fold, 0, playerId);
            return userAction;
        } else {
            // Bot Logic (Simple Passive)
            // Prefer Check
            for (const auto &a : legalActions) {
                if (a.type == ActionType::Check) return a;
            }
            // Prefer Call
            for (const auto &a : legalActions) {
                if (a.type == ActionType::Call) return a;
            }
             // Accept All-In
            for (const auto &a : legalActions) {
                if (a.type == ActionType::AllIn) return a;
            }
            // Fold
            return legalActions.front();
        }
    }
};

// --- Game Loop Thread ---
void gameThreadFunc() {
    auto rng = std::make_shared<Mt19937Generator>();
    auto actionProvider = std::make_shared<GuiActionProvider>();
    PokerEngine engine(actionProvider, rng);

    engine.setEventCallback([](const std::string &event, const GameState &state) {
        std::lock_guard<std::mutex> lock(stateMutex);
        renderState.gs = state;
        renderState.lastEvent = event;
        
        std::string msg = event;
        if (event == "hand_start") msg = "--- New Hand ---";
        else if (event.starts_with("street_")) msg = "--- " + event.substr(7) + " ---";
        else if (event.starts_with("winner_")) msg = "Winner: " + event.substr(7);
        else if (event == "action") {
            const auto& a = state.getActionHistory().back();
            msg = "Player " + std::to_string(a.playerId) + ": " + Action::actionTypeName(a.type);
            if(a.amount > 0) msg += " " + std::to_string(a.amount);
        }
        
        renderState.messages.push_back(msg);
        if (renderState.messages.size() > 20) renderState.messages.pop_front();
    });

    GameState state;
    std::vector<Player> players;
    players.emplace_back(0, "You", 1000);
    players.emplace_back(1, "Bot", 1000);
    state.setPlayers(players);
    state.setSmallBlind(5);
    state.setBigBlind(10);
    state.setDealerPosition(0);

    while (gameRunning) {
        // Check for busted players
        bool gameOver = false;
        {
             std::lock_guard<std::mutex> lock(stateMutex);
             for(const auto& p : renderState.gs.getPlayers()) {
                 if(p.getChips() <= 0) gameOver = true;
             }
        }
        if(gameOver) break;

        engine.playHand(state);
        
        // Small pause between hands
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Rotate dealer
        state.setDealerPosition((state.getDealerPosition() + 1) % state.getPlayers().size());
    }
}

// --- Main ---
int main() {
    sf::RenderWindow window(sf::VideoMode(1024, 768), "Poker Engine GUI Demo");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window)) return -1;

    // Start game thread
    std::thread gameThread(gameThreadFunc);

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) {
                gameRunning = false;
                actionReady = true; // Wake up thread if waiting
                actionCV.notify_all();
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // --- Render UI ---
        std::lock_guard<std::mutex> lock(stateMutex);
        const auto& gs = renderState.gs;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(window.getSize().x, window.getSize().y));
        ImGui::Begin("Poker Table", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        // Community Cards
        ImGui::Text("Community Cards:");
        ImGui::Separator();
        if (gs.getCommunityCards().empty()) {
            ImGui::Text("(No cards dealt)");
        } else {
            for (const auto &card : gs.getCommunityCards()) {
                ImGui::SameLine();
                ImGui::Button(card.toString().c_str(), ImVec2(50, 70));
            }
        }
        ImGui::NewLine();

        // Pot Info
        ImGui::Text("Pot: %s", formatMoney(gs.getPot().getTotal()).c_str());
        
        int64_t currentBet = 0;
        for(const auto& p : gs.getPlayers()) {
            currentBet = std::max(currentBet, p.getCurrentBet());
        }
        ImGui::Text("Current Bet: %s", formatMoney(currentBet).c_str());
        ImGui::Separator();

        // Players
        ImGui::Columns(2, "players");
        
        // Player 0 (You)
        const auto& p0 = gs.getPlayer(0);
        ImGui::Text("Player: %s (You)", p0.getName().c_str());
        ImGui::Text("Chips: %s", formatMoney(p0.getChips()).c_str());
        ImGui::Text("Status: %s", p0.isFolded() ? "Folded" : (p0.isAllIn() ? "All-In" : "Active"));
        if (!p0.getHoleCards().empty()) {
            ImGui::Text("Cards:");
            for (const auto &card : p0.getHoleCards()) {
                ImGui::SameLine();
                ImGui::Button(card.toString().c_str(), ImVec2(50, 70));
            }
        }
        ImGui::NextColumn();

        // Player 1 (Bot)
        const auto& p1 = gs.getPlayer(1);
        ImGui::Text("Player: %s (Bot)", p1.getName().c_str());
        ImGui::Text("Chips: %s", formatMoney(p1.getChips()).c_str());
        ImGui::Text("Status: %s", p1.isFolded() ? "Folded" : (p1.isAllIn() ? "All-In" : "Active"));
        if (gs.getStreet() == Street::Showdown && !p1.isFolded()) {
             ImGui::Text("Cards:");
             for (const auto &card : p1.getHoleCards()) {
                ImGui::SameLine();
                ImGui::Button(card.toString().c_str(), ImVec2(50, 70));
            }
        } else {
            ImGui::Text("Cards: [X] [X]");
        }
        ImGui::Columns(1);
        ImGui::Separator();

        // Action Controls
        if (renderState.waitingForAction && renderState.activePlayerId == 0) {
            ImGui::Text("Your Action Needed:");
            
            const auto& p = renderState.gs.getPlayer(0);
            int64_t stack = p.getChips();
            int64_t currentBet = p.getCurrentBet();
            int64_t callAmt = poker::engine::RuleEngine::getCallAmount(renderState.gs, 0);
            int64_t minRaiseTotal = poker::engine::RuleEngine::getMinRaise(renderState.gs, 0);
            
            // Calculate Min/Max for Input Box
            int64_t minInput = 0;
            int64_t maxInput = stack;

            if (callAmt == 0) {
                 // Betting
                 minInput = std::min((int64_t)renderState.gs.getBigBlind(), stack);
            } else {
                 // Raising: Amount to ADD (on top of current bet)
                 minInput = minRaiseTotal - currentBet;
                 if (minInput > stack) minInput = stack;
            }
            
            static int betAmount = 0;
            // Clamp logic
            if (betAmount < minInput) betAmount = (int)minInput;
            if (betAmount > maxInput) betAmount = (int)maxInput;
            
            // Draw Fold
            if (ImGui::Button("Fold", ImVec2(80, 40))) {
                 userAction = Action(ActionType::Fold, 0, 0);
                 actionReady = true;
                 actionCV.notify_one();
            }
            ImGui::SameLine();
            
            // Draw Check/Call
            if (callAmt == 0) {
                if (ImGui::Button("Check", ImVec2(80, 40))) {
                     userAction = Action(ActionType::Check, 0, 0);
                     actionReady = true;
                     actionCV.notify_one();
                }
            } else {
                 std::string label = "Call " + std::to_string(callAmt);
                 if (ImGui::Button(label.c_str(), ImVec2(100, 40))) {
                     userAction = Action(ActionType::Call, callAmt, 0);
                     actionReady = true;
                     actionCV.notify_one();
                 }
            }
            ImGui::SameLine();

            // Bet/Raise Input
            ImGui::PushItemWidth(100);
            int step = (int)renderState.gs.getBigBlind();
            int stepFast = step * 5;
            ImGui::InputInt("##betamt", &betAmount, step, stepFast);
            ImGui::PopItemWidth();

            // Clamp again
            if (betAmount < minInput) betAmount = (int)minInput;
            if (betAmount > maxInput) betAmount = (int)maxInput;
            
            ImGui::SameLine();
            if (callAmt == 0) {
                 std::string label = "Bet " + std::to_string(betAmount);
                 if (ImGui::Button(label.c_str(), ImVec2(100, 40))) {
                      userAction = Action(ActionType::Bet, betAmount, 0);
                      actionReady = true;
                      actionCV.notify_one();
                 }
            } else {
                 std::string label = "Raise " + std::to_string(betAmount);
                 if (ImGui::Button(label.c_str(), ImVec2(100, 40))) {
                      userAction = Action(ActionType::Raise, betAmount, 0);
                      actionReady = true;
                      actionCV.notify_one();
                 }
            }
            ImGui::NewLine();

        } else if (renderState.waitingForAction) {
             ImGui::Text("Waiting for opponent...");
        } else {
            ImGui::Text("Processing...");
        }

        // Event Log
        ImGui::Separator();
        ImGui::Text("Game Log:");
        ImGui::BeginChild("LogRegion", ImVec2(0, 150), true);
        for (const auto &msg : renderState.messages) {
            ImGui::Text("%s", msg.c_str());
        }
        if (renderState.lastEvent != "") {
             ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    gameRunning = false;
    actionReady = true;
    actionCV.notify_all();
    if (gameThread.joinable()) {
        gameThread.join();
    }

    ImGui::SFML::Shutdown();
    return 0;
}