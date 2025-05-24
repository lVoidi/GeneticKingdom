#define _USE_MATH_DEFINES // For M_PI
#include "framework.h"
#include "Projectile.h"
#include "Enemy.h" // Required for Enemy class details
#include "Economy.h" // Required for Economy class
#include <cmath>
#include "Map.h" // Para incluir la definición completa de DummyTarget

const float PROJECTILE_SIZE_RADIUS = 5.0f; // Example radius for collision
const float ENEMY_SIZE_RADIUS = CELL_SIZE / 4.0f; // Example, should match enemy drawing/size

// Constructor del proyectil
Projectile::Projectile(ProjectileType type, int startRow, int startCol, int targetCellRow, int targetCellCol, int cs, float actualTargetX, float actualTargetY)
    : type(type), pImage(NULL), cellSize(cs), isActive(true), hasPreciseTarget(false), initialTargetX(0), initialTargetY(0)
{
    // Posición inicial (centro de la celda)
    x = (startCol + 0.5f) * cellSize;
    y = (startRow + 0.5f) * cellSize;

    if (actualTargetX != -1.0f && actualTargetY != -1.0f) {
        targetX = actualTargetX;
        targetY = actualTargetY;
        hasPreciseTarget = true;
    } else {
        targetX = (targetCellCol + 0.5f) * cellSize;
        targetY = (targetCellRow + 0.5f) * cellSize;
        hasPreciseTarget = false;
    }

    // Calcular ángulo y velocidad
    float dx = targetX - x;
    float dy = targetY - y;
    angle = atan2(dy, dx);

    switch (type) {
    case ProjectileType::ARROW:
    case ProjectileType::FIREARROW:
        speed = 400.0f; // Píxeles por segundo
        break;
    case ProjectileType::FIREBALL:
    case ProjectileType::PURPLEFIREBALL:
        speed = 300.0f;
        break;
    case ProjectileType::CANNONBALL:
    case ProjectileType::NUKEBOMB:
        speed = 250.0f;
        break;
    default:
        speed = 300.0f;
    }
    LoadImage();
}

// Destructor
Projectile::~Projectile()
{
    if (pImage) {
        // GDI+ handles image cleanup with GdiplusShutdown
        pImage = NULL;
    }
}

// Dibuja el proyectil
void Projectile::Draw(HDC hdc)
{
    if (!isActive || !pImage || pImage->GetLastStatus() != Gdiplus::Ok) return;

    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    float imgWidth = static_cast<float>(pImage->GetWidth());
    float imgHeight = static_cast<float>(pImage->GetHeight());
    
    // Define a desired size for the projectile image
    const float desiredProjectileWidth = 15.0f;
    const float desiredProjectileHeight = 15.0f;

    Gdiplus::Matrix matrix;
    matrix.Translate(x, y); // Mover al punto de anclaje (posición del proyectil)
    matrix.Rotate(angle * (180.0f / static_cast<float>(M_PI))); // Rotar
    // Adjust translation to account for the new desired size, so rotation is around the center of the scaled image
    matrix.Translate(-desiredProjectileWidth / 2.0f, -desiredProjectileHeight / 2.0f); 
    graphics.SetTransform(&matrix);

    // Draw the image scaled to the desired size
    graphics.DrawImage(pImage, 0.0f, 0.0f, desiredProjectileWidth, desiredProjectileHeight);
    
    graphics.ResetTransform(); // Restaurar la transformación
}

