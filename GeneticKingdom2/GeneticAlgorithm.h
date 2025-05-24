#pragma once

#include "Enemy.h"
#include <vector>
#include <memory> // For std::unique_ptr if managing enemies with pointers

class Map; // Forward declaration for map-related information if needed for GA

class GeneticAlgorithm {
public:
    GeneticAlgorithm(int populationSize, float mutationRate, float crossoverRate, 
                     const std::pair<int, int>& entryPoint, 
                     const std::pair<int, int>& bridgeLocation, 
                     const Map* gameMap); // Pass necessary game info
    ~GeneticAlgorithm();

    void InitializePopulation();
    void EvaluateFitness(float timeSurvivedByWave, bool waveReachedBridge); // Simplified parameters for now
    void SelectParents();
    void CrossoverAndMutate();
    std::vector<Enemy> GenerateNewGeneration(); // Returns the new wave of enemies

    const std::vector<Enemy>& GetCurrentPopulation() const;
    void SetCurrentPopulation(const std::vector<Enemy>& population);

    void SetMapDetails(const Map* map); // If map details change or needed later

private:
    std::vector<Enemy> population;
    std::vector<Enemy> parents;
    
    int populationSize;
    float mutationRate;
    float crossoverRate;

    std::pair<int, int> enemyEntryPoint;    // Grid coordinates for enemy spawn
    std::pair<int, int> bridgeLocation;     // Grid coordinates for enemy goal
    const Map* currentMap;                  // Pointer to the map for pathfinding, dimensions etc.

    Enemy SelectParentRoulette() const;
    std::pair<Enemy, Enemy> PerformCrossover(const Enemy& parent1, const Enemy& parent2) const;
    // Initial path for enemies, might be set once or recalculated
    std::vector<std::pair<int, int>> initialEnemyPath; 

    // Helper to generate a random enemy for initial population (if attributes are randomized initially)
    Enemy CreateRandomEnemy() const;
    
    // Helper to ensure exactly 3 of each enemy type
    void RebalanceEnemyTypes(std::vector<Enemy>& enemies);
}; 