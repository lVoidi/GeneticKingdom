// este archivo maneja toda la logica de las torres defensivas del juego
// las torres son las que atacan a los enemigos y pueden ser mejoradas
// hay diferentes tipos de torres con diferentes caracteristicas
// cada torre tiene un rango de ataque, daño y velocidad de ataque
// las torres se pueden mejorar hasta 3 veces para aumentar sus stats

#include "framework.h"
#include "Tower.h"
#include "Enemy.h" // necesitamos esto para que las torres puedan atacar enemigos
#include <cmath>
#include "Map.h" // para los objetivos falsos que usan las torres
#include <random> // para generar numeros aleatorios 
#include <cstdlib> // mas random porque nunca es suficiente
#include <ctime>   // tiempo para la semilla, aunque no lo usamos aqui

// un constructor super simple que inicializa todo lo basico
// type es el tipo de torre (arquero, mago, etc)
// row y col son la posicion en el mapa, porque windows es especial y usa matrices
Tower::Tower(TowerType type, int row, int col)
    : type(type), level(TowerLevel::LEVEL_1), row(row), col(col), 
      attackCooldown(0.0f), pTowerImage(NULL), showRange(false)
{
    LoadImage();
}

// el destructor mas basura del mundo
// ni siquiera borra la imagen porque gdi+ se encarga de eso
// que conveniente, no?
Tower::~Tower()
{
    pTowerImage = NULL;
}

// Dibuja la torre en la posición especificada
void Tower::Draw(HDC hdc, int cellSize)
{
    if (pTowerImage && pTowerImage->GetLastStatus() == Gdiplus::Ok) {
        try {
            Gdiplus::Graphics graphics(hdc);
            
            // Establecer la calidad de dibujo
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
            graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            
            // Dibujar la imagen en la posición de la celda
            graphics.DrawImage(
                pTowerImage,
                Gdiplus::Rect(col * cellSize, row * cellSize, cellSize, cellSize), // Destino
                0, 0, pTowerImage->GetWidth(), pTowerImage->GetHeight(), // Fuente
                Gdiplus::UnitPixel
            );
        }
        catch (...) {
            // Si ocurre alguna excepción al dibujar, solo marcamos el puntero como NULL
            // pero no lo eliminamos, ya que GDI+ se encargará de eso
            OutputDebugStringW(L"Error al dibujar la imagen de la torre\n");
            pTowerImage = NULL;
        }
    }
}

// puta que es facil mejorar una torre, solo hay que subirle el nivel
// y cargar una imagen nueva, que podria malir sal?
bool Tower::Upgrade()
{
    if (level == TowerLevel::LEVEL_3) {
        return false;
    }
    
    level = static_cast<TowerLevel>(static_cast<int>(level) + 1);
    
    LoadImage();
    
    return true;
}

// devuelve el tipo de torre, por si te olvidaste que construiste /s
TowerType Tower::GetType() const
{
    return type;
}

// devuelve el nivel actual, por si no te acuerdas /s
TowerLevel Tower::GetLevel() const
{
    return level;
}

// devuelve la fila, porque al parecer no puedes recordar donde pusiste tu torre /s
int Tower::GetRow() const
{
    return row;
}

// devuelve la columna, en serio necesito explicar esto?
int Tower::GetCol() const
{
    return col;
}

// comprueba si puedes mejorar la torre
// spoiler: si no es nivel 3, se puede
bool Tower::CanUpgrade() const
{
    return level != TowerLevel::LEVEL_3;
}

