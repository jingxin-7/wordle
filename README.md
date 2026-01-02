# C++ Wordle Game Simulator and Lexicographical Solver

## Project Overview
This project is a comprehensive C++ implementation of the popular Wordle game. It is split into two primary components: a **Wordle Game Engine** and a **Deterministic Solver**. The design utilizes **Object-Oriented Programming (OOP)**, leveraging inheritance and polymorphism to handle various game modes.

## I. Wordle Game Engine (Inheritance & Polymorphism)
This component implements the core game logic and features three concrete game variants that derive from an abstract base class (`IWordGame`).

### **Key Features**
*   **Exact Feedback Algorithm:** Implements the official Wordle color-coding rules (`G`/`Y`/`B`), correctly handling duplicate letters and priority (Green > Yellow > Black).
*   **Game State Management:** Tracks the current state (PLAYING, WON, LOST, NOT_STARTED), current round, and the number of possible secret words remaining.
*   **Game Modes (Polymorphism):**
    *   **`TrivialWordle`:** Allows any valid 5-letter word to be guessed.
    *   **`ClassicWordle`:** Requires the guessed word to be present in the provided dictionary.
    *   **`HardWordle`:** Enforces the strictest rule: the guess must be consistent with all past feedback.

## II. Deterministic Solver (`MySolver` Class)
This component implements the specific guessing logic required to solve the classic game mode following a strict, fixed policy for reproducible evaluation.

### **Solver Policy**
The solver employs a rigorous filtering strategy after each guess to maintain a set of possible candidate words.
*   **Best Guess Selection:** The core policy dictates that the next guess must be the **lexicographically smallest** word among all words that are currently consistent with all previous feedback.
*   **Evaluation:** Supports both `SINGLE` (step-by-step) and `BATCH` (summary statistics) evaluation modes, calculating success rate and average steps needed for successful games.

---
**Note:** *The code template utilizes advanced C++ features like `std::unique_ptr` and `std::move` for efficient object management.*
