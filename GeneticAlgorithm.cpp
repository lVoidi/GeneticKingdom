void GeneticAlgorithm::GenerateAlternativePaths(int numPathsToAttempt) {
    alternativePaths.clear();
    if (!currentMap) {
        OutputDebugStringW(L"GeneticAlgorithm::GenerateAlternativePaths - Error: currentMap is null.\n");
        return;
    }

    Map* mutableMap = const_cast<Map*>(currentMap);
    std::wstringstream wss_paths;
    wss_paths << L"GeneticAlgorithm::GenerateAlternativePaths - Attempting to generate up to " << numPathsToAttempt << L" paths.\n";

    std::vector<std::pair<int, int>> path1 = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
    if (!path1.empty()) {
        alternativePaths.push_back(path1);
        wss_paths << L"  Added Path 1 (Optimal). Length: " << path1.size() << L"\n";
    }

    int nRows = currentMap->GetNumRows();
    int nCols = currentMap->GetNumCols();
    int h_sup_r1 = nRows / 4 - 1; int h_sup_r2 = nRows / 4;
    int h_mid_r = nRows / 2; 
    int h_inf_r1 = 3 * nRows / 4; int h_inf_r2 = 3 * nRows / 4 + 1;
    
    // Usar los mismos límites de columna que Map::LoadConstructionSpots para las hileras
    int cols_hileras_start = nCols / 5;
    int cols_hileras_end = 4 * nCols / 5;
    std::pair<int,int> bridgeNode = currentMap->GetBridgeGridLocation(); // Celda representativa del puente

    auto addObstacleIfNotCritical = [&](int r, int c) {
        if ( (r == enemyEntryPoint.first && c == enemyEntryPoint.second) || 
             (r == bridgeNode.first && c == bridgeNode.second) ) { // Evitar bloquear entrada/salida directa
            return; 
        }
        mutableMap->AddTemporaryObstacle(r, c);
    };

    auto addObstaclesInRow = [&](int r, int cStart, int cEnd) {
        if (r >= 0 && r < nRows) {
            for (int c = cStart; c <= cEnd; ++c) {
                if (c >= 0 && c < nCols) addObstacleIfNotCritical(r, c);
            }
        }
    };
    auto addObstaclesInRowRange = [&](int r_start, int r_end, int cStart, int cEnd) {
        for (int r = r_start; r <= r_end; ++r) addObstaclesInRow(r, cStart, cEnd);
    };

    // Path 2: Try to force path via UPPER lane
    if (alternativePaths.size() < static_cast<size_t>(numPathsToAttempt)) {
        mutableMap->ClearTemporaryObstacles();
        wss_paths << L"  Generating Path 2 (attempting upper lane by blocking mid/low hileras)...\n";
        addObstaclesInRow(h_mid_r, cols_hileras_start, cols_hileras_end);    
        addObstaclesInRowRange(h_inf_r1, h_inf_r2, cols_hileras_start, cols_hileras_end);    
        if (h_mid_r + 1 < h_inf_r1) addObstaclesInRow(h_mid_r + 1, cols_hileras_start, cols_hileras_end); // Bloqueo agresivo adyacente
        
        std::vector<std::pair<int, int>> path2 = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
        if (!path2.empty() && path2 != path1) {
            alternativePaths.push_back(path2);
            wss_paths << L"    Added Path 2 (Upper). Length: " << path2.size() << L"\n";
        } else if(!path2.empty()) {
            wss_paths << L"    Path 2 was duplicate or A* could not find a distinct upper path.\n";
        } else {
            wss_paths << L"    Path 2 (Upper) FAILED to generate.\n";
        }
        mutableMap->ClearTemporaryObstacles();
    }

    // Path 3: Try to force path via LOWER lane
    if (alternativePaths.size() < static_cast<size_t>(numPathsToAttempt)) {
        mutableMap->ClearTemporaryObstacles();
        wss_paths << L"  Generating Path 3 (attempting lower lane by blocking mid/high hileras)...\n";
        addObstaclesInRowRange(h_sup_r1, h_sup_r2, cols_hileras_start, cols_hileras_end);    
        addObstaclesInRow(h_mid_r, cols_hileras_start, cols_hileras_end);    
        if (h_mid_r - 1 > h_sup_r2) addObstaclesInRow(h_mid_r - 1, cols_hileras_start, cols_hileras_end); // Bloqueo agresivo adyacente

        std::vector<std::pair<int, int>> path3 = currentMap->GetPath(enemyEntryPoint, bridgeLocation);
        if (!path3.empty() && path3 != path1 && (alternativePaths.size() < 2 || path3 != alternativePaths[1])) {
            alternativePaths.push_back(path3);
            wss_paths << L"    Added Path 3 (Lower). Length: " << path3.size() << L"\n";
        } else if(!path3.empty()){
            wss_paths << L"    Path 3 was duplicate or A* could not find a distinct lower path.\n";
        } else {
            wss_paths << L"    Path 3 (Lower) FAILED to generate.\n";
        }
        mutableMap->ClearTemporaryObstacles();
    }

    if (alternativePaths.empty() && currentMap) { 
         wss_paths << L"  Fallback: No paths generated. Adding emergency optimal path (ignoring spots for this one call if necessary, but GetPath should handle it).\n";
         // Si GetPath sigue sin encontrar nada, hay un problema de diseño del mapa o A*
         std::vector<std::pair<int, int>> emergencyPath = currentMap->GetPath(enemyEntryPoint, bridgeLocation); 
         if(!emergencyPath.empty()) alternativePaths.push_back(emergencyPath);
    }

    if (!alternativePaths.empty()) {
        initialEnemyPath = alternativePaths[0]; 
    } else {
        OutputDebugStringW(L"CRITICAL ERROR in GenerateAlternativePaths: No paths could be generated for enemies! Map might be unnavigable.\n");
        // Considerar un estado de error o un camino por defecto muy simple si esto ocurre.
    }
    
    wss_paths << L"GeneticAlgorithm::GenerateAlternativePaths - Finished. Total unique paths found: " << alternativePaths.size() << L"\n";
    OutputDebugStringW(wss_paths.str().c_str());
} 

