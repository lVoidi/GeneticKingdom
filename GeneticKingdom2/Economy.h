#pragma once

#include <Windows.h>
#include "Tower.h" // Añadimos esta dependencia para usar TowerType

// Precios de las torres
#define ARCHER_COST 75     // Costo bajo para torre de arqueros
#define MAGE_COST 150      // Costo medio para torre de magos
#define GUNNER_COST 225    // Costo alto para torre de cañones
#define UPGRADE_LEVEL_2_COST 150  // Costo para mejorar a nivel 2
#define UPGRADE_LEVEL_3_COST 250  // Costo para mejorar a nivel 3

// Clase para gestionar la economía del juego
class Economy {
public:
    Economy();
    ~Economy();

    // Inicializa la economía con un valor inicial de oro
    void Initialize(int initialGold = 350);

    // Obtiene la cantidad actual de oro
    int GetGold() const;

    // Añade oro a la economía (por matar enemigos, etc.)
    void AddGold(int amount);

    // Intenta gastar una cantidad de oro (devuelve true si hay suficiente)
    bool SpendGold(int amount);

    // Obtiene el costo de construir una torre nueva según su tipo
    int GetTowerCost(TowerType type) const;

    // Obtiene el costo de mejorar una torre al nivel especificado
    int GetUpgradeCost(int currentLevel) const;

    // Dibuja el contador de oro en la pantalla
    void Draw(HDC hdc, int x, int y);

private:
    int gold;  // Cantidad actual de oro
}; 