// Actualiza la posición del proyectil
bool Projectile::Update(float deltaTime)
{
    if (!isActive) return false; // No actualizar si no está activo

    float moveX = speed * cos(angle) * deltaTime;
    float moveY = speed * sin(angle) * deltaTime;

    x += moveX;
    y += moveY;

    // Comprobar si ha llegado al objetivo (o lo ha pasado)
    // Esto es simplificado. Para objetivos que se mueven, se necesitaría recalcular el ángulo
    // o usar una lógica de homing, o simplemente un tiempo de vida / distancia máxima.
    float distToInitialTargetSq = (x - initialTargetX) * (x - initialTargetX) + (y - initialTargetY) * (y - initialTargetY);
    float travelDistSq = (moveX * moveX + moveY * moveY); // squared distance moved in this frame

    // Si el proyectil ha pasado el punto donde estaba el target inicialmente
    // O si vuela demasiado lejos (ej. 2000 pixels de su origen - un rango máximo)
    float dx_origin = x - ((static_cast<int>(x/cellSize) +0.5f)*cellSize); // this logic is not right for origin
    // Simpler: check distance from where it was fired initially, or just if it passed target
    // For non-homing, if it has moved roughly the initial distance to target.
    float initialDx = initialTargetX - ( (int)(x/cellSize - moveX/cellSize) * cellSize + cellSize/2.0f); // Approximation of start x
    float initialDy = initialTargetY - ( (int)(y/cellSize - moveY/cellSize) * cellSize + cellSize/2.0f);
    // This check is problematic. A better way: fixed lifetime or max range from tower.
    // For now, let's say if it gets very close to where the target cell was.
    float distToTargetCellSq = (x - targetX)*(x-targetX) + (y-targetY)*(y-targetY);
    if (distToTargetCellSq < (cellSize * cellSize / 4.0f)) { // Reached vicinity of target cell
        isActive = false;
    }
    // Add a max range or lifetime if necessary, e.g.:
    // if (sqrt( (x-startX_at_spawn)*(x-startX_at_spawn) + (y-startY_at_spawn)*(y-startY_at_spawn) ) > MAX_PROJECTILE_RANGE ) isActive = false;

    return isActive;
}

// Obtiene el tipo de proyectil
ProjectileType Projectile::GetType() const
{
    return type;
}

// Carga la imagen adecuada para el tipo de proyectil
bool Projectile::LoadImage()
{
    pImage = NULL; // GDI+ will manage freeing existing if any on new FromFile, but good to clear pointer
    std::wstring fileName;
    switch (type) {
    case ProjectileType::ARROW: fileName = L"Arrow.png"; break;
    case ProjectileType::FIREBALL: fileName = L"Fireball.png"; break;
    case ProjectileType::CANNONBALL: fileName = L"Bomb.png"; break;
    case ProjectileType::FIREARROW: fileName = L"FireArrow.png"; break; // Needs this asset
    case ProjectileType::PURPLEFIREBALL: fileName = L"PurpleFireball.png"; break; // Needs this asset
    case ProjectileType::NUKEBOMB: fileName = L"NukeBomb.png"; break; // Needs this asset
    default: return false;
    }

    const wchar_t* possiblePaths[] = { L"Assets\\Projectiles\\", L"..\\GeneticKingdom2\\Assets\\Projectiles\\", L"GeneticKingdom2\\Assets\\Projectiles\\" };
    for (const wchar_t* basePath : possiblePaths) {
        std::wstring fullPath = basePath + fileName;
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            pImage = tempImage;
            return true;
        }
        if (tempImage) delete tempImage; // Clean up if loaded but status not OK
    }
    // Try from exe path
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WCHAR* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = L'\0';
        std::wstring fullPath = exePath;
        fullPath += L"Assets\\Projectiles\\";
        fullPath += fileName;
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            pImage = tempImage;
            return true;
        }
        if (tempImage) delete tempImage;
    }
    return false;
}

// Implementación del ProjectileManager
ProjectileManager::ProjectileManager()
{
}

ProjectileManager::~ProjectileManager()
{
    // Liberar todos los proyectiles
    for (auto& projectile : projectiles) {
        delete projectile;
    }
    projectiles.clear();
}

