#include "engine/RuleEngine.h"

#include <algorithm>

namespace poker::engine {

int64_t RuleEngine::getCallAmount(const core::GameState& state, size_t playerId) {
    const auto& player = state.getPlayer(playerId);

    // Find the maximum current bet among all players.
    int64_t maxBet = 0;
    for (const auto& p : state.getPlayers()) {
        maxBet = std::max(maxBet, p.getCurrentBet());
    }

    int64_t toCall = maxBet - player.getCurrentBet();
    return std::min(toCall, player.getChips()); // Cap at player's stack
}

int64_t RuleEngine::getMinRaise(const core::GameState& state, size_t playerId) {
    const auto& player = state.getPlayer(playerId);

    int64_t maxBet = 0;
    int64_t prevRaiseSize = state.getBigBlind(); // Default min raise = BB

    for (const auto& p : state.getPlayers()) {
        maxBet = std::max(maxBet, p.getCurrentBet());
    }

    // Look at last raise in action history to determine min raise increment.
    for (auto it = state.getActionHistory().rbegin();
         it != state.getActionHistory().rend(); ++it) {
        if (it->type == core::ActionType::Raise || it->type == core::ActionType::Bet) {
            // The raise size is the amount ABOVE the previous bet.
            prevRaiseSize = std::max(prevRaiseSize, it->amount - player.getCurrentBet());
            break;
        }
    }

    int64_t minRaise = maxBet + prevRaiseSize;
    return std::min(minRaise, player.getCurrentBet() + player.getChips());
}

int64_t RuleEngine::getMaxRaise(const core::GameState& state, size_t playerId) {
    const auto& player = state.getPlayer(playerId);
    return player.getCurrentBet() + player.getChips(); // All-in
}

std::vector<core::Action> RuleEngine::getLegalActions(
    const core::GameState& state, size_t playerId)
{
    std::vector<core::Action> actions;
    const auto& player = state.getPlayer(playerId);

    if (player.isFolded() || player.isAllIn()) {
        return actions; // No actions available.
    }

    // Fold is always legal (except if no bet to face, but folding is still allowed).
    actions.emplace_back(core::ActionType::Fold, 0, playerId);

    int64_t callAmount = getCallAmount(state, playerId);

    if (callAmount == 0) {
        // No bet to face: can check.
        actions.emplace_back(core::ActionType::Check, 0, playerId);

        // Can bet (min = BB, max = stack).
        if (player.getChips() > 0) {
            int64_t minBet = std::min(state.getBigBlind(), player.getChips());
            if (player.getChips() <= minBet) {
                // Only option is all-in.
                actions.emplace_back(core::ActionType::AllIn, player.getChips(), playerId);
            } else {
                actions.emplace_back(core::ActionType::Bet, minBet, playerId);
                if (player.getChips() > minBet) {
                    actions.emplace_back(core::ActionType::AllIn, player.getChips(), playerId);
                }
            }
        }
    } else {
        // There is a bet to face.
        if (callAmount >= player.getChips()) {
            // Calling would put us all-in.
            actions.emplace_back(core::ActionType::AllIn, player.getChips(), playerId);
        } else {
            actions.emplace_back(core::ActionType::Call, callAmount, playerId);

            // Can raise.
            int64_t minRaise = getMinRaise(state, playerId);
            int64_t totalForMinRaise = minRaise - player.getCurrentBet();
            if (totalForMinRaise >= player.getChips()) {
                // Raising would be all-in.
                actions.emplace_back(core::ActionType::AllIn, player.getChips(), playerId);
            } else {
                actions.emplace_back(core::ActionType::Raise, totalForMinRaise, playerId);
                // All-in raise.
                if (player.getChips() > totalForMinRaise) {
                    actions.emplace_back(core::ActionType::AllIn, player.getChips(), playerId);
                }
            }
        }
    }

    return actions;
}

bool RuleEngine::isActionLegal(const core::GameState& state,
                               const core::Action& action) {
    auto legal = getLegalActions(state, action.playerId);
    for (const auto& a : legal) {
        if (a.type == action.type) {
            if (a.type == core::ActionType::Fold || a.type == core::ActionType::Check) {
                return true;
            }
            if (action.type == core::ActionType::Bet || action.type == core::ActionType::Raise) {
                // Amount must be between min and max.
                int64_t minAmt = a.amount;
                int64_t maxAmt = state.getPlayer(action.playerId).getChips();
                return action.amount >= minAmt && action.amount <= maxAmt;
            }
            if (action.type == core::ActionType::Call || action.type == core::ActionType::AllIn) {
                return action.amount == a.amount;
            }
        }
    }
    return false;
}

} // namespace poker::engine
