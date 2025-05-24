#include "framework.h"
#include "Tower.h"
#include "Enemy.h" // Include full definition of Enemy for usage
#include <cmath>
#include "Map.h" // Para incluir la definición completa de DummyTarget
#include <random> // Para std::mt19937 y std::uniform_int_distribution
#include <cstdlib> // Para rand() y srand()
#include <ctime>   // Para time() para la semilla de srand()

// Constructor de la torre
Tower::Tower(TowerType type, int row, int col)
    : type(type), level(TowerLevel::LEVEL_1), row(row), col(col), 
      attackCooldown(0.0f), pTowerImage(NULL), showRange(false)
{
    // Cargar la imagen correspondiente al tipo y nivel
    LoadImage();
    // Inicializar la semilla para el generador de números aleatorios una vez por torre
    // o mejor aún, hacerlo globalmente una sola vez en la aplicación.
    // Por simplicidad aquí, si se quiere más aleatoriedad, mover a una inicialización global.
    // srand(static_cast<unsigned int>(time(NULL))); 
    // ^^^ Nota: Poner srand() aquí haría que todas las torres creadas en el mismo segundo
    // tuvieran la misma secuencia. Es mejor hacerlo globalmente o usar <random>.
}

// Destructor
Tower::~Tower()
{
    // No eliminamos la imagen aquí, ya que GDI+ se encargará de eso
    // cuando se llame a GdiplusShutdown
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

// Mejora la torre al siguiente nivel si es posible
bool Tower::Upgrade()
{
    // Comprobar si ya está al nivel máximo
    if (level == TowerLevel::LEVEL_3) {
        return false; // No se puede mejorar más
    }
    
    // Incrementar el nivel
    level = static_cast<TowerLevel>(static_cast<int>(level) + 1);
    
    // Cargar la nueva imagen correspondiente al nivel
    LoadImage();
    
    return true;
}

// Obtiene el tipo de torre
TowerType Tower::GetType() const
{
    return type;
}

// Obtiene el nivel actual de la torre
TowerLevel Tower::GetLevel() const
{
    return level;
}

// Obtiene la posición de la torre (fila)
int Tower::GetRow() const
{
    return row;
}

// Obtiene la posición de la torre (columna)
int Tower::GetCol() const
{
    return col;
}

// Comprueba si la torre puede ser mejorada
bool Tower::CanUpgrade() const
{
    return level != TowerLevel::LEVEL_3;
}

// Carga la imagen adecuada para el tipo y nivel de torre
bool Tower::LoadImage()
{
    // Primero liberamos la referencia a cualquier imagen previamente cargada
    // pero no la eliminamos, ya que GDI+ se encargará de eso
    pTowerImage = NULL;

    // Determinar el nombre de archivo según el tipo y nivel
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
    
    // Añadir el nivel
    fileName += std::to_wstring(static_cast<int>(level));
    fileName += L".png";
    
    // Lista de posibles rutas para la imagen
    const wchar_t* possiblePaths[] = {
        L"Assets\\Towers\\",
        L"..\\GeneticKingdom2\\Assets\\Towers\\",
        L"GeneticKingdom2\\Assets\\Towers\\",
        L"C:\\Users\\Admin\\source\\repos\\GeneticKingdom2\\GeneticKingdom2\\Assets\\Towers\\"
    };
    
    // Intentar cargar la imagen desde una de las rutas posibles
    for (const wchar_t* basePath : possiblePaths) {
        std::wstring fullPath = basePath + fileName;
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            // Solo asignamos el puntero si la imagen se cargó correctamente
            pTowerImage = tempImage;
            
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de torre cargada correctamente: %s\n", fullPath.c_str());
            OutputDebugStringW(debugMsg);
            return true;
        }
        else if (tempImage) {
            // Si hubo un error, no asignar el puntero y continuar con la siguiente ruta
            tempImage = NULL;
        }
    }
    
    // Si no se pudo cargar la imagen, obtener la ruta del ejecutable e intentar otras rutas
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    
    // Obtener la carpeta del ejecutable
    WCHAR* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = L'\0';
        
        // Construir ruta completa
        std::wstring fullPath = exePath;
        fullPath += L"Assets\\Towers\\";
        fullPath += fileName;
        
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            // Solo asignamos el puntero si la imagen se cargó correctamente
            pTowerImage = tempImage;
            
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de torre cargada correctamente: %s\n", fullPath.c_str());
            OutputDebugStringW(debugMsg);
            return true;
        }
        else if (tempImage) {
            // Si hubo un error, no asignar el puntero
            tempImage = NULL;
        }
    }
    
    // No se pudo cargar la imagen
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

