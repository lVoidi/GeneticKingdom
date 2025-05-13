# Reino Genético

Un juego de Tower Defense que implementa algoritmos genéticos y pathfinding en C++.

## Descripción del Proyecto

Este proyecto consiste en implementar un juego estilo "Tower Defense" ambientado en la edad media, donde se generan oleadas de enemigos de distintas clases y categorías. El jugador se encarga de colocar torres en lugares predeterminados para evitar que los enemigos crucen el puente del castillo. Después de cada oleada, los enemigos evolucionan, haciendo más difícil evitar que los enemigos crucen el puente.

## Requerimientos

### 1. Mapa (10 puntos)
- Cuadrícula de tamaño fijo definida por el equipo
- Las torres pueden colocarse en cualquiera de los cuadros sin bloquear todos los posibles caminos hacia el puente
- Único punto de ingreso de los enemigos en el lado contrario al puente del castillo

### 2. Atributos de las Torres (10 puntos)
- Atributos: daño, velocidad, alcance, tiempo de regeneración del poder especial, tiempo de recarga de ataque
- Las torres atacan a los enemigos cuando están en su alcance
- Oro recompensado basado en la categoría y tipo de enemigo

### 3. Mejoras de Torres (10 puntos)
- 3 mejoras por torre
- Cada mejora aumenta el daño y tiene un costo en oro
- Ataques especiales ocurren periódicamente con una probabilidad definida
- Cada mejora modifica los atributos

### 4. Tipos de Torres (5 puntos)
- Arqueros: bajo costo, alto alcance, poco daño, tiempo de recarga bajo
- Magos: costo medio, alcance medio, daño medio, tiempo de recarga medio
- Artilleros: costo alto, alcance bajo, daño alto, tiempo de recarga alto

### 5. Colocación de Torres (10 puntos)
- El jugador puede colocar torres en lugares disponibles
- El jugador puede escoger el tipo de torre al seleccionar un lugar disponible
- Cada torre tiene un costo en oro

### 6. Enemigos (20 puntos)
- Los enemigos aparecen por oleadas (generaciones que evolucionan)
- Tipos: Ogros, Elfos Oscuros, Harpías, Mercenarios
- Atributos: Vida, Velocidad, Resistencia a flechas, Resistencia a la magia, Resistencia a la artillería

### 7. Algoritmo Genético (10 puntos)
- Cada generación selecciona los individuos con el mejor fitness
- Los cruza e ingresa los nuevos individuos a la población
- Pueden ocurrir mutaciones con cierto grado de probabilidad
- Oleadas de tamaño variable generadas con un intervalo parametrizable

### 8. Pathfinding A* (20 puntos)
- Los enemigos utilizan Pathfinding A* para encontrar el camino hacia el puente del castillo

### 9. Panel de Estadísticas (5 puntos)
- Muestra: Generaciones transcurridas, Enemigos muertos en cada oleada, Fitness de cada individuo de la oleada, Nivel de cada torre, Probabilidad de mutaciones y cantidad de mutaciones ocurridas

## División de Tareas

### Responsabilidades de David:
- **Implementación del Mapa** (Requerimiento 1)
- **Atributos y Mecánicas de Torres** (Requerimiento 2)
- **Mejoras de Torres** (Requerimiento 3)
- **Tipos de Torres** (Requerimiento 4)
- **Panel de Estadísticas** (Requerimiento 9)

### Responsabilidades de Rodrigo:
- **Colocación de Torres** (Requerimiento 5)
- **Tipos y Atributos de Enemigos** (Requerimiento 6)
- **Implementación del Algoritmo Genético** (Requerimiento 7)
- **Pathfinding A*** (Requerimiento 8)
