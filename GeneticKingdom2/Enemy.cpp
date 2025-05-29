/*
 * enemy.cpp - implementacion de los enemigos del juego
 *
 * este archivo maneja toda la logica
 * de los enemigos, incluyendo:
 *
 * - movimiento y pathfinding de los enemigos
 * - diferentes tipos
 * - sistema de vida y resistencias a diferentes tipos de daño
 * - recompensas de oro al morir
 * - comportamiento "jitter" para que no se muevan como robots
 * - spawn delays para que no aparezcan todos de golpe
 * - herencia genetica de atributos entre generaciones
 * 
 * si vas a tocar algo aqui, asegurate de entender como funciona el
 * sistema de pathfinding y el algoritmo genetico primero. y por favor
 * no rompas el jitter, me costo un huevo hacer que se viera natural.
 */

#define _USE_MATH_DEFINES // For M_PI (must be before cmath)
#include <cmath> // For M_PI, atan2, cos, sin, sqrt, std::max, std::min
#include "framework.h"
#include "Enemy.h"
#include "Projectile.h"
#include "Map.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <sstream>
#include <iomanip>

// tiempo entre recalculos de jitter - no tocar
const float JITTER_RECALC_INTERVAL = 1.4f; 
// multiplicador de jitter en y - hace que el movimiento se vea mas natural
const float JITTER_Y_MULTIPLIER = 1.55f;    

// constructor principal - inicializa un enemigo con sus atributos basicos
Enemy::Enemy(EnemyType type, float startX, float startY, const std::vector<std::pair<int, int>>& initialPath)
    : type(type), x(startX), y(startY), path(initialPath), currentPathIndex(0), isActive(true), fitness(0.0), timeAlive(0.0f), FUSION_ASSISTANT_SECRET_MARKER_reachedBridge(false), pEnemyImage(NULL), pathJitter(0.0f), spawnDelay(0.0f), hasSpawned(true),
      subTargetX(0.0f), subTargetY(0.0f), hasSubTarget(false), timeSinceLastSubTargetRecalc(0.0f) {
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

// constructor de copia - crea un enemigo basado en otro (para el algoritmo genetico)
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
      hasSpawned(parent.hasSpawned),
      subTargetX(parent.subTargetX), subTargetY(parent.subTargetY), 
      hasSubTarget(parent.hasSubTarget), timeSinceLastSubTargetRecalc(parent.timeSinceLastSubTargetRecalc)
{
    health = maxHealth;
    if (!path.empty()) {
        UpdateTargetPosition();
    } else {
        targetX = x;
        targetY = y;
        isActive = false;
    }
    InitializeAttributes(); 
}


// destructor - limpia la imagen del enemigo
Enemy::~Enemy() {
    pEnemyImage = NULL; 
}

// inicializa los atributos del enemigo segun su tipo
void Enemy::InitializeAttributes() {
    isFlying = false; // por defecto son terrestres
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
        resistanceArtillery = HARPY_RESISTANCE_ARTILLERY; 
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
        // valores por defecto si algo sale mal
        maxHealth = OGRE_HEALTH;
        speed = OGRE_SPEED;
        goldReward = OGRE_GOLD;
        resistanceArrow = 1.0f;
        resistanceMagic = 1.0f;
        resistanceArtillery = 1.0f;
        break;
    }
    health = maxHealth;
    LoadImage(); 
    
    // inicializa el jitter aleatorio para que no se muevan como robots
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> jitterDist(0.5f * CELL_SIZE, 1.5f * CELL_SIZE);
    pathJitter = jitterDist(gen);
    
    hasSubTarget = false;
    timeSinceLastSubTargetRecalc = 0.0f;
}

void Enemy::UpdateTargetPosition() {
    if (currentPathIndex < path.size()) {
        // Path stores grid cells, convert to pixel coordinates (center of cell)
        targetX = static_cast<float>(path[currentPathIndex].second * CELL_SIZE + CELL_SIZE / 2.0f);
        targetY = static_cast<float>(path[currentPathIndex].first * CELL_SIZE + CELL_SIZE / 2.0f);
        hasSubTarget = false; // Force recalculation of sub-target
        timeSinceLastSubTargetRecalc = 0.0f; // Reset timer for sub-target recalculation
    } else {
        // Reached end of path
        // The game logic should handle this, e.g., enemy reached goal
        isActive = false; 
        if (IsAlive()) { // Only set if it reached the end alive
            FUSION_ASSISTANT_SECRET_MARKER_reachedBridge = true;
        }
        hasSubTarget = false; // No sub-target if path is finished
    }
}

