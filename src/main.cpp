#include "NetworkResourceAllocation.h"
#include "Heuristics.h"
#include "MetaHeuristics.h"
#include "MathModels.h"

/**
 * @brief Initializes and runs a complete simulation flow for a given algorithm.
 * @details This function serves as a high-level controller. It first calls the
 * `pre_calculation` step to load data and prepare the initial simulation
 * state. It can optionally activate a bottleneck scenario for testing purposes
 * by modifying the state before dispatching it to the appropriate `bootup`
 * function. When enabled, the bottleneck creation is deterministic.
 *
 * @param[in] simulation The type of simulation to run (e.g., "Mathematical").
 * @param[in] algorithm The specific algorithm to execute (e.g., "Minimize_Cost").
 * @param[in] numDevices The number of devices for the simulation.
 * @param[in] numServersEC The number of Edge servers for the simulation.
 * @param[in] numServersCC The number of Cloud servers for the simulation.
 * @param[in] techType The mobile network technology ID (e.g., 5 for 5G).
 * @param[in] bottlenecks If true, activates a test scenario where the first devices from
 * the covered list are modified to create a resource bottleneck on Cloud servers. 
 * Defaults to false.
 * @param[in] heuristic The heuristic for generating an initial solution, used
 * mainly by MetaHeuristic simulations. Defaults to "Random".
 */
void initializeSimulation(std::string simulation, std::string algorithm, int numDevices, int numServersEC, int numServersCC, int techType, bool bottlenecks = false, std::string heuristic = "Random") {
    auto state = NetworkResourceAllocation::pre_calculation(simulation, algorithm, numDevices, numServersEC, numServersCC, techType);

    if (bottlenecks) NetworkResourceAllocation::createBottleneck(state, false);
    
    if (state) {
        if (simulation == "Mathematical") {
            MathModels::bootup(algorithm, *state);
        } else if (simulation == "Heuristic") {
            Heuristics::bootup(algorithm, *state);
        } else if (simulation == "MetaHeuristic") {
            int loopTest = 120;
            double T = 100.0;
            double alpha = 0.95;
            MetaHeuristics::bootup(algorithm, *state, T, alpha, heuristic, loopTest);
        } else {
            std::cerr << "Erro: Tipo de simulação desconhecido." << std::endl;
        }
    } else {
        std::cout << "Falha na fase de pre-calculo. A simulacao nao pode continuar." << std::endl;
    }
    
}

/**
 * @brief The main entry point of the simulation program.
 * @details This function sets up the simulation parameters and runs a batch of
 * simulations. It contains a loop to test the algorithms with an increasing
 * number of devices, allowing for scalability analysis. A global try-catch
 * block is used to handle any exceptions that may occur during the process.
 *
 * @return Returns 0 on successful completion.
 */
int main() {
    int numDevices = 300;
    int numServersEC = 100;
    int numServersCC = 5;
    int techType = 4;
    bool bottlenecks = true;
    
    try {
        while (numDevices <= 500) {
            std::cout << "\n==============================================================\n" ;
            std::cout <<   "******************** INICIANDO SIMULACOES ********************" ;
            std::cout << "\n==============================================================\n" ;

            initializeSimulation("Mathematical", "Minimize_Cost", numDevices, numServersEC, numServersCC, techType, bottlenecks);
            initializeSimulation("MetaHeuristic", "SA", numDevices, numServersEC, numServersCC, techType, bottlenecks);
            initializeSimulation("MetaHeuristic", "SA", numDevices, numServersEC, numServersCC, techType, bottlenecks, "Greedy_DescAsc");
            numDevices += 100;

            std::cout << "\n==============================================================\n" ;
            std::cout <<   "******************* FINALIZANDO SIMULACOES *******************" ;
            std::cout << "\n==============================================================\n" ;
        }

    } catch (const std::exception& e) {
        std::cout << "Falha na simulacao: " << e.what() << std::endl;
    }

    return 0;
}