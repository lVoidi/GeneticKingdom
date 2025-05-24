#include "framework.h"
#include "GeneticAlgorithm.h"
#include "Map.h" // For map dimensions, pathfinding, etc.
#include <random>
#include <algorithm> // For std::sort, std::transform, etc.
#include <numeric>   // For std::accumulate
#include <iostream>  // For debugging, remove later
#include <sstream>   // For logging
#include <iomanip>   // For logging

// CELL_SIZE is defined as a macro in Map.h
// extern const int CELL_SIZE;

GeneticAlgorithm::GeneticAlgorithm(int popSize, float mutRate, float crossRate,
                                   const std::pair<int, int>& entry, 
                                   const std::pair<int, int>& bridge, 
                                   const Map* gameMap)
    : populationSize(popSize), mutationRate(mutRate), crossoverRate(crossRate),
      enemyEntryPoint(entry), bridgeLocation(bridge), currentMap(gameMap) {
    
    if (currentMap) {
        // Attempt to get an initial path from the map.
        // This assumes Map class has a method like GetPath(start, end).
        // The actual start/end for pathfinding might be the center of the cells.
        initialEnemyPath = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
        if (initialEnemyPath.empty()) {
            // std::cerr << "Warning: Initial pathfinding failed in GeneticAlgorithm constructor." << std::endl;
            // Fallback or error handling needed here. Maybe a default straight path or mark GA as non-functional.
        }
    } else {
        // std::cerr << "Error: Map object is null in GeneticAlgorithm constructor." << std::endl;
        // Handle this critical error, perhaps by throwing an exception or setting an error state.
    }
}

GeneticAlgorithm::~GeneticAlgorithm() {
}

void GeneticAlgorithm::InitializePopulation() {
    population.clear();
    for (int i = 0; i < populationSize; ++i) {
        // For now, create a mix of enemy types, or a default one.
        // Attributes will be base, or slightly randomized if CreateRandomEnemy is implemented.
        EnemyType type = static_cast<EnemyType>(i % 4); // Cycle through Ogre, Dark Elf, Harpy, Mercenary
        
        float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2.0f); // col for X
        float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2.0f);  // row for Y

        Enemy newEnemy(type, startX, startY, initialEnemyPath);
        population.push_back(newEnemy);
    }
}

// Fitness evaluation is now handled by the game loop, calling enemy.CalculateFitness() directly.
// This method is no longer needed here, or can be re-purposed if GA needs to trigger it.
/*
void GeneticAlgorithm::EvaluateFitness(float timeSurvivedOverall, bool anyEnemyReachedBridge) {
    if (!currentMap) return;

    float mapPixelWidth = static_cast<float>(currentMap->GetGridWidth() * CELL_SIZE);
    float mapPixelHeight = static_cast<float>(currentMap->GetGridHeight() * CELL_SIZE);

    for (Enemy& enemy : population) {
        bool thisEnemyReachedBridge = false; 
        if (anyEnemyReachedBridge) { 
            float distToBridgeSq = (enemy.GetX() - (bridgeLocation.first * CELL_SIZE + CELL_SIZE / 2.0f)) * 
                                   (enemy.GetX() - (bridgeLocation.first * CELL_SIZE + CELL_SIZE / 2.0f)) +
                                   (enemy.GetY() - (bridgeLocation.second * CELL_SIZE + CELL_SIZE / 2.0f)) * 
                                   (enemy.GetY() - (bridgeLocation.second * CELL_SIZE + CELL_SIZE / 2.0f));
            if (distToBridgeSq < (CELL_SIZE * CELL_SIZE / 4.0f)) { 
                 thisEnemyReachedBridge = true;
            }
        }
        enemy.CalculateFitness(bridgeLocation, mapPixelWidth, mapPixelHeight, enemy.GetTimeAlive(), thisEnemyReachedBridge);
    }
}
*/