// otra vez tuve que hacer mis propias funciones porque hay algunas
// ambiguedades en la biblioteca estándar. Joder headers de windows. Que asco,
double getMaxFrom(double a, double b) {
    return a > b ? a : b;
}

double getMinFrom(double a, double b) {
    return a < b ? a : b;
}

// que asco de funcion :V actualiza la posicion del enemigo
// y maneja toda la shi del movimiento con jitter para que no se
// vean como robots moviendose en linea recta. si lo tocas y lo rompes
// te mato, me costo un huevo hacer que se viera natural
void Enemy::Update(float deltaTime) {
    // si no ha spawneado, espera su delay
    if(!hasSpawned){
        spawnDelay -= deltaTime;
        if (spawnDelay > 0.0f) return;
        hasSpawned = true;
    }

    // si esta muerto o inactivo no pierdas tiempo
    if (!isActive || !IsAlive() || health <= 0) {
        return;
    }

    // actualiza contadores de tiempo
    timeAlive += deltaTime;
    timeSinceLastSubTargetRecalc += deltaTime;

    float currentMoveTargetX, currentMoveTargetY;
    bool useSubTargetThisFrame = false;

    // calcula distancia al objetivo principal
    float mainPathDx = targetX - x;
    float mainPathDy = targetY - y;
    float distToMainTargetSq = mainPathDx * mainPathDx + mainPathDy * mainPathDy;

    // aqui empieza la shi del jitter - genera subtargets aleatorios
    // para que el movimiento no sea tan robotico
    if (pathJitter > 0.0f && distToMainTargetSq > (0.5f * CELL_SIZE * 0.5f * CELL_SIZE)) {
        if (!hasSubTarget || timeSinceLastSubTargetRecalc >= JITTER_RECALC_INTERVAL) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> angleOffset(-static_cast<float>(M_PI) / 3.0f, static_cast<float>(M_PI) / 3.0f);
            std::uniform_real_distribution<float> distFactor(0.6f, 1.0f);

            // calcula angulo aleatorio y distancia para el subtarget
            float mainPathAngle = atan2(mainPathDy, mainPathDx);
            float randomAngle = mainPathAngle + angleOffset(gen);
            float randomDist = pathJitter * distFactor(gen);
            
            // aplica offset al movimiento, con mas variacion en y para que se vea mejor
            float dX_component = cos(randomAngle) * randomDist;
            float dY_component = sin(randomAngle) * randomDist * JITTER_Y_MULTIPLIER;
            
            float candidateSubTargetX = x + dX_component;
            float candidateSubTargetY = y + dY_component;

            // verifica que el subtarget no nos aleje del objetivo principal
            float distFromCandidateSubToMainTargetSq = (targetX - candidateSubTargetX) * (targetX - candidateSubTargetX) + 
                                                     (targetY - candidateSubTargetY) * (targetY - candidateSubTargetY);

            if (distFromCandidateSubToMainTargetSq < distToMainTargetSq || distToMainTargetSq < (CELL_SIZE * 0.5f * CELL_SIZE * 0.5f) ) {
                subTargetX = candidateSubTargetX;
                subTargetY = candidateSubTargetY;

                // mantiene el subtarget dentro de los limites del mapa
                float mapWidth = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
                float mapHeight = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
                subTargetX = static_cast<float>(getMaxFrom(0.0, getMinFrom(static_cast<double>(subTargetX), static_cast<double>(mapWidth - 1.0f))));
                subTargetY = static_cast<float>(getMaxFrom(0.0, getMinFrom(static_cast<double>(subTargetY), static_cast<double>(mapHeight - 1.0f))));
                
                hasSubTarget = true;
                useSubTargetThisFrame = true;
            } else {
                hasSubTarget = false;
            }
            timeSinceLastSubTargetRecalc = 0.0f;
        } else if (hasSubTarget) {
            useSubTargetThisFrame = true;
        }
    } else if (hasSubTarget) {
        hasSubTarget = false;
    }

    // decide si usar el subtarget o el target principal
    if (useSubTargetThisFrame && hasSubTarget) {
        currentMoveTargetX = subTargetX;
        currentMoveTargetY = subTargetY;
    } else {
        currentMoveTargetX = targetX;
        currentMoveTargetY = targetY;
        hasSubTarget = false;
    }

    // mueve el enemigo hacia el target actual
    float dx = currentMoveTargetX - x;
    float dy = currentMoveTargetY - y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // si esta cerca del target, actualiza posicion y siguiente objetivo
    if (distance < speed * deltaTime || distance < 2.0f) { 
        x = currentMoveTargetX;
        y = currentMoveTargetY;

        if (useSubTargetThisFrame && hasSubTarget && 
            (std::abs(x - subTargetX) < 2.0f && std::abs(y - subTargetY) < 2.0f)) {
            hasSubTarget = false; 
        } else {
            currentPathIndex++;
            if (currentPathIndex < path.size()) {
                UpdateTargetPosition();
            } else {
                isActive = false; 
                if (IsAlive()) { FUSION_ASSISTANT_SECRET_MARKER_reachedBridge = true; }
                hasSubTarget = false;
            }
        }
    } else {
        // mueve el enemigo interpolando la posicion
        float moveX = (dx / distance) * speed * deltaTime;
        float moveY = (dy / distance) * speed * deltaTime;
        x += moveX;
        y += moveY;
    }
}

