#include "framework.h"
#include "Enemy.h"
#include "Projectile.h" 
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>

extern const int CELL_SIZE;
// const int CELL_SIZE = 50; // Temporary placeholder if not defined globally

Enemy::Enemy(EnemyType type, float startX, float startY, const std::vector<std::pair<int, int>>& initialPath)
    : type(type), x(startX), y(startY), path(initialPath), currentPathIndex(0), isActive(true), fitness(0.0), timeAlive(0.0f) {
    InitializeAttributes();
    health = maxHealth;
    if (!path.empty()) {
        UpdateTargetPosition();
    } else {
        targetX = x;
        targetY = y;
        isActive = false; 
    }
}

Enemy::Enemy(const Enemy& parent)
    : type(parent.type), 
      maxHealth(parent.maxHealth), 
      speed(parent.speed), 
      goldReward(parent.goldReward), 
      x(parent.x),
      y(parent.y),
      path(parent.path), 
      currentPathIndex(0), 
      isActive(true), 
      isFlying(parent.isFlying),
      resistanceArrow(parent.resistanceArrow),
      resistanceMagic(parent.resistanceMagic),
      resistanceArtillery(parent.resistanceArtillery),
      fitness(0.0),
      timeAlive(0.0f)
{
    health = maxHealth;
    if (!path.empty()) {
        UpdateTargetPosition();
    } else {
        targetX = x;
        targetY = y;
        isActive = false;
    }
    // Note: Path and start position for children might need specific logic 
    // depending on how new generations are introduced.
}


Enemy::~Enemy() {
}

void Enemy::InitializeAttributes() {
    isFlying = false; // Default to ground unit
    switch (type) {
    case EnemyType::OGRE:
        maxHealth = OGRE_HEALTH;
        speed = OGRE_SPEED;
        goldReward = OGRE_GOLD;
        resistanceArrow = OGRE_RESISTANCE_ARROW;
        resistanceMagic = OGRE_RESISTANCE_MAGIC;
        resistanceArtillery = OGRE_RESISTANCE_ARTILLERY;
        break;
    case EnemyType::DARK_ELF:
        maxHealth = DARK_ELF_HEALTH;
        speed = DARK_ELF_SPEED;
        goldReward = DARK_ELF_GOLD;
        resistanceArrow = DARK_ELF_RESISTANCE_ARROW;
        resistanceMagic = DARK_ELF_RESISTANCE_MAGIC;
        resistanceArtillery = DARK_ELF_RESISTANCE_ARTILLERY;
        break;
    case EnemyType::HARPY:
        maxHealth = HARPY_HEALTH;
        speed = HARPY_SPEED;
        goldReward = HARPY_GOLD;
        isFlying = true;
        resistanceArrow = HARPY_RESISTANCE_ARROW;
        resistanceMagic = HARPY_RESISTANCE_MAGIC;
        resistanceArtillery = HARPY_RESISTANCE_ARTILLERY; // Immune
        break;
    case EnemyType::MERCENARY:
        maxHealth = MERCENARY_HEALTH;
        speed = MERCENARY_SPEED;
        goldReward = MERCENARY_GOLD;
        resistanceArrow = MERCENARY_RESISTANCE_ARROW;
        resistanceMagic = MERCENARY_RESISTANCE_MAGIC;
        resistanceArtillery = MERCENARY_RESISTANCE_ARTILLERY;
        break;
    default:
        // Default to Ogre or handle as an error
        maxHealth = OGRE_HEALTH;
        speed = OGRE_SPEED;
        goldReward = OGRE_GOLD;
        resistanceArrow = 1.0f;
        resistanceMagic = 1.0f;
        resistanceArtillery = 1.0f;
        break;
    }
    health = maxHealth;
}

void Enemy::UpdateTargetPosition() {
    if (currentPathIndex < path.size()) {
        // Path stores grid cells, convert to pixel coordinates (center of cell)
        targetX = static_cast<float>(path[currentPathIndex].first * CELL_SIZE + CELL_SIZE / 2.0f);
        targetY = static_cast<float>(path[currentPathIndex].second * CELL_SIZE + CELL_SIZE / 2.0f);
    } else {
        // Reached end of path
        // The game logic should handle this, e.g., enemy reached goal
        isActive = false; 
    }
}

