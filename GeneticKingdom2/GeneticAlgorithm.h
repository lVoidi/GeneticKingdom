/*
 * algoritmo genetico que controla la evolucion de los enemigos.
 * es un asco pero funciona. si lo tocas y lo rompes te mato <3.
 *
 * basicamente maneja:
 * - poblacion de enemigos y su evolucion
 * - diferentes tipos de enemigos (ogros, elfos, etc)
 * - caminos alternativos para que lleguen al puente
 * - seleccion, cruce y mutacion entre generaciones
 * - dificultad progresiva aumentando enemigos por tipo
 * 
 * si no sabes que es un algoritmo genetico mejor no toques nada
 */

#pragma once

#include "Enemy.h"
#include <vector>
#include <memory> // para los unique_ptr si manejamos enemigos con punteros

class Map; // declaracion forward para info del mapa que necesita el GA

class GeneticAlgorithm {
public:
    // constructor con toda la shi que necesita para funcionar
    GeneticAlgorithm(int populationSize, float mutationRate, float crossoverRate, 
                     const std::pair<int, int>& entryPoint, 
                     const std::pair<int, int>& bridgeLocation, 
                     const Map* gameMap);
    ~GeneticAlgorithm();

    // funciones principales del algoritmo genetico
    void InitializePopulation();
    void EvaluateFitness(float timeSurvivedByWave, bool waveReachedBridge);
    void SelectParents();
    void CrossoverAndMutate();
    std::vector<Enemy> GenerateNewGeneration(); // devuelve la nueva oleada de enemigos

    // getters y setters basicos
    const std::vector<Enemy>& GetCurrentPopulation() const;
    void SetCurrentPopulation(const std::vector<Enemy>& population);
    void SetMapDetails(const Map* map);

private:
    // variables principales
    std::vector<Enemy> population;
    std::vector<Enemy> parents;
    
    int populationSize;
    float mutationRate;
    float crossoverRate;

    // variables para la dificultad progresiva
    int enemiesPerTypeBase;      // enemigos base por tipo
    int maxEnemiesPerType;       // maximo de enemigos por tipo
    int wavesPerIncrement;       // oleadas antes de incrementar
    int wavesGeneratedCount;     // contador de oleadas generadas

    // coordenadas importantes
    std::pair<int, int> enemyEntryPoint;    // donde aparecen los enemigos
    std::pair<int, int> bridgeLocation;     // donde esta el puente
    const Map* currentMap;                   // puntero al mapa para pathfinding

    // funciones auxiliares para el algoritmo
    Enemy SelectParentRoulette() const;
    std::pair<Enemy, Enemy> PerformCrossover(const Enemy& parent1, const Enemy& parent2) const;
    
    // rutas que pueden seguir los enemigos
    std::vector<std::pair<int, int>> initialEnemyPath;
    std::vector<std::vector<std::pair<int, int>>> alternativePaths;

    // helpers para crear y balancear enemigos
    Enemy CreateRandomEnemy() const;
    void RebalanceEnemyTypes(std::vector<Enemy>& enemies, int targetPerType);
    void GenerateAlternativePaths(int numPathsToGenerate);
}; 