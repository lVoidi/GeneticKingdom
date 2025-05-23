#include "framework.h"
#include "Map.h"
#include "Enemy.h" // Ensure full Enemy definition is available
#include <wincodec.h> // Para cargar imágenes
#include <vector>
#include <queue>    // For std::priority_queue
#include <cmath>    // For sqrt, fabs
#include <algorithm> // For std::reverse
#include <map> // For parent tracking or cost storage if not using 2D vector

// Define M_SQRT2 if not available (e.g. MSVC)
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880f
#endif

namespace {
    struct AStarNode {
        int row, col;
        float gCost; // Cost from start to this node
        float hCost; // Heuristic cost from this node to end
        float fCost; // gCost + hCost
        std::pair<int, int> parent; // Parent's coordinates

        AStarNode(int r, int c, float g, float h, std::pair<int, int> p = {-1, -1})
            : row(r), col(c), gCost(g), hCost(h), fCost(g + h), parent(p) {}

        // For priority queue: we want the smallest fCost
        bool operator>(const AStarNode& other) const {
            return fCost > other.fCost;
        }
    };

    float CalculateHeuristic(int r1, int c1, int r2, int c2) {
        float dr = static_cast<float>(r1 - r2);
        float dc = static_cast<float>(c1 - c2);
        return std::sqrt(dr * dr + dc * dc); // Euclidean distance
    }
}

Map::Map() : numRows(0), numCols(0), entryRow(0), entryCol(0), gridPen(NULL), constructionSpotBrush(NULL),
            pConstructionImage(NULL), constructionState(ConstructionState::NONE), selectedRow(-1), selectedCol(-1),
            rng(std::random_device()()) { // Inicializar el generador de números aleatorios
    // Crear un pincel para dibujar la cuadrícula (gris claro)
    gridPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
    // Crear un pincel para los puntos de construcción (amarillo con transparencia)
    constructionSpotBrush = CreateSolidBrush(RGB(255, 255, 0));
}

Map::~Map() {
    // Liberar recursos
    if (gridPen) {
        DeleteObject(gridPen);
        gridPen = NULL;
    }
    if (constructionSpotBrush) {
        DeleteObject(constructionSpotBrush);
        constructionSpotBrush = NULL;
    }
    if (pConstructionImage) {
        // Solo marcamos como NULL, ya que GDI+ se encargará de liberarlo
        // cuando se llame a GdiplusShutdown en la aplicación principal
        pConstructionImage = NULL;
    }
}

void Map::Initialize(int screenWidth, int screenHeight) {
    // Calcular el número de filas y columnas basado en el tamaño de pantalla
    numRows = screenHeight / CELL_SIZE;
    numCols = screenWidth / CELL_SIZE;

    // Inicializar la matriz de la cuadrícula
    grid.resize(numRows);
    for (int i = 0; i < numRows; i++) {
        grid[i].resize(numCols);
    }

    // Por defecto, establecer el punto de entrada en la parte izquierda (a la mitad)
    entryRow = numRows / 2;
    entryCol = 0;
    grid[entryRow][entryCol].isEntryPoint = true;

    // Por defecto, establecer el puente en la parte derecha
    int bridgeWidth = numCols / 10; 
    int bridgeStart = numCols - bridgeWidth;
    int bridgeHeight = numRows / 10; 
    int bridgeTop = (numRows - bridgeHeight) / 2;

    // Marcar las celdas del puente
    for (int row = bridgeTop; row < bridgeTop + bridgeHeight; row++) {
        for (int col = bridgeStart; col < numCols; col++) {
            grid[row][col].isBridge = true;
        }
    }

    // Cargar los espacios predeterminados para construcción
    LoadConstructionSpots();
    
    // Cargar la imagen de construcción
    LoadConstructionImage();
    
    // Inicializar la economía con 500 monedas
    economy.Initialize(500);
}

