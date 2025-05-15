#include "framework.h"
#include "Map.h"

Map::Map() : numRows(0), numCols(0), entryRow(0), entryCol(0), gridPen(NULL) {
    // Crear un pincel para dibujar la cuadrícula (gris claro)
    gridPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
}

Map::~Map() {
    // Liberar recursos
    if (gridPen) {
        DeleteObject(gridPen);
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