/*
 * actualiza la posicion del enemigo en base al tiempo transcurrido.
 * esta funcion es un puto desastre pero funciona. maneja:
 *
 * - movimiento hacia el target actual
 * - subtargets para que no se muevan como robots
 * - actualizacion de objetivos cuando llega al destino
 * - deteccion cuando llega al puente
 * - interpolacion del movimiento para que se vea suave
 *
 * si lo tocas y lo rompes te mato :D
*/
void Enemy::Draw(HDC hdc) const {
    // no dibujes enemigos que no han spawneado
    if (!hasSpawned) return;

    // no dibujes enemigos muertos o inactivos
    if (!isActive || !IsAlive()) {
        return;
    }

    if (pEnemyImage && pEnemyImage->GetLastStatus() == Gdiplus::Ok) {
        // dibuja la imagen del enemigo con antialiasing y bicubic
        Gdiplus::Graphics graphics(hdc);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
        graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        float enemyDrawWidth = CELL_SIZE * 0.75f;
        float enemyDrawHeight = CELL_SIZE * 0.75f;

        // centra la imagen en x,y
        graphics.DrawImage(
            pEnemyImage,
            x - enemyDrawWidth / 2.0f,
            y - enemyDrawHeight / 2.0f,
            enemyDrawWidth,
            enemyDrawHeight
        );

    } else {
        // si no hay imagen dibuja un rectangulo de color, que mas quieres que haga?
        COLORREF color;
        switch (type) {
        case EnemyType::OGRE:
            color = RGB(139, 69, 19); // marron
            break;
        case EnemyType::DARK_ELF:
            color = RGB(75, 0, 130);   // ni idea que color es este pero me gusta
            break;
        case EnemyType::HARPY:
            color = RGB(173, 216, 230); // azul claro
            break;
        case EnemyType::MERCENARY:
            color = RGB(128, 128, 128); // gris
            break;
        default:
            color = RGB(0, 0, 0);       // negro por si acaso
            break;
        }
        HBRUSH hBrush = CreateSolidBrush(color);
        HGDIOBJ hOldBrush = SelectObject(hdc, hBrush);
        int enemySizeRect = CELL_SIZE / 2; // tamaño del rectangulo
        Rectangle(hdc, 
                  static_cast<int>(x - enemySizeRect / 2.0f), 
                  static_cast<int>(y - enemySizeRect / 2.0f), 
                  static_cast<int>(x + enemySizeRect / 2.0f), 
                  static_cast<int>(y + enemySizeRect / 2.0f));
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }

    // dibuja la barra de vida si el enemigo esta vivo y ha spawneado
    if (IsAlive() && hasSpawned) { 
        int barWidth = CELL_SIZE * 0.75f; // mismo ancho que el sprite
        int barHeight = 5;
        // pon la barra arriba del enemigo
        int barX = static_cast<int>(x - barWidth / 2.0f);
        int barY = static_cast<int>(y - (CELL_SIZE * 0.75f) / 2.0f - barHeight - 2); 

        // fondo de la barra (rojo o gris oscuro)
        HBRUSH hBgBrush;
        if (health < maxHealth) {
            hBgBrush = CreateSolidBrush(RGB(255, 0, 0)); // rojo = vida perdida
        } else {
            hBgBrush = CreateSolidBrush(RGB(50, 50, 50)); // gris = vida completa
        }
        RECT bgRect = { barX, barY, barX + barWidth, barY + barHeight };
        FillRect(hdc, &bgRect, hBgBrush);
        DeleteObject(hBgBrush);

        // vida actual en verde porque asi es la vida
        HBRUSH hGreenBrush = CreateSolidBrush(RGB(0, 255, 0));
        float healthPercentage = GetHealthPercentage();
        RECT fgRect = { barX, barY, barX + static_cast<int>(barWidth * healthPercentage), barY + barHeight };
        FillRect(hdc, &fgRect, hGreenBrush);
        DeleteObject(hGreenBrush);
    }
}