bool Map::LoadConstructionImage() {
    // Primero liberamos cualquier imagen previamente cargada
    if (pConstructionImage) {
        delete pConstructionImage;
        pConstructionImage = NULL;
    }
    
    // Lista de posibles rutas para la imagen
    const WCHAR* possiblePaths[] = {
        L"Assets\\Towers\\Construction.png",
        L"..\\GeneticKingdom2\\Assets\\Towers\\Construction.png",
        L"GeneticKingdom2\\Assets\\Towers\\Construction.png",
        L"C:\\Users\\Admin\\source\\repos\\GeneticKingdom2\\GeneticKingdom2\\Assets\\Towers\\Construction.png"
    };
    
    // Intentar cargar la imagen desde una de las rutas posibles
    for (const WCHAR* path : possiblePaths) {
        pConstructionImage = Gdiplus::Image::FromFile(path);
        if (pConstructionImage && pConstructionImage->GetLastStatus() == Gdiplus::Ok) {
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de construcción cargada correctamente: %s\n", path);
            OutputDebugStringW(debugMsg);
            return true;
        } else if (pConstructionImage) {
            delete pConstructionImage;
            pConstructionImage = NULL;
        }
    }
    
    // Si no se pudo cargar la imagen, obtener la ruta del ejecutable e intentar otras rutas
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    
    // Obtener la carpeta del ejecutable
    WCHAR* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = L'\0';
        
        // Construir rutas completas
        WCHAR fullPath[MAX_PATH];
        
        // Intentar con ruta relativa desde el ejecutable
        wcscpy_s(fullPath, exePath);
        wcscat_s(fullPath, L"Assets\\Towers\\Construction.png");
        pConstructionImage = Gdiplus::Image::FromFile(fullPath);
        
        if (pConstructionImage && pConstructionImage->GetLastStatus() == Gdiplus::Ok) {
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de construcción cargada correctamente: %s\n", fullPath);
            OutputDebugStringW(debugMsg);
            return true;
        } else if (pConstructionImage) {
            delete pConstructionImage;
            pConstructionImage = NULL;
        }
    }
    
    OutputDebugStringW(L"Error al cargar la imagen de construcción\n");
    pConstructionImage = NULL; // Asegurarse de que el puntero sea NULL si falló la carga
    return false;
}

void Map::LoadConstructionSpots() {
    // Limpiar spots de construcción existentes
    constructionSpots.clear();

    // Definir puntos de construcción predeterminados
    // Estos puntos están distribuidos estratégicamente alrededor del camino principal
    
    // Puntos a lo largo del lado izquierdo
    constructionSpots.push_back(std::make_pair(numRows / 4, numCols / 10));
    constructionSpots.push_back(std::make_pair(numRows / 2 - 3, numCols / 10));
    constructionSpots.push_back(std::make_pair(numRows / 2 + 3, numCols / 10));
    constructionSpots.push_back(std::make_pair(3 * numRows / 4, numCols / 10));

    // Puntos en el centro
    constructionSpots.push_back(std::make_pair(numRows / 4, numCols / 3));
    constructionSpots.push_back(std::make_pair(numRows / 2, numCols / 3));
    constructionSpots.push_back(std::make_pair(3 * numRows / 4, numCols / 3));
    
    constructionSpots.push_back(std::make_pair(numRows / 4, numCols / 2));
    constructionSpots.push_back(std::make_pair(numRows / 2, numCols / 2));
    constructionSpots.push_back(std::make_pair(3 * numRows / 4, numCols / 2));
    
    // Puntos cerca del puente (derecha)
    constructionSpots.push_back(std::make_pair(numRows / 4, 2 * numCols / 3));
    constructionSpots.push_back(std::make_pair(numRows / 2 - 3, 2 * numCols / 3));
    constructionSpots.push_back(std::make_pair(numRows / 2 + 3, 2 * numCols / 3));
    constructionSpots.push_back(std::make_pair(3 * numRows / 4, 2 * numCols / 3));

    // Marcar las celdas correspondientes como puntos de construcción en la grilla
    for (const auto& spot : constructionSpots) {
        int row = spot.first;
        int col = spot.second;
        
        // Asegurarse de que el punto está dentro de los límites
        if (row >= 0 && row < numRows && col >= 0 && col < numCols) {
            grid[row][col].isConstructionSpot = true;
        }
    }
}

