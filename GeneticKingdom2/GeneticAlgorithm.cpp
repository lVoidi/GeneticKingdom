/*
 * GeneticAlgorithm.cpp
 * 
 * Este archivo implementa el algoritmo genético que controla la generación y evolución 
 * de enemigos en el juego. Los aspectos más importantes son:
 *
 * - Maneja la población de enemigos y su evolución entre oleadas
 * - Utiliza un sistema de tipos de enemigos (Archer, Gunner, Mage, etc)
 * - Genera caminos alternativos para que los enemigos lleguen al puente
 * - Implementa selección, cruce y mutación de enemigos entre generaciones
 * - Escala la dificultad incrementando enemigos por tipo (3-5) cada cierto número de oleadas
 * - Evalúa el fitness de cada enemigo basado en daño causado y distancia recorrida
 * - Mantiene balance entre tipos de enemigos para variedad táctica
 * - Usa pathfinding A* para generar rutas óptimas hacia el objetivo
 *
 * La clase trabaja en conjunto con Map.cpp para el pathfinding y manejo del terreno,
 * y con Enemy.cpp para la implementación específica de cada tipo de enemigo.
 */

#include "framework.h"
#include "GeneticAlgorithm.h"
#include "Map.h" // For map dimensions, pathfinding, etc.
#include <random>
#include <algorithm> // For std::sort, std::transform, etc.
#include <numeric>   // For std::accumulate
#include <iostream>  
#include <sstream>   
#include <iomanip>   
#include <map>       

// Helper functions to avoid conflicts with Windows macros
double getMaxFromDouble(double a, double b) {
    return a > b ? a : b;
}

double getMinFromDouble(double a, double b) {
    return a < b ? a : b;
}

/*
 * constructor principal del algoritmo genetico - aqui es donde empieza toda la magia.
 * configuramos los parametros iniciales de la poblacion y generamos los caminos que 
 * nuestros pequeños bastardos seguiran para intentar destruir el puente del jugador.
 *
 * el algoritmo usa un numero base de enemigos por tipo (3) que puede crecer hasta
 * un maximo de 5 por tipo conforme avanzan las oleadas. tambien llevamos la cuenta
 * de cuantas oleadas hemos generado para escalar la dificultad con el tiempo.
 *
 * si vas a tocar este codigo, mas te vale saber lo que estas haciendo. esto maneja
 * la logica central de generacion y evolucion de enemigos para todo el juego.
 */
GeneticAlgorithm::GeneticAlgorithm(int populationSize, float mutationRate, float crossoverRate,
                                   const std::pair<int, int>& entryPoint,
                                   const std::pair<int, int>& bridgeLocation,
                                   const Map* gameMap)
    : populationSize(populationSize), mutationRate(0.15f), crossoverRate(crossoverRate),
      enemyEntryPoint(entryPoint), bridgeLocation(bridgeLocation), currentMap(gameMap),
      enemiesPerTypeBase(3), maxEnemiesPerType(5), wavesPerIncrement(2), wavesGeneratedCount(0) {
    
    if (currentMap) {
        GenerateAlternativePaths(4);
        if (alternativePaths.empty()) {
            initialEnemyPath = currentMap->GetPath(entryPoint, bridgeLocation); 
             if (!initialEnemyPath.empty()) alternativePaths.push_back(initialEnemyPath);
             OutputDebugStringW(L"GeneticAlgorithm Warning: Could not generate multiple alternative paths. Using single initial path.\n");
        } else {
            initialEnemyPath = alternativePaths[0];
        }
    }
    
    OutputDebugStringW(L"GeneticAlgorithm initialized\n");
}

// Destructor
GeneticAlgorithm::~GeneticAlgorithm() {
    OutputDebugStringW(L"GeneticAlgorithm destroyed\n");
}

/*
 * inicializa la poblacion de enemigos para una nueva oleada
 * 
 * esta funcion es un puto desastre pero funciona. crea un monton de enemigos
 * basandose en el numero base por tipo que definimos. usa diferentes caminos
 * para que los enemigos no se amontonen como idiotos siguiendo la misma ruta.
 * 
 * si no hay caminos disponibles entra en panico y crea uno directo - mejor
 * que nada supongo. los enemigos se crean con un delay de spawn para que no
 * aparezcan todos de golpe como si fuera una estampida.
 */