Enemy GeneticAlgorithm::SelectParentRoulette() const {
    if (population.empty()) {
        // Should not happen if initialized. Return a default or throw.
        // std::cerr << "Error: Attempting to select parent from empty population." << std::endl;
        float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2.0f);
        float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2.0f);
        return Enemy(EnemyType::OGRE, startX, startY, initialEnemyPath); 
    }

    double totalFitness = 0.0;
    for (const auto& enemy : population) {
        totalFitness += enemy.GetFitness();
    }

    if (totalFitness == 0) {
        // All enemies have 0 fitness, pick one at random
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, population.size() - 1);
        return population[dis(gen)];
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, totalFitness);
    double randomPoint = dis(gen);

    double currentSum = 0.0;
    for (const auto& enemy : population) {
        currentSum += enemy.GetFitness();
        if (currentSum >= randomPoint) {
            return enemy;
        }
    }
    return population.back(); // Should be unreachable if totalFitness > 0
}

void GeneticAlgorithm::SelectParents() {
    parents.clear();
    for (int i = 0; i < populationSize; ++i) { // Selecting populationSize parents for populationSize offspring
        parents.push_back(SelectParentRoulette());
    }
}

// Single-point crossover for health and speed genes
std::pair<Enemy, Enemy> GeneticAlgorithm::PerformCrossover(const Enemy& parent1, const Enemy& parent2) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    Enemy offspring1 = parent1; // Start with a copy of parent1
    Enemy offspring2 = parent2; // Start with a copy of parent2

    if (dis(gen) < crossoverRate) {
        // Perform crossover for maxHealth
        int tempHealth = offspring1.GetMaxHealth();
        offspring1.SetMaxHealth(offspring2.GetMaxHealth());
        offspring2.SetMaxHealth(tempHealth);

        // Perform crossover for speed
        float tempSpeed = offspring1.GetSpeed();
        offspring1.SetSpeed(offspring2.GetSpeed());
        offspring2.SetSpeed(tempSpeed);
        
        // Note: Resistances are not part of crossover/mutation in this basic version
        // but could be added if they are part of the genetic makeup.
    }
    return {offspring1, offspring2};
}

// This method is now effectively replaced by calling enemy.ResetForNewWave()
/*
void GeneticAlgorithm::EnsurePathForOffspring(Enemy& offspring) {
    offspring.SetPath(initialEnemyPath);
    float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2.0f); // col for X
    float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2.0f);  // row for Y
}
*/

void GeneticAlgorithm::CrossoverAndMutate() {
    std::vector<Enemy> newPopulation;
    std::random_device rd;
    std::mt19937 gen(rd());

    float startPixelX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2.0f); // col for X
    float startPixelY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2.0f);  // row for Y

    for (size_t i = 0; i < parents.size() / 2; ++i) {
        const Enemy& parent1 = parents[i * 2];
        const Enemy& parent2 = parents[i * 2 + 1];

        auto offspringPair = PerformCrossover(parent1, parent2);
        
        offspringPair.first.Mutate(mutationRate);
        offspringPair.first.ResetForNewWave(startPixelX, startPixelY, initialEnemyPath);
        newPopulation.push_back(offspringPair.first);

        if (newPopulation.size() < populationSize) {
            offspringPair.second.Mutate(mutationRate);
            offspringPair.second.ResetForNewWave(startPixelX, startPixelY, initialEnemyPath);
            newPopulation.push_back(offspringPair.second);
        }
    }
    while (newPopulation.size() < populationSize && !parents.empty()) {
         Enemy singleOffspring = parents.back(); 
         parents.pop_back(); // Ensure we don't reuse the same parent if loop continues
         singleOffspring.Mutate(mutationRate);
         singleOffspring.ResetForNewWave(startPixelX, startPixelY, initialEnemyPath);
         newPopulation.push_back(singleOffspring);
         if (parents.empty() && newPopulation.size() < populationSize && !population.empty()){
             // If we run out of selected parents but still need to fill, grab from original population (less ideal)
             Enemy filler = SelectParentRoulette(); // Select another parent from original population
             filler.Mutate(mutationRate);
             filler.ResetForNewWave(startPixelX, startPixelY, initialEnemyPath);
             newPopulation.push_back(filler);
         }
    }
    // If the population is still not full (e.g. parents was empty or small), fill with new random enemies
    while (newPopulation.size() < populationSize) {
        Enemy randomNewEnemy = CreateRandomEnemy(); // Assumes CreateRandomEnemy sets path and start internally or we reset it
        randomNewEnemy.ResetForNewWave(startPixelX, startPixelY, initialEnemyPath); // Ensure it's reset
        newPopulation.push_back(randomNewEnemy);
    }


    population = newPopulation;
}