void Map::Draw(HDC hdc) {
    // Guardar el pincel original
    HPEN oldPen = (HPEN)SelectObject(hdc, gridPen);

    // Dibujar la cuadrícula
    for (int row = 0; row <= numRows; row++) {
        // Líneas horizontales
        MoveToEx(hdc, 0, row * CELL_SIZE, NULL);
        LineTo(hdc, numCols * CELL_SIZE, row * CELL_SIZE);
    }

    for (int col = 0; col <= numCols; col++) {
        // Líneas verticales
        MoveToEx(hdc, col * CELL_SIZE, 0, NULL);
        LineTo(hdc, col * CELL_SIZE, numRows * CELL_SIZE);
    }

    // Dibujar puntos de construcción con la imagen usando GDI+
    if (pConstructionImage && pConstructionImage->GetLastStatus() == Gdiplus::Ok) {
        try {
            Gdiplus::Graphics graphics(hdc);
            
            // Establecer la calidad de dibujo
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
            graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            
            for (const auto& spot : constructionSpots) {
                int row = spot.first;
                int col = spot.second;
                
                // Solo mostrar imagen de construcción si no hay una torre
                if (!HasTower(row, col)) {
                    // Dibujar la imagen en la posición de la celda
                    graphics.DrawImage(
                        pConstructionImage,
                        Gdiplus::Rect(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE), // Destino
                        0, 0, pConstructionImage->GetWidth(), pConstructionImage->GetHeight(), // Fuente
                        Gdiplus::UnitPixel
                    );
                }
            }
        } catch (...) {
            // Si ocurre alguna excepción al dibujar, descartar la imagen
            OutputDebugStringW(L"Error al dibujar la imagen de construcción\n");
            pConstructionImage = NULL;
        }
    } else {
        // Si no se pudo cargar la imagen, dibujar los puntos de construcción con un color sólido
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, constructionSpotBrush);
        for (const auto& spot : constructionSpots) {
            int row = spot.first;
            int col = spot.second;
            
            // Solo mostrar imagen de construcción si no hay una torre
            if (!HasTower(row, col)) {
                RECT spotRect = { col * CELL_SIZE, row * CELL_SIZE,
                                (col + 1) * CELL_SIZE, (row + 1) * CELL_SIZE };
                FillRect(hdc, &spotRect, constructionSpotBrush);
            }
        }
        SelectObject(hdc, oldBrush);
    }
    
    // Dibujar los objetivos dummy
    DrawDummyTargets(hdc);
    
    // Dibujar los rangos de las torres
    towerManager.DrawTowerRanges(hdc, CELL_SIZE);
    
    // Dibujar todas las torres
    towerManager.DrawTowers(hdc, CELL_SIZE);
    
    // Dibujar todos los proyectiles
    projectileManager.DrawProjectiles(hdc);
    
    // Dibujar punto de entrada (rojo)
    HBRUSH entryBrush = CreateSolidBrush(RGB(255, 0, 0));
    RECT entryRect = { entryCol * CELL_SIZE, entryRow * CELL_SIZE,
                      (entryCol + 1) * CELL_SIZE, (entryRow + 1) * CELL_SIZE };
    FillRect(hdc, &entryRect, entryBrush);
    DeleteObject(entryBrush);

    // Dibujar puente (azul)
    HBRUSH bridgeBrush = CreateSolidBrush(RGB(0, 0, 255));
    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
            if (grid[row][col].isBridge) {
                RECT bridgeRect = { col * CELL_SIZE, row * CELL_SIZE,
                                   (col + 1) * CELL_SIZE, (row + 1) * CELL_SIZE };
                FillRect(hdc, &bridgeRect, bridgeBrush);
            }
        }
    }
    DeleteObject(bridgeBrush);

    // Dibujar el contador de oro en la esquina superior derecha
    economy.Draw(hdc, GetSystemMetrics(SM_CXSCREEN) - 220, 20);

    // Dibujar menú de construcción si está activo
    if (constructionState != ConstructionState::NONE) {
        DrawConstructionMenu(hdc);
    }

    // Restaurar el pincel original
    SelectObject(hdc, oldPen);
}

int Map::GetNumCols() const {
    return numCols;
}

int Map::GetNumRows() const {
    return numRows;
}

int Map::GetGridWidth() const {
    return numCols;
}

int Map::GetGridHeight() const {
    return numRows;
}

bool Map::IsCellOccupied(int row, int col) const {
    // Verificar que las coordenadas estén dentro de los límites
    if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
        return true; // Fuera de límites se considera ocupado
    }
    return grid[row][col].occupied;
}

void Map::SetCellOccupied(int row, int col, bool occupied) {
    // Verificar que las coordenadas estén dentro de los límites
    if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
        return;
    }
    grid[row][col].occupied = occupied;
}

void Map::SetEntryPoint(int row, int col) {
    // Limpiar el punto de entrada anterior
    if (entryRow >= 0 && entryRow < numRows && entryCol >= 0 && entryCol < numCols) {
        grid[entryRow][entryCol].isEntryPoint = false;
    }

    // Establecer el nuevo punto de entrada
    if (row >= 0 && row < numRows && col >= 0 && col < numCols) {
        entryRow = row;
        entryCol = col;
        grid[entryRow][entryCol].isEntryPoint = true;
    }
}

void Map::SetBridge(int startRow, int startCol, int width) {
    // Limpiar el puente anterior
    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
            grid[row][col].isBridge = false;
        }
    }

    // Establecer el nuevo puente
    int bridgeHeight = numRows / 5; // 20% del alto de la pantalla
    for (int row = startRow; row < startRow + bridgeHeight && row < numRows; row++) {
        for (int col = startCol; col < startCol + width && col < numCols; col++) {
            grid[row][col].isBridge = true;
        }
    }
}

bool Map::IsConstructionSpot(int row, int col) const {
    // Verificar que las coordenadas estén dentro de los límites
    if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
        return false;
    }
    return grid[row][col].isConstructionSpot;
}

std::vector<std::pair<int, int>> Map::GetConstructionSpots() const {
    return constructionSpots;
}

