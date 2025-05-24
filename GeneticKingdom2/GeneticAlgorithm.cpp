#include "framework.h"
#include "GeneticAlgorithm.h"
#include "Map.h" // For map dimensions, pathfinding, etc.
#include <random>
#include <algorithm> // For std::sort, std::transform, etc.
#include <numeric>   // For std::accumulate
#include <iostream>  // For debugging, remove later
#include <sstream>   // For logging
#include <iomanip>   // For logging

// Helper functions to avoid conflicts with Windows macros
double getMaxFromDouble(double a, double b) {
    return a > b ? a : b;
}

double getMinFromDouble(double a, double b) {
    return a < b ? a : b;
}

// Constructor
GeneticAlgorithm::GeneticAlgorithm(int populationSize, float mutationRate, float crossoverRate,
                                   const std::pair<int, int>& entryPoint,
                                   const std::pair<int, int>& bridgeLocation,
                                   const Map* gameMap)
    : populationSize(populationSize), mutationRate(mutationRate), crossoverRate(crossoverRate),
      enemyEntryPoint(entryPoint), bridgeLocation(bridgeLocation), currentMap(gameMap) {
    
    // Calculate initial path from entry to bridge
    if (currentMap) {
        initialEnemyPath = currentMap->GetPath(entryPoint, bridgeLocation);
    }
    
    OutputDebugStringW(L"GeneticAlgorithm initialized\n");
}

// Destructor
GeneticAlgorithm::~GeneticAlgorithm() {
    OutputDebugStringW(L"GeneticAlgorithm destroyed\n");
}

// Initialize population with 3 enemies of each type (12 total)
void GeneticAlgorithm::InitializePopulation() {
    population.clear();
    
    std::wstringstream wss;
    wss << L"GeneticAlgorithm::InitializePopulation - Creating " << populationSize << L" enemies\n";
    OutputDebugStringW(wss.str().c_str());
    
    // Create exactly 3 of each enemy type
    EnemyType types[] = { EnemyType::OGRE, EnemyType::DARK_ELF, EnemyType::HARPY, EnemyType::MERCENARY };
    
    for (EnemyType type : types) {
        for (int i = 0; i < 3; ++i) {
            // Convert grid coordinates to pixel coordinates
            float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
            float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
            
            Enemy enemy(type, startX, startY, initialEnemyPath);
            
            // Apply heavy mutation for initial diversity
            enemy.Mutate(1.0f); // 100% mutation rate for initial population
            
            // Ensure each enemy has unique path jitter for movement variety
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> jitterDist(0.2f, 0.5f);
            enemy.SetPathJitter(jitterDist(gen));
            
            // Set spawn delay to space out enemies (each enemy spawns 1 second after the previous)
            float spawnDelay = static_cast<float>(population.size()) * 1.0f;
            enemy.SetSpawnDelay(spawnDelay);
            
            population.push_back(enemy);
        }
    }
    
    wss.str(L"");
    wss << L"GeneticAlgorithm::InitializePopulation - Created " << population.size() << L" enemies\n";
    OutputDebugStringW(wss.str().c_str());
}

// Evaluate fitness for all enemies in the population
void GeneticAlgorithm::EvaluateFitness(float timeSurvivedByWave, bool waveReachedBridge) {
    std::wstringstream wss;
    wss << L"GeneticAlgorithm::EvaluateFitness - Evaluating " << population.size() << L" enemies\n";
    OutputDebugStringW(wss.str().c_str());
    
    for (Enemy& enemy : population) {
        enemy.CalculateFitness(bridgeLocation,
                              currentMap ? currentMap->GetMapPixelWidth() : 800.0f,
                              currentMap ? currentMap->GetMapPixelHeight() : 600.0f,
                              enemy.GetTimeAlive(),
                              enemy.HasReachedBridge());
    }
    
    // Sort population by fitness (highest first)
    std::sort(population.begin(), population.end(),
              [](const Enemy& a, const Enemy& b) {
                  return a.GetFitness() > b.GetFitness();
              });
              
    // Log best and worst fitness
    if (!population.empty()) {
        wss.str(L"");
        wss << L"Best fitness: " << population[0].GetFitness() 
            << L", Worst fitness: " << population.back().GetFitness() << L"\n";
        OutputDebugStringW(wss.str().c_str());
    }
}

// Select parents for reproduction using tournament selection
void GeneticAlgorithm::SelectParents() {
    parents.clear();
    
    // Elitism: Keep top 10% directly
    int eliteCount = static_cast<int>(population.size() * 0.1f);
    if (eliteCount < 1) eliteCount = 1;
    
    for (int i = 0; i < eliteCount && i < static_cast<int>(population.size()); ++i) {
        parents.push_back(population[i]);
    }
    
    // Fill the rest with tournament selection
    int parentsNeeded = populationSize - eliteCount;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < parentsNeeded; ++i) {
        parents.push_back(SelectParentRoulette());
    }
    
    std::wstringstream wss;
    wss << L"GeneticAlgorithm::SelectParents - Selected " << static_cast<int>(parents.size()) << L" parents\n";
    OutputDebugStringW(wss.str().c_str());
}

