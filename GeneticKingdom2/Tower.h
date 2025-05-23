#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// Incluir el gestor de proyectiles
#include "Projectile.h"

// Forward declaration para evitar inclusión circular
struct DummyTarget;
class Enemy; // Forward declare Enemy class

// Definición de tipos de torre
enum class TowerType {
    ARCHER,    // Torre de arqueros (bajo costo, alto alcance, poco daño, tiempo de recarga bajo)
    MAGE,      // Torre de magos (costo medio, alcance medio, daño medio, tiempo de recarga medio)
    GUNNER     // Torre de cañones (costo alto, alcance bajo, daño alto, tiempo de recarga alto)
};

// Niveles de torre
enum class TowerLevel {
    LEVEL_1 = 1,
    LEVEL_2 = 2,
    LEVEL_3 = 3
};

// Clase para manejar una torre individual
class Tower {
public:
    Tower(TowerType type, int row, int col);
    ~Tower();

    // Dibuja la torre en la posición especificada
    void Draw(HDC hdc, int cellSize);
    
    // Dibuja el rango de ataque de la torre
    void DrawRange(HDC hdc, int cellSize);

    // Mejora la torre al siguiente nivel si es posible
    bool Upgrade();

    // Obtiene el tipo de torre
    TowerType GetType() const;

    // Obtiene el nivel actual de la torre
    TowerLevel GetLevel() const;

    // Obtiene la posición de la torre (fila)
    int GetRow() const;

    // Obtiene la posición de la torre (columna)
    int GetCol() const;

    // Comprueba si la torre puede ser mejorada
    bool CanUpgrade() const;

    // Obtiene el rango de ataque (en celdas)
    int GetRange() const;

    // Obtiene la velocidad de ataque (disparos por segundo)
    float GetAttackSpeed() const;

    // Obtiene el daño base
    int GetDamage() const;

    // Actualiza la lógica de la torre e intenta disparar si es posible (hacia la dirección por defecto)
    void Update(float deltaTime, ProjectileManager& projectileManager, int cellSize);
    
    // Actualiza la lógica de la torre e intenta disparar hacia el enemigo más cercano
    void Update(float deltaTime, ProjectileManager& projectileManager, int cellSize, const std::vector<Enemy>& enemies);
    
    // Muestra/oculta el rango de la torre
    void SetShowRange(bool show);
    
    // Comprueba si el rango está visible
    bool IsShowingRange() const;

private:
    // Carga la imagen adecuada para el tipo y nivel de torre
    bool LoadImage();
    
    // Obtiene el tipo de proyectil para esta torre
    ProjectileType GetProjectileType() const;

    TowerType type;                // Tipo de torre
    TowerLevel level;              // Nivel actual de la torre
    int row;                       // Fila en la cuadrícula
    int col;                       // Columna en la cuadrícula
    float attackCooldown;          // Tiempo restante para el próximo ataque
    Gdiplus::Image* pTowerImage;   // Imagen de la torre
    bool showRange;                // Indica si se debe mostrar el rango
};

// Gestor de torres para el juego
class TowerManager {
public:
    TowerManager();
    ~TowerManager();

    // Inicializa el gestor de torres
    void Initialize();

    // Agrega una nueva torre en la posición especificada
    bool AddTower(TowerType type, int row, int col);

    // Mejora la torre en la posición especificada
    bool UpgradeTower(int row, int col);

    // Dibuja todas las torres
    void DrawTowers(HDC hdc, int cellSize);
    
    // Dibuja los rangos de las torres
    void DrawTowerRanges(HDC hdc, int cellSize);

    // Comprueba si hay una torre en la posición especificada
    bool HasTower(int row, int col) const;

    // Obtiene una referencia a la torre en la posición especificada (o nullptr si no hay)
    Tower* GetTowerAt(int row, int col) const;

    // Actualiza la lógica de todas las torres (disparo por defecto)
    void Update(float deltaTime, ProjectileManager& projectileManager, int cellSize);
    
    // Actualiza la lógica de todas las torres (apuntando a enemigos)
    void Update(float deltaTime, ProjectileManager& projectileManager, int cellSize, const std::vector<Enemy>& enemies);
    
    // Activa la visualización del rango para una torre específica
    void ShowRangeForTower(int row, int col);
    
    // Oculta todos los rangos
    void HideAllRanges();
    
    // Obtiene el número total de torres
    size_t GetTowerCount() const { return towers.size(); }

private:
    std::vector<Tower*> towers;  // Vector de torres
}; 