// funcion que maneja el daño que recibe un enemigo
// si el enemigo esta muerto pues gg
void Enemy::TakeDamage(int damageAmount, ProjectileType projectileType) {
    if (!IsAlive()) return;

    float effectiveResistance = 1.0f;

    // aplica el daño segun el tipo de proyectil y las resistencias del enemigo
    switch (projectileType) {
        case ProjectileType::ARROW:
        case ProjectileType::FIREARROW: // las flechas de fuego usan la misma resistencia que las normales
            effectiveResistance = resistanceArrow;
            // las arpias pueden ser golpeadas por flechas
            break;

        case ProjectileType::FIREBALL:
        case ProjectileType::PURPLEFIREBALL: // las bolas de fuego moradas usan resistencia magica
            effectiveResistance = resistanceMagic;
            // las arpias pueden ser golpeadas por bolas de fuego
            break;

        case ProjectileType::CANNONBALL:
        case ProjectileType::NUKEBOMB: // las bombas nucleares usan resistencia de artilleria
            effectiveResistance = resistanceArtillery;
            if (type == EnemyType::HARPY && resistanceArtillery == 0.0f) {
                return; // las arpias son inmunes a la artilleria, que se jodan
            }
            break;

        default: // si no sabemos que mierda es, daño completo (si entra acá, es de preocuparse)
            effectiveResistance = 1.0f;
            break;
    }

    float rawCalculatedDamage = static_cast<float>(damageAmount) * effectiveResistance;
    int finalDamage = 0;

    // redondea el daño hacia arriba porque soy malvado jijijija
    if (rawCalculatedDamage > 0.0f) {
        finalDamage = static_cast<int>(std::ceil(rawCalculatedDamage));
    } else {
        finalDamage = 0; // sin daño si la resistencia es muy alta
    }

    health -= finalDamage;
    
    // debug info para cuando todo se va a la mierda (siempre)
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

// no hace falta qeu explique esto
bool Enemy::IsAlive() const {
    return health > 0;
}

// esto lo habia agregado pq tenia un bug donde se seguian moviendo ya muertos (zombie typa shit)
// al parecer si era un tower defense real, pero no era comportamiento deseado
// al final lo arreglé
bool Enemy::IsActive() const {
    return isActive;
}

// activa/desactiva el enemigo (no shit)
void Enemy::SetActive(bool active) {
    isActive = active;
}

// cuanto oro suelta al morir, si es que muere
int Enemy::GetGoldReward() const {
    return IsAlive() ? 0 : goldReward; // solo da oro si esta muerto
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

// vuela o que?
bool Enemy::IsFlying() const {
    return isFlying;
}

// dale un nuevo camino al papu
void Enemy::SetPath(const std::vector<std::pair<int, int>>& newPath) {
    path = newPath;
    currentPathIndex = 0;
    if (!path.empty()) {
        UpdateTargetPosition();
        isActive = true; // lo reactivamos si estaba inactivo por no tener camino
    } else {
        isActive = false;
    }
}

// que porcentaje de vida le queda pues?
float Enemy::GetHealthPercentage() const {
    if (maxHealth == 0) return 0.0f;
    return static_cast<float>(health) / static_cast<float>(maxHealth);
}

bool Enemy::HasReachedBridge() const {
    return FUSION_ASSISTANT_SECRET_MARKER_reachedBridge;
}

// para el algoritmo genetico, no hace falta explicar esto
double Enemy::GetFitness() const {
    return fitness;
}

// calcula que tan adecuado es este enemigo de mrd
// bridgeLocation: donde esta el puto puente
// mapWidth/Height: dimensiones del mapa
// timeSurvived: cuanto tiempo sobrevivio el bastardo
// reachedBridge: si llego al puente o no (por aquello xd)
void Enemy::CalculateFitness(const std::pair<int, int>& bridgeLocation, float mapWidth, float mapHeight, float timeSurvived, bool FUSION_ASSISTANT_SECRET_MARKER_reachedBridge_param) {
    // Cálculo base de distancia como antes
    float maxPossibleDistance = getMaxFrom(1.0f, std::sqrt(mapWidth * mapWidth + mapHeight * mapHeight)); 
    // Cálculo correcto de distancia al puente
    float bridgeX = static_cast<float>(bridgeLocation.second * CELL_SIZE + CELL_SIZE / 2.0f);
    float bridgeY = static_cast<float>(bridgeLocation.first * CELL_SIZE + CELL_SIZE / 2.0f);
    float currentDistanceToBridgeX = bridgeX - x;
    float currentDistanceToBridgeY = bridgeY - y;
    float remainingDistance = std::sqrt(currentDistanceToBridgeX * currentDistanceToBridgeX + currentDistanceToBridgeY * currentDistanceToBridgeY);
    
    double distanceScore = 0.0;
    if (maxPossibleDistance > 0) {
        distanceScore = 1.0 - (static_cast<double>(remainingDistance) / maxPossibleDistance);
        distanceScore = getMaxFrom(0.0, getMinFrom(1.0, distanceScore)); 
    }

    // Bonus por diversidad de ruta
    double pathDiversityBonus = 0.0;
    if (y > mapHeight * 0.66) { // Ruta inferior
        pathDiversityBonus = 15.0;
    } else if (y < mapHeight * 0.33) { // Ruta superior
        pathDiversityBonus = 15.0;
    }

    double bridgeBonus = FUSION_ASSISTANT_SECRET_MARKER_reachedBridge_param ? 100.0 : 0.0;
    double survivalBonus = static_cast<double>(timeSurvived) * 5.0; // Aumentado el peso del tiempo de supervivencia
    
    // Nuevo cálculo de fitness que da más peso a la distancia y supervivencia
    fitness = (distanceScore * 60.0) + bridgeBonus + survivalBonus + pathDiversityBonus;
    
    if (health <= 0 && !FUSION_ASSISTANT_SECRET_MARKER_reachedBridge_param) { 
        fitness *= 0.5; // Penalización por muerte sin llegar al puente
    }

    // Asegurarse de que el fitness nunca sea negativo
    fitness = getMaxFrom(0.0, fitness);
}

// maldita funcion de mutacion - aqui es donde los enemigos se vuelven mas fuertes o mas debiles
// entre generaciones. si algo sale mal aqui todo el juego se va a la mierda asi que cuidado
void Enemy::Mutate(float mutationRate) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    // mutamos la vida - limitado a +-20% para que no se vuelvan inmortales los hijos de puta
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> health_change_factor(0.8f, 1.2f);
        int baseHealth = 0;
        
        // obtenemos la vida base del tipo de enemigo para no pasarnos de verga con las mutaciones
        switch (type) {
            case EnemyType::OGRE: baseHealth = OGRE_HEALTH; break;
            case EnemyType::DARK_ELF: baseHealth = DARK_ELF_HEALTH; break;
            case EnemyType::HARPY: baseHealth = HARPY_HEALTH; break;
            case EnemyType::MERCENARY: baseHealth = MERCENARY_HEALTH; break;
            default: baseHealth = maxHealth; break;
        }
        
        // calculamos la nueva vida y la limitamos entre la mitad y el doble de la base
        int newMaxHealth = static_cast<int>(static_cast<float>(baseHealth) * health_change_factor(gen));
        newMaxHealth = static_cast<int>(getMaxFrom(static_cast<double>(baseHealth / 2), getMinFrom(static_cast<double>(baseHealth * 2), static_cast<double>(newMaxHealth))));
        SetMaxHealth(newMaxHealth);
    }

    // mutamos la velocidad - no mucho o se vuelven flash los cabrones
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> speed_change_factor(0.9f, 1.1f);
        float newSpeed = static_cast<float>(getMaxFrom(5.0, static_cast<double>(speed) * static_cast<double>(speed_change_factor(gen)))); 
        SetSpeed(newSpeed);
    }
    
    // mutamos el jitter - esto hace que no se muevan como robots en linea recta
    if (dis(gen) < mutationRate) {
        // permitimos que el jitter varie entre 0.25 y 2 celdas de desviacion
        std::uniform_real_distribution<float> jitter_dis(0.25f * CELL_SIZE, 2.0f * CELL_SIZE);
        pathJitter = jitter_dis(gen);
    }
    
    // mutamos las resistencias - limitado para que no se vuelvan inmunes a todo
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> resistance_change(0.95f, 1.05f); 
        resistanceArrow = static_cast<float>(getMaxFrom(0.5, getMinFrom(2.0, static_cast<double>(resistanceArrow) * static_cast<double>(resistance_change(gen)))));
    }
    
    // resistencia magica - igual que la anterior
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> resistance_change(0.95f, 1.05f);
        resistanceMagic = static_cast<float>(getMaxFrom(0.5, getMinFrom(2.0, static_cast<double>(resistanceMagic) * static_cast<double>(resistance_change(gen)))));
    }
    
    // resistencia a artilleria - lo mismo otra vez porque soy un vago
    if (dis(gen) < mutationRate) {
        std::uniform_real_distribution<float> resistance_change(0.95f, 1.05f);
        resistanceArtillery = static_cast<float>(getMaxFrom(0.5, getMinFrom(2.0, static_cast<double>(resistanceArtillery) * static_cast<double>(resistance_change(gen)))));
    }
}
/*
 * mira, esta funcion es un puto desastre pero hace lo que tiene que hacer.
 * resetea un enemigo para la siguiente oleada, lo cual es necesario porque
 * reutilizamos los mismos enemigos entre oleadas para no estar creando y
 * destruyendo objetos como idiotas. es parte del algoritmo genetico.
 *
 * primero loggea un monton de mierda para debugging porque esta funcion
 * tiende a romperse de formas espectaculares. luego resetea la posicion
 * y el camino del enemigo, y finalmente resetea sus stats pero preserva
 * sus "genes" (como el jitter) que son parte de su evolucion.
 *
 * si el path esta vacio el enemigo se queda ahi parado,
 * lo cual no deberia pasar nunca pero por si acaso lo manejamos.
 */