// Procesa el clic en el mapa
void Map::HandleClick(int x, int y) {
    // Convertir coordenadas de pantalla a coordenadas de celda
    int col = x / CELL_SIZE;
    int row = y / CELL_SIZE;
    
    // Si estamos en modo de selección de torre
    if (constructionState == ConstructionState::SELECTING_TOWER) {
        // Verificar si se hizo clic en una de las opciones del menú
        
        // Las coordenadas son relativas al menú, no al mapa
        int menuX = x - (selectedCol * CELL_SIZE + CELL_SIZE + 10);
        int menuY = y - (selectedRow * CELL_SIZE);
        
        // Comprobar clic en botón de torre Archer - Actualizado para el botón más alto
        if (menuX >= 10 && menuX <= 90 && menuY >= 45 && menuY <= 95) {
            // Verificar si hay suficiente oro
            if (economy.SpendGold(economy.GetTowerCost(TowerType::ARCHER))) {
                BuildTower(TowerType::ARCHER);
                constructionState = ConstructionState::NONE;
            }
            return;
        }
        
        // Comprobar clic en botón de torre Gunner - Actualizado para el botón más alto
        if (menuX >= 110 && menuX <= 190 && menuY >= 45 && menuY <= 95) {
            // Verificar si hay suficiente oro
            if (economy.SpendGold(economy.GetTowerCost(TowerType::GUNNER))) {
                BuildTower(TowerType::GUNNER);
                constructionState = ConstructionState::NONE;
            }
            return;
        }
        
        // Comprobar clic en botón de torre Mage - Actualizado para el botón más alto
        if (menuX >= 210 && menuX <= 290 && menuY >= 45 && menuY <= 95) {
            // Verificar si hay suficiente oro
            if (economy.SpendGold(economy.GetTowerCost(TowerType::MAGE))) {
                BuildTower(TowerType::MAGE);
                constructionState = ConstructionState::NONE;
            }
            return;
        }
        
        // Si se hizo clic fuera del menú, cancelar
        constructionState = ConstructionState::NONE;
        return;
    }
    
    // Si estamos en modo de mejora de torre
    if (constructionState == ConstructionState::UPGRADING) {
        // Verificar si se hizo clic en el botón de mejora
        
        // Las coordenadas son relativas al menú, no al mapa
        int menuX = x - (selectedCol * CELL_SIZE + CELL_SIZE + 10);
        int menuY = y - (selectedRow * CELL_SIZE);
        
        // Comprobar clic en botón de mejora
        if (menuX >= 10 && menuX <= 190 && menuY >= 30 && menuY <= 70) {
            Tower* tower = towerManager.GetTowerAt(selectedRow, selectedCol);
            if (tower) {
                int currentLevel = static_cast<int>(tower->GetLevel());
                int upgradeCost = economy.GetUpgradeCost(currentLevel);
                
                // Verificar si hay suficiente oro
                if (economy.SpendGold(upgradeCost)) {
                    UpgradeTower();
                }
            }
            constructionState = ConstructionState::NONE;
            return;
        }
        
        // Si se hizo clic fuera del menú, cancelar
        constructionState = ConstructionState::NONE;
        return;
    }
    
    // Modo normal - verificar si se hizo clic en un punto de construcción o torre
    if (IsConstructionSpot(row, col)) {
        // Si hay una torre en este punto, mostrar menú de mejora y su rango
        if (HasTower(row, col)) {
            Tower* tower = towerManager.GetTowerAt(row, col);
            if (tower && tower->CanUpgrade()) {
                selectedRow = row;
                selectedCol = col;
                constructionState = ConstructionState::UPGRADING;
                
                // Mostrar el rango de la torre seleccionada
                towerManager.ShowRangeForTower(row, col);
            }
        }
        // Si no hay torre, mostrar menú de selección de torre
        else {
            selectedRow = row;
            selectedCol = col;
            constructionState = ConstructionState::SELECTING_TOWER;
            
            // Ocultar todos los rangos al seleccionar un punto de construcción
            towerManager.HideAllRanges();
        }
    } else {
        // Si se hace clic en un área vacía que no es un punto de construcción,
        // crear un objetivo dummy en esa posición
        if (!IsConstructionSpot(row, col) && !HasTower(row, col) && 
            !grid[row][col].isBridge && !grid[row][col].isEntryPoint) {
            // Si ya había un estado de construcción activo, cancelarlo
            if (constructionState != ConstructionState::NONE) {
                constructionState = ConstructionState::NONE;
                selectedRow = -1;
                selectedCol = -1;
                towerManager.HideAllRanges();
            }
            
            // Añadir objetivo dummy
            AddDummyTarget(row, col);
        }
        
        // Clic fuera de un punto de construcción, cancelar cualquier estado
        else if (constructionState != ConstructionState::NONE) {
            constructionState = ConstructionState::NONE;
            selectedRow = -1;
            selectedCol = -1;
            
            // Ocultar todos los rangos
            towerManager.HideAllRanges();
        }
    }
}