// esta funcion es un puto desastre
// intenta cargar una imagen desde 500 lugares diferentes
// porque windows es una mierda y no sabe donde estan los archivos
bool Tower::LoadImage()
{
    pTowerImage = NULL;

    std::wstring fileName;
    
    switch (type) {
    case TowerType::ARCHER:
        fileName = L"Archerlvl";
        break;
    case TowerType::GUNNER:
        fileName = L"Gunnerlvl";
        break;
    case TowerType::MAGE:
        fileName = L"Magelvl";
        break;
    }
    
    fileName += std::to_wstring(static_cast<int>(level));
    fileName += L".png";
    
    const wchar_t* possiblePaths[] = {
        L"Assets\\Towers\\",
        L"..\\GeneticKingdom2\\Assets\\Towers\\",
        L"GeneticKingdom2\\Assets\\Towers\\",
        L"C:\\Users\\Admin\\source\\repos\\GeneticKingdom2\\GeneticKingdom2\\Assets\\Towers\\"
    };
    
    for (const wchar_t* basePath : possiblePaths) {
        std::wstring fullPath = basePath + fileName;
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            pTowerImage = tempImage;
            
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de torre cargada correctamente: %s\n", fullPath.c_str());
            OutputDebugStringW(debugMsg);
            return true;
        }
        else if (tempImage) {
            tempImage = NULL;
        }
    }
    
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    
    WCHAR* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = L'\0';
        
        std::wstring fullPath = exePath;
        fullPath += L"Assets\\Towers\\";
        fullPath += fileName;
        
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            pTowerImage = tempImage;
            
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de torre cargada correctamente: %s\n", fullPath.c_str());
            OutputDebugStringW(debugMsg);
            return true;
        }
        else if (tempImage) {
            tempImage = NULL;
        }
    }
    
    WCHAR errorMsg[256];
    swprintf_s(errorMsg, L"Error al cargar la imagen de la torre: %s\n", fileName.c_str());
    OutputDebugStringW(errorMsg);
    return false;
}

// Obtiene el rango de ataque (en celdas)
int Tower::GetRange() const
{
    // Valores base de rango para cada tipo de torre
    int baseRange = 0;
    
    switch (type) {
    case TowerType::ARCHER:
        baseRange = 5; // Alto alcance
        break;
    case TowerType::MAGE:
        baseRange = 3; // Alcance medio
        break;
    case TowerType::GUNNER:
        baseRange = 2; // Bajo alcance
        break;
    }
    
    // El rango aumenta ligeramente con cada nivel
    return baseRange + (static_cast<int>(level) - 1) / 2;
}

// Obtiene la velocidad de ataque (disparos por segundo)
float Tower::GetAttackSpeed() const
{
    // Velocidad base de ataque para cada tipo de torre
    float baseSpeed = 0.0f;
    
    switch (type) {
    case TowerType::ARCHER:
        baseSpeed = 1.2f; // Tiempo de recarga bajo (más disparos por segundo)
        break;
    case TowerType::MAGE:
        baseSpeed = 0.8f; // Tiempo de recarga medio
        break;
    case TowerType::GUNNER:
        baseSpeed = 0.4f; // Tiempo de recarga alto (menos disparos por segundo)
        break;
    }
    
    // La velocidad aumenta un 15% con cada nivel
    return baseSpeed * (1.0f + 0.15f * (static_cast<int>(level) - 1));
}

// Obtiene el daño base
int Tower::GetDamage() const
{
    // Daño base para cada tipo de torre (aumentado para balancear mejor)
    int baseDamage = 0;
    
    switch (type) {
    case TowerType::ARCHER:
        baseDamage = 15;  // Aumentado de 8 a 15
        break;
    case TowerType::MAGE:
        baseDamage = 25; // Aumentado de 15 a 25
        break;
    case TowerType::GUNNER:
        baseDamage = 40; // Aumentado de 25 a 40
        break;
    }
    
    // El daño aumenta con cada nivel
    return baseDamage * static_cast<int>(level);
}