void GeneticAlgorithm::InitializePopulation() {
    population.clear();
    if (alternativePaths.empty() && currentMap) {
        GenerateAlternativePaths(10);
        if (alternativePaths.empty()) {
            initialEnemyPath = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
            if (!initialEnemyPath.empty()) alternativePaths.push_back(initialEnemyPath);
        }
    }
    if (alternativePaths.empty()) {
        OutputDebugStringW(L"GeneticAlgorithm::InitializePopulation - ERROR: No paths available!\n");
        return;
    }
    
    std::wstringstream wss;
    int initialEnemiesToCreate = enemiesPerTypeBase * 4;
    wss << L"GeneticAlgorithm::InitializePopulation - Creating initial population. Base per type: " << enemiesPerTypeBase 
      << L", Total initial: " << initialEnemiesToCreate << L" using " << alternativePaths.size() << L" base paths.\n";
    OutputDebugStringW(wss.str().c_str());
    
    EnemyType types[] = { EnemyType::OGRE, EnemyType::DARK_ELF, EnemyType::HARPY, EnemyType::MERCENARY };
    int pathIndex = 0;
    
    // cada tipo tiene su numero base y los vamos rotando por los caminos disponibles
    for (EnemyType type : types) {
        for (int i = 0; i < enemiesPerTypeBase; ++i) {
            float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
            float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
            
            const auto& chosenPath = alternativePaths[pathIndex % alternativePaths.size()];
            Enemy enemy(type, startX, startY, chosenPath);
            pathIndex++;
            
            // mutamos al 100% para que no sean todos iguales desde el principio
            enemy.Mutate(1.5f); 
            
            // delay de spawn para que no salgan en manada
            float spawnDelay = static_cast<float>(population.size()) * 1.0f; 
            enemy.SetSpawnDelay(spawnDelay);
            
            population.push_back(enemy);
        }
    }
    
    // nos aseguramos que el tamaño maximo de poblacion pueda contener
    // el maximo de enemigos por tipo cuando escale la dificultad
    if(this->populationSize < this->maxEnemiesPerType * 4){
        this->populationSize = this->maxEnemiesPerType * 4;
        wss.str(L"");
        wss << L"Adjusted GA populationSize to " << this->populationSize << L" to accommodate max enemies per type.\n";
        OutputDebugStringW(wss.str().c_str());
    }

    wss.str(L"");
    wss << L"GeneticAlgorithm::InitializePopulation - Created " << population.size() << L" enemies for initial population.\n";
    OutputDebugStringW(wss.str().c_str());
}

/*
 * mira, esta funcion es la que evalua que tan buenos son los enemigos
 * basicamente les da una puntuacion segun que tan bien lo hicieron
 * 
 * parametros:
 * - timeSurvivedByWave: tiempo que duro la oleada (aunque no lo usamos, que pendejo)
 * - waveReachedBridge: si algun enemigo llego al puente
 *
 * la funcion calcula el fitness de cada enemigo usando:
 * - su posicion vs la del puente
 * - el tamano del mapa
 * - cuanto tiempo vivio
 * - si llego al puente
 *
 * al final ordena a todos por su fitness, el mejor primero
 * porque? porque asi podemos agarrar a los mejores para la siguiente generacion
 */
void GeneticAlgorithm::EvaluateFitness(float timeSurvivedByWave, bool waveReachedBridge) {
    std::wstringstream wss;
    wss << L"GeneticAlgorithm::EvaluateFitness - Evaluating " << population.size() << L" enemies\n";
    OutputDebugStringW(wss.str().c_str());
    
    int currentWaveDeadEnemies = 0;
    float totalFitness = 0.0f;
    
    for (Enemy& enemy : population) {
        // Contar enemigos eliminados
        if (!enemy.IsAlive()) {
            currentWaveDeadEnemies++;
        }
        
        // Calcular fitness para cada enemigo
        enemy.CalculateFitness(bridgeLocation,
                              currentMap->GetMapPixelWidth(),
                              currentMap->GetMapPixelHeight(),
                              enemy.GetTimeAlive(),
                              enemy.HasReachedBridge());
        
        totalFitness += static_cast<float>(enemy.GetFitness());
    }
    
    // Actualizar el contador total de enemigos eliminados
    deadEnemiesCount += currentWaveDeadEnemies;
    
    // Ordenar la población por fitness
    std::sort(population.begin(), population.end(),
              [](const Enemy& a, const Enemy& b) {
                  return a.GetFitness() > b.GetFitness();
              });
              
    if (!population.empty()) {
        wss.str(L"");
        wss << L"Wave stats - Best fitness: " << population[0].GetFitness() 
            << L", Worst fitness: " << population.back().GetFitness()
            << L", Average fitness: " << (totalFitness / population.size())
            << L"\nDead enemies this wave: " << currentWaveDeadEnemies 
            << L", Total dead enemies: " << deadEnemiesCount << L"\n";
        OutputDebugStringW(wss.str().c_str());
    }

    // Actualizar estadísticas en el mapa
    if (currentMap) {
        UpdateMapStatistics(const_cast<Map*>(currentMap));
    }
}

/*
 * selecciona los parents para la reproduccion, esta funcion es un asco total pero funciona.
 * primero agarra al mejor enemigo de todos y lo mete como padre elite.
 * luego busca el mejor de cada tipo (ogro, elfo oscuro, etc) y los mete tambien.
 * finalmente usa ruleta para meter mas padres hasta llenar el cupo.
 * 
 * la ruleta es una mierda pero es lo que hay. los enemigos con mejor fitness
 * tienen mas chance de ser elegidos. esto evita que los mas pendejos se reproduzcan mucho.
 */