// Dibuja el menú de construcción o mejora
void Map::DrawConstructionMenu(HDC hdc) {
    if (selectedRow < 0 || selectedCol < 0) {
        return;
    }
    
    // Crear un rectángulo de menú cerca del punto de construcción
    RECT menuRect = {
        selectedCol * CELL_SIZE + CELL_SIZE + 10,
        selectedRow * CELL_SIZE,
        selectedCol * CELL_SIZE + CELL_SIZE + 310,
        selectedRow * CELL_SIZE + 150  // Aumentar la altura del menú
    };
    
    // Ajustar el rectángulo si está muy cerca del borde derecho
    if (menuRect.right > GetSystemMetrics(SM_CXSCREEN) - 20) {
        menuRect.left = selectedCol * CELL_SIZE - 310;
        menuRect.right = selectedCol * CELL_SIZE;
    }
    
    // Crear un fondo para el menú
    HBRUSH menuBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &menuRect, menuBrush);
    
    // Definir una fuente grande para títulos
    HFONT titleFont = CreateFontW(
        20,                        // Altura (20 puntos)
        0,                         // Ancho (0 = auto)
        0,                         // Ángulo de escapamiento
        0,                         // Ángulo de orientación
        FW_BOLD,                   // Peso de la fuente (negrita)
        FALSE,                     // Cursiva
        FALSE,                     // Subrayado
        FALSE,                     // Tachado
        DEFAULT_CHARSET,           // Conjunto de caracteres
        OUT_OUTLINE_PRECIS,        // Precisión de salida
        CLIP_DEFAULT_PRECIS,       // Precisión de recorte
        CLEARTYPE_QUALITY,         // Calidad
        DEFAULT_PITCH | FF_SWISS,  // Familia y paso
        L"Arial"                   // Nombre de la fuente
    );
    
    // Definir una fuente para textos normales
    HFONT normalFont = CreateFontW(
        18,                        // Altura (18 puntos)
        0,                         // Ancho (0 = auto)
        0,                         // Ángulo de escapamiento
        0,                         // Ángulo de orientación
        FW_NORMAL,                 // Peso de la fuente (normal)
        FALSE,                     // Cursiva
        FALSE,                     // Subrayado
        FALSE,                     // Tachado
        DEFAULT_CHARSET,           // Conjunto de caracteres
        OUT_OUTLINE_PRECIS,        // Precisión de salida
        CLIP_DEFAULT_PRECIS,       // Precisión de recorte
        CLEARTYPE_QUALITY,         // Calidad
        DEFAULT_PITCH | FF_SWISS,  // Familia y paso
        L"Arial"                   // Nombre de la fuente
    );
    
    // Configurar el color del texto
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    
    // Seleccionar la fuente para el título
    HFONT oldFont = (HFONT)SelectObject(hdc, titleFont);
    
    // Título del menú
    WCHAR title[256];
    if (constructionState == ConstructionState::SELECTING_TOWER) {
        // Mostrar los costos diferenciados
        swprintf_s(title, L"Seleccione tipo de torre");
    } else if (constructionState == ConstructionState::UPGRADING) {
        Tower* tower = towerManager.GetTowerAt(selectedRow, selectedCol);
        if (tower) {
            int currentLevel = static_cast<int>(tower->GetLevel());
            int upgradeCost = economy.GetUpgradeCost(currentLevel);
            swprintf_s(title, L"Mejorar torre - Costo: %d", upgradeCost);
        } else {
            swprintf_s(title, L"Mejorar torre");
        }
    }
    
    RECT textRect = menuRect;
    textRect.top += 10;
    textRect.left += 10;
    DrawTextW(hdc, title, -1, &textRect, DT_LEFT);
    
    // Seleccionar la fuente para el texto normal
    SelectObject(hdc, normalFont);
    
    // Dibujar botones según el estado
    if (constructionState == ConstructionState::SELECTING_TOWER) {
        // Botón de torre Archer - Aumentamos la altura del botón
        RECT archerButton = {
            menuRect.left + 10,
            menuRect.top + 45,  // Ajustar posición
            menuRect.left + 90,
            menuRect.top + 95   // Aumentar la altura del botón
        };
        HBRUSH archerBrush = CreateSolidBrush(RGB(0, 150, 0));
        FillRect(hdc, &archerButton, archerBrush);
        DeleteObject(archerBrush);
        
        // Definimos un rectángulo para el nombre y otro para el precio
        RECT archerNameRect = archerButton;
        archerNameRect.top += 10;
        archerNameRect.bottom = archerNameRect.top + 20;
        DrawTextW(hdc, L"Archer", -1, &archerNameRect, DT_CENTER);
        
        RECT archerPriceRect = archerButton;
        archerPriceRect.top += 35;
        archerPriceRect.bottom = archerPriceRect.top + 20;
        WCHAR archerPrice[16];
        swprintf_s(archerPrice, L"%d", economy.GetTowerCost(TowerType::ARCHER));
        DrawTextW(hdc, archerPrice, -1, &archerPriceRect, DT_CENTER);
        
        // Botón de torre Gunner - Aumentamos la altura del botón
        RECT gunnerButton = {
            menuRect.left + 110,
            menuRect.top + 45,  // Ajustar posición
            menuRect.left + 190,
            menuRect.top + 95   // Aumentar la altura del botón
        };
        HBRUSH gunnerBrush = CreateSolidBrush(RGB(150, 0, 0));
        FillRect(hdc, &gunnerButton, gunnerBrush);
        DeleteObject(gunnerBrush);
        
        // Definimos un rectángulo para el nombre y otro para el precio
        RECT gunnerNameRect = gunnerButton;
        gunnerNameRect.top += 10;
        gunnerNameRect.bottom = gunnerNameRect.top + 20;
        DrawTextW(hdc, L"Gunner", -1, &gunnerNameRect, DT_CENTER);
        
        RECT gunnerPriceRect = gunnerButton;
        gunnerPriceRect.top += 35;
        gunnerPriceRect.bottom = gunnerPriceRect.top + 20;
        WCHAR gunnerPrice[16];
        swprintf_s(gunnerPrice, L"%d", economy.GetTowerCost(TowerType::GUNNER));
        DrawTextW(hdc, gunnerPrice, -1, &gunnerPriceRect, DT_CENTER);
        
        // Botón de torre Mage - Aumentamos la altura del botón
        RECT mageButton = {
            menuRect.left + 210,
            menuRect.top + 45,  // Ajustar posición
            menuRect.left + 290,
            menuRect.top + 95   // Aumentar la altura del botón
        };
        HBRUSH mageBrush = CreateSolidBrush(RGB(0, 0, 150));
        FillRect(hdc, &mageButton, mageBrush);
        DeleteObject(mageBrush);
        
        // Definimos un rectángulo para el nombre y otro para el precio
        RECT mageNameRect = mageButton;
        mageNameRect.top += 10;
        mageNameRect.bottom = mageNameRect.top + 20;
        DrawTextW(hdc, L"Mage", -1, &mageNameRect, DT_CENTER);
        
        RECT magePriceRect = mageButton;
        magePriceRect.top += 35;
        magePriceRect.bottom = magePriceRect.top + 20;
        WCHAR magePrice[16];
        swprintf_s(magePrice, L"%d", economy.GetTowerCost(TowerType::MAGE));
        DrawTextW(hdc, magePrice, -1, &magePriceRect, DT_CENTER);
    } else if (constructionState == ConstructionState::UPGRADING) {
        // Botón de mejora
        RECT upgradeButton = {
            menuRect.left + 10,
            menuRect.top + 45,  // Ajustar posición
            menuRect.left + 190,
            menuRect.top + 85   // Ajustar posición
        };
        HBRUSH upgradeBrush = CreateSolidBrush(RGB(150, 150, 0));
        FillRect(hdc, &upgradeButton, upgradeBrush);
        DeleteObject(upgradeBrush);
        
        RECT upgradeTextRect = upgradeButton;
        upgradeTextRect.top += 15;
        DrawTextW(hdc, L"Mejorar torre", -1, &upgradeTextRect, DT_CENTER);
    }
    
    // Mostrar monedas disponibles
    SetTextColor(hdc, RGB(255, 215, 0)); // Color dorado
    WCHAR goldText[64];
    swprintf_s(goldText, L"Monedas: %d", economy.GetGold());
    RECT goldRect = menuRect;
    goldRect.top += 115;  // Incrementar para evitar superposición con los botones más altos
    goldRect.left += 10;
    DrawTextW(hdc, goldText, -1, &goldRect, DT_LEFT);
    
    // Restaurar la fuente original
    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);
    DeleteObject(normalFont);
    
    DeleteObject(menuBrush);
}