// dibuja el puto rango de ataque de la torre, que es un circulo semitransparente 
// con el color correspondiente al tipo de torre. que elegancia la de francia
void Tower::DrawRange(HDC hdc, int cellSize)
{
    if (!showRange) {
        return;
    }
    
    int range = GetRange();
    
    COLORREF rangeColor;
    
    switch (type) {
    case TowerType::ARCHER:
        rangeColor = RGB(0, 150, 0); 
        break;
    case TowerType::MAGE:
        rangeColor = RGB(0, 0, 150); 
        break;
    case TowerType::GUNNER:
        rangeColor = RGB(150, 0, 0); 
        break;
    default:
        rangeColor = RGB(150, 150, 0); 
    }
    
    try {
        Gdiplus::Graphics graphics(hdc);
        
        Gdiplus::Color color(80, GetRValue(rangeColor), GetGValue(rangeColor), GetBValue(rangeColor));
        Gdiplus::SolidBrush brush(color);
        
        float centerX = (col + 0.5f) * cellSize;
        float centerY = (row + 0.5f) * cellSize;
        
        float radius = range * cellSize;
        
        graphics.FillEllipse(&brush, centerX - radius, centerY - radius, radius * 2, radius * 2);
        
        Gdiplus::Pen pen(Gdiplus::Color(150, GetRValue(rangeColor), GetGValue(rangeColor), GetBValue(rangeColor)), 2);
        
        for (int r = row - range; r <= row + range; r++) {
            for (int c = col - range; c <= col + range; c++) {
                float dx = c - col;
                float dy = r - row;
                float distance = sqrtf(dx * dx + dy * dy);
                
                if (distance <= range) {
                    graphics.DrawRectangle(&pen, c * cellSize, r * cellSize, cellSize, cellSize);
                }
            }
        }
    }
    catch (...) {
        OutputDebugStringW(L"Error al dibujar el rango de la torre\n");
    }
}

// devuelve el tipo de proyectil que dispara la torre. cada torre tiene su propio
// tipo de proyectil base y una probabilidad del 20% de disparar uno poderoso.
// por qué 20%? porque puedo, y porque queda epiko 
ProjectileType Tower::GetProjectileType() const
{
    ProjectileType baseProjectileType;
    switch (type) {
    case TowerType::ARCHER:
        baseProjectileType = ProjectileType::ARROW;
        break;
    case TowerType::MAGE:
        baseProjectileType = ProjectileType::FIREBALL;
        break;
    case TowerType::GUNNER:
        baseProjectileType = ProjectileType::CANNONBALL;
        break;
    default:
        baseProjectileType = ProjectileType::ARROW;
        break;
    }

    if ((std::rand() % 100) < 20) {
        switch (type) {
        case TowerType::ARCHER:
            return ProjectileType::FIREARROW;
        case TowerType::MAGE:
            return ProjectileType::PURPLEFIREBALL;
        case TowerType::GUNNER:
            return ProjectileType::NUKEBOMB;
        default:
            return baseProjectileType;
        }
    }
    
    return baseProjectileType;
}

// aqui esta la funcion que actualiza las torres y dispara a los enemigos
// esta mierda es un desastre pero funciona, asi que no la toques
// si la tocas y se rompe, es tu culpa, no la mia
void Tower::Update(float deltaTime, ProjectileManager& projectileManager, int cellSize, const std::vector<Enemy>& enemies)
{
    if (attackCooldown > 0) {
        attackCooldown -= deltaTime;
        if (attackCooldown < 0) {
            attackCooldown = 0;
        }
    }
    
    if (attackCooldown <= 0 && !enemies.empty()) {
        int range = GetRange();
        float towerCenterX = (col + 0.5f) * cellSize;
        float towerCenterY = (row + 0.5f) * cellSize;

        const Enemy* closestEnemy = nullptr;
        float minDistanceSq = FLT_MAX;

        for (const auto& enemy : enemies) {
            if (!enemy.IsActive() || !enemy.IsAlive()) {
                continue;
            }

            if (type == TowerType::GUNNER && enemy.IsFlying()) {
                continue; 
            }

            float dx = enemy.GetX() - towerCenterX;
            float dy = enemy.GetY() - towerCenterY;
            float distanceSq = dx * dx + dy * dy;
            float rangePixels = static_cast<float>(range * cellSize);

            if (distanceSq <= (rangePixels * rangePixels) && distanceSq < minDistanceSq) {
                closestEnemy = &enemy;
                minDistanceSq = distanceSq;
            }
        }
        
        if (closestEnemy) {
            int targetEnemyCellCol = static_cast<int>(closestEnemy->GetX() / cellSize);
            int targetEnemyCellRow = static_cast<int>(closestEnemy->GetY() / cellSize);

            projectileManager.AddProjectile(
                GetProjectileType(),
                row, col,                       
                targetEnemyCellRow, targetEnemyCellCol, 
                cellSize,
                closestEnemy->GetX(), closestEnemy->GetY()
            );
            
            attackCooldown = 1.0f / GetAttackSpeed();
        }
    }
}

