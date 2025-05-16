#pragma once

#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <string>
#include <vector>

// Forward declaration
struct DummyTarget;

// Tipos de proyectiles
enum class ProjectileType {
    ARROW,      // Flecha normal para torre Archer
    FIREBALL,   // Bola de fuego para torre Mage
    CANNONBALL  // Bala de cañón para torre Gunner
};

// Clase para manejar un proyectil individual
class Projectile {
public:
    Projectile(ProjectileType type, int startRow, int startCol, int targetRow, int targetCol, int cellSize);
    ~Projectile();

    // Dibuja el proyectil
    void Draw(HDC hdc);

    // Actualiza la posición del proyectil
    bool Update(float deltaTime);

    // Comprueba si el proyectil ha llegado a su destino
    bool IsFinished() const;
    
    // Marca el proyectil como finalizado
    void SetFinished();

    // Obtiene el tipo de proyectil
    ProjectileType GetType() const;
    
    // Obtiene la posición actual del proyectil
    void GetPosition(float& outX, float& outY) const;
    
    // Verifica si el proyectil colisiona con un objetivo dummy
    bool CheckCollision(const DummyTarget& target, int cellSize) const;

private:
    // Carga la imagen adecuada para el tipo de proyectil
    bool LoadImage();

    ProjectileType type;    // Tipo de proyectil
    float x, y;             // Posición actual
    float targetX, targetY; // Posición objetivo
    float angle;            // Ángulo de movimiento
    float speed;            // Velocidad en píxeles por segundo
    bool finished;          // Indica si el proyectil ha terminado su recorrido
    Gdiplus::Image* pImage; // Imagen del proyectil
    int cellSize;           // Tamaño de celda para calcular colisiones
};

// Gestor de proyectiles
class ProjectileManager {
public:
    ProjectileManager();
    ~ProjectileManager();

    // Añade un nuevo proyectil
    void AddProjectile(ProjectileType type, int startRow, int startCol, int targetRow, int targetCol, int cellSize);

    // Dibuja todos los proyectiles
    void DrawProjectiles(HDC hdc);

    // Actualiza todos los proyectiles
    void Update(float deltaTime);
    
    // Comprueba colisiones con los objetivos dummy y devuelve los índices impactados
    std::vector<size_t> CheckCollisions(const std::vector<DummyTarget>& targets, int cellSize);
    
    // Obtiene la lista de proyectiles
    const std::vector<Projectile*>& GetProjectiles() const { return projectiles; }

private:
    std::vector<Projectile*> projectiles; // Lista de proyectiles activos
}; 