void GeneticAlgorithm::SelectParents() {
    parents.clear();
    if (population.empty()) return;

    std::sort(population.begin(), population.end(), [](const Enemy& a, const Enemy& b) {
        return a.GetFitness() > b.GetFitness();
    });

    std::wstringstream wss_select;
    wss_select << L"GeneticAlgorithm::SelectParents - Population sorted. Size: " << population.size() << L"\n";

    std::map<EnemyType, int> typeCountsInParents;
    EnemyType allTypes[] = { EnemyType::OGRE, EnemyType::DARK_ELF, EnemyType::HARPY, EnemyType::MERCENARY };

    if (!population.empty()) {
        parents.push_back(population[0]);
        typeCountsInParents[population[0].GetType()]++;
        wss_select << L"  Elite parent (best overall): Type " << static_cast<int>(population[0].GetType()) << L", Fitness: " << population[0].GetFitness() << L"\n";
    }

    for (EnemyType type : allTypes) {
        if (typeCountsInParents.find(type) == typeCountsInParents.end() || typeCountsInParents[type] < 1) {
            const Enemy* bestOfType = nullptr;
            for (const Enemy& enemy : population) {
                if (enemy.GetType() == type) {
                    bool alreadyParent = false;
                    for(const Enemy& p : parents) { 
                        // esta comparacion es una mierda pero funciona
                        // deberiamos tener ids unicos pero me dio flojera implementarlos
                    }
                    if(!alreadyParent) {
                        if (!bestOfType || enemy.GetFitness() > bestOfType->GetFitness()) {
                            bestOfType = &enemy;
                        }
                    }
                }
            }
            if (bestOfType) {
                parents.push_back(*bestOfType);
                typeCountsInParents[type]++;
                wss_select << L"  Diverse parent (best of type " << static_cast<int>(type) << L"): Fitness: " << bestOfType->GetFitness() << L"\n";
            }
        }
    }

    // aqui metemos mas padres usando ruleta hasta llenar el cupo
    // el cupo es la mitad de la poblacion o 8, lo que sea mas grande
    int desiredParentPoolSize = static_cast<int>(getMaxFromDouble(8.0, static_cast<double>(populationSize) / 2.0));
    while (parents.size() < static_cast<size_t>(desiredParentPoolSize) && parents.size() < population.size()) {
        Enemy selectedParent = SelectParentRoulette();
        // revisamos que no sea un duplicado exacto, aunque la revision es medio inutil
        bool alreadySelected = false;
        for(const auto& p : parents) {
            if(p.GetX() == selectedParent.GetX() && p.GetY() == selectedParent.GetY() && p.GetType() == selectedParent.GetType() && p.GetFitness() == selectedParent.GetFitness()) { 
                alreadySelected = true; break; 
            }
        }
        if(!alreadySelected) parents.push_back(selectedParent);
        else { 
             if(population.size() > parents.size()) parents.push_back(SelectParentRoulette());
        } 
    }

    // Nuevo: Asegurarnos de tener una distribución equilibrada de rutas
    std::map<std::string, int> routeTypeCount; // superior, medio, inferior
    for (const Enemy& enemy : parents) {
        std::string routeType;
        if (enemy.GetY() < currentMap->GetMapPixelHeight() * 0.33) {
            routeType = "superior";
        } else if (enemy.GetY() > currentMap->GetMapPixelHeight() * 0.66) {
            routeType = "inferior";
        } else {
            routeType = "medio";
        }
        routeTypeCount[routeType]++;
    }

    // Si hay desequilibrio, intentar corregirlo
    int maxRouteCount = getMaxFromDouble(
        getMaxFromDouble(static_cast<double>(routeTypeCount["superior"]), 
                        static_cast<double>(routeTypeCount["medio"])),
        static_cast<double>(routeTypeCount["inferior"])
    );
    
    if (maxRouteCount > static_cast<int>(parents.size() / 3)) {
        // Reemplazar algunos padres para balancear
        std::vector<Enemy> balancedParents;
        for (const Enemy& parent : parents) {
            std::string routeType;
            if (parent.GetY() < currentMap->GetMapPixelHeight() * 0.33) {
                routeType = "superior";
            } else if (parent.GetY() > currentMap->GetMapPixelHeight() * 0.66) {
                routeType = "inferior";
            } else {
                routeType = "medio";
            }
            
            if (routeTypeCount[routeType] <= parents.size() / 3) {
                balancedParents.push_back(parent);
            }
        }
        parents = balancedParents;
    }

    wss_select << L"GeneticAlgorithm::SelectParents - Total selected " << parents.size() << L" parents.\n";
    OutputDebugStringW(wss_select.str().c_str());
}

/* 
 * aqui es donde la magia negra ocurre - creamos una nueva generacion de enemigos
 * usando crossover y mutacion. si, es un desastre pero funciona.
 * 
 * incrementamos el contador de oleadas y ajustamos el numero de enemigos
 * basado en eso. es como un juego de tetris pero con monstruos.
 */