// muestra u oculta el rango de la torre, por si eres ciego y no lo ves
void Tower::SetShowRange(bool show)
{
    showRange = show;
}

// devuelve si el rango esta visible, por si te olvidaste que lo activaste
bool Tower::IsShowingRange() const
{
    return showRange;
}

//////////////////////////////////////////////////////////////
// esta cosa maneja todas las torres del juego
// si algo explota aqui, todo el juego se va a la shi
//////////////////////////////////////////////////////////////

TowerManager::TowerManager()
{
}

// destructor que no hace una mierda porque somos vagos
TowerManager::~TowerManager()
{
    towers.clear();
}

// inicializa esta mierda, aunque no hace nada util
void TowerManager::Initialize()
{
}

// agrega una torre nueva, si ya hay una ahi te manda a la shi
bool TowerManager::AddTower(TowerType type, int row, int col)
{
    if (HasTower(row, col)) {
        return false;
    }
    
    Tower* newTower = new Tower(type, row, col);
    towers.push_back(newTower);
    
    return true;
}

// mejora una torre si puedes, si no puedes ps xd
bool TowerManager::UpgradeTower(int row, int col)
{
    Tower* tower = GetTowerAt(row, col);
    
    if (!tower || !tower->CanUpgrade()) {
        return false;
    }
    
    return tower->Upgrade();
}

// dibuja todas las torres, si no se ven es tu problema
void TowerManager::DrawTowers(HDC hdc, int cellSize)
{
    for (Tower* tower : towers) {
        tower->Draw(hdc, cellSize);
    }
}

// comprueba si hay una torre en esa posicion, util para no cagarla
bool TowerManager::HasTower(int row, int col) const
{
    return GetTowerAt(row, col) != nullptr;
}

// Obtiene una referencia a la torre en la posición especificada (o nullptr si no hay)
Tower* TowerManager::GetTowerAt(int row, int col) const
{
    for (Tower* tower : towers) {
        if (tower->GetRow() == row && tower->GetCol() == col) {
            return tower;
        }
    }
    
    return nullptr;
}

// Dibuja los rangos de las torres
void TowerManager::DrawTowerRanges(HDC hdc, int cellSize)
{
    for (auto& tower : towers) {
        tower->DrawRange(hdc, cellSize);
    }
}

// Actualiza la lógica de todas las torres (apuntando a enemigos)
void TowerManager::Update(float deltaTime, ProjectileManager& projectileManager, int cellSize, const std::vector<Enemy>& enemies)
{
    for (auto& tower : towers) {
        tower->Update(deltaTime, projectileManager, cellSize, enemies);
    }
}

// Activa la visualización del rango para una torre específica
void TowerManager::ShowRangeForTower(int row, int col)
{
    // Ocultar todos los rangos primero
    HideAllRanges();
    
    // Mostrar el rango de la torre seleccionada
    Tower* tower = GetTowerAt(row, col);
    if (tower) {
        tower->SetShowRange(true);
    }
}

// Oculta todos los rangos
void TowerManager::HideAllRanges()
{
    for (auto& tower : towers) {
        tower->SetShowRange(false);
    }
} 