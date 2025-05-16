#include "framework.h"
#include "Projectile.h"
#include <cmath>
#include "Map.h" // Para incluir la definición completa de DummyTarget

// Constructor de Projectile
Projectile::Projectile(ProjectileType type, int startRow, int startCol, int targetRow, int targetCol, int cellSize)
    : type(type), finished(false), pImage(NULL), cellSize(cellSize)
{
    // Convertir coordenadas de celda a píxeles (centro de la celda)
    x = (startCol + 0.5f) * cellSize;
    y = (startRow + 0.5f) * cellSize;
    targetX = (targetCol + 0.5f) * cellSize;
    targetY = (targetRow + 0.5f) * cellSize;
    
    // Calcular ángulo del proyectil
    float dx = targetX - x;
    float dy = targetY - y;
    angle = atan2f(dy, dx);
    
    // Establecer velocidad según el tipo de proyectil
    switch (type) {
    case ProjectileType::ARROW:
        speed = 300.0f;  // Flechas rápidas
        break;
    case ProjectileType::FIREBALL:
        speed = 200.0f;  // Bolas de fuego a velocidad media
        break;
    case ProjectileType::CANNONBALL:
        speed = 150.0f;  // Balas de cañón lentas pero potentes
        break;
    default:
        speed = 250.0f;
    }
    
    // Cargar la imagen correspondiente
    LoadImage();
}

// Destructor
Projectile::~Projectile()
{
    // No eliminamos la imagen, ya que GDI+ se encargará de eso
    pImage = NULL;
}

// Dibuja el proyectil
void Projectile::Draw(HDC hdc)
{
    if (pImage && pImage->GetLastStatus() == Gdiplus::Ok) {
        try {
            Gdiplus::Graphics graphics(hdc);
            
            // Configurar calidad de dibujo
            graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
            graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
            
            // Calcular rectángulo de destino (centrado en la posición actual)
            int imgWidth = cellSize * 0.6f;  // 60% del tamaño de celda - más grande que antes
            int imgHeight = cellSize * 0.6f; // Mantenemos proporciones cuadradas
            
            // Crear una transformación para rotar el objeto
            Gdiplus::Matrix matrix;
            
            // Método correcto de rotación alrededor de un punto
            matrix.RotateAt(
                angle * 180.0f / 3.14159f, // Convertir radianes a grados
                Gdiplus::PointF(x, y)      // Punto de rotación (centro del proyectil)
            );
            
            graphics.SetTransform(&matrix);
            
            // Dibujar la imagen con más tamaño
            Gdiplus::Rect destRect(
                static_cast<int>(x - imgWidth / 2),
                static_cast<int>(y - imgHeight / 2),
                imgWidth,
                imgHeight
            );
            
            // Dibujar la imagen
            graphics.DrawImage(
                pImage,
                destRect,
                0, 0, pImage->GetWidth(), pImage->GetHeight(),
                Gdiplus::UnitPixel
            );
            
            // Restaurar la transformación original
            graphics.ResetTransform();
            
            // Opcional: Dibujar un punto para depuración en el centro del proyectil
            Gdiplus::SolidBrush debugBrush(Gdiplus::Color(255, 255, 0, 0)); // Rojo opaco
            graphics.FillEllipse(&debugBrush, (Gdiplus::REAL)(x - 3), (Gdiplus::REAL)(y - 3), (Gdiplus::REAL)6, (Gdiplus::REAL)6);
        }
        catch (...) {
            OutputDebugStringW(L"Error al dibujar el proyectil\n");
        }
    } else {
        // Si no se cargó la imagen correctamente, dibujar un círculo de color como alternativa
        Gdiplus::Graphics graphics(hdc);
        Gdiplus::Color color;
        
        switch (type) {
        case ProjectileType::ARROW:
            color = Gdiplus::Color(255, 200, 200, 0); // Amarillo
            break;
        case ProjectileType::FIREBALL:
            color = Gdiplus::Color(255, 255, 100, 0); // Naranja
            break;
        case ProjectileType::CANNONBALL:
            color = Gdiplus::Color(255, 50, 50, 50);  // Gris oscuro
            break;
        default:
            color = Gdiplus::Color(255, 255, 255, 255); // Blanco
        }
        
        Gdiplus::SolidBrush brush(color);
        float radius = cellSize * 0.2f; // 20% del tamaño de celda
        graphics.FillEllipse(&brush, (Gdiplus::REAL)(x - radius), (Gdiplus::REAL)(y - radius), (Gdiplus::REAL)(radius * 2), (Gdiplus::REAL)(radius * 2));
    }
}

// Actualiza la posición del proyectil
bool Projectile::Update(float deltaTime)
{
    if (finished) {
        return false;
    }
    
    // Calcular desplazamiento en este frame
    float distance = speed * deltaTime;
    float dx = cosf(angle) * distance;
    float dy = sinf(angle) * distance;
    
    // Actualizar posición
    x += dx;
    y += dy;
    
    // Comprobar si ha llegado al objetivo (distancia cuadrática)
    float remainingDx = targetX - x;
    float remainingDy = targetY - y;
    float squaredDistance = remainingDx * remainingDx + remainingDy * remainingDy;
    
    // Si está cerca del objetivo (menos de 5 píxeles), marcar como terminado
    if (squaredDistance < 25.0f) {
        finished = true;
        return false;
    }
    
    return true;
}

