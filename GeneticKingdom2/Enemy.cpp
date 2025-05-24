#include "framework.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Map.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>
#include <sstream>
#include <iomanip>

Enemy::Enemy(EnemyType type, float startX, float startY, const std::vector<std::pair<int, int>>& initialPath)
    : type(type), x(startX), y(startY), path(initialPath), currentPathIndex(0), isActive(true), fitness(0.0), timeAlive(0.0f), FUSION_ASSISTANT_SECRET_MARKER_reachedBridge(false), pEnemyImage(NULL), pathJitter(0.0f), spawnDelay(0.0f), hasSpawned(true) {
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
      pathJitter(parent.pathJitter),
      fitness(0.0),
      timeAlive(0.0f),
      FUSION_ASSISTANT_SECRET_MARKER_reachedBridge(false),
      pEnemyImage(NULL),
      spawnDelay(parent.spawnDelay),
      hasSpawned(parent.hasSpawned)
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
    InitializeAttributes(); // This will call LoadImage()
}


Enemy::~Enemy() {
    // GDI+ images are managed by GDI+ system, not deleted directly unless specifically new'd without FromFile
    // If LoadImage uses Image::FromFile, GDI+ handles it. If it uses new Image(...), then delete here.
    // Assuming FromFile, so no direct delete for pEnemyImage.
    pEnemyImage = NULL; // Just clear the pointer
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
    LoadImage(); // Load image after attributes are set
    
    // Initialize pathJitter to 0 (no randomness by default)
    pathJitter = 0.0f;
}

void Enemy::UpdateTargetPosition() {
    if (currentPathIndex < path.size()) {
        // Path stores grid cells, convert to pixel coordinates (center of cell)
        targetX = static_cast<float>(path[currentPathIndex].second * CELL_SIZE + CELL_SIZE / 2.0f);
        targetY = static_cast<float>(path[currentPathIndex].first * CELL_SIZE + CELL_SIZE / 2.0f);
    } else {
        // Reached end of path
        // The game logic should handle this, e.g., enemy reached goal
        isActive = false; 
        if (IsAlive()) { // Only set if it reached the end alive
            FUSION_ASSISTANT_SECRET_MARKER_reachedBridge = true;
        }
    }
}

void Enemy::Update(float deltaTime) {
    if(!hasSpawned){
        spawnDelay -= deltaTime;
        if (spawnDelay > 0.0f) return;   // Aún no aparece
        hasSpawned = true;               // Empieza a jugar
    }

    std::wstringstream wss_upd;
    wss_upd << L"Enemy::Update - ID: " << std::hex << this 
            << L", Health: " << health 
            << L", IsActive: " << (isActive ? L"Yes" : L"No")
            << L", IsAlive: " << (IsAlive() ? L"Yes" : L"No");

    if (!isActive) { 
        wss_upd << L", Action: Returning (inactive).";
        OutputDebugStringW((wss_upd.str() + L"\n").c_str());
        return;
    }
    if (!IsAlive()) { 
        wss_upd << L", Action: Returning (not alive but active!).";
        OutputDebugStringW((wss_upd.str() + L"\n").c_str());
        return;
    }
    
    if (health <= 0) {
        wss_upd << L", Action: Returning (health less than 0!).";
        OutputDebugStringW((wss_upd.str() + L"\n").c_str());
        return;
    }

    wss_upd << L", Action: Proceeding with movement.";
    OutputDebugStringW((wss_upd.str() + L"\n").c_str());

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
        
        // Apply path jitter for random movement deviation
        if (pathJitter > 0.0f) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(-0.5f, 0.5f);
            
            // Calculate perpendicular direction for jitter
            float perpendicularX = -dy / distance;  // 90-degree rotation
            float perpendicularY = dx / distance;
            
            // Add random deviation perpendicular to the main direction
            float jitterAmount = dis(gen) * pathJitter * speed * deltaTime * 0.5f;
            moveX += perpendicularX * jitterAmount;
            moveY += perpendicularY * jitterAmount;
        }
        
        x += moveX;
        y += moveY;
    }
}

