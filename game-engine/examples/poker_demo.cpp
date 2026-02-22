#include "core/Deck.h"
#include "core/GameState.h"
#include "engine/PokerEngine.h"
#include "engine/RuleEngine.h"
#include "interfaces/IActionProvider.h"
#include "interfaces/IRandomGenerator.h"


#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <string>


using namespace poker::core;
using namespace poker::engine;

// ────────────────────────────────────────────────────────
// Interactive action provider:
//   Player 0 = Human (reads from stdin)
//   Player 1 = Bot   (always calls or checks)
// ────────────────────────────────────────────────────────
class InteractiveActionProvider : public poker::interfaces::IActionProvider {
public:
  Action getAction(size_t playerId, const GameState &state,
                   const std::vector<Action> &legalActions) override {
    if (playerId == 0) {
      return getHumanAction(state, legalActions);
    } else {
      return getBotAction(legalActions);
    }
  }

private:
  // ── Human: show options and read choice ──
  Action getHumanAction(const GameState &state,
                        const std::vector<Action> &legalActions) {
    std::cout << "\n  Your options:\n";

    // Build a simplified menu with custom bet/raise support.
    struct MenuEntry {
      std::string label;
      Action action;
      bool allowCustomAmount;
    };
    std::vector<MenuEntry> menu;

    // Track min/max for bet and raise to allow custom amounts.
    int64_t minBet = 0, maxBet = 0;
    int64_t minRaise = 0, maxRaise = 0;
    bool hasBet = false, hasRaise = false;

    for (const auto &a : legalActions) {
      switch (a.type) {
      case ActionType::Fold:
        menu.push_back({"Fold", a, false});
        break;
      case ActionType::Check:
        menu.push_back({"Check", a, false});
        break;
      case ActionType::Call:
        menu.push_back({"Call " + std::to_string(a.amount), a, false});
        break;
      case ActionType::Bet:
        minBet = a.amount;
        maxBet = state.getPlayer(0).getChips();
        hasBet = true;
        break;
      case ActionType::Raise:
        minRaise = a.amount;
        maxRaise = state.getPlayer(0).getChips();
        hasRaise = true;
        break;
      case ActionType::AllIn:
        // We'll present all-in as part of bet/raise range,
        // or standalone if there's no bet/raise option.
        if (!hasBet && !hasRaise) {
          menu.push_back({"All-In " + std::to_string(a.amount), a, false});
        }
        maxBet = a.amount;
        maxRaise = a.amount;
        break;
      }
    }

    // Add bet option with custom amount.
    if (hasBet) {
      std::string label = "Bet (min " + std::to_string(minBet) + ", max " +
                          std::to_string(maxBet) + " all-in)";
      Action betAction(ActionType::Bet, minBet, 0);
      menu.push_back({label, betAction, true});
    }

    // Add raise option with custom amount.
    if (hasRaise) {
      std::string label = "Raise (min " + std::to_string(minRaise) + ", max " +
                          std::to_string(maxRaise) + " all-in)";
      Action raiseAction(ActionType::Raise, minRaise, 0);
      menu.push_back({label, raiseAction, true});
    }

    // Display menu.
    for (size_t i = 0; i < menu.size(); ++i) {
      std::cout << "    [" << i << "] " << menu[i].label << "\n";
    }

    // Get choice.
    size_t choice = 0;
    while (true) {
      std::cout << "  > Choose (0-" << menu.size() - 1 << "): ";
      if (std::cin >> choice && choice < menu.size()) {
        break;
      }
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cout << "  Invalid choice. Try again.\n";
    }

    auto selected = menu[choice];

    // If custom amount allowed, ask for it.
    if (selected.allowCustomAmount) {
      int64_t minAmt =
          selected.action.type == ActionType::Bet ? minBet : minRaise;
      int64_t maxAmt =
          selected.action.type == ActionType::Bet ? maxBet : maxRaise;

      int64_t amount = 0;
      while (true) {
        std::cout << "  > Amount (" << minAmt << "-" << maxAmt << "): ";
        if (std::cin >> amount && amount >= minAmt && amount <= maxAmt) {
          break;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  Invalid amount. Try again.\n";
      }

      // If the amount equals all-in, change to AllIn type.
      if (amount == maxAmt) {
        return Action(ActionType::AllIn, amount, 0);
      }
      return Action(selected.action.type, amount, 0);
    }

    return selected.action;
  }

  // ── Bot: passive strategy (check > call > all-in > fold) ──
  Action getBotAction(const std::vector<Action> &legalActions) {
    // Prefer Check.
    for (const auto &a : legalActions) {
      if (a.type == ActionType::Check)
        return a;
    }
    // Prefer Call.
    for (const auto &a : legalActions) {
      if (a.type == ActionType::Call)
        return a;
    }
    // Accept All-In (when call amount equals remaining stack).
    for (const auto &a : legalActions) {
      if (a.type == ActionType::AllIn)
        return a;
    }
    // Last resort: Fold.
    return legalActions.front();
  }
};

// ────────────────────────────────────────────────────────
// Event printer — shows game progress
// ────────────────────────────────────────────────────────
void printEvent(const std::string &event, const GameState &state) {
  if (event == "hand_start") {
    std::cout << "\n--- New Hand ---\n";
    std::cout << "Dealer: "
              << state.getPlayer(state.getDealerPosition()).getName() << "\n";
  } else if (event == "deal_hole_cards") {
    const auto &you = state.getPlayer(0);
    std::cout << "Your cards: " << you.getHoleCards()[0] << " "
              << you.getHoleCards()[1] << "\n";
  } else if (event.starts_with("street_")) {
    std::string street = event.substr(7);
    street[0] = static_cast<char>(std::toupper(street[0]));
    std::cout << "\n--- " << street << " ---\n";
    if (!state.getCommunityCards().empty()) {
      std::cout << "Board: ";
      for (const auto &c : state.getCommunityCards())
        std::cout << c << " ";
      std::cout << "\n";
    }
    std::cout << "Pot: " << state.getPot().getTotal()
              << "  |  You: " << state.getPlayer(0).getChips()
              << "  Bot: " << state.getPlayer(1).getChips() << "\n";
  } else if (event == "action") {
    const auto &a = state.getActionHistory().back();
    if (a.playerId == 1) {
      std::cout << "  Bot: " << Action::actionTypeName(a.type);
      if (a.amount > 0)
        std::cout << " " << a.amount;
      std::cout << "\n";
    }
  } else if (event == "showdown") {
    std::cout << "\n=== Showdown ===\n";
    std::cout << "Board: ";
    for (const auto &c : state.getCommunityCards())
      std::cout << c << " ";
    std::cout << "\n";
    const auto &bot = state.getPlayer(1);
    if (!bot.isFolded()) {
      std::cout << "Bot's cards: " << bot.getHoleCards()[0] << " "
                << bot.getHoleCards()[1] << "\n";
    }
  } else if (event.starts_with("winner_")) {
    std::cout << "Winner: " << event.substr(7) << "\n";
  } else if (event == "pot_awarded") {
    std::cout << "Pot awarded.\n";
  }
}

// ────────────────────────────────────────────────────────
int main() {
  std::cout << "========================================\n";
  std::cout << "   Texas Hold'em  -  You vs. Passive Bot\n";
  std::cout << "========================================\n";
  std::cout << "  Bot strategy: always calls / checks\n\n";

  const int64_t startingChips = 1000;
  const int64_t smallBlind = 5;
  const int64_t bigBlind = 10;

  auto rng = std::make_shared<Mt19937Generator>(); // random seed
  auto actionProvider = std::make_shared<InteractiveActionProvider>();

  PokerEngine engine(actionProvider, rng);
  engine.setEventCallback(printEvent);

  GameState state;
  std::vector<Player> players;
  players.emplace_back(0, "You", startingChips);
  players.emplace_back(1, "Bot", startingChips);
  state.setPlayers(std::move(players));
  state.setSmallBlind(smallBlind);
  state.setBigBlind(bigBlind);
  state.setDealerPosition(0);

  int hand = 0;
  while (true) {
    ++hand;

    // Check for busted player.
    bool gameOver = false;
    for (const auto &p : state.getPlayers()) {
      if (p.getChips() <= 0) {
        std::cout << "\n" << p.getName() << " is out of chips!\n";
        gameOver = true;
      }
    }
    if (gameOver)
      break;

    std::cout << "\n========== Hand " << hand << " ==========\n";
    std::cout << "Stacks  -  You: " << state.getPlayer(0).getChips()
              << "   Bot: " << state.getPlayer(1).getChips() << "\n";

    engine.playHand(state);

    // Print result.
    std::cout << "\nResult  -  You: " << state.getPlayer(0).getChips()
              << "   Bot: " << state.getPlayer(1).getChips() << "\n";

    // Rotate dealer.
    state.setDealerPosition((state.getDealerPosition() + 1) %
                            state.getPlayers().size());

    // Ask to continue.
    std::cout << "\nPress Enter for next hand (q to quit)... ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::string line;
    std::getline(std::cin, line);
    if (!line.empty() && (line[0] == 'q' || line[0] == 'Q')) {
      break;
    }
  }

  std::cout << "\n=== Final Standings ===\n";
  for (const auto &p : state.getPlayers()) {
    std::cout << p.getName() << ": " << p.getChips() << " chips\n";
  }
  std::cout << "Thanks for playing!\n";

  return 0;
}