int GeneticAlgorithm::SelectParents(const std::vector<Enemy>& population, std::vector<Enemy>& parents) {
    int desiredParentPoolSize = static_cast<int>(getMaxFromDouble(8.0, static_cast<double>(populationSize) / 2.0)); // Explicit casts
    while (parents.size() < static_cast<size_t>(desiredParentPoolSize) && parents.size() < population.size()) {
        // ... (resto de SelectParents)
    }
    return desiredParentPoolSize;
}

void GeneticAlgorithm::RebalanceEnemyTypes(std::vector<Enemy>& enemiesToRebalance, int targetPerType) {
    // ... (inicio de la función, logs, agrupación por tipo)

    // Fill missing slots with new random enemies of this type
    int needed = targetPerType - numAddedForThisType;
    for (int j = 0; j < needed; ++j) {
        float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
        float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
        
        std::vector<std::pair<int, int>> chosenPathForNewEnemy; // Declare with definite type
        if (!alternativePaths.empty()) {
            chosenPathForNewEnemy = alternativePaths[(pathAssignIndex++) % alternativePaths.size()];
        } else if (!initialEnemyPath.empty()) {
            chosenPathForNewEnemy = initialEnemyPath;
            wss_rebalance << L"    WARN: Creating new enemy in Rebalance, alternativePaths empty, using initialEnemyPath.\n";
        } else {
            wss_rebalance << L"    CRITICAL ERROR: No paths available in RebalanceEnemyTypes for new enemy of type " << static_cast<int>(currentProcessingType) << L". Skipping creation.\n";
            // OutputDebugStringW(wss_rebalance.str().c_str()); // Log this critical state if it happens
            continue; // Cannot create enemy without a path
        }

        Enemy newEnemy(currentProcessingType, startX, startY, chosenPathForNewEnemy);
        newEnemy.Mutate(1.0f); 
        enemiesToRebalance.push_back(newEnemy);
    }
    // ... (resto de la función)
}

Enemy GeneticAlgorithm::CreateRandomEnemy() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> typeDist(0, 3); // Assuming 4 enemy types
    
    EnemyType types[] = { EnemyType::OGRE, EnemyType::DARK_ELF, EnemyType::HARPY, EnemyType::MERCENARY };
    EnemyType randomType = types[typeDist(gen)];
    
    float startX = static_cast<float>(enemyEntryPoint.second * CELL_SIZE + CELL_SIZE / 2);
    float startY = static_cast<float>(enemyEntryPoint.first * CELL_SIZE + CELL_SIZE / 2);
    
    std::vector<std::pair<int, int>> chosenPathForNewEnemy;
    if (!alternativePaths.empty()) {
        chosenPathForNewEnemy = alternativePaths[gen() % alternativePaths.size()]; // Use gen() for better randomness if std::rand() was an issue
    } else if (!initialEnemyPath.empty()) {
        chosenPathForNewEnemy = initialEnemyPath;
    } else {
        // CRITICAL: No path available. Return a dummy/default enemy or handle error.
        // For now, creating with an empty path, which might make it inactive.
        OutputDebugStringW(L"CRITICAL ERROR in CreateRandomEnemy: No paths available!\n");
    }

    Enemy enemy(randomType, startX, startY, chosenPathForNewEnemy);
    enemy.Mutate(1.0f); 
    return enemy;
} 