#pragma once

#include <vector>
#include <Windows.h>

// Tamaño de cada celda en píxeles
#define CELL_SIZE 50

// Representa una celda en el mapa
struct Cell {
    bool occupied = false;     // Indica si la celda está ocupada por una torre
    bool isPath = false;       // Indica si la celda es parte del camino
    bool isEntryPoint = false; // Indica si es un punto de entrada de enemigos
    bool isBridge = false;     // Indica si es parte del puente del castillo
};

// Clase para gestionar el mapa del juego
class Map {
public:
    Map();
    ~Map();

    // Inicializa el mapa con el tamaño de la pantalla
    void Initialize(int screenWidth, int screenHeight);

    // Dibuja el mapa
    void Draw(HDC hdc);

    // Obtiene el número de columnas
    int GetNumCols() const;

    // Obtiene el número de filas
    int GetNumRows() const;

    // Verifica si una celda está ocupada
    bool IsCellOccupied(int row, int col) const;

    // Marca una celda como ocupada (para colocar torres)
    void SetCellOccupied(int row, int col, bool occupied);

    // Método para establecer el punto de entrada de enemigos
    void SetEntryPoint(int row, int col);

    // Método para establecer las celdas del puente
    void SetBridge(int startRow, int startCol, int width);

private:
    std::vector<std::vector<Cell>> grid;  // Matriz 2D para la cuadrícula
    int numRows;                          // Número de filas en la cuadrícula
    int numCols;                          // Número de columnas en la cuadrícula
    int entryRow;                         // Fila del punto de entrada
    int entryCol;                         // Columna del punto de entrada
    HPEN gridPen;                         // Pincel para dibujar la cuadrícula
}; 