std::vector<Enemy> GeneticAlgorithm::GenerateNewGeneration() {
    std::wstringstream wss_gnn_entry;
    wss_gnn_entry << L"GA::GenerateNewGeneration - Entry. Current pop size: " << population.size();
    OutputDebugStringW((wss_gnn_entry.str() + L"\n").c_str());

    // Step 1: Evaluate fitness (Done externally)
    // Step 2: Select parents based on fitness
    SelectParents();
    std::wstringstream wss_gnn_parents;
    wss_gnn_parents << L"GA::GenerateNewGeneration - Parents selected: " << parents.size();
    OutputDebugStringW((wss_gnn_parents.str() + L"\n").c_str());

    // Step 3: Create new generation through crossover and mutation
    CrossoverAndMutate(); // This replaces internal 'population'
    std::wstringstream wss_gnn_mutate;
    wss_gnn_mutate << L"GA::GenerateNewGeneration - CrossoverAndMutate done. New internal pop size: " << population.size();
    OutputDebugStringW((wss_gnn_mutate.str() + L"\n").c_str());

    return population; 
}

const std::vector<Enemy>& GeneticAlgorithm::GetCurrentPopulation() const {
    return population;
}

void GeneticAlgorithm::SetCurrentPopulation(const std::vector<Enemy>& newPopulationFromGame) {
    std::wstringstream wss_scp;
    wss_scp << L"GA::SetCurrentPopulation - Received population from game with " << newPopulationFromGame.size() << L" enemies. Internal pop size before: " << population.size() << L".\n";
    for(size_t i = 0; i < newPopulationFromGame.size(); ++i) {
        const auto& en = newPopulationFromGame[i];
        wss_scp << L"  Inbound Enemy [" << i << L"] ID: " << std::hex << &en 
                << L", Health: " << en.GetHealth()
                << L"/" << en.GetMaxHealth()
                << L", IsActive: " << (en.IsActive() ? L"Yes" : L"No") 
                << L", X: " << en.GetX() << L", Y: " << en.GetY() << L"\n";
    }
    
    population = newPopulationFromGame; // This is a copy
    wss_scp << L"GA::SetCurrentPopulation - Internal population updated. Size now: " << population.size();
    OutputDebugStringW((wss_scp.str() + L"\n").c_str());
}

void GeneticAlgorithm::SetMapDetails(const Map* map) {
    currentMap = map;
    if (currentMap) {
        initialEnemyPath = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
         if (initialEnemyPath.empty()) {
            // std::cerr << "Warning: Pathfinding failed in SetMapDetails." << std::endl;
        }
    } else {
        // std::cerr << "Error: Map object is null in SetMapDetails." << std::endl;
    }
}

// Helper to generate a random enemy for initial population
// This is a basic version. You might want more sophisticated randomization.
Enemy GeneticAlgorithm::CreateRandomEnemy() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> typeDist(0, 3); // For 4 enemy types
    EnemyType randomType = static_cast<EnemyType>(typeDist(gen));

    float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2.0f); // col for X
    float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2.0f);  // row for Y

    Enemy enemy(randomType, startX, startY, initialEnemyPath);
    // Optionally, slightly randomize base stats like health/speed here for initial diversity
    // enemy.Mutate(0.1f); // Example: apply a small mutation for initial variety
    return enemy;
} 