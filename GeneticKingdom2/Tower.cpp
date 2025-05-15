#include "framework.h"
#include "Tower.h"

// Constructor de la torre
Tower::Tower(TowerType type, int row, int col)
    : type(type), level(TowerLevel::LEVEL_1), row(row), col(col), 
      attackCooldown(0.0f), pTowerImage(NULL)
{
    // Cargar la imagen correspondiente al tipo y nivel
    LoadImage();
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
        baseRange = 3; // Rango medio
        break;
    case TowerType::GUNNER:
        baseRange = 2; // Rango corto
        break;
    case TowerType::MAGE:
        baseRange = 4; // Rango largo
        break;
    }
    
    // El rango aumenta con cada nivel
    return baseRange + static_cast<int>(level) - 1;
}

// Obtiene la velocidad de ataque (disparos por segundo)
float Tower::GetAttackSpeed() const
{
    // Velocidad base de ataque para cada tipo de torre
    float baseSpeed = 0.0f;
    
    switch (type) {
    case TowerType::ARCHER:
        baseSpeed = 1.0f; // 1 disparo por segundo (nivel 1)
        break;
    case TowerType::GUNNER:
        baseSpeed = 0.7f; // 0.7 disparos por segundo (nivel 1)
        break;
    case TowerType::MAGE:
        baseSpeed = 0.5f; // 0.5 disparos por segundo (nivel 1)
        break;
    }
    
    // La velocidad aumenta un 20% con cada nivel
    return baseSpeed * (1.0f + 0.2f * (static_cast<int>(level) - 1));
}

// Obtiene el daño base
int Tower::GetDamage() const
{
    // Daño base para cada tipo de torre
    int baseDamage = 0;
    
    switch (type) {
    case TowerType::ARCHER:
        baseDamage = 10; // Daño medio
        break;
    case TowerType::GUNNER:
        baseDamage = 15; // Daño alto
        break;
    case TowerType::MAGE:
        baseDamage = 8;  // Daño bajo (pero afecta a área)
        break;
    }
    
    // El daño aumenta con cada nivel
    return baseDamage * static_cast<int>(level);
}

// Actualiza la lógica de la torre (para luego)
void Tower::Update(float deltaTime)
{
    // Disminuir tiempo de recarga
    if (attackCooldown > 0) {
        attackCooldown -= deltaTime;
        if (attackCooldown < 0) {
            attackCooldown = 0;
        }
    }
    
    // Aquí se implementará luego la lógica de detección de enemigos y disparo
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

// Actualiza la lógica de todas las torres
void TowerManager::Update(float deltaTime)
{
    for (Tower* tower : towers) {
        tower->Update(deltaTime);
    }
} 