#include <iostream>
#include <vector>
#include <string>
#include <set> //alphabetical order,unique
#include <map>
#include <algorithm>
#include "wordle_common.h"
#include <iomanip> // Used for formatting output (std::setprecision)

//abstract base class
class ISolver {
public:
    Config config_;//stores the game settings(word lenth, distionary)
    // Pure virtual functions (= 0)
    // Remember to implement in MySolver

    virtual std::string chooseBestGuess() = 0;
    virtual void update(std::string guess, std::string pattern) = 0;
    virtual void reset() = 0;
    //constructor and distructor
    explicit ISolver(const Config& cfg) : config_(cfg) {}
    virtual ~ISolver() = default;

    //Game loop
    int solve(IWordGame& game, const std::string& secret) {
        //resets solver
        reset();
        //loops until game over
        //ask for guess,send to game,get pattern,update list based on pattern
        game.start(secret);
        int guesses_made = 0;

        while (!game.isGameOver()) {
            std::string guess = chooseBestGuess();
            if (guess.empty()) break;

            std::string pattern;
            try {
                pattern = game.makeGuess(guess);
                std::cout << "GUESS " << guess << '\n';
                std::cout << "PATTERN " << pattern << '\n';
            } catch (const std::invalid_argument& e) {
                std::cout << "Error: " << e.what() << '\n';
                continue;
            }

            guesses_made++;
            update(guess, pattern);
        }
        return game.hasWon() ? guesses_made : -1;
    }
};
// run the test
class Evaluator {
public:
//single mode
    static int evaluateSingleGame(IWordGame& game, ISolver& solver, const std::string& secret) {
        int result = solver.solve(game, secret);
        if (result != -1) std::cout << "RESULT WON " << result << '\n';
        else              std::cout << "RESULT LOST -1\n";
        return result;
    }
//batch mode
//print result won or summary stats
    static void evaluateBatch(IWordGame& game, ISolver& solver, const std::vector<std::string>& secrets) {
        int k = (int)secrets.size();
        int success = 0, total_steps = 0;

        for (int i = 0; i < k; ++i) {
            std::cout << "GAME " << (i + 1) << '\n';
            int result = solver.solve(game, secrets[i]);
            if (result != -1) {
                std::cout << "RESULT WON " << result << '\n';
                ++success;
                total_steps += result;
            } else {
                std::cout << "RESULT LOST -1\n";
            }
        }

        double avg = (success > 0) ? (double)total_steps / success : 0.0;
        std::cout << "SUMMARY success=" << success << "/" << k
                  << " avg_steps=" << std::fixed << std::setprecision(2) << avg
                  << '\n';
    }
};

//my code

class MySolver : public ISolver {
    //set automically keeps words sorted alphabetically,prevents duplicates
    //lexicographically smallest
    std::set<std::string> candidates_;

    // Helper function: Logic to check if a word is consistent with the feedback.
    // Simulates: If 'candidate' was the secret, would 'guess' produce 'pattern'?
    std::string generatePattern(const std::string& candidate, const std::string& guess) {
        std::string res(guess.length(), 'B'); // assume all Black
        std::map<char, int> counts;//Keeps track of letter frequency in the candidate
        //map for key value pairs


        // Count character frequencies in the candidate (hypothetical secret)
        for (char c : candidate) {
            counts[c]++;
        }

        // Pass 1: Identify Greens (Correct Letter, Correct Position)
        //higher priority than yellow
        for (size_t i = 0; i < guess.length(); ++i) {
            if (candidate[i] == guess[i]) {
                res[i] = 'G';//mark green
                counts[candidate[i]]--;//use one instance of this letter
            }
        }

        // Pass 2: Identify Yellows (Correct Letter, Wrong Position)
        for (size_t i = 0; i < guess.length(); ++i) {
            if (res[i] != 'G') { // Only check if not already marked Green
                //is it in candidate?
                if (counts[guess[i]] > 0) {
                    res[i] = 'Y';//mark yellow
                    counts[guess[i]]--;//use up this letter
                }
            }
        }
        return res;//return calculated pattern(eg:GGYYB)
    }

public:
    explicit MySolver(const Config& cfg) : ISolver(cfg) {}
//reset function
    void reset() override {
        // Clear the set to remove data from previous games
        candidates_.clear();
        
        // Insert all dictionary from config_ into the set.
        // std::set automatically sorts elements,lexicographically smallest
        for (const auto& word : config_.dict_words) {
            candidates_.insert(word);
        }
    }
//choose best guess
    std::string chooseBestGuess() override {
        // If the set is empty, return empty string (stops game loop)
        if (candidates_.empty()) return "";
        
        // return the first element
        // std::set used, the first element is the lexicographically smallest
        return *candidates_.begin();
    }
//update to filter my list
    void update(std::string guess, std::string pattern) override {
        // Iterate through all current candidates
        for (auto it = candidates_.begin(); it != candidates_.end(); ) {
            // Check,If the word (*it) was the secret, would it produce this 'pattern' for this 'guess'
            if (generatePattern(*it, guess) != pattern) {
                // NO, It cannot be the secret word, Remove 
                // erase(it) removes the element and returns an iterator to the next element.
                it = candidates_.erase(it);
            } else {
                // YES, It might be the secret, keep and move to next
                ++it;
            }
        }
    }
};

//reads input(Mode, Config, Dictionary, Secrets).
//Creates the ClassicWordle game object.
//creates MySolver object
//runs evaluator
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string mode;
    std::cin >> mode;

    Config config;
    std::cin >> config.L >> config.T;

    int dict_count = 0;
    std::cin >> dict_count;
    config.dict_words.clear();
    config.dict_words.reserve(dict_count);

    for (int i = 0; i < dict_count; ++i) {
        std::string w;
        if (!(std::cin >> w)) {
            //cerr for standard error output
            std::cerr << "Error: premature end while reading dictionary.\n";
            return 1;
        }
        if ((int)w.size() == config.L) config.dict_words.push_back(w);
    }
    ClassicWordle game(config);
    MySolver solver(config);
    if (mode == "SINGLE") {
        std::string secret_header, secret_word;
        if (!(std::cin >> secret_header >> secret_word) || secret_header != "SECRET") {
            std::cerr << "Error: invalid SINGLE input (expect: SECRET <word>).\n";
            return 1;
        }
        if ((int)secret_word.size() != config.L) {
            std::cerr << "Error: SECRET length != word_length.\n";
            return 1;
        }
        Evaluator::evaluateSingleGame(game, solver, secret_word);

    } else if (mode == "BATCH") {
        int k = 0;
        if (!(std::cin >> k) || k < 1) {
            std::cerr << "Error: invalid BATCH k.\n";
            return 1;
        }
        std::vector<std::string> secrets(k);
        for (int i = 0; i < k; ++i) {
            std::string secret_header;
            if (!(std::cin >> secret_header >> secrets[i]) || secret_header != "SECRET") {
                std::cerr << "Error: invalid BATCH secret line at GAME " << (i+1) << ".\n";
                return 1;
            }
            if ((int)secrets[i].size() != config.L) {
                std::cerr << "Error: SECRET length != word_length at GAME " << (i+1) << ".\n";
                return 1;
            }
        }
        Evaluator::evaluateBatch(game, solver, secrets);

    } else {
        std::cerr << "Error: unknown mode. Use SINGLE or BATCH.\n";
        return 1;
    }

    return 0;
}

