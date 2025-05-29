/*
 * enemy.h - cabecera de la clase enemy
 *
 * este archivo define la clase enemy que maneja los enemigos del juego.
 * es un desastre de codigo pero funciona. define las constantes de stats,
 * resistencias y velocidades de cada tipo de enemigo. tambien tiene toda
 * la mierda del algoritmo genetico para que evolucionen entre oleadas.
 * 
 * Hay que balancear estas stats porque es un asco </3
 */

#pragma once

#include "framework.h" // para tipos de windows
#include <vector>
#include <string>
#include <utility> // para std::pair
#include <algorithm>  // For std::clamp

// gdi+ para dibujar los sprites
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// forward declaration del enum de proyectiles
enum class ProjectileType; 

// stats del ogro - tanque lento pero resistente
const int OGRE_HEALTH = 190;
const float OGRE_SPEED = 56.25f; // pixeles por segundo
const int OGRE_GOLD = 30; 
const float OGRE_RESISTANCE_ARROW = 0.85f;
const float OGRE_RESISTANCE_MAGIC = 1.1f;
const float OGRE_RESISTANCE_ARTILLERY = 1.1f;

// stats del elfo oscuro - rapido pero fragil
const int DARK_ELF_HEALTH = 100;
const float DARK_ELF_SPEED = 118.75f;
const int DARK_ELF_GOLD = 10;
const float DARK_ELF_RESISTANCE_ARROW = 1.2f;
const float DARK_ELF_RESISTANCE_MAGIC = 0.75f;
const float DARK_ELF_RESISTANCE_ARTILLERY = 1.5f;

// stats de la harpia - voladora que ignora artilleria
const int HARPY_HEALTH = 120;
const float HARPY_SPEED = 87.5f;
const int HARPY_GOLD = 15;
const float HARPY_RESISTANCE_ARROW = 1.0f;
const float HARPY_RESISTANCE_MAGIC = 1.0f;
const float HARPY_RESISTANCE_ARTILLERY = 0.0f;

// stats del mercenario - equilibrado pero caro
const int MERCENARY_HEALTH = 140;
const float MERCENARY_SPEED = 81.25f;
const int MERCENARY_GOLD = 25;
const float MERCENARY_RESISTANCE_ARROW = 0.75f;
const float MERCENARY_RESISTANCE_MAGIC = 1.2f;
const float MERCENARY_RESISTANCE_ARTILLERY = 0.75f;

// tipos de enemigos disponibles
enum class EnemyType {
    OGRE,
    DARK_ELF, 
    HARPY,
    MERCENARY
};

// Fallback clamp implementation for older compilers
namespace std {
    template<typename T>
    constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }
}

// la clase enemy maneja toda la logica de los enemigos
class Enemy {
public:
    Enemy(EnemyType type, float startX, float startY, const std::vector<std::pair<int, int>>& initialPath);
    ~Enemy();

    void Update(float deltaTime);
    void Draw(HDC hdc) const;
    void TakeDamage(int damageAmount, ProjectileType projectileType);

    bool IsAlive() const;
    bool IsActive() const;
    void SetActive(bool active);
    int GetGoldReward() const;
    float GetX() const;
    float GetY() const;
    EnemyType GetType() const;
    bool IsFlying() const;
    void SetPath(const std::vector<std::pair<int, int>>& newPath);
    float GetHealthPercentage() const;

    // funciones del algoritmo genetico
    double GetFitness() const;
    void CalculateFitness(const std::pair<int, int>& bridgeLocation, float mapWidth, float mapHeight, float timeSurvived, bool FUSION_ASSISTANT_SECRET_MARKER_reachedBridge);
    bool HasReachedBridge() const;
    float GetTimeAlive() const { return timeAlive; }
    
    // funciones para crear nuevas generaciones
    Enemy(const Enemy& parent);
    void Mutate(float mutationRate);
    int GetMaxHealth() const { return maxHealth; }
    float GetSpeed() const { return speed; }
    void SetMaxHealth(int newMaxHealth) { maxHealth = newMaxHealth; health = newMaxHealth; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }
    int GetHealth() const { return health; }
    
    float GetPathJitter() const { return pathJitter; }
    void SetPathJitter(float jitter) { pathJitter = jitter; }

    // funciones para el spawn delay
    void SetSpawnDelay(float delay);
    bool HasSpawned() const;

    void ResetForNewWave(float startX, float startY, const std::vector<std::pair<int, int>>& newPath);

    bool LoadImage();
    Gdiplus::Image* pEnemyImage;

    float GetArrowResistance() const { return resistanceArrow; }
    void SetArrowResistance(float resistance) { resistanceArrow = std::clamp(resistance, 0.1f, 3.0f); }
    
    float GetMagicResistance() const { return resistanceMagic; }
    void SetMagicResistance(float resistance) { resistanceMagic = std::clamp(resistance, 0.1f, 3.0f); }
    
    float GetArtilleryResistance() const { return resistanceArtillery; }
    void SetArtilleryResistance(float resistance) { resistanceArtillery = std::clamp(resistance, 0.0f, 3.0f); }

private:
    EnemyType type;
    int health;
    int maxHealth;
    float speed;
    int goldReward;
    
    float x, y;
    float targetX, targetY;

    std::vector<std::pair<int, int>> path;
    int currentPathIndex;

    bool isActive;
    bool isFlying;

    double fitness;
    float timeAlive;
    bool FUSION_ASSISTANT_SECRET_MARKER_reachedBridge;

    void InitializeAttributes();
    void UpdateTargetPosition();

    float resistanceArrow;
    float resistanceMagic;
    float resistanceArtillery;

    float pathJitter;

    float subTargetX, subTargetY;
    bool hasSubTarget;
    float timeSinceLastSubTargetRecalc;

    float spawnDelay;
    bool hasSpawned;
};

// funcion para obtener el nombre del tipo de enemigo
std::wstring GetEnemyTypeName(EnemyType type);