void GeneticAlgorithm::CrossoverAndMutate() {
    wavesGeneratedCount++;
    mutationCount = 0;  // Reiniciar contador de mutaciones para la nueva oleada

    // calculamos cuantos enemigos queremos por tipo en esta oleada
    int incrementFactor = (wavesGeneratedCount -1) / wavesPerIncrement;
    int currentTargetEnemiesPerType = enemiesPerTypeBase + incrementFactor;
    if (currentTargetEnemiesPerType > maxEnemiesPerType) {
        currentTargetEnemiesPerType = maxEnemiesPerType;
    }

    std::wstringstream wss_debug_crossover;
    wss_debug_crossover << L"GeneticAlgorithm::CrossoverAndMutate - Wave: " << wavesGeneratedCount 
                        << L", Target per type: " << currentTargetEnemiesPerType 
                        << L", Starting mutations: " << mutationCount << L"\n";
    OutputDebugStringW(wss_debug_crossover.str().c_str());

    std::vector<Enemy> newOffspringPopulation;
    newOffspringPopulation.reserve(populationSize);

    if (parents.empty()) { 
        for(int i=0; i < populationSize; ++i) {
            Enemy newEnemy = CreateRandomEnemy();
            Mutate(newEnemy);  // Asegurarnos de contar las mutaciones
            newOffspringPopulation.push_back(newEnemy);
        }
    } else {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        std::uniform_int_distribution<int> parentDist(0, static_cast<int>(parents.size()) - 1);

        while (newOffspringPopulation.size() < static_cast<size_t>(populationSize)) {
            if (parents.size() >= 2 && dis(gen) < crossoverRate) {
                int parent1Idx = parentDist(gen);
                int parent2Idx = parentDist(gen);
                while (parent2Idx == parent1Idx && parents.size() > 1) { 
                    parent2Idx = parentDist(gen);
                }
                
                std::pair<Enemy, Enemy> offspring = PerformCrossover(parents[parent1Idx], parents[parent2Idx]);
                Mutate(offspring.first);
                Mutate(offspring.second);
                
                newOffspringPopulation.push_back(offspring.first);
                if (newOffspringPopulation.size() < static_cast<size_t>(populationSize)) {
                    newOffspringPopulation.push_back(offspring.second);
                }
            } else { 
                Enemy newEnemy = parents[parentDist(gen)]; 
                Mutate(newEnemy);
                newOffspringPopulation.push_back(newEnemy);
            }
        }
    }
    
    RebalanceEnemyTypes(newOffspringPopulation, currentTargetEnemiesPerType);
    population = newOffspringPopulation;

    wss_debug_crossover.str(L"");
    wss_debug_crossover << L"CrossoverAndMutate complete - New population size: " << population.size() 
                        << L", Final mutations this wave: " << mutationCount << L"\n";
    OutputDebugStringW(wss_debug_crossover.str().c_str());
    
    // Actualizar estadísticas en el mapa
    if (currentMap) {
        UpdateMapStatistics(const_cast<Map*>(currentMap));
    }
}

/* 
 * genera una nueva generacion de enemigos a partir de la poblacion actual
 * si la poblacion esta vacia o no hay caminos disponibles, retorna un vector vacio
 */
std::vector<Enemy> GeneticAlgorithm::GenerateNewGeneration() {
    std::wstringstream wss_gen_new_wave;
    wss_gen_new_wave << L"GeneticAlgorithm::GenerateNewGeneration - Starting. Population size: " << population.size() << L"\n";
    
    /* verifica que tengamos una poblacion con la que trabajar */
    if (population.empty()) {
        wss_gen_new_wave << L"  ERROR: Current GA population is empty. Cannot generate new wave.\n";
        OutputDebugStringW(wss_gen_new_wave.str().c_str());
        return {}; 
    }

    /* sin caminos no hay juego BV */
    if (alternativePaths.empty()) {
         wss_gen_new_wave << L"  WARNING: No alternative paths. Will use initialEnemyPath for all.\n";
        if(currentMap && initialEnemyPath.empty()) initialEnemyPath = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
        if(initialEnemyPath.empty() && (alternativePaths.empty() || alternativePaths[0].empty())){
            wss_gen_new_wave << L"  CRITICAL ERROR: No paths available AT ALL. Cannot generate new wave.\n";
            OutputDebugStringW(wss_gen_new_wave.str().c_str());
            return {};
        }
        if(alternativePaths.empty() && !initialEnemyPath.empty()) alternativePaths.push_back(initialEnemyPath);
    }

    /* aqui es donde la magia sucede - creamos los nuevos enemigos */
    std::vector<Enemy> newWaveEnemies; 
    newWaveEnemies.reserve(population.size());

    /* distribuye los enemigos en diferentes caminos, porque si no esto seria muy facil */
    int pathIndex = 0;
    for (const Enemy& blueprintEnemy : population) {
        float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
        float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
        
        Enemy newEnemy = blueprintEnemy;
        
        /* elige un camino para este enemigo, rotando entre los disponibles */
        const auto& chosenPath = !alternativePaths.empty() ? 
                                 alternativePaths[pathIndex % alternativePaths.size()] : // Esto de usar el % estuvo épico tbh
                                 initialEnemyPath; 
        pathIndex++;

        /* prepara al enemigo para la nueva oleada y le da un retraso de spawn */
        newEnemy.ResetForNewWave(startX, startY, chosenPath);
        newEnemy.SetSpawnDelay(static_cast<float>(newWaveEnemies.size()) * 1.0f); 
        
        newWaveEnemies.push_back(newEnemy);
        wss_gen_new_wave << L"  Added enemy to new wave. Type: " << static_cast<int>(newEnemy.GetType()) << L", Path assigned. Total in wave: " << newWaveEnemies.size() << L"\n";
    }
    
    wss_gen_new_wave << L"GeneticAlgorithm::GenerateNewGeneration - Finished. Generated " 
        << newWaveEnemies.size() << L" enemies for new wave.\n";
    OutputDebugStringW(wss_gen_new_wave.str().c_str());
    
    return newWaveEnemies;
}