// Comprueba si el proyectil ha llegado a su destino
bool Projectile::IsFinished() const
{
    return finished;
}

// Obtiene el tipo de proyectil
ProjectileType Projectile::GetType() const
{
    return type;
}

// Carga la imagen adecuada para el tipo de proyectil
bool Projectile::LoadImage()
{
    // Determinar el nombre de archivo según el tipo
    std::wstring fileName;
    
    switch (type) {
    case ProjectileType::ARROW:
        fileName = L"Arrow.png";
        break;
    case ProjectileType::FIREBALL:
        fileName = L"Fireball.png"; // Bola de fuego para torre Mage
        break;
    case ProjectileType::CANNONBALL:
        fileName = L"Bomb.png"; // Bomba para torre Gunner
        break;
    default:
        fileName = L"Arrow.png";
    }
    
    // Lista de posibles rutas para la imagen
    const wchar_t* possiblePaths[] = {
        L"Assets\\Projectiles\\",
        L"..\\GeneticKingdom2\\Assets\\Projectiles\\",
        L"GeneticKingdom2\\Assets\\Projectiles\\",
        L"C:\\Users\\Admin\\source\\repos\\GeneticKingdom2\\GeneticKingdom2\\Assets\\Projectiles\\"
    };
    
    // Intentar cargar desde una de las rutas posibles
    for (const wchar_t* basePath : possiblePaths) {
        std::wstring fullPath = basePath + fileName;
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            pImage = tempImage;
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de proyectil cargada correctamente: %s\n", fullPath.c_str());
            OutputDebugStringW(debugMsg);
            return true;
        }
        else if (tempImage) {
            tempImage = NULL;
        }
    }
    
    // Si no se pudo cargar, intentar con la ruta del ejecutable
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
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de proyectil cargada correctamente: %s\n", fullPath.c_str());
            OutputDebugStringW(debugMsg);
            return true;
        }
        else if (tempImage) {
            tempImage = NULL;
        }
    }
    
    WCHAR errorMsg[256];
    swprintf_s(errorMsg, L"Error al cargar la imagen del proyectil: %s\n", fileName.c_str());
    OutputDebugStringW(errorMsg);
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
void ProjectileManager::AddProjectile(ProjectileType type, int startRow, int startCol, int targetRow, int targetCol, int cellSize)
{
    WCHAR debugMsg[256];
    swprintf_s(debugMsg, L"Creando proyectil tipo %d desde [%d,%d] hacia [%d,%d]\n", 
              static_cast<int>(type), startRow, startCol, targetRow, targetCol);
    OutputDebugStringW(debugMsg);
    
    Projectile* newProjectile = new Projectile(type, startRow, startCol, targetRow, targetCol, cellSize);
    projectiles.push_back(newProjectile);
}

// Dibuja todos los proyectiles
void ProjectileManager::DrawProjectiles(HDC hdc)
{
    for (auto& projectile : projectiles) {
        projectile->Draw(hdc);
    }
}

// Actualiza todos los proyectiles
void ProjectileManager::Update(float deltaTime)
{
    // Usar un enfoque seguro para eliminar proyectiles completados
    auto it = projectiles.begin();
    while (it != projectiles.end()) {
        if ((*it)->Update(deltaTime) == false || (*it)->IsFinished()) {
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

// Verifica si el proyectil colisiona con un objetivo dummy
bool Projectile::CheckCollision(const DummyTarget& target, int cellSize) const
{
    // Obtener el centro del objetivo
    float targetCenterX = (target.col + 0.5f) * cellSize;
    float targetCenterY = (target.row + 0.5f) * cellSize;
    
    // Calcular distancia entre el proyectil y el objetivo
    float dx = x - targetCenterX;
    float dy = y - targetCenterY;
    float distance = sqrtf(dx * dx + dy * dy);
    
    // Radio de colisión del objetivo (30% del tamaño de celda)
    float targetRadius = cellSize * 0.3f;
    
    // Radio de colisión del proyectil (más pequeño para mayor precisión)
    float projectileRadius = cellSize * 0.15f;
    
    // Hay colisión si la distancia es menor que la suma de los radios
    return distance < (targetRadius + projectileRadius);
}

// Marca el proyectil como finalizado
void Projectile::SetFinished()
{
    finished = true;
}

// Comprueba colisiones con los objetivos dummy y devuelve los índices impactados
std::vector<size_t> ProjectileManager::CheckCollisions(const std::vector<DummyTarget>& targets, int cellSize)
{
    std::vector<size_t> hitTargets;
    
    // Para cada proyectil
    for (auto& projectile : projectiles) {
        // Para cada objetivo
        for (size_t i = 0; i < targets.size(); ++i) {
            // Si hay colisión, añadir el índice del objetivo a la lista
            if (projectile->CheckCollision(targets[i], cellSize)) {
                hitTargets.push_back(i);
                // Marcar el proyectil como terminado
                projectile->SetFinished();
                break; // Un proyectil solo puede impactar un objetivo
            }
        }
    }
    
    return hitTargets;
} 