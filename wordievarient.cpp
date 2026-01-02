#include <string>
#include <vector>
#include <stdexcept>
#include <set>
#include <fstream>
#include <map>
#include <iostream>
#include <memory>
#include <algorithm> // Required for std::max



//holds one turn of the libary, remember what u guess and what pattern u got
struct HistoryEntry {
    std::string guess;
    std::string pattern;
};
//state machine, keep track of where we are 
enum class GameState {
    NOT_STARTED,//game object created but secret not set
    PLAYING,//currently guessing
    WON,//guessed right
    LOST//no more tries
};

struct Config {
    int L;//word length
    int T;//max tries
    int S;//dictionary size
    std::vector<std::string> dict_words;//list of allowed words
};

class IWordGame {
public:
//static so  can use it without creating a game obj
// calculates the g/b/y string 
    static std::string feedback(const std::string& secret, const std::string& guess);

protected:
/// Variables available to children classes (Trivial/Classic/Hard)
    Config config_;
    std::string secret_;
    std::vector<HistoryEntry> history_;
    GameState state_;
    int current_round_;
    bool is_universe_;
    // The set of all allowed words. Using a 'set' makes searching very fast.
    std::set<std::string> dictionary_;
    //check if game ends after a guess
    void updateGameState(const std::string& last_guess) {
        if (last_guess == secret_) {
            state_ = GameState::WON;
        } else if (current_round_ >= config_.T) {
            state_ = GameState::LOST;
        }
    }

private:
 // Helper function ,recursion
 //pos tells which letter we are currently trying to decide.
    void generateAllWords(std::string& current, int pos);

protected:
// Fills the 'dictionary_' set
    void buildDictionary() {
        dictionary_.clear();
        is_universe_ = (config_.S == 0);// If S is 0, we use ALL strings

        if (is_universe_) {
            // Create a starting string like "aaaa"
            std::string temp(config_.L, 'a');
            // Recursively generate every combination 26^L words
            generateAllWords(temp, 0);
        } else {//run when S greater 0
            // Normal mode: Copy words from input to our set, filtering by length
            for (const auto& word : config_.dict_words) {
                if ((int)word.length() == config_.L) {
                    dictionary_.insert(word);
                }
            }
        }
    }

public:
//constructer that sets up the game based on config
    explicit IWordGame(const Config& cfg)
        : config_(cfg),
          state_(GameState::NOT_STARTED),
          current_round_(0)
    {
        if (config_.L <= 0 || config_.T <= 0) {
            throw std::invalid_argument("Config L and T must be positive");
        }
        buildDictionary();//prepare the word list immediately
    }
    // Virtual Destructor: Essential when using inheritance so memory is cleaned up correctly

    virtual ~IWordGame() = default;
    const Config& cfg() const { return config_; }

    // Pure virtuals to be implemented by subclasses(trival/class/hard)
    virtual bool isValidWord(const std::string& w) const = 0;
    virtual bool isValidGuess(const std::string& guess) const = 0;
    
    // Calculates how many words in the dictionary are still possible answers
    int getRemainingWords() const; 
    // Starts a new round
    void start(const std::string& secret) {
        secret_ = secret;
        history_.clear();
        current_round_ = 0;
        state_ = GameState::PLAYING;
    }
    //main gameplay function

    std::string makeGuess(const std::string& guess) {
        //check if we can play
        if (state_ != GameState::PLAYING) {
            throw std::invalid_argument("Invalid gamestate");
        }
        //Ask the specific child class: "Is this guess allowed?
        if (!isValidGuess(guess)) {
            throw std::invalid_argument("Invalid guess: " + guess);
        }
        //calculate colours

        std::string pattern = feedback(secret_, guess);
        //save history
        history_.push_back({guess, pattern});

        current_round_++;
        //check win or loss
        updateGameState(guess);
        return pattern;
    }
    // --- Game state queries ---

    // Get current game state

    GameState getState() const { return state_; }
    // Check if game is over (won or lost)
    bool isGameOver() const { return state_ == GameState::WON || state_ == GameState::LOST; }
    // Check if player has won
    bool hasWon() const { return state_ == GameState::WON; }
    // Check if player has lost
    bool hasLost() const { return state_ == GameState::LOST; }
    // Get current round number (1-indexed for display)
    int getCurrentRound() const { return current_round_; }
    // Get remaining tries
    int getRemainingTries() const {
        return std::max(0, config_.T - current_round_);
    }

