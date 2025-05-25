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

#include "Map.h"
#include "Enemy.h"
#include <vector>
#include <memory> // para los unique_ptr si manejamos enemigos con punteros
#include <random>
#include <sstream>
#include <windows.h>

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

    // Método para actualizar las estadísticas en el mapa
    void UpdateMapStatistics(Map* map) {
        if (map) {
            map->SetGenerationCount(wavesGeneratedCount);
            map->SetDeadEnemiesCount(deadEnemiesCount);
            
            // Obtener fitness de la población actual
            std::vector<float> currentFitness;
            for (const auto& enemy : population) {
                currentFitness.push_back(enemy.GetFitness());
            }
            map->SetCurrentFitness(currentFitness);
            
            // Actualizar estadísticas de mutación
            map->SetMutationStats(mutationRate, mutationCount);
        }
    }

    void Mutate(Enemy& enemy) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        
        if (dis(gen) < mutationRate) {
            enemy.Mutate(mutationRate);
            mutationCount++;
            
            std::wstringstream wss;
            wss << L"Mutation applied. Total mutations: " << mutationCount << L"\n";
            OutputDebugStringW(wss.str().c_str());
        }
    }

    void GenerateNextWave() {
        wavesGeneratedCount++;
        mutationCount = 0;  // Reiniciar contador de mutaciones para la nueva oleada
        // Incrementar el número de enemigos por tipo si es necesario
        if (wavesGeneratedCount % wavesPerIncrement == 0 && enemiesPerTypeBase < maxEnemiesPerType) {
            enemiesPerTypeBase++;
        }
        
        // Seleccionar padres para la siguiente generación
        SelectParents();
        
        // Generar nueva población mediante cruce y mutación
        std::vector<Enemy> newPopulation;
        while (newPopulation.size() < populationSize) {
            // Seleccionar padres al azar de la población actual
            Enemy parent1 = SelectRandomParent();
            Enemy parent2 = SelectRandomParent();
            
            // Realizar cruce
            Enemy offspring = CrossOver(parent1, parent2);
            
            // Aplicar mutación
            Mutate(offspring);
            
            newPopulation.push_back(offspring);
        }
        
        // Reemplazar la población anterior con la nueva
        population = std::move(newPopulation);
        
        // Actualizar estadísticas en el mapa
        if (currentMap) {
            UpdateMapStatistics(const_cast<Map*>(currentMap));
        }
    }

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

    int deadEnemiesCount = 0;
    int mutationCount = 0;

    Enemy SelectRandomParent() const;
    Enemy CrossOver(const Enemy& parent1, const Enemy& parent2) const;
}; 