// Añade un nuevo proyectil
void ProjectileManager::AddProjectile(ProjectileType type, int startRow, int startCol, int targetRow, int targetCol, int cellSize, float targetActualX, float targetActualY)
{
    WCHAR debugMsg[256];
    swprintf_s(debugMsg, L"Creando proyectil tipo %d desde [%d,%d] hacia [%d,%d]\n", 
              static_cast<int>(type), startRow, startCol, targetRow, targetCol);
    OutputDebugStringW(debugMsg);
    
    projectiles.push_back(new Projectile(type, startRow, startCol, targetRow, targetCol, cellSize, targetActualX, targetActualY));
}

// Dibuja todos los proyectiles
void ProjectileManager::DrawProjectiles(HDC hdc)
{
    for (auto& projectile : projectiles) {
        if (projectile->IsActive()) {
            projectile->Draw(hdc);
        }
    }
}

// Actualiza todos los proyectiles
void ProjectileManager::Update(float deltaTime)
{
    auto it = projectiles.begin();
    while (it != projectiles.end()) {
        (*it)->Update(deltaTime); // Call update first
        if (!(*it)->IsActive()) { // Then check if it became inactive
            delete *it;
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

// Obtiene la posición actual del proyectil
void Projectile::GetPosition(float& outX, float& outY) const
{
    outX = x;
    outY = y;
}
/*
// Verifica si el proyectil colisiona con un objetivo dummy
bool Projectile::CheckCollision(const DummyTarget& target, int cellSize) const
{
    if (!isActive) return false;
    float targetCenterX = (target.col + 0.5f) * cellSize;
    float targetCenterY = (target.row + 0.5f) * cellSize;
    float distSq = (x - targetCenterX) * (x - targetCenterX) + (y - targetCenterY) * (y - targetCenterY);
    float combinedRadius = PROJECTILE_SIZE_RADIUS + (static_cast<float>(cellSize) / 2.0f * 0.6f); 
    return distSq <= (combinedRadius * combinedRadius);
}
*/

// Ensure this definition is pristine and correctly scoped
bool Projectile::CheckCollision(const Enemy& enemy, int cs) const 
{
    // Accessing member variables directly (this-> is implicit)
    if (!isActive || !enemy.IsActive() || !enemy.IsAlive()) {
        return false;
    }
    float enemyX = enemy.GetX();
    float enemyY = enemy.GetY();
    float distSq = (x - enemyX) * (x - enemyX) + (y - enemyY) * (y - enemyY);
    
    float currentEnemyRadius = static_cast<float>(cs) / 4.0f; 
    float combinedRadius = PROJECTILE_SIZE_RADIUS + currentEnemyRadius; 
    return distSq <= (combinedRadius * combinedRadius);
}

bool Projectile::IsActive() const { return isActive; }
void Projectile::SetActive(bool active) { isActive = active; }

int Projectile::GetDamage() const {
    switch (type) {
        case ProjectileType::ARROW: return 15;
        case ProjectileType::FIREBALL: return 25;
        case ProjectileType::CANNONBALL: return 45;
        case ProjectileType::FIREARROW: return 30;
        case ProjectileType::PURPLEFIREBALL: return 50;
        case ProjectileType::NUKEBOMB: return 150;
        default: return 7;
    }
}

void ProjectileManager::CheckCollisions(std::vector<Enemy>& enemies, int cs, Economy& economy) {
    for (size_t i = 0; i < projectiles.size(); ++i) {
        if (!projectiles[i]->IsActive()) continue;

        for (Enemy& enemy : enemies) {
            if (!enemy.IsActive() || !enemy.IsAlive()) continue;

            if (projectiles[i]->CheckCollision(enemy, cs)) {
                enemy.TakeDamage(projectiles[i]->GetDamage(), projectiles[i]->GetType());
                projectiles[i]->SetActive(false);

                if (!enemy.IsAlive()) {
                    economy.AddGold(enemy.GetGoldReward());
                }
                break;
            }
        }
    }
} 