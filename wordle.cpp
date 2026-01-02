#include <iostream>
#include <string>
#include <vector>
#include <utility>//for std::pair(store guess and pattern)
#include <algorithm>

using namespace std;
//judge
string generate_feedback(const string& secret, const string& guess) {
    const int L = 5;// length of words fixed at 5
    string pattern(L, ' ');// placeholder 


    //count letters in secret, for 0-26
    //know how many 'a's etc are in secret
    vector<int> secret_counts(26, 0);
    for (char c : secret) {
        //gives the correct index (0 for 'a', 1 for 'b', etc.)
        secret_counts[c - 'a']++;//'a' is ascii 97,so 'a'-'a' =0
    }
    //green first
    for (int i = 0; i < L; ++i) {
        if (guess[i] == secret[i]) {
            pattern[i] = 'G';
            secret_counts[guess[i] - 'a']--;//// Use up one instance of this letter
        }
    }
    //black and yellow
    for (int i = 0; i < L; ++i) {
        if (pattern[i] != 'G') {
            int char_index = guess[i] - 'a'; 

            //check if letter is in secret
            if (secret_counts[char_index] > 0) {
                pattern[i] = 'Y';//yellow if yes
                secret_counts[char_index]--;
            } else {
                pattern[i] = 'B';//black if no
    
            }
        }
    }
    
    return pattern;//return the string which is like GBYGG
}

void solve_feedback() {
    string secret, guess;
    if (!(cin >> secret >> guess)) return;
    
    cout << generate_feedback(secret, guess) << endl;
}
//the detective
void solve_filter() {
    int N;//6
    if (!(cin >> N)) return;//read dic size
    
    vector<string> dictionary(N);//dic[6]
    for (int i = 0; i < N; ++i) { //reads all the dic words
        if (!(cin >> dictionary[i])) return;
    }
    
    int Q;//between 0 to 6, max guess is 6
    if (!(cin >> Q)) return;//read number of past guesses

    // Store past rounds. 'pair' holds the guess word and the resulting pattern.
    //vector contains the entire history of guesses and results, for the consistency check in the main part of the filter logic.

    vector<pair<string, string>> past_rounds(Q);
    for (int i = 0; i < Q; ++i) {
        if (!(cin >> past_rounds[i].first >> past_rounds[i].second)) return;
    }
    
    int consistent_count = 0;
    //Loop through every word in the dictionary ,find Candidate Secrets
    for (const string& candidate_secret : dictionary) {
        bool is_consistent = true;
        // Check this candidate against ALL past history
        for (const auto& round : past_rounds) {
            const string& guess = round.first;
            const string& required_pattern = round.second;

        
            //this line to match probably correct word,using the generate feedback fuc above
            string actual_pattern = generate_feedback(candidate_secret, guess);
            // If the pattern we generated doesn't match what actually happened,
            // then 'candidate_secret' CANNOT be the answer.
            if (actual_pattern != required_pattern) {
                is_consistent = false;
                break;// Stop checking this word, it's already failed
            }
        }
        // If it passed all historical checks, count it.
        if (is_consistent) {
            consistent_count++;
        }
    }
    
    cout << consistent_count << endl;
}

int main() {
    //speed up optimization
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    
    string mode;
    if (!(cin >> mode)) return 0;//read feedback and filter
    
    if (mode == "FEEDBACK") {
        solve_feedback();
    } else if (mode == "FILTER") {
        solve_filter();
    }
    
    return 0;
}