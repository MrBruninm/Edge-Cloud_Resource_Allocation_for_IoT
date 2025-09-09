<div align="right">
Read this in other languages: <a href="README-pt-br.md">Português (BR)</a> 🇧🇷
</div>

# Network Resource Allocation Simulator

![Language](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)
![Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)

A C++ simulation framework for optimizing resource allocation in a network of devices, edge servers, and cloud servers. This project evaluates different strategies, from mathematical models to meta-heuristics, to minimize operational costs while ensuring service quality.

## Key Features

-   **Data Generation:** Dynamically generates datasets for devices, services, and servers (Edge and Cloud).
-   **Pre-calculation Phase:** Performs network analysis, including device coverage, latency, and response time calculations.
-   **Multiple Optimization Approaches:**
    -   **Mathematical Model:** Implements an Integer Linear Programming (ILP) model using IBM ILOG CPLEX to find the optimal solution.
    -   **Heuristics:** Includes fast allocation algorithms like Random and several Greedy variations.
    -   **Meta-Heuristics:** Uses Simulated Annealing (SA) to find near-optimal solutions in a reasonable time.
-   **Detailed Metrics:** Collects and saves comprehensive metrics for each simulation run, allowing for detailed performance analysis.

## Project Structure

```
.
├── include/              # Header files (.h)
├── src/                  # Source files (.cpp)
├── data/                 # Base data files for generation
├── analysis/             # Analysis files (e.g., spreadsheets with charts)
├── Results/              # Output directory for simulation results (ignored by git)
├── build/                # Build directory (ignored by git)
├── .gitignore            # Git ignore file
└── README.md             # This file
```

## Prerequisites

Before you begin, ensure you have met the following requirements:

* **C++17 Compiler:** A modern C++ compiler (like GCC or Clang) that supports C++17.
* **CMake:** Version 3.10 or higher is recommended for building the project.
* **IBM ILOG CPLEX:** This project depends on the CPLEX optimization libraries. You must have CPLEX installed on your system.
    * Ensure that the CPLEX environment variables (`CPLEX_DIR`, etc.) are set correctly, or that the installer has integrated it into your system path.

## How to Compile and Run

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/MrBruninm/Edge-Cloud_Resource_Allocation_for_IoT.git
    cd Edge-Cloud_Resource_Allocation_for_IoT
    ```

2.  **Configure the project with CMake:**
    ```bash
    cmake -S . -B build
    ```
    *If CMake cannot find CPLEX automatically, you may need to provide its path.*

3.  **Build the project:**
    ```bash
    cmake --build build
    ```

4.  **Run the simulation:**
    The `main.cpp` file is configured to run a default set of simulations. To execute them, run:
    ```bash
    ./build/main_app
    ```

## How It Works

The simulation follows a clear, multi-stage process:

1.  **Data Loading & Generation:** The program first loads or generates the necessary data for devices and servers.
2.  **Pre-calculation:** It then determines which devices are within the coverage of edge servers and pre-calculates essential metrics like connection and processing times for all potential device-server pairings.
3.  **Execution:** The simulation state is passed to one of the selected algorithms:
    * **Mathematical:** Solves the problem to optimality using CPLEX.
    * **Heuristic:** Applies a fast, rule-based method to find a good solution quickly.
    * **Meta-Heuristic:** Starts with a solution from a heuristic and iteratively improves it.
4.  **Results:** After each run, a `Metrics` object is populated, displayed on the console, and saved to a `.txt` file in the `Results/` directory.

## Algorithms Implemented

-   **Mathematical Model:**
    -   `Minimize_Cost`: An ILP model that minimizes total operational costs.
-   **Heuristics:**
    -   `Random`: Assigns devices to random available servers.
    -   `Greedy`: Assigns devices based on sorted lists of devices (by cost) and servers (by response time). Variations include `Greedy_AscAsc`, `Greedy_DescAsc`, etc.
-   **Meta-Heuristic:**
    -   `SA` (Simulated Annealing): A probabilistic method for finding a global optimum.