/* 
 * devuelve la poblacion actual de enemigos, util para debugging y para 
 * que otros modulos puedan ver que diablos esta pasando con la evolucion
 * sin tener que meter mano en las tripas del algoritmo genetico
 */
const std::vector<Enemy>& GeneticAlgorithm::GetCurrentPopulation() const {
    return population;
}

/*
 * permite reemplazar toda la poblacion actual con una nueva. esto es 
 * especialmente util cuando necesitas meter mano manualmente o restaurar
 * un estado anterior.
 */
void GeneticAlgorithm::SetCurrentPopulation(const std::vector<Enemy>& population) {
    this->population = population;
}

/*
 * actualiza la referencia al mapa y recalcula el camino inicial.
 * sin esto los enemigos andarian a lo pendejo por ahi sin saber
 * a donde ir. el camino inicial es como el adn base que heredan
 * todos al principio
 */
void GeneticAlgorithm::SetMapDetails(const Map* map) {
    currentMap = map;
    if (currentMap) {
        initialEnemyPath = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
    }
}

/* 
 * selecciona un padre usando el metodo de la ruleta
 * basicamente, entre mas fitness tenga un enemigo, mas probabilidad tiene de ser seleccionado
 * si la poblacion esta vacia, crea un enemigo random porque yolo
 */
Enemy GeneticAlgorithm::SelectParentRoulette() const {
    if (population.empty()) {
        return CreateRandomEnemy();
    }
    
    double totalFitness = 0.0;
    for (const Enemy& enemy : population) {
        totalFitness += getMaxFromDouble(0.1, enemy.GetFitness()); 
    }
    
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
    
    return population.back();
}

/*
 * hace el apareamiento entre dos padres para crear dos hijos
 * usa crossover uniforme - cada atributo tiene 50% de probabilidad de venir de cada padre
 * es como jugar a la ruleta con cada gen, pero mas justo que partir al enemigo por la mitad
 * porque eso seria una masacre
 */
std::pair<Enemy, Enemy> GeneticAlgorithm::PerformCrossover(const Enemy& parent1, const Enemy& parent2) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    // Crear dos nuevos enemigos con posiciones iniciales
    float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
    float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
    
    Enemy offspring1(parent1.GetType(), startX, startY, initialEnemyPath);
    Enemy offspring2(parent2.GetType(), startX, startY, initialEnemyPath);

    // Mezclar atributos usando crossover uniforme
    auto crossoverAttribute = [&](auto& attr1, auto& attr2) {
        if (dis(gen) < 0.5f) {
            std::swap(attr1, attr2);
        }
    };

    // Mezclar salud
    int health1 = parent1.GetMaxHealth();
    int health2 = parent2.GetMaxHealth();
    crossoverAttribute(health1, health2);
    offspring1.SetMaxHealth(health1);
    offspring2.SetMaxHealth(health2);

    // Mezclar velocidad
    float speed1 = parent1.GetSpeed();
    float speed2 = parent2.GetSpeed();
    crossoverAttribute(speed1, speed2);
    offspring1.SetSpeed(speed1);
    offspring2.SetSpeed(speed2);

    // Mezclar resistencias
    float arrowRes1 = parent1.GetArrowResistance();
    float arrowRes2 = parent2.GetArrowResistance();
    crossoverAttribute(arrowRes1, arrowRes2);
    offspring1.SetArrowResistance(arrowRes1);
    offspring2.SetArrowResistance(arrowRes2);

    float magicRes1 = parent1.GetMagicResistance();
    float magicRes2 = parent2.GetMagicResistance();
    crossoverAttribute(magicRes1, magicRes2);
    offspring1.SetMagicResistance(magicRes1);
    offspring2.SetMagicResistance(magicRes2);

    float artilleryRes1 = parent1.GetArtilleryResistance();
    float artilleryRes2 = parent2.GetArtilleryResistance();
    crossoverAttribute(artilleryRes1, artilleryRes2);
    offspring1.SetArtilleryResistance(artilleryRes1);
    offspring2.SetArtilleryResistance(artilleryRes2);

    // Asignar caminos diferentes a cada hijo
    if (!alternativePaths.empty()) {
        int pathIndex1 = gen() % alternativePaths.size();
        int pathIndex2 = gen() % alternativePaths.size();
        offspring1.SetPath(alternativePaths[pathIndex1]);
        offspring2.SetPath(alternativePaths[pathIndex2]);
    }
    
    return std::make_pair(offspring1, offspring2);
}
/*
 * mira, esta funcion es bastante simple pero importante - crea un enemigo aleatorio
 * para la poblacion inicial del algoritmo genetico. es como tirar los dados para ver
 * que tipo de monstruo sale, pero con un toque de ciencia.
 *
 * primero elegimos un tipo al azar entre los 4 que tenemos (ogro, elfo oscuro, harpia, mercenario)
 * luego calculamos donde va a aparecer el bicho en el mapa, convirtiendo coordenadas de la cuadricula
 * a pixeles reales.
 * 
 * lo interesante viene con el path - intentamos darle una ruta aleatoria de las alternativas
 * que tenemos, pero si no hay ninguna usamos la ruta por defecto. y si ni siquiera tenemos eso,
 * pues la hemos cagado y el enemigo se quedara parado como un pasmarote.
 *
 * al final le metemos mutacion al 100% para que salga bien random
 */