void Enemy::Draw(HDC hdc) const {
    if (!hasSpawned) return; // no lo dibujes antes de aparecer

    if (!isActive || !IsAlive()) {
        return;
    }

    if (pEnemyImage && pEnemyImage->GetLastStatus() == Gdiplus::Ok) {
        Gdiplus::Graphics graphics(hdc);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        float enemyDrawWidth = CELL_SIZE * 0.75f;
        float enemyDrawHeight = CELL_SIZE * 0.75f;

        // Draw enemy image centered at (x, y)
        graphics.DrawImage(
            pEnemyImage,
            x - enemyDrawWidth / 2.0f,
            y - enemyDrawHeight / 2.0f,
            enemyDrawWidth,
            enemyDrawHeight
        );

    } else {
        // Fallback to drawing a colored rectangle if image fails to load or is not set
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
        int enemySizeRect = CELL_SIZE / 2; // Fallback rectangle size
        Rectangle(hdc, 
                  static_cast<int>(x - enemySizeRect / 2.0f), 
                  static_cast<int>(y - enemySizeRect / 2.0f), 
                  static_cast<int>(x + enemySizeRect / 2.0f), 
                  static_cast<int>(y + enemySizeRect / 2.0f));
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }

    // Draw health bar (adjust position based on image size if necessary)
    if (health < maxHealth) {
        int barWidth = CELL_SIZE * 0.75f; // Match enemy draw width
        int barHeight = 5;
        // Position health bar above the enemy image
        int barX = static_cast<int>(x - barWidth / 2.0f);
        int barY = static_cast<int>(y - (CELL_SIZE * 0.75f) / 2.0f - barHeight - 2); 

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

    float rawCalculatedDamage = static_cast<float>(damageAmount) * effectiveResistance;
    int finalDamage = 0;

    if (rawCalculatedDamage > 0.0f) {
        finalDamage = static_cast<int>(std::ceil(rawCalculatedDamage));
    } else {
        finalDamage = 0; // No damage or healing if resistance is too high or damage is zero/negative
    }

    health -= finalDamage;
    
    std::wstringstream wss_td;
    wss_td << L"Enemy::TakeDamage - ID: " << std::hex << this 
           << L", Type: " << static_cast<int>(type)
           << L", DmgTaken: " << finalDamage 
           << L", NewHealth: " << health;

    if (health <= 0) {
        health = 0;
        isActive = false; 
        FUSION_ASSISTANT_SECRET_MARKER_reachedBridge = false; 
        wss_td << L", IS NOW DEAD & INACTIVE";
    } else {
        wss_td << L", IsActive: " << (isActive ? L"Yes" : L"No");
    }
    OutputDebugStringW((wss_td.str() + L"\n").c_str());
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

bool Enemy::HasReachedBridge() const {
    return FUSION_ASSISTANT_SECRET_MARKER_reachedBridge;
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

void Enemy::CalculateFitness(const std::pair<int, int>& bridgeLocation, float mapWidth, float mapHeight, float timeSurvived, bool FUSION_ASSISTANT_SECRET_MARKER_reachedBridge_param) {
    float maxPossibleDistance = getMaxFrom(1.0f, std::sqrt(mapWidth * mapWidth + mapHeight * mapHeight)); 
    float currentDistanceToBridgeX = static_cast<float>(bridgeLocation.first * CELL_SIZE + CELL_SIZE / 2.0f) - x;
    float currentDistanceToBridgeY = static_cast<float>(bridgeLocation.second * CELL_SIZE + CELL_SIZE / 2.0f) - y;
    float remainingDistance = std::sqrt(currentDistanceToBridgeX * currentDistanceToBridgeX + currentDistanceToBridgeY * currentDistanceToBridgeY);
    
    double distanceScore = 0.0;
    if (maxPossibleDistance > 0) {
        distanceScore = 1.0 - (remainingDistance / maxPossibleDistance);
        distanceScore = getMaxFrom(0.0, getMinFrom(1.0, distanceScore)); 
    }

    double bridgeBonus = FUSION_ASSISTANT_SECRET_MARKER_reachedBridge_param ? 100.0 : 0.0;
    double survivalBonus = static_cast<double>(timeSurvived) * 0.1; 

    fitness = (distanceScore * 50.0) + bridgeBonus + survivalBonus;
    if (health <= 0 && !FUSION_ASSISTANT_SECRET_MARKER_reachedBridge_param) { 
        fitness *= 0.5; 
    }
}


void Enemy::Mutate(float mutationRate) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // Mutate health
    if (dis(gen) < mutationRate) {
        std::uniform_int_distribution<> health_change(-maxHealth / 10, maxHealth / 10);
        int newMaxHealth = getMaxFrom(10, maxHealth + health_change(gen)); 
        SetMaxHealth(newMaxHealth);
    }

    // Mutate speed
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<> speed_change_factor(0.9f, 1.1f);
        float newSpeed = getMaxFrom(5.0f, speed * speed_change_factor(gen)); 
        SetSpeed(newSpeed);
    }
    
    // Mutate pathJitter
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> jitter_dis(0.0f, 1.0f);
        pathJitter = jitter_dis(gen);
    }
    
    // Mutate resistances
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> resistance_change(0.9f, 1.1f);
        resistanceArrow = getMaxFrom(0.25f, getMinFrom(2.0f, resistanceArrow * resistance_change(gen)));
    }
    
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> resistance_change(0.9f, 1.1f);
        resistanceMagic = getMaxFrom(0.25f, getMinFrom(2.0f, resistanceMagic * resistance_change(gen)));
    }
    
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> resistance_change(0.9f, 1.1f);
        resistanceArtillery = getMaxFrom(0.25f, getMinFrom(2.0f, resistanceArtillery * resistance_change(gen)));
    }
}