// Actualiza la lógica del mapa
void Map::Update(float deltaTime, std::vector<Enemy>& enemies) {
    WCHAR debugMsg[256];
    swprintf_s(debugMsg, L"Map::Update - deltaTime: %.4f, Enemies: %zu, Torres: %zu\n", 
              deltaTime, enemies.size(), towerManager.GetTowerCount());
    OutputDebugStringW(debugMsg);
    
    // Actualizar torres y hacer que apunten a enemigos si existen
    // Placeholder: towerManager.Update will need to take the enemies vector
    towerManager.Update(deltaTime, projectileManager, CELL_SIZE, enemies); // Needs TowerManager to be updated
    
    projectileManager.Update(deltaTime);
    
    swprintf_s(debugMsg, L"Proyectiles activos: %zd\n", 
              projectileManager.GetProjectiles().size());
    OutputDebugStringW(debugMsg);
    
    projectileManager.CheckCollisions(enemies, CELL_SIZE, economy); 
    
    UpdateDummyTargets();
    
    if (rand() % 100 < 2 && dummyTargets.size() < 5) {
        GenerateRandomTargets(1);
    }
}

// Obtiene el estado de construcción actual
ConstructionState Map::GetConstructionState() const {
    return constructionState;
}

// Verifica si existe una torre en la posición indicada
bool Map::HasTower(int row, int col) const {
    return towerManager.HasTower(row, col);
}