    void printStatus() const {
        if (state_ == GameState::NOT_STARTED) { std::cout << "NOT_STARTED "; }
        if (state_ == GameState::PLAYING) { std::cout << "PLAYING "; }
        if (state_ == GameState::WON) { std::cout << "WON "; }
        if (state_ == GameState::LOST) { std::cout << "LOST "; }
        std::cout << current_round_ << " " << getRemainingWords() << std::endl;
    }
    //get game history

    const std::vector<HistoryEntry>& getHistory() const { return history_; }
};

// --- [Implementations of Shared/Base Methods] ---

void IWordGame::generateAllWords(std::string& current, int pos) {
    //TODO:implementation
    // Base case: if we filled the string, insert it
    if (pos == config_.L) {
        dictionary_.insert(current);
        return;
    }
    // Recursive step: try 'a' through 'z' at current position
    for (char c = 'a'; c <= 'z'; ++c) {
        current[pos] = c;
        generateAllWords(current, pos + 1);
    }
}

// Counts how many words in the dictionary are still possible secrets
int IWordGame::getRemainingWords() const {
    int count = 0;
    // Check every word in the dictionary
    for (const auto& candidate : dictionary_) {
        bool possible = true;
        // Does this candidate match the feedback history?
        for (const auto& entry : history_) {
            // If 'candidate' was the secret, would 'entry.guess' produce 'entry.pattern'?
            if (feedback(candidate, entry.guess) != entry.pattern) {
                possible = false;
                break;
            }
        }
        if (possible) {
            count++;
        }
    }
    return count;
}

std::string IWordGame::feedback(const std::string& secret, const std::string& guess) {
    std::string result(secret.length(), 'B');
    std::map<char, int> secret_counts;

    // Count frequencies in secret
    for (char c : secret) {
        secret_counts[c]++;
    }

    // Pass 1: Identify Greens (Exact matches)
    for (size_t i = 0; i < secret.length(); ++i) {
        if (guess[i] == secret[i]) {
            result[i] = 'G';
            secret_counts[guess[i]]--;
        }
    }

    // Pass 2: Identify Yellows (Wrong position) vs Blacks
    for (size_t i = 0; i < secret.length(); ++i) {
        if (result[i] != 'G') { // Skip already marked greens
            if (secret_counts[guess[i]] > 0) {
                result[i] = 'Y';
                secret_counts[guess[i]]--;
            }
            // Else remains 'B' (Black)
        }
    }
    return result;
}

// --- [Concrete Class Implementations] ---

class TrivialWordle : public IWordGame {
public:
    using IWordGame::IWordGame; // Inherit constructor by parent
    
    //Only requirement: Length must be L, characters must be a-z.
    // It does not have to be in the dictionary.
    bool isValidWord(const std::string& w) const override {
        //check length and char
        if ((int)w.length() != config_.L) return false;
        for (char c : w) {
            if (c < 'a' || c > 'z') return false;
        }
        return true;
    }

    // Trivial mode allows guessing any valid string, even if not in dictionary
    bool isValidGuess(const std::string& guess) const override {
        return isValidWord(guess);
    }
};

class ClassicWordle : public IWordGame {
public:
    using IWordGame::IWordGame;

    // Must be found in dictionary
    bool isValidWord(const std::string& w) const override {
        //Return true if the word was found
        return dictionary_.find(w) != dictionary_.end();
    }

    bool isValidGuess(const std::string& guess) const override {
        return isValidWord(guess);
    }
};

class HardWordle : public IWordGame {
public:
    using IWordGame::IWordGame;
    //must be in dictionary

    bool isValidWord(const std::string& w) const override {
        //must be real word
        return dictionary_.find(w) != dictionary_.end();
    }

    bool isValidGuess(const std::string& guess) const override {
        // Must be in dictionary AND consistent with history
        // We call our own isValidWord function to check this first.
        // If it fails, we immediately return false.
        if (!isValidWord(guess)) return false;
        // If we pretend the current 'guess' is the secret
        // Compare it against every previous guess 
        // Does 'guess' generate the SAME colors for the old guesses that the real secret 
        for (const auto& entry : history_) {
            // This is the simulation step. We pretend the new 'guess' IS the secret word.
            // Then we ask: what feedback would this fake secret ('guess') give for our
            // OLD guess ('entry.guess')?
            std::string projected_feedback = feedback(guess, entry.guess);
            // Now we compare. If the feedback we just simulated ('projected_feedback')
            // is NOT THE SAME as the feedback we ACTUALLY got on that past turn ('entry.pattern'),
            // it means this new guess is illogical and contradicts the clues.
            if (projected_feedback != entry.pattern) {
                return false;//inconsistent 
            }
        }
        // If the guess passed the dictionary check AND the consistency check
        // for every single past turn, it is a valid Hard mode guess. Return true.
        return true;
    }
};