// Perform crossover and mutation to create new generation
void GeneticAlgorithm::CrossoverAndMutate() {
    std::vector<Enemy> newPopulation;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    std::uniform_int_distribution<int> parentDist(0, static_cast<int>(parents.size()) - 1);
    
    // Keep track of enemy types to maintain 3 of each
    std::vector<std::vector<Enemy>> enemiesByType(4); // 4 enemy types
    
    while (newPopulation.size() < populationSize) {
        if (dis(gen) < crossoverRate && parents.size() >= 2) {
            // Select two random parents
            int parent1Idx = parentDist(gen);
            int parent2Idx = parentDist(gen);
            while (parent2Idx == parent1Idx && parents.size() > 1) {
                parent2Idx = parentDist(gen);
            }
            
            auto offspring = PerformCrossover(parents[parent1Idx], parents[parent2Idx]);
            
            // Apply mutation
            offspring.first.Mutate(mutationRate);
            offspring.second.Mutate(mutationRate);
            
            newPopulation.push_back(offspring.first);
            if (newPopulation.size() < populationSize) {
                newPopulation.push_back(offspring.second);
            }
        } else {
            // Direct copy with mutation
            Enemy newEnemy = parents[parentDist(gen)];
            newEnemy.Mutate(mutationRate);
            newPopulation.push_back(newEnemy);
        }
    }
    
    // Ensure we have exactly 3 of each type
    RebalanceEnemyTypes(newPopulation);
    
    population = newPopulation;
    
    std::wstringstream wss;
    wss << L"GeneticAlgorithm::CrossoverAndMutate - Created new generation with " 
        << population.size() << L" enemies\n";
    OutputDebugStringW(wss.str().c_str());
}


// Generate new generation of enemies
std::vector<Enemy> GeneticAlgorithm::GenerateNewGeneration() {
    std::vector<Enemy> newGeneration;
    
    // Only use enemies that survived (reached the bridge) or performed well for breeding
    std::vector<Enemy> survivors;
    for (const Enemy& enemy : population) {
        // Include enemies that either reached the bridge or have high fitness
        if (enemy.HasReachedBridge() || enemy.GetFitness() > 50.0) {
            survivors.push_back(enemy);
        }
    }
    
    // If no survivors, use the best performing enemies based on fitness
    if (survivors.empty()) {
        // Sort by fitness and take the top 50%
        std::vector<Enemy> sortedPopulation = population;
        std::sort(sortedPopulation.begin(), sortedPopulation.end(),
                  [](const Enemy& a, const Enemy& b) {
                      return a.GetFitness() > b.GetFitness();
                  });
        
        int survivorCount = getMaxFromDouble(1, static_cast<int>(sortedPopulation.size()) / 2);
        for (int i = 0; i < survivorCount; ++i) {
            survivors.push_back(sortedPopulation[i]);
        }
    }
    
    std::wstringstream wss_survivors;
    wss_survivors << L"GeneticAlgorithm::GenerateNewGeneration - Using " << survivors.size() 
                  << L" survivors for breeding\n";
    OutputDebugStringW(wss_survivors.str().c_str());
    
    // Create new generation based on survivors
    for (const Enemy& survivor : survivors) {
        // Convert grid coordinates to pixel coordinates
        float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
        float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
        
        // Create a copy and reset it for the new wave
        Enemy newEnemy = survivor;
        newEnemy.ResetForNewWave(startX, startY, initialEnemyPath);
        
        // Set spawn delay to space out enemies
        float spawnDelay = static_cast<float>(newGeneration.size()) * 1.0f;
        newEnemy.SetSpawnDelay(spawnDelay);
        
        newGeneration.push_back(newEnemy);
    }
    
    // If we don't have enough enemies, create more by copying and mutating survivors
    while (newGeneration.size() < 12) { // Ensure we have at least 12 enemies
        if (!survivors.empty()) {
            Enemy newEnemy = survivors[newGeneration.size() % survivors.size()];
            
            float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
            float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
            
            newEnemy.ResetForNewWave(startX, startY, initialEnemyPath);
            newEnemy.Mutate(0.5f); // Apply mutation for diversity
            
            // Set spawn delay to space out enemies
            float spawnDelay = static_cast<float>(newGeneration.size()) * 1.0f;
            newEnemy.SetSpawnDelay(spawnDelay);
            
            newGeneration.push_back(newEnemy);
        }
    }
    
    std::wstringstream wss;
    wss << L"GeneticAlgorithm::GenerateNewGeneration - Generated " 
        << newGeneration.size() << L" enemies for new wave\n";
    OutputDebugStringW(wss.str().c_str());
    
    return newGeneration;
}