// Construye una torre del tipo especificado en la celda seleccionada
bool Map::BuildTower(TowerType type) {
    if (selectedRow < 0 || selectedCol < 0) {
        return false;
    }
    
    // Marcar la celda como ocupada
    SetCellOccupied(selectedRow, selectedCol, true);
    
    // Construir la torre
    return towerManager.AddTower(type, selectedRow, selectedCol);
}

// Mejora la torre en la celda seleccionada
bool Map::UpgradeTower() {
    if (selectedRow < 0 || selectedCol < 0) {
        return false;
    }
    
    return towerManager.UpgradeTower(selectedRow, selectedCol);
}

// Obtiene una referencia a la economía
Economy& Map::GetEconomy() {
    return economy;
}

// Añade un objetivo dummy en la posición especificada
void Map::AddDummyTarget(int row, int col) {
    // Verificar que las coordenadas estén dentro de los límites
    if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
        return;
    }
    
    // Mensaje de depuración
    WCHAR debugMsg[256];
    swprintf_s(debugMsg, L"Añadiendo objetivo dummy en [%d,%d]\n", row, col);
    OutputDebugStringW(debugMsg);
    
    // Crear un color aleatorio para el objetivo
    COLORREF colors[] = {
        RGB(255, 0, 0),    // Rojo
        RGB(0, 0, 255),    // Azul
        RGB(255, 165, 0),  // Naranja
        RGB(128, 0, 128),  // Púrpura
        RGB(255, 255, 0)   // Amarillo
    };
    
    int colorIndex = rand() % 5;
    
    // Crear el objetivo y añadirlo al vector
    DummyTarget target;
    target.row = row;
    target.col = col;
    target.lifeTime = 500; // 500 frames de vida (aproximadamente 8-10 segundos)
    target.color = colors[colorIndex];
    
    dummyTargets.push_back(target);
    swprintf_s(debugMsg, L"Total de objetivos dummy: %zd\n", dummyTargets.size());
    OutputDebugStringW(debugMsg);
}

// Actualiza los objetivos dummy
void Map::UpdateDummyTargets() {
    // Actualizar tiempo de vida de los objetivos y eliminar los que han expirado
    auto it = dummyTargets.begin();
    while (it != dummyTargets.end()) {
        it->lifeTime--;
        
        // Si el tiempo de vida ha terminado, eliminar el objetivo
        if (it->lifeTime <= 0) {
            it = dummyTargets.erase(it);
        } else {
            ++it;
        }
    }
}

// Dibuja los objetivos dummy
void Map::DrawDummyTargets(HDC hdc) {
    for (const auto& target : dummyTargets) {
        // Crear un pincel con el color del objetivo
        HBRUSH targetBrush = CreateSolidBrush(target.color);
        
        // Dibujar el objetivo como un círculo
        Gdiplus::Graphics graphics(hdc);
        Gdiplus::SolidBrush brush(Gdiplus::Color(
            255, // Alpha
            GetRValue(target.color),
            GetGValue(target.color),
            GetBValue(target.color)
        ));
        
        // Centro de la celda
        float centerX = (target.col + 0.5f) * CELL_SIZE;
        float centerY = (target.row + 0.5f) * CELL_SIZE;
        
        // Dibujar el objetivo
        float radius = CELL_SIZE * 0.3f; // 30% del tamaño de la celda
        graphics.FillEllipse(&brush, centerX - radius, centerY - radius, radius * 2, radius * 2);
        
        // Dibujar un borde blanco
        Gdiplus::Pen pen(Gdiplus::Color(255, 255, 255, 255), 2);
        graphics.DrawEllipse(&pen, centerX - radius, centerY - radius, radius * 2, radius * 2);
        
        // Liberar recursos
        DeleteObject(targetBrush);
    }
}

// Obtiene los objetivos dummy
const std::vector<DummyTarget>& Map::GetDummyTargets() const {
    return dummyTargets;
}