Enemy GeneticAlgorithm::CreateRandomEnemy() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> typeDist(0, 3);
    
    EnemyType types[] = { EnemyType::OGRE, EnemyType::DARK_ELF, EnemyType::HARPY, EnemyType::MERCENARY };
    EnemyType randomType = types[typeDist(gen)];
    
    float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
    float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
    
    // elige un camino aleatorio de las alternativas o usa el default
    std::vector<std::pair<int, int>> chosenPathForNewEnemy;
    if (!alternativePaths.empty()) {
        chosenPathForNewEnemy = alternativePaths[gen() % alternativePaths.size()]; 
    } else if (!initialEnemyPath.empty()) {
        chosenPathForNewEnemy = initialEnemyPath;
        OutputDebugStringW(L"CreateRandomEnemy: alternativePaths empty, using initialEnemyPath.\n");
    } else {
        OutputDebugStringW(L"CRITICAL ERROR in CreateRandomEnemy: No paths available! Returning enemy with empty path.\n");
    }

    Enemy enemy(randomType, startX, startY, chosenPathForNewEnemy);
    enemy.Mutate(1.0f);
    
    return enemy;
}

/*
 * funcion privada que se asegura de tener exactamente el mismo numero de cada tipo de enemigo
 * en la poblacion. esto es importante porque si no, el algoritmo genetico se vuelve loco y
 * empieza a generar solo un tipo de enemigo que le gusta mas.
 * 
 * mira, lo que hace es:
 * 1. ordena los enemigos por fitness (los mejores primero)
 * 2. los separa por tipo en diferentes grupos
 * 3. toma los mejores de cada tipo hasta llegar al numero que queremos
 * 4. si faltan enemigos de algun tipo, los crea desde cero
 * 
 * es como un control de calidad para que no se joda la evolucion
 */
void GeneticAlgorithm::RebalanceEnemyTypes(std::vector<Enemy>& enemiesToRebalance, int targetPerType) {
    std::wstringstream wss_rebalance;
    wss_rebalance << L"GeneticAlgorithm::RebalanceEnemyTypes - Starting. Target per type: " << targetPerType 
                  << L". Initial size: " << enemiesToRebalance.size() << L"\n";

    std::vector<std::vector<Enemy>> enemiesByType(4);
    std::sort(enemiesToRebalance.begin(), enemiesToRebalance.end(), [](const Enemy& a, const Enemy& b) {
        return a.GetFitness() > b.GetFitness();
    });

    for (const Enemy& enemy : enemiesToRebalance) {
        int typeIndex = static_cast<int>(enemy.GetType());
        if (typeIndex >= 0 && typeIndex < 4) {
            enemiesByType[typeIndex].push_back(enemy);
        }
    }

    enemiesToRebalance.clear(); 
    EnemyType allTypes[] = { EnemyType::OGRE, EnemyType::DARK_ELF, EnemyType::HARPY, EnemyType::MERCENARY };
    int pathAssignIndex = 0; 

    for (int typeIdx = 0; typeIdx < 4; ++typeIdx) {
        EnemyType currentProcessingType = allTypes[typeIdx];
        std::vector<Enemy>& specificTypeGroup = enemiesByType[typeIdx];
        int numAddedForThisType = 0;

        // mira, aqui primero metemos los enemigos que ya tenemos y son buenos
        for (const Enemy& existingEnemy : specificTypeGroup) {
            if (numAddedForThisType < targetPerType) {
                enemiesToRebalance.push_back(existingEnemy);
                numAddedForThisType++;
            } else {
                break; 
            }
        }
        wss_rebalance << L"  Type " << static_cast<int>(currentProcessingType) << L": Added " << numAddedForThisType << L" existing. Need " << (targetPerType - numAddedForThisType) << L" new.\n";

        // y si nos faltan, creamos nuevos desde cero
        int needed = targetPerType - numAddedForThisType;
        for (int j = 0; j < needed; ++j) {
            float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
            float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
            
            // intentamos darle un camino aleatorio, si no hay usamos el default
            std::vector<std::pair<int, int>> chosenPathForNewEnemy; 
            if (!alternativePaths.empty()) {
                chosenPathForNewEnemy = alternativePaths[(pathAssignIndex++) % alternativePaths.size()];
            } else if (!initialEnemyPath.empty()) {
                chosenPathForNewEnemy = initialEnemyPath;
                wss_rebalance << L"    WARN: Creating new enemy in Rebalance (type " << static_cast<int>(currentProcessingType) 
                              << L"), alternativePaths empty, using initialEnemyPath.\n";
            } else {
                wss_rebalance << L"    CRITICAL ERROR: No paths available in RebalanceEnemyTypes for new enemy of type " << static_cast<int>(currentProcessingType) << L". Skipping creation for this one.\n";
                continue; 
            }

            Enemy newEnemy(currentProcessingType, startX, startY, chosenPathForNewEnemy);
            newEnemy.Mutate(1.0f); 
            enemiesToRebalance.push_back(newEnemy);
        }
    }
    wss_rebalance << L"GeneticAlgorithm::RebalanceEnemyTypes - Finished. Final size: " << enemiesToRebalance.size() << L"\n";
    OutputDebugStringW(wss_rebalance.str().c_str());
}