// Dibuja el rango de ataque de la torre
void Tower::DrawRange(HDC hdc, int cellSize)
{
    if (!showRange) {
        return;
    }
    
    int range = GetRange();
    
    // Crear un pincel semitransparente para mostrar el rango
    COLORREF rangeColor;
    
    switch (type) {
    case TowerType::ARCHER:
        rangeColor = RGB(0, 150, 0); // Verde para arqueros
        break;
    case TowerType::MAGE:
        rangeColor = RGB(0, 0, 150); // Azul para magos
        break;
    case TowerType::GUNNER:
        rangeColor = RGB(150, 0, 0); // Rojo para cañones
        break;
    default:
        rangeColor = RGB(150, 150, 0); // Amarillo por defecto
    }
    
    try {
        Gdiplus::Graphics graphics(hdc);
        
        // Crear un pincel semitransparente
        Gdiplus::Color color(80, GetRValue(rangeColor), GetGValue(rangeColor), GetBValue(rangeColor));
        Gdiplus::SolidBrush brush(color);
        
        // Centro de la torre
        float centerX = (col + 0.5f) * cellSize;
        float centerY = (row + 0.5f) * cellSize;
        
        // Calcular radio en píxeles
        float radius = range * cellSize;
        
        // Dibujar círculo semitransparente
        graphics.FillEllipse(&brush, centerX - radius, centerY - radius, radius * 2, radius * 2);
        
        // También resaltar las celdas dentro del rango
        Gdiplus::Pen pen(Gdiplus::Color(150, GetRValue(rangeColor), GetGValue(rangeColor), GetBValue(rangeColor)), 2);
        
        // Calcular las celdas dentro del rango
        for (int r = row - range; r <= row + range; r++) {
            for (int c = col - range; c <= col + range; c++) {
                // Calcular distancia euclidiana
                float dx = c - col;
                float dy = r - row;
                float distance = sqrtf(dx * dx + dy * dy);
                
                // Si está dentro del rango
                if (distance <= range) {
                    // Dibujar borde de la celda
                    graphics.DrawRectangle(&pen, c * cellSize, r * cellSize, cellSize, cellSize);
                }
            }
        }
    }
    catch (...) {
        OutputDebugStringW(L"Error al dibujar el rango de la torre\n");
    }
}

// Obtiene el tipo de proyectil para esta torre
ProjectileType Tower::GetProjectileType() const
{
    // Determinar el tipo de proyectil base
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
        baseProjectileType = ProjectileType::ARROW; // Default
        break;
    }

    // Decidir si es un ataque poderoso (20% de probabilidad)
    // std::rand() devuelve un valor entre 0 y RAND_MAX
    if ((std::rand() % 100) < 20) { // 20% de probabilidad
        switch (type) {
        case TowerType::ARCHER:
            return ProjectileType::FIREARROW;
        case TowerType::MAGE:
            return ProjectileType::PURPLEFIREBALL;
        case TowerType::GUNNER:
            return ProjectileType::NUKEBOMB;
        default:
            return baseProjectileType; // En caso de un tipo de torre no manejado para poderosos
        }
    }
    
    return baseProjectileType;
}

// Actualiza la lógica de la torre e intenta disparar hacia el objetivo más cercano
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
        float minDistanceSq = FLT_MAX; // Use squared distance to avoid sqrt

        for (const auto& enemy : enemies) {
            if (!enemy.IsActive() || !enemy.IsAlive()) {
                continue; // Skip inactive or dead enemies
            }

            // Harpy targeting rule: Gunners cannot target flying enemies
            if (type == TowerType::GUNNER && enemy.IsFlying()) {
                continue; 
            }
            // Note: Your requirement "Harpías: ... solo pueden ser atacadas por torres de magos y arqueros"
            // is covered if Gunners can't target flying, and Archers/Mages can.

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
            // Convert enemy pixel coordinates to target grid cell for AddProjectile
            // AddProjectile expects target grid cell, not precise pixel.
            // The projectile itself will fly towards the enemy's continuously updated pixel position if implemented that way.
            // For now, AddProjectile takes targetRow, targetCol which is a grid cell.
            int targetEnemyCellCol = static_cast<int>(closestEnemy->GetX() / cellSize);
            int targetEnemyCellRow = static_cast<int>(closestEnemy->GetY() / cellSize);

            projectileManager.AddProjectile(
                GetProjectileType(),
                row, col,                       
                targetEnemyCellRow, targetEnemyCellCol, 
                cellSize,
                closestEnemy->GetX(), closestEnemy->GetY() // Pass precise target coords for homing/direct shot
            );
            
            attackCooldown = 1.0f / GetAttackSpeed();
        }
        // No default firing if no valid enemy in range, tower waits.
    }
}

// Muestra/oculta el rango de la torre
void Tower::SetShowRange(bool show)
{
    showRange = show;
}

// Comprueba si el rango está visible
bool Tower::IsShowingRange() const
{
    return showRange;
}

//////////////////////////////////////////////////////////////
// Implementación de TowerManager
//////////////////////////////////////////////////////////////

TowerManager::TowerManager()
{
}

TowerManager::~TowerManager()
{
    // Solo liberamos referencias a las torres, no las eliminamos directamente
    // ya que podría causar problemas al cerrar la aplicación
    towers.clear();
}

// Inicializa el gestor de torres
void TowerManager::Initialize()
{
    // No hay nada específico que inicializar por ahora
}

// Agrega una nueva torre en la posición especificada
bool TowerManager::AddTower(TowerType type, int row, int col)
{
    // Comprobar si ya hay una torre en esa posición
    if (HasTower(row, col)) {
        return false;
    }
    
    // Crear una nueva torre
    Tower* newTower = new Tower(type, row, col);
    
    // Añadir la torre al vector
    towers.push_back(newTower);
    
    return true;
}

// Mejora la torre en la posición especificada
bool TowerManager::UpgradeTower(int row, int col)
{
    Tower* tower = GetTowerAt(row, col);
    
    if (!tower || !tower->CanUpgrade()) {
        return false;
    }
    
    return tower->Upgrade();
}

// Dibuja todas las torres
void TowerManager::DrawTowers(HDC hdc, int cellSize)
{
    for (Tower* tower : towers) {
        tower->Draw(hdc, cellSize);
    }
}

// Comprueba si hay una torre en la posición especificada
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