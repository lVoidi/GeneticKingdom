#pragma once

#include <vector>
#include <string>
#include <utility> // For std::pair
// Forward declare HDC for drawing, assuming windows.h will be included in .cpp
typedef void* HDC; 

// Forward declare ProjectileType, assuming it's defined elsewhere (e.g., Projectile.h)
enum class ProjectileType; 

// Define constants for enemy stats - these can be tuned later
const int OGRE_HEALTH = 200;
const float OGRE_SPEED = 20.0f; // Pixels per second
const int OGRE_GOLD = 20;
const float OGRE_RESISTANCE_ARROW = 0.5f; // Takes 50% damage from arrows
const float OGRE_RESISTANCE_MAGIC = 1.25f; // Takes 125% damage from magic
const float OGRE_RESISTANCE_ARTILLERY = 1.25f;

const int DARK_ELF_HEALTH = 75;
const float DARK_ELF_SPEED = 80.0f;
const int DARK_ELF_GOLD = 15;
const float DARK_ELF_RESISTANCE_ARROW = 1.25f;
const float DARK_ELF_RESISTANCE_MAGIC = 0.5f;
const float DARK_ELF_RESISTANCE_ARTILLERY = 1.25f;

const int HARPY_HEALTH = 100;
const float HARPY_SPEED = 50.0f;
const int HARPY_GOLD = 25;
// Harpies might only be targetable by certain towers, handled in Tower logic or here
// For TakeDamage, assume they take normal damage unless specified
const float HARPY_RESISTANCE_ARROW = 1.0f; 
const float HARPY_RESISTANCE_MAGIC = 1.0f;
const float HARPY_RESISTANCE_ARTILLERY = 0.0f; // Immune to cannonballs (cannot be hit)


const int MERCENARY_HEALTH = 120;
const float MERCENARY_SPEED = 40.0f;
const int MERCENARY_GOLD = 30;
const float MERCENARY_RESISTANCE_ARROW = 0.75f;
const float MERCENARY_RESISTANCE_MAGIC = 1.25f;
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
    void CalculateFitness(const std::pair<int, int>& bridgeLocation, float mapWidth, float mapHeight, float timeSurvived, bool reachedBridge);
    
    // For creating new generation
    Enemy(const Enemy& parent); // Copy constructor for offspring
    void Mutate(float mutationRate);
    // Attributes that can be inherited/mutated
    int GetMaxHealth() const { return maxHealth; }
    float GetSpeed() const { return speed; }
    void SetMaxHealth(int newMaxHealth) { maxHealth = newMaxHealth; health = newMaxHealth; }
    void SetSpeed(float newSpeed) { speed = newSpeed; }


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

    // Resistances (1.0 = normal damage, <1.0 = resistant, >1.0 = vulnerable)
    float resistanceArrow;
    float resistanceMagic;
    float resistanceArtillery;

    // Genetic Algorithm
    double fitness;
    float timeAlive; // For fitness calculation

    void InitializeAttributes(); // Helper to set attributes based on type
    void UpdateTargetPosition();
};

// Function to get a string representation of enemy type (for debugging, UI)
std::wstring GetEnemyTypeName(EnemyType type); 