/*
 * mira, esta funcion es la que genera los diferentes caminos que pueden tomar los enemigos
 * para llegar al puente. es una parte critica del algoritmo genetico porque necesitamos
 * que los enemigos tengan diferentes rutas para que el juego no sea tan predecible y aburrido.
 * 
 * basicamente lo que hace es:
 * 1. genera un camino optimo sin obstaculos (el mas directo)
 * 2. intenta forzar un camino por la parte superior del mapa
 * 3. intenta forzar un camino por la parte inferior
 * 4. intenta crear un camino en zigzag diagonal
 *
 * si todo falla, al menos nos aseguramos de tener un camino de emergencia
 * para que los enemigos no se queden atascados como idiotas
 */
void GeneticAlgorithm::GenerateAlternativePaths(int numPathsToAttempt) {
    alternativePaths.clear();
    if (!currentMap) {
        OutputDebugStringW(L"GeneticAlgorithm::GenerateAlternativePaths - Error: currentMap is null.\n");
        return;
    }

    Map* mutableMap = const_cast<Map*>(currentMap);
    std::wstringstream wss_paths;
    wss_paths << L"GeneticAlgorithm::GenerateAlternativePaths - Attempting to generate up to " << numPathsToAttempt << L" paths.\n";

    /* primer camino: el mas simple y directo, sin trucos */
    std::vector<std::pair<int, int>> path1 = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
    if (!path1.empty()) {
        alternativePaths.push_back(path1);
        wss_paths << L"  Added Path 1 (Optimal). Length: " << path1.size() << L"\n";
    }

    /* calculamos algunas dimensiones importantes del mapa para poner obstaculos */
    int nRows = currentMap->GetNumRows();
    int nCols = currentMap->GetNumCols();
    int h_sup_r1 = nRows / 4 - 1; int h_sup_r2 = nRows / 4;
    int h_mid_r = nRows / 2; 
    int h_inf_r1 = 3 * nRows / 4; int h_inf_r2 = 3 * nRows / 4 + 1;
    int cols_hileras_start = nCols / 5;
    int cols_hileras_end = 4 * nCols / 5;

    /* dimensiones del puente - no queremos bloquearlo porque seria estupido */
    int bridgeWidthOriginal = nCols / 10;
    int bridgeStartColOriginal = nCols - bridgeWidthOriginal;
    int bridgeHeightOriginal = nRows / 10;
    int bridgeTopRowOriginal = (nRows - bridgeHeightOriginal) / 2;

    /* funcion lambda para agregar obstaculos solo si no joden algo importante */
    auto addObstacleIfNotCritical = [&](int r, int c) {
        if ( (r == enemyEntryPoint.first && c == enemyEntryPoint.second) ) return; 
        if (c >= bridgeStartColOriginal && c < nCols &&
            r >= bridgeTopRowOriginal && r < bridgeTopRowOriginal + bridgeHeightOriginal) {
            return;
        }
        mutableMap->AddTemporaryObstacle(r, c);
    };

    /* funciones helper para poner obstaculos en filas y rangos */
    auto addObstaclesInRow = [&](int r_target, int cStart, int cEnd) {
        if (r_target >= 0 && r_target < nRows) {
            for (int c = cStart; c <= cEnd; ++c) {
                if (c >= 0 && c < nCols) addObstacleIfNotCritical(r_target, c);
            }
        }
    };
    auto addObstaclesInRowRange = [&](int r_start, int r_end, int cStart, int cEnd) {
        for (int r_loop = r_start; r_loop <= r_end; ++r_loop) addObstaclesInRow(r_loop, cStart, cEnd);
    };

    /* segundo camino: forzamos a los enemigos a ir por arriba */
    if (alternativePaths.size() < static_cast<size_t>(numPathsToAttempt)) {
        mutableMap->ClearTemporaryObstacles();
        wss_paths << L"  Generating Path 2 (attempting upper lane)...\n";
        
        // Bloqueamos la parte media y baja del mapa
        int blockStartCol = nCols / 4;
        int blockEndCol = 3 * nCols / 4;
        
        // Bloqueo más fuerte en la parte media-baja
        for (int c = blockStartCol; c <= blockEndCol; c += 2) {
            addObstaclesInRowRange(h_mid_r, h_inf_r1, c, c + 1);
        }
        
        std::vector<std::pair<int, int>> path2 = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
        if (!path2.empty() && path2 != path1) {
            alternativePaths.push_back(path2);
            wss_paths << L"    Added Path 2 (Upper). Length: " << path2.size() << L"\n";
        }
        mutableMap->ClearTemporaryObstacles();
    }

    /* tercer camino: forzamos a los enemigos a ir por abajo */
    if (alternativePaths.size() < static_cast<size_t>(numPathsToAttempt)) {
        mutableMap->ClearTemporaryObstacles();
        wss_paths << L"  Generating Path 3 (attempting lower lane)...\n";
        
        // Bloqueamos la parte media y alta del mapa
        int blockStartCol = nCols / 4;
        int blockEndCol = 3 * nCols / 4;
        
        // Bloqueo más fuerte en la parte media-alta
        for (int c = blockStartCol; c <= blockEndCol; c += 2) {
            addObstaclesInRowRange(h_sup_r1, h_mid_r, c, c + 1);
        }
        
        std::vector<std::pair<int, int>> path3 = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
        bool isDifferent = true;
        for (const auto& existingPath : alternativePaths) {
            if (path3 == existingPath) {
                isDifferent = false;
                break;
            }
        }
        
        if (!path3.empty() && isDifferent) {
            alternativePaths.push_back(path3);
            wss_paths << L"    Added Path 3 (Lower). Length: " << path3.size() << L"\n";
        }
        mutableMap->ClearTemporaryObstacles();
    }

    /* cuarto camino: intentamos hacer que vayan en zigzag, porque yolo */
    if (alternativePaths.size() < static_cast<size_t>(numPathsToAttempt)) {
        mutableMap->ClearTemporaryObstacles();
        wss_paths << L"  Generating Path 4 (attempting diagonal route)...\n";
        
        int blockStartCol = nCols / 4;
        int blockEndCol = 3 * nCols / 4;
        
        for (int c = blockStartCol; c <= blockEndCol; c += 6) {
            if (c % 12 == blockStartCol % 12) {
                addObstaclesInRow(h_sup_r1, c, c + 2);
                addObstaclesInRow(h_inf_r2, c + 3, c + 5);
            } else {
                addObstaclesInRow(h_sup_r2, c, c + 2);
                addObstaclesInRow(h_inf_r1, c + 3, c + 5);
            }
        }

        std::vector<std::pair<int, int>> path4 = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
        if (!path4.empty() && path4 != path1 && 
            (alternativePaths.size() < 2 || path4 != alternativePaths[1]) &&
            (alternativePaths.size() < 3 || path4 != alternativePaths[2])) {
            alternativePaths.push_back(path4);
            wss_paths << L"    Added Path 4 (Diagonal). Length: " << path4.size() << L"\n";
        } else if(!path4.empty()){
            wss_paths << L"    Path 4 was duplicate or A* could not find a distinct diagonal path.\n";
        } else {
            wss_paths << L"    Path 4 (Diagonal) FAILED to generate.\n";
        }
        mutableMap->ClearTemporaryObstacles();
    }

    /* si todo fallo, al menos aseguramos un camino basico */
    if (alternativePaths.empty() && currentMap) { 
         wss_paths << L"  Fallback: No paths generated despite efforts. Adding emergency optimal path.\n";
         std::vector<std::pair<int, int>> emergencyPath = currentMap->GetPath(enemyEntryPoint, bridgeLocation); 
         if(!emergencyPath.empty()) alternativePaths.push_back(emergencyPath);
    }
    if (!alternativePaths.empty()) {
        initialEnemyPath = alternativePaths[0]; 
    } else {
        OutputDebugStringW(L"CRITICAL ERROR in GenerateAlternativePaths: No paths could be generated for enemies! Map might be unnavigable.\n");
    }
    
    wss_paths << L"GeneticAlgorithm::GenerateAlternativePaths - Finished. Total unique paths found: " << alternativePaths.size() << L"\n";
    OutputDebugStringW(wss_paths.str().c_str());
}
