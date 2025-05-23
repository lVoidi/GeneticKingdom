#pragma once

#include <Windows.h>
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#include <string>
#include <vector>

// Forward declaration
struct DummyTarget;
class Enemy;
class Economy;

// Tipos de proyectiles
enum class ProjectileType {
    ARROW,      // Flecha normal para torre Archer
    FIREBALL,   // Bola de fuego para torre Mage
    CANNONBALL, // Bala de cañón para torre Gunner
    FIREARROW,  // Flecha de fuego (poderosa) para torre Archer
    PURPLEFIREBALL, // Bola de fuego púrpura (poderosa) para torre Mage
    NUKEBOMB    // Bomba nuclear (poderosa) para torre Gunner
};

// Clase para manejar un proyectil individual
class Projectile {
public:
    Projectile(ProjectileType type, int startRow, int startCol, int targetRow, int targetCol, int cellSize, float targetActualX = -1.0f, float targetActualY = -1.0f);
    ~Projectile();

    // Dibuja el proyectil
    void Draw(HDC hdc);

    // Actualiza la posición del proyectil
    bool Update(float deltaTime);

    // bool IsFinished() const; // Replaced by IsActive
    // void SetFinished(); // Replaced by SetActive(false)

    // Obtiene el tipo de proyectil
    ProjectileType GetType() const;
    
    // Obtiene la posición actual del proyectil
    void GetPosition(float& outX, float& outY) const;
    
    // Verifica si el proyectil colisiona con un objetivo dummy
    bool CheckCollision(const Enemy& enemy, int cs) const;

    bool IsActive() const; 
    void SetActive(bool active);

    int GetDamage() const; // Needs implementation based on type

private:
    // Carga la imagen adecuada para el tipo de proyectil
    bool LoadImage();

    ProjectileType type;    // Tipo de proyectil
    float x, y;             // Posición actual
    float targetX, targetY; // Posición objetivo
    float initialTargetX, initialTargetY; // Store initial precise target if provided
    bool hasPreciseTarget;                // Flag if precise target was given
    float angle;            // Ángulo de movimiento
    float speed;            // Velocidad en píxeles por segundo
    bool isActive;          
    Gdiplus::Image* pImage; // Imagen del proyectil
    int cellSize;           // Tamaño de celda para calcular colisiones
};

// Gestor de proyectiles
class ProjectileManager {
public:
    ProjectileManager();
    ~ProjectileManager();

    // Añade un nuevo proyectil
    void AddProjectile(ProjectileType type, int startRow, int startCol, int targetRow, int targetCol, int cellSize, float targetActualX = -1.0f, float targetActualY = -1.0f);

    // Dibuja todos los proyectiles
    void DrawProjectiles(HDC hdc);

    // Actualiza todos los proyectiles
    void Update(float deltaTime);
    
    // Comprueba colisiones con los objetivos dummy y devuelve los índices impactados
    void CheckCollisions(std::vector<Enemy>& enemies, int cellSize, Economy& economy);
    
    // Obtiene la lista de proyectiles
    const std::vector<Projectile*>& GetProjectiles() const { return projectiles; }

private:
    std::vector<Projectile*> projectiles; // Lista de proyectiles activos
}; 