// Getter for current population
const std::vector<Enemy>& GeneticAlgorithm::GetCurrentPopulation() const {
    return population;
}

// Setter for current population
void GeneticAlgorithm::SetCurrentPopulation(const std::vector<Enemy>& population) {
    this->population = population;
}

// Update map details if needed
void GeneticAlgorithm::SetMapDetails(const Map* map) {
    currentMap = map;
    if (currentMap) {
        initialEnemyPath = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
    }
}

// Private helper: Select parent using roulette wheel selection
Enemy GeneticAlgorithm::SelectParentRoulette() const {
    if (population.empty()) {
        // Fallback: create a random enemy
        return CreateRandomEnemy();
    }
    
    // Calculate total fitness
    double totalFitness = 0.0;
    for (const Enemy& enemy : population) {
        totalFitness += getMaxFromDouble(0.1, enemy.GetFitness()); // Ensure minimum fitness
    }
    
    // Random selection based on fitness
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, totalFitness);
    
    double randomValue = dis(gen);
    double cumulativeFitness = 0.0;
    
    for (const Enemy& enemy : population) {
        cumulativeFitness += getMaxFromDouble(0.1, enemy.GetFitness());
        if (cumulativeFitness >= randomValue) {
            return enemy;
        }
    }
    
    // Fallback: return last enemy
    return population.back();
}

// Private helper: Perform crossover between two parents
std::pair<Enemy, Enemy> GeneticAlgorithm::PerformCrossover(const Enemy& parent1, const Enemy& parent2) const {
    // Create children as copies of parents
    Enemy child1 = parent1;
    Enemy child2 = parent2;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    // Uniform crossover for each attribute
    if (dis(gen) < 0.5f) {
        child1.SetMaxHealth(parent2.GetMaxHealth());
        child2.SetMaxHealth(parent1.GetMaxHealth());
    }
    
    if (dis(gen) < 0.5f) {
        child1.SetSpeed(parent2.GetSpeed());
        child2.SetSpeed(parent1.GetSpeed());
    }
    
    if (dis(gen) < 0.5f) {
        child1.SetPathJitter(parent2.GetPathJitter());
        child2.SetPathJitter(parent1.GetPathJitter());
    }
    
    return std::make_pair(child1, child2);
}

// Private helper: Create a random enemy for initial population
Enemy GeneticAlgorithm::CreateRandomEnemy() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> typeDist(0, 3);
    
    EnemyType types[] = { EnemyType::OGRE, EnemyType::DARK_ELF, EnemyType::HARPY, EnemyType::MERCENARY };
    EnemyType randomType = types[typeDist(gen)];
    
    // Convert grid coordinates to pixel coordinates
    float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
    float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
    
    Enemy enemy(randomType, startX, startY, initialEnemyPath);
    enemy.Mutate(1.0f); // Full mutation for randomness
    
    return enemy;
}

// Private helper: Ensure we have exactly 3 of each enemy type
void GeneticAlgorithm::RebalanceEnemyTypes(std::vector<Enemy>& enemies) {
    // Group enemies by type
    std::vector<std::vector<Enemy>> enemiesByType(4);
    
    for (const Enemy& enemy : enemies) {
        int typeIndex = static_cast<int>(enemy.GetType());
        if (typeIndex >= 0 && typeIndex < 4) {
            enemiesByType[typeIndex].push_back(enemy);
        }
    }
    
    enemies.clear();
    
    // Ensure exactly 3 of each type
    EnemyType types[] = { EnemyType::OGRE, EnemyType::DARK_ELF, EnemyType::HARPY, EnemyType::MERCENARY };
    
    for (int i = 0; i < 4; ++i) {
        std::vector<Enemy>& typeGroup = enemiesByType[i];
        
        if (typeGroup.size() >= 3) {
            // Take the best 3
            std::sort(typeGroup.begin(), typeGroup.end(),
                      [](const Enemy& a, const Enemy& b) {
                          return a.GetFitness() > b.GetFitness();
                      });
            for (int j = 0; j < 3; ++j) {
                enemies.push_back(typeGroup[j]);
            }
        } else {
            // Add what we have
            for (const Enemy& enemy : typeGroup) {
                enemies.push_back(enemy);
            }
            
            // Fill missing slots with new random enemies of this type
            int needed = 3 - static_cast<int>(typeGroup.size());
            for (int j = 0; j < needed; ++j) {
                float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
                float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
                
                Enemy newEnemy(types[i], startX, startY, initialEnemyPath);
                newEnemy.Mutate(1.0f);
                enemies.push_back(newEnemy);
            }
        }
    }
}