// Genera objetivos dummy aleatorios
void Map::GenerateRandomTargets(int count) {
    // Distribuir uniformemente en el mapa, excepto en construcciones y puentes
    std::uniform_int_distribution<int> rowDist(1, numRows - 2);
    std::uniform_int_distribution<int> colDist(1, numCols - 2);
    
    for (int i = 0; i < count; i++) {
        int row = rowDist(rng);
        int col = colDist(rng);
        
        // Asegurarse de que no es un punto de construcción ni está ocupado
        if (!IsConstructionSpot(row, col) && !grid[row][col].isBridge && !grid[row][col].occupied) {
            AddDummyTarget(row, col);
        }
    }
}

// Placeholder for A* pathfinding. Replace with your actual A* implementation.
std::vector<std::pair<int, int>> Map::GetPath(std::pair<int, int> startCell, std::pair<int, int> endCell) const {
    std::vector<std::pair<int, int>> path;
    if (startCell == endCell) {
        path.push_back(startCell);
        return path;
    }
    if (startCell.first < 0 || startCell.first >= numRows || startCell.second < 0 || startCell.second >= numCols ||
        endCell.first < 0 || endCell.first >= numRows || endCell.second < 0 || endCell.second >= numCols) {
        return path; // Invalid start or end
    }

    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openList;
    
    // Using a 2D vector to store costs and visited status for quick access.
    // Initialize with high values.
    std::vector<std::vector<float>> gCosts(numRows, std::vector<float>(numCols, FLT_MAX));
    // Store parent coordinates to reconstruct path
    std::vector<std::vector<std::pair<int,int>>> parents(numRows, std::vector<std::pair<int,int>>(numCols, {-1,-1}));
    // Closed list: true if cell has been processed
    std::vector<std::vector<bool>> closedList(numRows, std::vector<bool>(numCols, false));

    float startHCost = CalculateHeuristic(startCell.first, startCell.second, endCell.first, endCell.second);
    openList.push(AStarNode(startCell.first, startCell.second, 0.0f, startHCost, {-1,-1}));
    gCosts[startCell.first][startCell.second] = 0.0f;

    // Possible moves (8 directions)
    int dr[] = {-1, 1, 0, 0, -1, -1, 1, 1}; // Row changes
    int dc[] = {0, 0, -1, 1, -1, 1, -1, 1}; // Col changes
    float moveCost[] = {1.0f, 1.0f, 1.0f, 1.0f, M_SQRT2, M_SQRT2, M_SQRT2, M_SQRT2}; // Cost for each move

    while(!openList.empty()) {
        AStarNode currentNode = openList.top();
        openList.pop();

        int r = currentNode.row;
        int c = currentNode.col;

        if (closedList[r][c]) {
            continue; // Already processed
        }
        closedList[r][c] = true;

        if (r == endCell.first && c == endCell.second) { // Goal reached
            std::pair<int,int> currentPathNode = endCell;
            while(currentPathNode.first != -1) { // Backtrack using parents
                path.push_back(currentPathNode);
                if (currentPathNode == startCell) break;
                currentPathNode = parents[currentPathNode.first][currentPathNode.second];
                if (path.size() > numRows * numCols * 2) return {}; // Safety break for very long/cyclic paths
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (int i = 0; i < 8; ++i) { // Check all 8 neighbors
            int nextR = r + dr[i];
            int nextC = c + dc[i];

            if (nextR >= 0 && nextR < numRows && nextC >= 0 && nextC < numCols && 
                !grid[nextR][nextC].occupied && !closedList[nextR][nextC]) {
                
                float tentativeGCost = gCosts[r][c] + moveCost[i];

                if (tentativeGCost < gCosts[nextR][nextC]) {
                    parents[nextR][nextC] = {r, c};
                    gCosts[nextR][nextC] = tentativeGCost;
                    float hCost = CalculateHeuristic(nextR, nextC, endCell.first, endCell.second);
                    openList.push(AStarNode(nextR, nextC, tentativeGCost, hCost, {r,c} ));
                }
            }
        }
    }
    return path; // No path found
}

std::pair<int, int> Map::GetBridgeGridLocation() const {
    // This should return a representative grid cell for the bridge, e.g., its center or entry.
    // Based on Initialize, bridge starts at (bridgeTop, bridgeStart)
    // For simplicity, returning the top-left-most cell of the bridge area.
    // You might want to refine this to be the center or a specific target cell within the bridge.
    int bridgeActualStartCol = numCols - (numCols / 10); // Recalculate or store bridgeStartCol if not a member
    int bridgeActualTopRow = (numRows - (numRows / 10)) / 2; // Recalculate or store bridgeTopRow
    int bridgeActualHeight = numRows / 10;

    // Return the center of the first column of the bridge area
    return std::make_pair(bridgeActualTopRow + bridgeActualHeight / 2, bridgeActualStartCol);
}

float Map::GetMapPixelWidth() const {
    return static_cast<float>(numCols * CELL_SIZE);
}

float Map::GetMapPixelHeight() const {
    return static_cast<float>(numRows * CELL_SIZE);
} 