void Enemy::Update(float deltaTime) {
    if (!isActive || !IsAlive()) {
        return;
    }

    timeAlive += deltaTime;

    float dx = targetX - x;
    float dy = targetY - y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < speed * deltaTime) {
        x = targetX;
        y = targetY;
        currentPathIndex++;
        if (currentPathIndex < path.size()) {
            UpdateTargetPosition();
        } else {
            // Reached the end of the path
            isActive = false; // Mark as inactive, game logic will handle removal/damage to base
            // std::cout << "Enemy reached the end of its path." << std::endl;
        }
    } else {
        float moveX = (dx / distance) * speed * deltaTime;
        float moveY = (dy / distance) * speed * deltaTime;
        x += moveX;
        y += moveY;
    }
}

void Enemy::Draw(HDC hdc) const {
    if (!isActive || !IsAlive()) {
        return;
    }

    // Simple colored rectangle for now, replace with sprite later
    COLORREF color;
    switch (type) {
    case EnemyType::OGRE:
        color = RGB(139, 69, 19); // Brown
        break;
    case EnemyType::DARK_ELF:
        color = RGB(75, 0, 130);   // Indigo
        break;
    case EnemyType::HARPY:
        color = RGB(173, 216, 230); // Light Blue
        break;
    case EnemyType::MERCENARY:
        color = RGB(128, 128, 128); // Gray
        break;
    default:
        color = RGB(0, 0, 0);       // Black (error or default)
        break;
    }

    HBRUSH hBrush = CreateSolidBrush(color);
    HGDIOBJ hOldBrush = SelectObject(hdc, hBrush);

    // Draw enemy as a circle or square (using Rectangle for simplicity here)
    // Assuming x, y is the center, adjust to top-left for Rectangle
    int enemySize = CELL_SIZE / 2; // Adjust as needed
    Rectangle(hdc, 
              static_cast<int>(x - enemySize / 2.0f), 
              static_cast<int>(y - enemySize / 2.0f), 
              static_cast<int>(x + enemySize / 2.0f), 
              static_cast<int>(y + enemySize / 2.0f));

    // Draw health bar
    if (health < maxHealth) {
        int barWidth = enemySize;
        int barHeight = 5;
        int barX = static_cast<int>(x - barWidth / 2.0f);
        int barY = static_cast<int>(y - enemySize / 2.0f - barHeight - 2); // Above enemy

        // Background of health bar (red)
        HBRUSH hRedBrush = CreateSolidBrush(RGB(255, 0, 0));
        RECT bgRect = { barX, barY, barX + barWidth, barY + barHeight };
        FillRect(hdc, &bgRect, hRedBrush);
        DeleteObject(hRedBrush);

        // Current health (green)
        HBRUSH hGreenBrush = CreateSolidBrush(RGB(0, 255, 0));
        float healthPercentage = GetHealthPercentage();
        RECT fgRect = { barX, barY, barX + static_cast<int>(barWidth * healthPercentage), barY + barHeight };
        FillRect(hdc, &fgRect, hGreenBrush);
        DeleteObject(hGreenBrush);
    }

    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

void Enemy::TakeDamage(int damageAmount, ProjectileType projectileType) {
    if (!IsAlive()) return;

    float effectiveResistance = 1.0f;

    // Apply damage based on projectile type and enemy resistances
    switch (projectileType) {
        case ProjectileType::ARROW:
        case ProjectileType::FIREARROW: // Assuming fire arrows use arrow resistance
            effectiveResistance = resistanceArrow;
            // Harpies can be hit by arrows
            break;

        case ProjectileType::FIREBALL:
        case ProjectileType::PURPLEFIREBALL: // Assuming purple fireballs use magic resistance
            effectiveResistance = resistanceMagic;
            // Harpies can be hit by fireballs
            break;

        case ProjectileType::CANNONBALL:
        case ProjectileType::NUKEBOMB: // Assuming nuke bombs use artillery resistance
            effectiveResistance = resistanceArtillery;
            if (type == EnemyType::HARPY && resistanceArtillery == 0.0f) {
                return; // Harpies are immune to (cannot be hit by) artillery projectiles
            }
            break;

        default: // Unknown projectile type
            effectiveResistance = 1.0f; // Takes full damage
            break;
    }

    int finalDamage = static_cast<int>(static_cast<float>(damageAmount) * effectiveResistance);
    health -= finalDamage;
    
    if (health <= 0) {
        health = 0;
        isActive = false; // Mark as dead and inactive
    }
}

bool Enemy::IsAlive() const {
    return health > 0;
}

bool Enemy::IsActive() const {
    return isActive;
}

void Enemy::SetActive(bool active) {
    isActive = active;
}

int Enemy::GetGoldReward() const {
    return IsAlive() ? 0 : goldReward; // Only give gold if dead
}

float Enemy::GetX() const {
    return x;
}

float Enemy::GetY() const {
    return y;
}

EnemyType Enemy::GetType() const {
    return type;
}

bool Enemy::IsFlying() const {
    return isFlying;
}

void Enemy::SetPath(const std::vector<std::pair<int, int>>& newPath) {
    path = newPath;
    currentPathIndex = 0;
    if (!path.empty()) {
        UpdateTargetPosition();
        isActive = true; // Reactivate if it was inactive due to no path
    } else {
        isActive = false;
    }
}

float Enemy::GetHealthPercentage() const {
    if (maxHealth == 0) return 0.0f;
    return static_cast<float>(health) / static_cast<float>(maxHealth);
}

// --- Genetic Algorithm Related Methods ---
double Enemy::GetFitness() const {
    return fitness;
}

// I had to make my own functions because there are some 
// ambiguities in the std library. Fuck windows headers.
float getMaxFrom(float a, float b) {
    return a > b ? a : b;
}

float getMinFrom(float a, float b) {
    return a < b ? a : b;
}

void Enemy::CalculateFitness(const std::pair<int, int>& bridgeLocation, float mapWidth, float mapHeight, float timeSurvived, bool reachedBridge) {
    float maxPossibleDistance = std::sqrt(mapWidth * mapWidth + mapHeight * mapHeight); 
    float currentDistanceToBridgeX = static_cast<float>(bridgeLocation.first * CELL_SIZE + CELL_SIZE / 2.0f) - x;
    float currentDistanceToBridgeY = static_cast<float>(bridgeLocation.second * CELL_SIZE + CELL_SIZE / 2.0f) - y;
    float remainingDistance = std::sqrt(currentDistanceToBridgeX * currentDistanceToBridgeX + currentDistanceToBridgeY * currentDistanceToBridgeY);
    
    double distanceScore = 0.0;
    if (maxPossibleDistance > 0) {
        distanceScore = 1.0 - (remainingDistance / maxPossibleDistance);
        distanceScore = getMaxFrom(0.0, getMinFrom(1.0, distanceScore)); 
    }

    double bridgeBonus = reachedBridge ? 100.0 : 0.0;
    double survivalBonus = static_cast<double>(timeSurvived) * 0.1; 

    fitness = (distanceScore * 50.0) + bridgeBonus + survivalBonus;
    if (health <= 0 && !reachedBridge) { 
        fitness *= 0.5; 
    }
}


void Enemy::Mutate(float mutationRate) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    if (dis(gen) < mutationRate) {
        std::uniform_int_distribution<> health_change(-maxHealth / 10, maxHealth / 10);
        int newMaxHealth = getMaxFrom(10, maxHealth + health_change(gen)); 
        SetMaxHealth(newMaxHealth);
    }

    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<> speed_change_factor(0.9f, 1.1f);
        float newSpeed = getMaxFrom(5.0f, speed * speed_change_factor(gen)); 
        SetSpeed(newSpeed);
    }
}


std::wstring GetEnemyTypeName(EnemyType type) {
    switch (type) {
    case EnemyType::OGRE:
        return L"Ogre";
    case EnemyType::DARK_ELF:
        return L"Dark Elf";
    case EnemyType::HARPY:
        return L"Harpy";
    case EnemyType::MERCENARY:
        return L"Mercenary";
    default:
        return L"Unknown Enemy";
    }
} 