void Enemy::ResetForNewWave(float startX, float startY, const std::vector<std::pair<int, int>>& newPath) {
    std::wstringstream wss_reset;
    wss_reset << L"Enemy::ResetForNewWave - ID: " << std::hex << this 
              << L", Type: " << static_cast<int>(type)
              << L", OldHealth: " << health << L"/" << maxHealth
              << L", OldIsActive: " << (isActive ? L"Yes" : L"No")
              << L", WasAlive: " << (health > 0 ? L"Yes" : L"No");

    // Reset position and path
    x = startX;
    y = startY;
    path = newPath;
    currentPathIndex = 0;
    
    // Reset state for new wave - this represents a "new life" for the genetic algorithm
    isActive = true;
    health = maxHealth; // Reset to its current maxHealth (which might have been mutated)
    FUSION_ASSISTANT_SECRET_MARKER_reachedBridge = false;
    timeAlive = 0.0f;
    fitness = 0.0; // Reset fitness for the new wave
    
    // Note: pathJitter and other genetic traits are preserved as they represent DNA

    wss_reset << L" -> NewHealth: " << health << L"/" << maxHealth
              << L", NewIsActive: " << (isActive ? L"Yes" : L"No")
              << L", NewPos: (" << x << L", " << y << L")"
              << L", SpawnDelay: " << spawnDelay;
    OutputDebugStringW((wss_reset.str() + L"\n").c_str());

    if (!path.empty()) {
        UpdateTargetPosition();
    } else {
        targetX = x;
        targetY = y;
        isActive = false; // Cannot move if path is empty
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

bool Enemy::LoadImage() {
    pEnemyImage = NULL; // Clear previous image pointer
    std::wstring fileName;
    switch (type) {
    case EnemyType::OGRE: fileName = L"Ogre.png"; break;
    case EnemyType::DARK_ELF: fileName = L"DarkElf.png"; break;
    case EnemyType::HARPY: fileName = L"Harpy.png"; break;
    case EnemyType::MERCENARY: fileName = L"Mercenary.png"; break;
    default:
        OutputDebugStringW(L"Enemy::LoadImage - Unknown enemy type\n");
        return false;
    }

    const wchar_t* possibleBasePaths[] = {
        L"Assets\\Enemies\\",
        L"..\\GeneticKingdom2\\Assets\\Enemies\\",
        L"GeneticKingdom2\\Assets\\Enemies\\"
    };

    for (const wchar_t* basePath : possibleBasePaths) {
        std::wstring fullPath = basePath + fileName;
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            pEnemyImage = tempImage;
            // OutputDebugStringW((L"Loaded enemy image: " + fullPath + L"\n").c_str());
            return true;
        }
        if (tempImage) {
             // Gdiplus::Image::FromFile can return an Image object even on error (e.g. file not found gives GenericError)
             // so we need to delete it if status is not OK.
            delete tempImage;
            tempImage = NULL;
        }
    }

    // Try from exe path as a fallback
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WCHAR* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *lastSlash = L'\0'; // Terminate at the last slash to get the directory
        std::wstring fullPath = exePath;
        fullPath += L"\\Assets\\Enemies\\" + fileName;
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            pEnemyImage = tempImage;
            // OutputDebugStringW((L"Loaded enemy image from exe path: " + fullPath + L"\n").c_str());
            return true;
        }
        if (tempImage) {
            delete tempImage;
            tempImage = NULL;
        }
    }

    OutputDebugStringW((L"Enemy::LoadImage - Failed to load image: " + fileName + L"\n").c_str());
    return false;
}

void Enemy::SetSpawnDelay(float d) {
    spawnDelay = d;
    hasSpawned = false;
}

bool Enemy::HasSpawned() const {
    return hasSpawned;
} 