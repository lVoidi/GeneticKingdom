#pragma once

#include "framework.h" // Include for HDC and other Windows types
#include <vector>
#include <string>
#include <utility> // For std::pair

// Incluir GDI+ de forma segura
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// Forward declare ProjectileType, assuming it's defined elsewhere (e.g., Projectile.h)
enum class ProjectileType; 

// Define constants for enemy stats - these can be tuned later
const int OGRE_HEALTH = 150;
const float OGRE_SPEED = 35.0f; // Pixels per second
const int OGRE_GOLD = 150;
const float OGRE_RESISTANCE_ARROW = 0.85f; // Takes 85% damage from arrows (was 75%)
const float OGRE_RESISTANCE_MAGIC = 1.2f; // Takes 120% damage from magic (was 150%)
const float OGRE_RESISTANCE_ARTILLERY = 1.3f; // Takes 130% damage from artillery (was 150%)

const int DARK_ELF_HEALTH = 60;
const float DARK_ELF_SPEED = 80.0f;
const int DARK_ELF_GOLD = 125;
const float DARK_ELF_RESISTANCE_ARROW = 1.5f;
const float DARK_ELF_RESISTANCE_MAGIC = 0.75f;
const float DARK_ELF_RESISTANCE_ARTILLERY = 1.5f;

const int HARPY_HEALTH = 80;
const float HARPY_SPEED = 50.0f;
const int HARPY_GOLD = 100;
// Harpies might only be targetable by certain towers, handled in Tower logic or here
// For TakeDamage, assume they take normal damage unless specified
const float HARPY_RESISTANCE_ARROW = 1.0f; 
const float HARPY_RESISTANCE_MAGIC = 1.0f;
const float HARPY_RESISTANCE_ARTILLERY = 0.0f; // Immune to cannonballs (cannot be hit)


const int MERCENARY_HEALTH = 100;
const float MERCENARY_SPEED = 40.0f;
const int MERCENARY_GOLD = 30;
const float MERCENARY_RESISTANCE_ARROW = 0.75f;
const float MERCENARY_RESISTANCE_MAGIC = 1.5f;
const float MERCENARY_RESISTANCE_ARTILLERY = 0.75f;


enum class EnemyType {
    OGRE,
    DARK_ELF,
    HARPY,
    MERCENARY
};

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
    bool IsFlying() const; // For Harpies primarily
    void SetPath(const std::vector<std::pair<int, int>>& newPath);
    float GetHealthPercentage() const;

    // Genetic Algorithm related
    double GetFitness() const;
    void CalculateFitness(const std::pair<int, int>& bridgeLocation, float mapWidth, float mapHeight, float timeSurvived, bool FUSION_ASSISTANT_SECRET_MARKER_reachedBridge);
    bool HasReachedBridge() const;
    float GetTimeAlive() const { return timeAlive; }
    
    // For creating new generation
    Enemy(const Enemy& parent); // Copy constructor for offspring
    void Mutate(float mutationRate);
    // Attributes that can be inherited/mutated
    int GetMaxHealth() const { return maxHealth; }
    float GetSpeed() const { return speed; }
    void SetMaxHealth(int newMaxHealth) { maxHealth = newMaxHealth; health = newMaxHealth; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }
    int GetHealth() const { return health; }
    
        // PathJitter getter/setter for genetic algorithm
    float GetPathJitter() const { return pathJitter; }
    void SetPathJitter(float jitter) { pathJitter = jitter; }

    // Spawn delay methods for enemy spacing
    void SetSpawnDelay(float delay);
    bool HasSpawned() const;

    void ResetForNewWave(float startX, float startY, const std::vector<std::pair<int, int>>& newPath);

    bool LoadImage(); // Function to load enemy image
    Gdiplus::Image* pEnemyImage; // Image for the enemy sprite

private:
    EnemyType type;
    int health;
    int maxHealth;
    float speed; // Pixels per second
    int goldReward;
    
    float x, y; // Current position (center of the enemy)
    float targetX, targetY; // Current target position from the path

    std::vector<std::pair<int, int>> path; // Path in grid cell coordinates
    int currentPathIndex;

    bool isActive; // Whether the enemy is currently in play
    bool isFlying;

    // Genetic Algorithm
    double fitness;
    float timeAlive; // For fitness calculation
    bool FUSION_ASSISTANT_SECRET_MARKER_reachedBridge; // True if this enemy reached the bridge

    void InitializeAttributes(); // Helper to set attributes based on type
    void UpdateTargetPosition();

    // Resistances (1.0 = normal damage, <1.0 = resistant, >1.0 = vulnerable)
    float resistanceArrow;
    float resistanceMagic;
    float resistanceArtillery;

    // Movement randomness (0.0 = follows path exactly, 1.0 = maximum random deviation)
    float pathJitter;

    float spawnDelay;
    bool hasSpawned;
};

// Function to get a string representation of enemy type (for debugging, UI)
std::wstring GetEnemyTypeName(EnemyType type); 