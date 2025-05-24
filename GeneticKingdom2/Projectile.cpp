/*
 * mira este codigo es para manejar los proyectiles del juego. es bastante simple:
 * - tenemos diferentes tipos de proyectiles (flechas, bolas de fuego, etc)
 * - cada proyectil tiene una posicion, velocidad y angulo
 * - se mueven en linea recta hacia su objetivo
 * - pueden tener un objetivo preciso o solo una celda objetivo
 * - se renderizan con gdi+ y rotan segun su direccion
 * - el codigo es una mierda pero funciona, asi que no lo toques mucho
 */

#define _USE_MATH_DEFINES // para poder usar M_PI
#include "framework.h"
#include "Projectile.h"
#include "Enemy.h"
#include "Economy.h" 
#include <cmath>
#include "Map.h" 

// estos valores los saque de mi trasero pero funcionan bien
const float PROJECTILE_SIZE_RADIUS = 6.0f; 
const float ENEMY_SIZE_RADIUS = CELL_SIZE / 5.0f;

Projectile::Projectile(ProjectileType type, int startRow, int startCol, int targetCellRow, int targetCellCol, int cs, float actualTargetX, float actualTargetY)
    : type(type), pImage(NULL), cellSize(cs), isActive(true), hasPreciseTarget(false), initialTargetX(0), initialTargetY(0)
{
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

    float dx = targetX - x;
    float dy = targetY - y;
    angle = atan2(dy, dx);

    switch (type) {
    case ProjectileType::ARROW:
    case ProjectileType::FIREARROW:
        speed = 350.0f; // velocidad en pixeles por segundo
        break;
    case ProjectileType::FIREBALL:
    case ProjectileType::PURPLEFIREBALL:
        speed = 250.0f;
        break;
    case ProjectileType::CANNONBALL:
    case ProjectileType::NUKEBOMB:
        speed = 200.0f;
        break;
    default:
        speed = 250.0f;
    }
    LoadImage();
}

// el destructor es una mierda pero gdi+ se encarga de limpiar todo
Projectile::~Projectile()
{
    if (pImage) {
        pImage = NULL;
    }
}

// dibuja esta basura usando gdi+, que dios nos ayude
void Projectile::Draw(HDC hdc)
{
    if (!isActive || !pImage || pImage->GetLastStatus() != Gdiplus::Ok) return;

    Gdiplus::Graphics graphics(hdc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    float imgWidth = static_cast<float>(pImage->GetWidth());
    float imgHeight = static_cast<float>(pImage->GetHeight());
    
    const float desiredProjectileWidth = 15.0f;
    const float desiredProjectileHeight = 15.0f;

    Gdiplus::Matrix matrix;
    matrix.Translate(x, y);
    matrix.Rotate(angle * (180.0f / static_cast<float>(M_PI)));
    matrix.Translate(-desiredProjectileWidth / 2.0f, -desiredProjectileHeight / 2.0f); 
    graphics.SetTransform(&matrix);

    graphics.DrawImage(pImage, 0.0f, 0.0f, desiredProjectileWidth, desiredProjectileHeight);
    
    graphics.ResetTransform();
}

// actualiza la posicion del proyectil, fisica basica para idiotas
bool Projectile::Update(float deltaTime)
{
    if (!isActive) return false;

    float moveX = speed * cos(angle) * deltaTime;
    float moveY = speed * sin(angle) * deltaTime;

    x += moveX;
    y += moveY;

    return true;
}

// retorna el tipo de proyectil, por si eres muy tonto para saberlo
ProjectileType Projectile::GetType() const
{
    return type;
}

// maldita sea, otra vez tengo que cargar imagenes de mierda
bool Projectile::LoadImage()
{
    pImage = NULL;
    std::wstring fileName;
    switch (type) {
    case ProjectileType::ARROW: fileName = L"Arrow.png"; break;
    case ProjectileType::FIREBALL: fileName = L"Fireball.png"; break;
    case ProjectileType::CANNONBALL: fileName = L"Bomb.png"; break;
    case ProjectileType::FIREARROW: fileName = L"FireArrow.png"; break;
    case ProjectileType::PURPLEFIREBALL: fileName = L"PurpleFireball.png"; break;
    case ProjectileType::NUKEBOMB: fileName = L"NukeBomb.png"; break;
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
        if (tempImage) delete tempImage;
    }
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

// esta mierda maneja todos los proyectiles, que dios nos ayude
ProjectileManager::ProjectileManager()
{
}

// destructor que limpia toda la basura que dejamos tirada
ProjectileManager::~ProjectileManager()
{
    for (auto& projectile : projectiles) {
        delete projectile;
    }
    projectiles.clear();
}

// agrega un nuevo proyectil al gestor, espero que sepas lo que haces
void ProjectileManager::AddProjectile(ProjectileType type, int startRow, int startCol, int targetRow, int targetCol, int cellSize, float targetActualX, float targetActualY)
{
    WCHAR debugMsg[256];
    swprintf_s(debugMsg, L"Creando proyectil tipo %d desde [%d,%d] hacia [%d,%d]\n", 
              static_cast<int>(type), startRow, startCol, targetRow, targetCol);
    OutputDebugStringW(debugMsg);
    
    projectiles.push_back(new Projectile(type, startRow, startCol, targetRow, targetCol, cellSize, targetActualX, targetActualY));
}

// dibuja todos los proyectiles activos, si es que hay alguno
void ProjectileManager::DrawProjectiles(HDC hdc)
{
    for (auto& projectile : projectiles) {
        if (projectile->IsActive()) {
            projectile->Draw(hdc);
        }
    }
}

// actualiza la posicion de los proyectiles y elimina los que se salen del mapa
void ProjectileManager::Update(float deltaTime, float mapWidth, float mapHeight)
{
    for (size_t i = 0; i < projectiles.size();) {
        if (projectiles[i]->Update(deltaTime)) {
            float projX, projY;
            projectiles[i]->GetPosition(projX, projY);
            if (projX < 0 || projX > mapWidth || projY < 0 || projY > mapHeight) {
                projectiles[i]->SetActive(false);
            }
        }

        if (!projectiles[i]->IsActive()) {
            delete projectiles[i];
            projectiles.erase(projectiles.begin() + i);
        } else {
            i++;
        }
    }
}

// devuelve la posicion actual del proyectil, por si te interesa
void Projectile::GetPosition(float& outX, float& outY) const
{
    outX = x;
    outY = y;
}

// verifica si el proyectil le dio a un enemigo, fisica simple para idiotas
bool Projectile::CheckCollision(const Enemy& enemy, int cs) const 
{
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

// funciones simples que cualquier idiota puede entender
bool Projectile::IsActive() const { return isActive; }
void Projectile::SetActive(bool active) { isActive = active; }

// devuelve el daño que hace cada tipo de proyectil, no lo cambies o la cagas
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

// revisa todas las colisiones y mata enemigos si es necesario
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