void Enemy::ResetForNewWave(float startX, float startY, const std::vector<std::pair<int, int>>& newPath) {
    std::wstringstream wss_reset;
    wss_reset << L"Enemy::ResetForNewWave - ID: " << std::hex << this 
              << L", Type: " << static_cast<int>(type)
              << L", OldHealth: " << health << L"/" << maxHealth
              << L", OldIsActive: " << (isActive ? L"Yes" : L"No")
              << L", WasAlive: " << (health > 0 ? L"Yes" : L"No");

    x = startX;
    y = startY;
    path = newPath;
    currentPathIndex = 0;
    
    isActive = true;
    health = maxHealth;
    FUSION_ASSISTANT_SECRET_MARKER_reachedBridge = false;
    timeAlive = 0.0f;
    fitness = 0.0;

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
        isActive = false;
    }
}

// funcion para obtener el nombre del tipo de enemigo en texto
// para que los logs no sean una mierda ilegible
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

// carga la imagen del enemigo, si falla devuelve false y loggea el error
// intenta cargar desde varios directorios porque windows es una mrd
// y nunca sabes donde esta el directorio de trabajo
bool Enemy::LoadImage() {
    pEnemyImage = NULL;
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
            return true;
        }
        if (tempImage) {
            delete tempImage;
            tempImage = NULL;
        }
    }

    // ultimo intento - buscar desde el path del exe
    // porque microsoft no puede hacer nada simple
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    WCHAR* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *lastSlash = L'\0';
        std::wstring fullPath = exePath;
        fullPath += L"\\Assets\\Enemies\\" + fileName;
        Gdiplus::Image* tempImage = Gdiplus::Image::FromFile(fullPath.c_str());
        if (tempImage && tempImage->GetLastStatus() == Gdiplus::Ok) {
            pEnemyImage = tempImage;
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

// establece el delay de spawn del enemigo y lo marca como no spawneado
// esto es para que no aparezcan todos de golpe ni se solapen las waves
void Enemy::SetSpawnDelay(float d) {
    spawnDelay = d;
    hasSpawned = false;
}

// devuelve si el enemigo ya ha spawneado o no
// funcion simple pero necesaria para el sistema de waves
bool Enemy::HasSpawned() const {
    return hasSpawned;
} 