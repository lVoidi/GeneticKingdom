#include "framework.h"
#include "Map.h"
#include <wincodec.h> // Para cargar imágenes

Map::Map() : numRows(0), numCols(0), entryRow(0), entryCol(0), gridPen(NULL), constructionSpotBrush(NULL),
            pConstructionImage(NULL), constructionState(ConstructionState::NONE), selectedRow(-1), selectedCol(-1) {
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
    
    // Dibujar todas las torres
    towerManager.DrawTowers(hdc, CELL_SIZE);
    
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
        
        // Comprobar clic en botón de torre Archer
        if (menuX >= 10 && menuX <= 90 && menuY >= 30 && menuY <= 70) {
            BuildTower(TowerType::ARCHER);
            constructionState = ConstructionState::NONE;
            return;
        }
        
        // Comprobar clic en botón de torre Gunner
        if (menuX >= 110 && menuX <= 190 && menuY >= 30 && menuY <= 70) {
            BuildTower(TowerType::GUNNER);
            constructionState = ConstructionState::NONE;
            return;
        }
        
        // Comprobar clic en botón de torre Mage
        if (menuX >= 210 && menuX <= 290 && menuY >= 30 && menuY <= 70) {
            BuildTower(TowerType::MAGE);
            constructionState = ConstructionState::NONE;
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
            UpgradeTower();
            constructionState = ConstructionState::NONE;
            return;
        }
        
        // Si se hizo clic fuera del menú, cancelar
        constructionState = ConstructionState::NONE;
        return;
    }
    
    // Modo normal - verificar si se hizo clic en un punto de construcción o torre
    if (IsConstructionSpot(row, col)) {
        // Si hay una torre en este punto, mostrar menú de mejora
        if (HasTower(row, col)) {
            Tower* tower = towerManager.GetTowerAt(row, col);
            if (tower && tower->CanUpgrade()) {
                selectedRow = row;
                selectedCol = col;
                constructionState = ConstructionState::UPGRADING;
            }
        }
        // Si no hay torre, mostrar menú de selección de torre
        else {
            selectedRow = row;
            selectedCol = col;
            constructionState = ConstructionState::SELECTING_TOWER;
        }
    } else {
        // Clic fuera de un punto de construcción, cancelar cualquier estado
        constructionState = ConstructionState::NONE;
        selectedRow = -1;
        selectedCol = -1;
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
        selectedRow * CELL_SIZE + 100
    };
    
    // Ajustar el rectángulo si está muy cerca del borde derecho
    if (menuRect.right > GetSystemMetrics(SM_CXSCREEN) - 20) {
        menuRect.left = selectedCol * CELL_SIZE - 310;
        menuRect.right = selectedCol * CELL_SIZE;
    }
    
    // Crear un fondo para el menú
    HBRUSH menuBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &menuRect, menuBrush);
    
    // Configurar el color del texto
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    
    // Título del menú
    WCHAR title[256];
    if (constructionState == ConstructionState::SELECTING_TOWER) {
        swprintf_s(title, L"Seleccione tipo de torre");
    } else if (constructionState == ConstructionState::UPGRADING) {
        swprintf_s(title, L"Mejorar torre");
    }
    
    RECT textRect = menuRect;
    textRect.top += 5;
    textRect.left += 10;
    DrawTextW(hdc, title, -1, &textRect, DT_LEFT);
    
    // Dibujar botones según el estado
    if (constructionState == ConstructionState::SELECTING_TOWER) {
        // Botón de torre Archer
        RECT archerButton = {
            menuRect.left + 10,
            menuRect.top + 30,
            menuRect.left + 90,
            menuRect.top + 70
        };
        HBRUSH archerBrush = CreateSolidBrush(RGB(0, 150, 0));
        FillRect(hdc, &archerButton, archerBrush);
        DeleteObject(archerBrush);
        
        RECT archerTextRect = archerButton;
        archerTextRect.top += 15;
        DrawTextW(hdc, L"Archer", -1, &archerTextRect, DT_CENTER);
        
        // Botón de torre Gunner
        RECT gunnerButton = {
            menuRect.left + 110,
            menuRect.top + 30,
            menuRect.left + 190,
            menuRect.top + 70
        };
        HBRUSH gunnerBrush = CreateSolidBrush(RGB(150, 0, 0));
        FillRect(hdc, &gunnerButton, gunnerBrush);
        DeleteObject(gunnerBrush);
        
        RECT gunnerTextRect = gunnerButton;
        gunnerTextRect.top += 15;
        DrawTextW(hdc, L"Gunner", -1, &gunnerTextRect, DT_CENTER);
        
        // Botón de torre Mage
        RECT mageButton = {
            menuRect.left + 210,
            menuRect.top + 30,
            menuRect.left + 290,
            menuRect.top + 70
        };
        HBRUSH mageBrush = CreateSolidBrush(RGB(0, 0, 150));
        FillRect(hdc, &mageButton, mageBrush);
        DeleteObject(mageBrush);
        
        RECT mageTextRect = mageButton;
        mageTextRect.top += 15;
        DrawTextW(hdc, L"Mage", -1, &mageTextRect, DT_CENTER);
    } else if (constructionState == ConstructionState::UPGRADING) {
        // Botón de mejora
        RECT upgradeButton = {
            menuRect.left + 10,
            menuRect.top + 30,
            menuRect.left + 190,
            menuRect.top + 70
        };
        HBRUSH upgradeBrush = CreateSolidBrush(RGB(150, 150, 0));
        FillRect(hdc, &upgradeButton, upgradeBrush);
        DeleteObject(upgradeBrush);
        
        RECT upgradeTextRect = upgradeButton;
        upgradeTextRect.top += 15;
        DrawTextW(hdc, L"Mejorar torre", -1, &upgradeTextRect, DT_CENTER);
    }
    
    DeleteObject(menuBrush);
}

// Actualiza la lógica del mapa
void Map::Update(float deltaTime) {
    // Actualizar torres
    towerManager.Update(deltaTime);
    
    // En el futuro, aquí se actualizará la lógica de movimiento de enemigos
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