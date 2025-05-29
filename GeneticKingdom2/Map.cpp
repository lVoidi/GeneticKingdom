#include "framework.h"
#include "Map.h"
#include "Enemy.h"
#include <wincodec.h>
#include <vector>
#include <queue>
#include <cmath>
#include <algorithm>
#include <map>
#include <sstream>
#include <iomanip>
#include <numeric>


// Esta basura me tiene CANSADO siempre windows.h jodiendo con su macro std::max
// tuve que hacer esta funcion para evitar conflictos 
/*
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▒▒▒▒▒▓▓▓▒▒▒▓▓▒▓▓▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓▓▓▓▓▓▓▓▓▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▓███▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓███▓▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓█▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
█▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▒▒▒▒▓▓▓▓▓▓▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▓▓▓▓▓▓▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓█▓▓▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▓▓▓▓▓▓▓▓▓▓▓███▓▓▓████▓▓▓▓▓▓▓▓█▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓██▓██████▓▓▓▓▓▓▓▓▓▓▓▓▓███▓▓▓██▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▓▓▓▓▓▓▓▓▓█████████▓▓▓▓▓▓██████▓▓▓▓▓▓██▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓███████▓▓▓▓▓▓▓█████▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓█▓▓▓█▓▓▓▓▓▓▓████▓▓▓▓▓▓█████▓▓▓▓█▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓█▓▓████▓▓▓▓█▓▓▓████████▓▓▓▓▓███▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓███████████▓███▓▓▓▓▓▓▓██▓█▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓████████▓▓▓▓▓▓▓▓▓▓▓▓█▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓██████▓▓▓▓▓▓▓▓▓▓▓▓▓▓█▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▓█▓▓▓▒▒▒▓▓▒▒▒▒▒▒▒▓▒▒▓▓▓▓▓▓█▓▓████████████████████▓▓▓▓▓███▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▒▓█▓██▓▓▒▒▒▒▓▓▓█▓▓▓▒▓▓▓▓▓▓▓▓▓██████████████████▓████▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▓█▓▓▓████▓▓▓▓▒▓▓▓▓█▓▓▓▒▓▓▓▓▓▓▓▓████████████████████▓▓▓▓▓▓▓▓▓▒▒▓▓▓▓▓▓▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
█▓▓▓▓███▓▓███▓▓▒▓▓▓▓▓▓▓▒▒▒▒▓▓▓▓▓▓▓▓██████████████▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
▓▓▓██▓████▓▓███▓▒▓▓▓▓▓▓▒▒▒▒▒▓▓▓▓▓▓▓▓▓██████████▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒
██▓▓███▓▓██▓▓▓███████▓▓▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒
█████████████▓▓█████▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▒▒▒
███████████████████▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▓▒▒▒▒
███████████████████▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▓▓▒▒▒▒▒▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓██▓▒▒▒▒▒
██████████████████▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒
█████████████████▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▓▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒
███▓██▓▓▓▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒
▓▓█▓████▓▓▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▓▓▓▓▒▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓
▓▓▓▓▒▒▓▓▓██▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▓▓▒▒▒▒▒▒▒▒▒▒▒
▓▓▓▓▓▒▓▓▓▓▓▓██▓▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▓▓▒▒▒▒▒▒▒▒▒▒
▓▓▓██▓▓▒▒▒▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▓▓▒▒▒▒▒▒▒▒▒
▓██▓▓██▓▒▓███▓▓▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▓▓▒▒▒▓▓▒▒▒▒▒▒▒▒
▓▒█▓▓▓██▓▓▓▓▓█▓▓▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▒▒▒▓▓▒▒▒▒▒▒▒
▓▓▓██▓▓████████▓▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▒▓▓▓▒▒▒▒▒▒
▓██▓██▓▓██▓▓▓▓█▓▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▓▓▓▓▓▓█▓▓▓▓▓▓▒▒▒▒▒
▓█▓▓▓██▓████▓█▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▓▓▓▓▓▓▓█▓▓▓▓▓▓▒▒▒▒
▓▓████████▓█▓█▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓███▓▓▓▓▒▒▒▒
███▓▓█████████▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓██████▓▒▒▒▒
▓▓▓▓▓██████▓█▓▒▓▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓███████▓▒▒▒
▓▓█▓▓███████▓▓▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▓▓▓▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▓▒▓▓▓▓▓█████▓██▓▓▒▒
*/
int getMaxInt(int a, int b) {
    return (a > b) ? a : b;
}

#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880f
#endif

namespace {
    /*
     * mira, este struct es para el pathfinding a*. es bastante simple:
     * - guarda la posicion (row,col) del nodo
     * - g_cost es el costo real desde el inicio hasta aqui
     * - h_cost es la estimacion heuristica hasta el objetivo
     * - f_cost es la suma de ambos, que usamos para decidir que nodo expandir
     * - parent guarda de donde venimos para reconstruir el camino
     * 
     * nada del otro mundo, pero si lo rompes todo el pathfinding se va a la mierda
     */
    struct AStarNode {
        int row, col;
        float gCost;
        float hCost;
        float fCost;
        std::pair<int, int> parent;

        AStarNode(int r, int c, float g, float h, std::pair<int, int> p = {-1, -1})
            : row(r), col(c), gCost(g), hCost(h), fCost(g + h), parent(p) {}

        bool operator>(const AStarNode& other) const {
            return fCost > other.fCost;
        }
    };

    /*
     * funcion simple que calcula la distancia euclidiana entre dos puntos
     * la usamos como heuristica para a*. podriamos usar manhattan pero
     * esta nos da mejores resultados para movimiento en 8 direcciones
     */
    float CalculateHeuristic(int r1, int c1, int r2, int c2) {
        float dr = static_cast<float>(r1 - r2);
        float dc = static_cast<float>(c1 - c2);
        return std::sqrt(dr * dr + dc * dc);
    }
}

/*
 * constructor del mapa. inicializa todo a valores por defecto y crea
 * los recursos graficos que necesitamos:
 * - un pen gris para dibujar la cuadricula
 * - un brush amarillo para marcar donde se puede construir
 * 
 * nada especial, pero sin esto el mapa se veria como el culo
 */
Map::Map() : numRows(0), numCols(0), entryRow(0), entryCol(0), gridPen(NULL), constructionSpotBrush(NULL),
            pConstructionImage(NULL), constructionState(ConstructionState::NONE), selectedRow(-1), selectedCol(-1) {
    gridPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
    constructionSpotBrush = CreateSolidBrush(RGB(255, 255, 0));
}

/*
 * destructor del mapa. limpia todos los recursos que creamos
 * si no haces esto windows se queja y tienes memory leaks
 * y nadie quiere memory leaks, verdad?
 */
Map::~Map() {
    if (gridPen) {
        DeleteObject(gridPen);
        gridPen = NULL;
    }
    if (constructionSpotBrush) {
        DeleteObject(constructionSpotBrush);
        constructionSpotBrush = NULL;
    }
    if (pConstructionImage) {
        pConstructionImage = NULL;
    }
}

/*
 * esta funcion es la que inicializa todo el mapa del juego.
 * toma el ancho y alto de la pantalla y configura una cuadricula basada en celdas.
 * es simple:
 * - divide la pantalla en celdas del mismo tamaño 
 * - crea una matriz 2d para el grid
 * - pone el punto de entrada a la izquierda en el medio (duh, obvio)
 * - dibuja un puente en el lado derecho que los enemigos intentaran destruir
 *
 * si vas a tocar esto, no la cagues pq el pathfinding y todo lo demas depende
 * de que esta estructura base este bien hecha :V 
 */
void Map::Initialize(int screenWidth, int screenHeight) {
    numRows = screenHeight / CELL_SIZE;
    numCols = screenWidth / CELL_SIZE;

    grid.resize(numRows);
    for (int i = 0; i < numRows; i++) {
        grid[i].resize(numCols);
    }

    entryRow = numRows / 2;
    entryCol = 0;
    grid[entryRow][entryCol].isEntryPoint = true;

    int bridgeWidth = numCols / 10; 
    int bridgeStart = numCols - bridgeWidth;
    int bridgeHeight = numRows / 10; 
    int bridgeTop = (numRows - bridgeHeight) / 2;

    for (int row = bridgeTop; row < bridgeTop + bridgeHeight; row++) {
        for (int col = bridgeStart; col < numCols; col++) {
            grid[row][col].isBridge = true;
        }
    }

    LoadConstructionSpots();
    // quito esto xd
    //LoadConstructionImage();
    economy.Initialize(500);
}

// carga la imagen de construccion desde varias rutas posibles
// Solo que al final no uso esta funcion pq no
/*
bool Map::LoadConstructionImage() {
    if (pConstructionImage) {
        delete pConstructionImage;
        pConstructionImage = NULL;
    }
    
    const WCHAR* possiblePaths[] = {
        L"Assets\\Towers\\Construction.png",
        L"..\\GeneticKingdom2\\Assets\\Towers\\Construction.png",
        L"GeneticKingdom2\\Assets\\Towers\\Construction.png",
        L"C:\\Users\\Admin\\source\\repos\\GeneticKingdom2\\GeneticKingdom2\\Assets\\Towers\\Construction.png"
    };
    
    for (const WCHAR* path : possiblePaths) {
        pConstructionImage = Gdiplus::Image::FromFile(path);
        if (pConstructionImage && pConstructionImage->GetLastStatus() == Gdiplus::Ok) {
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de construcción cargada correctamente: %s\n", path);
            OutputDebugStringW(debugMsg);
            return true;
        } else if (pConstructionImage) {
            delete pConstructionImage;
            pConstructionImage = NULL;
        }
    }
    
    WCHAR exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    
    WCHAR* lastSlash = wcsrchr(exePath, L'\\');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = L'\0';
        
        WCHAR fullPath[MAX_PATH];
        
        wcscpy_s(fullPath, exePath);
        wcscat_s(fullPath, L"Assets\\Towers\\Construction.png");
        pConstructionImage = Gdiplus::Image::FromFile(fullPath);
        
        if (pConstructionImage && pConstructionImage->GetLastStatus() == Gdiplus::Ok) {
            WCHAR debugMsg[256];
            swprintf_s(debugMsg, L"Imagen de construcción cargada correctamente: %s\n", fullPath);
            OutputDebugStringW(debugMsg);
            return true;
        } else if (pConstructionImage) {
            delete pConstructionImage;
            pConstructionImage = NULL;
        }
    }
    
    OutputDebugStringW(L"Error al cargar la imagen de construcción\n");
    pConstructionImage = NULL;
    return false;
}
*/


/*
 * funcion que configura los puntos de construccion en el grid.
 * esta cosa es crucial porque define donde puede el jugador poner sus torres.
 * 
 * basicamente divide el mapa en hileras horizontales:
 * - hilera superior (2 filas)
 * - hilera central (1 fila) 
 * - hilera inferior (2 filas)
 * y agrega algunos puntos extra entre ellas para que no sea tan aburrido
 */
void Map::LoadConstructionSpots() {
    // primero limpiamos toda la mierda anterior
    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            grid[r][c].isConstructionSpot = false;
        }
    }
    constructionSpots.clear();

    // calcula las posiciones de las hileras, matematica basica
    int hileraSuperior_r1 = numRows / 4 - 1;
    int hileraSuperior_r2 = numRows / 4;

    int hileraCentral_r = numRows / 2;

    int hileraInferior_r1 = 3 * numRows / 4;
    int hileraInferior_r2 = 3 * numRows / 4 + 1;

    // define el rango horizontal de las hileras, dejando margenes
    int colStartHileras = numCols / 5;
    int colEndHileras = 4 * numCols / 5;

    // lambda para agregar spots individuales, verifica que no joda otros elementos
    auto addSpot = [&](int r, int c) {
        if (r >= 0 && r < numRows && c >= 0 && c < numCols && 
            !grid[r][c].isEntryPoint && !grid[r][c].isBridge && !grid[r][c].isConstructionSpot) {
            
            bool isPartOfExistingBlock = false;
            if ((r == hileraSuperior_r1 || r == hileraSuperior_r2 || r == hileraCentral_r || r == hileraInferior_r1 || r == hileraInferior_r2) && (c >= colStartHileras && c <= colEndHileras)) {
                isPartOfExistingBlock = true;
            }
            if (!isPartOfExistingBlock) {
                grid[r][c].isConstructionSpot = true;
                constructionSpots.push_back({r, c});
            }
        }
    };

    // lambda para agregar spots en las hileras principales
    auto addSpotToHilera = [&](int r, int c) {
         if (r >= 0 && r < numRows && c >= 0 && c < numCols && 
            !grid[r][c].isEntryPoint && !grid[r][c].isBridge) {
            grid[r][c].isConstructionSpot = true;
            constructionSpots.push_back({r, c});
        }
    };

    // agrega los spots en la hilera superior
    if (hileraSuperior_r1 >= 0 && hileraSuperior_r2 < numRows) {
        for (int c = colStartHileras; c <= colEndHileras; ++c) {
            addSpotToHilera(hileraSuperior_r1, c);
            addSpotToHilera(hileraSuperior_r2, c);
        }
    }

    // agrega los spots en la hilera central
    if (hileraCentral_r >= 0 && hileraCentral_r < numRows) {
        for (int c = colStartHileras; c <= colEndHileras; ++c) {
            addSpotToHilera(hileraCentral_r, c);
        }
    }

    // agrega los spots en la hilera inferior
    if (hileraInferior_r1 >= 0 && hileraInferior_r2 < numRows) {
        for (int c = colStartHileras; c <= colEndHileras; ++c) {
            addSpotToHilera(hileraInferior_r1, c);
            addSpotToHilera(hileraInferior_r2, c);
        }
    }

    // agrega spots extra entre las hileras para que no sea tan predecible
    int midRow1 = (hileraSuperior_r2 + hileraCentral_r) / 2;
    int central_col_start = colStartHileras + (colEndHileras - colStartHileras) / 3;
    int central_col_end = colEndHileras - (colEndHileras - colStartHileras) / 3;
    int num_central_spots = 3; 
    int central_spacing = (central_col_end - central_col_start) / getMaxInt(1, num_central_spots -1);
    
    if (midRow1 > hileraSuperior_r2 && midRow1 < hileraCentral_r) { 
        for (int i = 0; i < num_central_spots; ++i) {
            int c = central_col_start + i * central_spacing;
            if (num_central_spots == 1) c = (central_col_start + central_col_end) / 2; 
            addSpot(midRow1, c);
        }
    }

    // mas spots extra en la parte inferior
    int midRow2 = (hileraCentral_r + hileraInferior_r1) / 2 + 1; 
    if (midRow2 > hileraCentral_r && midRow2 < hileraInferior_r1 +1) { 
         for (int i = 0; i < num_central_spots; ++i) {
            int c = central_col_start + i * central_spacing;
            if (num_central_spots == 1) c = (central_col_start + central_col_end) / 2;
            addSpot(midRow2, c);
        }
    }

    // debug info para los spots en las esquinas
    std::wstringstream wss_spots_debug;
    wss_spots_debug << L"Map::LoadConstructionSpots - DEBUGGING CORNER SPOTS:\n";

    // agrega spots en las esquinas para cubrir angulos muertos
    int r_ul1 = hileraSuperior_r1 - 2, c_ul1 = colStartHileras + 2;
    wss_spots_debug << L"  Attempting UL1: (" << r_ul1 << L"," << c_ul1 << L") Bridge? " << (grid[r_ul1][c_ul1].isBridge ? L"Y":L"N") << L" Entry? " << (grid[r_ul1][c_ul1].isEntryPoint ? L"Y":L"N") << L"\n";
    addSpot(r_ul1, c_ul1); 

    int r_ul2 = hileraSuperior_r2 + 2, c_ul2 = colStartHileras - 2;
    wss_spots_debug << L"  Attempting UL2: (" << r_ul2 << L"," << c_ul2 << L") Bridge? " << (grid[r_ul2][c_ul2].isBridge ? L"Y":L"N") << L" Entry? " << (grid[r_ul2][c_ul2].isEntryPoint ? L"Y":L"N") << L"\n";
    addSpot(r_ul2, c_ul2); 

    int r_lr1 = hileraInferior_r1 - 2, c_lr1 = colEndHileras + 2;
    if(r_lr1 >=0 && r_lr1 < numRows && c_lr1 >= 0 && c_lr1 < numCols)
      wss_spots_debug << L"  Attempting LR1: (" << r_lr1 << L"," << c_lr1 << L") Bridge? " << (grid[r_lr1][c_lr1].isBridge ? L"Y":L"N") << L" Entry? " << (grid[r_lr1][c_lr1].isEntryPoint ? L"Y":L"N") << L"\n";
    else wss_spots_debug << L"  Attempting LR1: (" << r_lr1 << L"," << c_lr1 << L") - OUT OF BOUNDS FOR LOGGING\n";
    addSpot(r_lr1, c_lr1); 

    int r_lr2 = hileraInferior_r2 + 2, c_lr2 = colEndHileras - 2;
     if(r_lr2 >=0 && r_lr2 < numRows && c_lr2 >= 0 && c_lr2 < numCols)
      wss_spots_debug << L"  Attempting LR2: (" << r_lr2 << L"," << c_lr2 << L") Bridge? " << (grid[r_lr2][c_lr2].isBridge ? L"Y":L"N") << L" Entry? " << (grid[r_lr2][c_lr2].isEntryPoint ? L"Y":L"N") << L"\n";
    else wss_spots_debug << L"  Attempting LR2: (" << r_lr2 << L"," << c_lr2 << L") - OUT OF BOUNDS FOR LOGGING\n";
    addSpot(r_lr2, c_lr2); 
    OutputDebugStringW(wss_spots_debug.str().c_str());

    // elimina duplicados porque somos programadores decentes
    std::sort(constructionSpots.begin(), constructionSpots.end());
    constructionSpots.erase(std::unique(constructionSpots.begin(), constructionSpots.end()), constructionSpots.end());

    // debug final para ver cuantos spots quedaron
    WCHAR msg[128];
    swprintf_s(msg, L"Map::LoadConstructionSpots - Loaded %zu construction spots.\n", constructionSpots.size());
    OutputDebugStringW(msg);
}

// funcion que dibuja todo el mapa
void Map::Draw(HDC hdc) {
    HPEN oldPen = (HPEN)SelectObject(hdc, gridPen);

    // dibuja las lineas horizontales del grid, que emocionante...
    for (int row = 0; row <= numRows; row++) {
        MoveToEx(hdc, 0, row * CELL_SIZE, NULL);
        LineTo(hdc, numCols * CELL_SIZE, row * CELL_SIZE);
    }

    // y ahora las verticales, porque no podemos tener solo horizontales verdad?
    for (int col = 0; col <= numCols; col++) {
        MoveToEx(hdc, col * CELL_SIZE, 0, NULL);
        LineTo(hdc, col * CELL_SIZE, numRows * CELL_SIZE);
    }

    // dibuja los spots de construccion con un color marron horrible
    // porque alguien penso que seria buena idea (yo xd)
    HBRUSH constructionSpotFillBrush = CreateSolidBrush(RGB(210, 180, 140));
    HPEN constructionSpotBorderPen = CreatePen(PS_SOLID, 1, RGB(160, 120, 90));
    HGDIOBJ oldBrushForSpots = SelectObject(hdc, constructionSpotFillBrush);
    HGDIOBJ oldPenForSpots = SelectObject(hdc, constructionSpotBorderPen);

    // recorre todo el grid buscando spots de construccion
    // y los dibuja si no hay una torre ya construida ahi
    for (int r = 0; r < numRows; ++r) {
        for (int c = 0; c < numCols; ++c) {
            if (grid[r][c].isConstructionSpot && !towerManager.HasTower(r,c)) {
                Rectangle(hdc, c * CELL_SIZE, r * CELL_SIZE, (c + 1) * CELL_SIZE, (r + 1) * CELL_SIZE);
            }
        }
    }
    SelectObject(hdc, oldBrushForSpots);
    SelectObject(hdc, oldPenForSpots);
    DeleteObject(constructionSpotFillBrush);
    DeleteObject(constructionSpotBorderPen);

    // dibuja los rangos y las torres, porque aparentemente
    // necesitamos ver esos circulos todo el tiempo
    towerManager.DrawTowerRanges(hdc, CELL_SIZE);
    towerManager.DrawTowers(hdc, CELL_SIZE);
    projectileManager.DrawProjectiles(hdc);
    
    // dibuja el punto de entrada en rojo brillante
    // para que los enemigos sepan por donde entrar, genial
    HBRUSH entryBrush = CreateSolidBrush(RGB(255, 0, 0));
    RECT entryRect = { entryCol * CELL_SIZE, entryRow * CELL_SIZE,
                      (entryCol + 1) * CELL_SIZE, (entryRow + 1) * CELL_SIZE };
    FillRect(hdc, &entryRect, entryBrush);
    DeleteObject(entryBrush);

    // dibuja el puente en azul, porque que otro color podria ser?
    HBRUSH bridgeBrush = CreateSolidBrush(RGB(0, 0, 255));
    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
            if (grid[row][col].isBridge) {
                RECT bridgeRect = { col * CELL_SIZE, row * CELL_SIZE,
                                   (col + 1) * CELL_SIZE, (row + 1) * CELL_SIZE };
                FillRect(hdc, &bridgeRect, bridgeBrush);
            }
        }
    }
    DeleteObject(bridgeBrush);

    // dibuja la economia en la esquina superior derecha
    // porque el dinero es importante, no?
    economy.Draw(hdc, GetSystemMetrics(SM_CXSCREEN) - 220, 20);

    // Mostrar estadísticas en la esquina superior izquierda
    RECT statsRect = {
        20,  
        20,  
        300, 
        300  
    };

    // Crear fuente para las estadísticas con tamaño más grande
    HFONT statsFont = CreateFontW(
        24,                    
        0,                     
        0,                     
        0,                     
        FW_BOLD,              
        FALSE,                
        FALSE,                
        FALSE,                
        DEFAULT_CHARSET,      
        OUT_OUTLINE_PRECIS,   
        CLIP_DEFAULT_PRECIS,  
        CLEARTYPE_QUALITY,    
        DEFAULT_PITCH | FF_SWISS, 
        L"Arial"              
    );

    HFONT oldFont = (HFONT)SelectObject(hdc, statsFont);
    SetBkMode(hdc, TRANSPARENT);

   
    int yOffset = 10;
    const int lineHeight = 30;  
    // Generaciones en celeste
    SetTextColor(hdc, RGB(135, 206, 235));  // Celeste
    std::wstring genText = L"Generacion: " + std::to_wstring(generationCount);
    TextOutW(hdc, 10, yOffset, genText.c_str(), genText.length());
    yOffset += lineHeight;

    // Enemigos eliminados en rojo tomate
    SetTextColor(hdc, RGB(255, 99, 71));  // Rojo tomate
    std::wstring deadText = L"Enemigos eliminados: " + std::to_wstring(deadEnemiesCount);
    TextOutW(hdc, 10, yOffset, deadText.c_str(), deadText.length());
    yOffset += lineHeight;

    // Fitness en verde pálido
    SetTextColor(hdc, RGB(152, 251, 152));  // Verde pálido
    if (!currentFitness.empty()) {
        int fitnessYOffset = yOffset;
        std::wstring fitnessHeader = L"Fitness de enemigos:";
        TextOutW(hdc, 10, fitnessYOffset, fitnessHeader.c_str(), fitnessHeader.length());
        fitnessYOffset += lineHeight/2;  // Medio espacio después del encabezado

        // Mostrar fitness individuales, máximo 5 por línea
        const int maxPerLine = 5;
        const int valueSpacing = 120;  // Aumentado de 60 a 120
        int currentInLine = 0;

        for (size_t i = 0; i < currentFitness.size(); ++i) {
            std::wstringstream wss;
            wss << std::fixed << std::setprecision(1) << currentFitness[i];
            std::wstring fitnessValue = wss.str();
            
            // Agregar un rectángulo de fondo para mejor visibilidad
            RECT textRect = {
                10 + (currentInLine * valueSpacing),
                fitnessYOffset,
                10 + (currentInLine * valueSpacing) + 100,
                fitnessYOffset + lineHeight/2
            };
            
            TextOutW(hdc, textRect.left, fitnessYOffset, 
                    fitnessValue.c_str(), fitnessValue.length());

            currentInLine++;
            if (currentInLine >= maxPerLine) {
                currentInLine = 0;
                fitnessYOffset += lineHeight/2;  
            }
        }
        yOffset = fitnessYOffset + lineHeight;  
    } else {
        yOffset += lineHeight;
    }

    // Mutaciones en orquídea
    SetTextColor(hdc, RGB(218, 112, 214));  // Orquídea
    std::wstring mutationText = L"Mutaciones: " + std::to_wstring(mutationCount) + 
                               L" (Prob: " + std::to_wstring(static_cast<int>(mutationProbability * 100)) + L"%)";
    TextOutW(hdc, 10, yOffset, mutationText.c_str(), mutationText.length());

    SelectObject(hdc, oldFont);
    DeleteObject(statsFont);

    // si estamos construyendo algo, muestra el menu
    // porque necesitamos mas interfaces
    if (constructionState != ConstructionState::NONE) {
        DrawConstructionMenu(hdc);
    }

    SelectObject(hdc, oldPen);
}

// malditos getters redundantes para las dimensiones del grid, pero los necesitamos
// porque algunos idiotas no saben cual usar
int Map::GetNumCols() const {
    return numCols;
}

int Map::GetNumRows() const {
    return numRows;
}

int Map::GetGridWidth() const {
    return numCols;
}

int Map::GetGridHeight() const {
    return numRows;
}

// revisa si una celda esta bloqueada, devuelve true si esta fuera 
// del mapa porque obviamente no puedes pasar por ahi, genio
bool Map::IsCellOccupied(int row, int col) const {
    if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
        return true;
    }
    return grid[row][col].occupied;
}

// marca una celda como ocupada o no, util para el pathfinding
// ignora coordenadas invalidas porque no soy tu niñera
void Map::SetCellOccupied(int row, int col, bool occupied) {
    if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
        return;
    }
    grid[row][col].occupied = occupied;
}

// configura el punto de entrada de enemigos, limpia el anterior
// si existe porque no queremos que salgan de todos lados
void Map::SetEntryPoint(int row, int col) {
    if (entryRow >= 0 && entryRow < numRows && entryCol >= 0 && entryCol < numCols) {
        grid[entryRow][entryCol].isEntryPoint = false;
    }

    if (row >= 0 && row < numRows && col >= 0 && col < numCols) {
        entryRow = row;
        entryCol = col;
        grid[entryRow][entryCol].isEntryPoint = true;
    }
}

/*
 * mira, esta funcion es bastante simple pero importante - configura donde va el puente
 * que los enemigos intentaran destruir. primero limpia cualquier puente anterior (porque no 
 * queremos dos puentes, seria estupido) y luego marca las celdas del nuevo puente.
 * el alto del puente es 1/5 del mapa porque me parecio un buen numero Bv.
 */
void Map::SetBridge(int startRow, int startCol, int width) {
    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < numCols; col++) {
            grid[row][col].isBridge = false;
        }
    }

    int bridgeHeight = numRows / 5;
    for (int row = startRow; row < startRow + bridgeHeight && row < numRows; row++) {
        for (int col = startCol; col < startCol + width && col < numCols; col++) {
            grid[row][col].isBridge = true;
        }
    }
}

/* 
 * funcion estupidamente simple que revisa si puedes construir en una celda.
 * si la celda esta fuera del mapa obviamente no puedes construir ahi, crack.
 * si no, revisa si la celda esta marcada como spot de construccion.
 */
bool Map::IsConstructionSpot(int row, int col) const {
    if (row < 0 || row >= numRows || col < 0 || col >= numCols) {
        return false;
    }
    return grid[row][col].isConstructionSpot;
}

/*
 * devuelve todos los lugares donde puedes construir torres.
 * si, podria calcularlos cada vez que los necesitamos, pero
 * eso seria una perdida de tiempo. mejor los guardamos en una lista
 * y los devolvemos cuando alguien los necesite.
 */
std::vector<std::pair<int, int>> Map::GetConstructionSpots() const {
    return constructionSpots;
}

/* 
 * maneja los clicks del mouse en el mapa. esta funcion basicamente maneja 3 estados:
 * 
 * - seleccion de torre: muestra menu para construir torres nuevas
 * - mejora de torre: muestra menu para mejorar torres existentes  
 * - estado normal: permite seleccionar spots de construccion
 *
 * el codigo es horrible pero hey, al menos esta organizado por estados
 * y no es un gran switch-case del infierno
 */
void Map::HandleClick(int x, int y) {
    int col = x / CELL_SIZE;
    int row = y / CELL_SIZE;

    /* si estamos seleccionando torre nueva, maneja el menu de construccion */
    if (constructionState == ConstructionState::SELECTING_TOWER) {
        int menuX = x - (selectedCol * CELL_SIZE + CELL_SIZE + 10);
        int menuY = y - (selectedRow * CELL_SIZE);
        
        /* boton de arquero - verifica si hay oro suficiente */
        if (menuX >= 10 && menuX <= 90 && menuY >= 45 && menuY <= 95) {
            if (economy.SpendGold(economy.GetTowerCost(TowerType::ARCHER))) {
                BuildTower(TowerType::ARCHER);
                constructionState = ConstructionState::NONE;
            }
            return;
        }
        
        /* boton de artillero - verifica si hay oro suficiente */
        if (menuX >= 110 && menuX <= 190 && menuY >= 45 && menuY <= 95) {
            if (economy.SpendGold(economy.GetTowerCost(TowerType::GUNNER))) {
                BuildTower(TowerType::GUNNER);
                constructionState = ConstructionState::NONE;
            }
            return;
        }
        
        /* boton de mago - verifica si hay oro suficiente */
        if (menuX >= 210 && menuX <= 290 && menuY >= 45 && menuY <= 95) {
            if (economy.SpendGold(economy.GetTowerCost(TowerType::MAGE))) {
                BuildTower(TowerType::MAGE);
                constructionState = ConstructionState::NONE;
            }
            return;
        }
        
        constructionState = ConstructionState::NONE;
        return;
    }
    
    /* si estamos mejorando torre, maneja el menu de mejoras */
    if (constructionState == ConstructionState::UPGRADING) {
        int menuX = x - (selectedCol * CELL_SIZE + CELL_SIZE + 10);
        int menuY = y - (selectedRow * CELL_SIZE);
        
        /* boton de mejora - verifica nivel y oro */
        if (menuX >= 10 && menuX <= 190 && menuY >= 30 && menuY <= 70) {
            Tower* tower = towerManager.GetTowerAt(selectedRow, selectedCol);
            if (tower) {
                int currentLevel = static_cast<int>(tower->GetLevel());
                int upgradeCost = economy.GetUpgradeCost(currentLevel);
                
                if (economy.SpendGold(upgradeCost)) {
                    UpgradeTower();
                }
            }
            constructionState = ConstructionState::NONE;
            return;
        }
        
        constructionState = ConstructionState::NONE;
        return;
    }
    
    /* maneja clicks en spots de construccion */
    if (IsConstructionSpot(row, col)) {
        if (HasTower(row, col)) {
            /* si hay torre, muestra menu de mejora si se puede mejorar */
            Tower* tower = towerManager.GetTowerAt(row, col);
            if (tower && tower->CanUpgrade()) {
                selectedRow = row;
                selectedCol = col;
                constructionState = ConstructionState::UPGRADING;
                towerManager.ShowRangeForTower(row, col);
            }
        }
        else {
            /* si no hay torre, muestra menu de construccion */
            selectedRow = row;
            selectedCol = col;
            constructionState = ConstructionState::SELECTING_TOWER;
            towerManager.HideAllRanges();
        }
    } else {
        /* click fuera de spots validos - limpia seleccion */
        if (!IsConstructionSpot(row, col) && !HasTower(row, col) && 
            !grid[row][col].isBridge && !grid[row][col].isEntryPoint) {
            if (constructionState != ConstructionState::NONE) {
                constructionState = ConstructionState::NONE;
                selectedRow = -1;
                selectedCol = -1;
                towerManager.HideAllRanges();
            }
        }
        
        else if (constructionState != ConstructionState::NONE) {
            constructionState = ConstructionState::NONE;
            selectedRow = -1;
            selectedCol = -1;
            towerManager.HideAllRanges();
        }
    }
}

// Renderiza los sitios en los que se pueden construir :V 
void Map::DrawConstructionMenu(HDC hdc) {
    if (selectedRow < 0 || selectedCol < 0) {
        return;
    }
    
    RECT menuRect = {
        selectedCol * CELL_SIZE + CELL_SIZE + 10,
        selectedRow * CELL_SIZE,
        selectedCol * CELL_SIZE + CELL_SIZE + 310,
        selectedRow * CELL_SIZE + 350  // Aumentado de 150 a 350 para dar más espacio
    };
    
    if (menuRect.right > GetSystemMetrics(SM_CXSCREEN) - 20) {
        menuRect.left = selectedCol * CELL_SIZE - 310;
        menuRect.right = selectedCol * CELL_SIZE;
    }
    
    HBRUSH menuBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &menuRect, menuBrush);
    
    HFONT titleFont = CreateFontW(
        20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
    );
    
    HFONT normalFont = CreateFontW(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"
    );
    
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT oldFont = (HFONT)SelectObject(hdc, titleFont);
    
    WCHAR title[256];
    if (constructionState == ConstructionState::SELECTING_TOWER) {
        swprintf_s(title, L"Seleccione tipo de torre");
    } else if (constructionState == ConstructionState::UPGRADING) {
        Tower* tower = towerManager.GetTowerAt(selectedRow, selectedCol);
        if (tower) {
            int currentLevel = static_cast<int>(tower->GetLevel());
            int upgradeCost = economy.GetUpgradeCost(currentLevel);
            swprintf_s(title, L"Mejorar torre - Costo: %d", upgradeCost);
        } else {
            swprintf_s(title, L"Mejorar torre");
        }
    }
    
    RECT textRect = menuRect;
    textRect.top += 10;
    textRect.left += 10;
    DrawTextW(hdc, title, -1, &textRect, DT_LEFT);
    
    SelectObject(hdc, normalFont);
    
    if (constructionState == ConstructionState::SELECTING_TOWER) {
        RECT archerButton = {
            menuRect.left + 10,
            menuRect.top + 45,
            menuRect.left + 90,
            menuRect.top + 95
        };
        HBRUSH archerBrush = CreateSolidBrush(RGB(0, 150, 0));
        FillRect(hdc, &archerButton, archerBrush);
        DeleteObject(archerBrush);
        
        RECT archerNameRect = archerButton;
        archerNameRect.top += 10;
        archerNameRect.bottom = archerNameRect.top + 20;
        DrawTextW(hdc, L"Archer", -1, &archerNameRect, DT_CENTER);
        
        RECT archerPriceRect = archerButton;
        archerPriceRect.top += 35;
        archerPriceRect.bottom = archerPriceRect.top + 20;
        WCHAR archerPrice[16];
        swprintf_s(archerPrice, L"%d", economy.GetTowerCost(TowerType::ARCHER));
        DrawTextW(hdc, archerPrice, -1, &archerPriceRect, DT_CENTER);
        
        RECT gunnerButton = {
            menuRect.left + 110,
            menuRect.top + 45,
            menuRect.left + 190,
            menuRect.top + 95
        };
        HBRUSH gunnerBrush = CreateSolidBrush(RGB(150, 0, 0));
        FillRect(hdc, &gunnerButton, gunnerBrush);
        DeleteObject(gunnerBrush);
        
        RECT gunnerNameRect = gunnerButton;
        gunnerNameRect.top += 10;
        gunnerNameRect.bottom = gunnerNameRect.top + 20;
        DrawTextW(hdc, L"Gunner", -1, &gunnerNameRect, DT_CENTER);
        
        RECT gunnerPriceRect = gunnerButton;
        gunnerPriceRect.top += 35;
        gunnerPriceRect.bottom = gunnerPriceRect.top + 20;
        WCHAR gunnerPrice[16];
        swprintf_s(gunnerPrice, L"%d", economy.GetTowerCost(TowerType::GUNNER));
        DrawTextW(hdc, gunnerPrice, -1, &gunnerPriceRect, DT_CENTER);
        
        RECT mageButton = {
            menuRect.left + 210,
            menuRect.top + 45,
            menuRect.left + 290,
            menuRect.top + 95
        };
        HBRUSH mageBrush = CreateSolidBrush(RGB(0, 0, 150));
        FillRect(hdc, &mageButton, mageBrush);
        DeleteObject(mageBrush);
        
        // Definimos un rectángulo para el nombre y otro para el precio
        RECT mageNameRect = mageButton;
        mageNameRect.top += 10;
        mageNameRect.bottom = mageNameRect.top + 20;
        DrawTextW(hdc, L"Mage", -1, &mageNameRect, DT_CENTER);
        
        RECT magePriceRect = mageButton;
        magePriceRect.top += 35;
        magePriceRect.bottom = magePriceRect.top + 20;
        WCHAR magePrice[16];
        swprintf_s(magePrice, L"%d", economy.GetTowerCost(TowerType::MAGE));
        DrawTextW(hdc, magePrice, -1, &magePriceRect, DT_CENTER);
    } else if (constructionState == ConstructionState::UPGRADING) {
        // Botón de mejora
        RECT upgradeButton = {
            menuRect.left + 10,
            menuRect.top + 45,  // Ajustar posición
            menuRect.left + 190,
            menuRect.top + 85   // Ajustar posición
        };
        HBRUSH upgradeBrush = CreateSolidBrush(RGB(150, 150, 0));
        FillRect(hdc, &upgradeButton, upgradeBrush);
        DeleteObject(upgradeBrush);
        
        RECT upgradeTextRect = upgradeButton;
        upgradeTextRect.top += 15;
        DrawTextW(hdc, L"Mejorar torre", -1, &upgradeTextRect, DT_CENTER);
    }
    
    // Mostrar monedas disponibles
    SetTextColor(hdc, RGB(255, 215, 0)); // Color dorado
    WCHAR goldText[64];
    swprintf_s(goldText, L"Monedas: %d", economy.GetGold());
    RECT goldRect = menuRect;
    goldRect.top += 115;  // Incrementar para evitar superposición con los botones más altos
    goldRect.left += 10;
    DrawTextW(hdc, goldText, -1, &goldRect, DT_LEFT);

    // Restaurar la fuente original
    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);
    DeleteObject(normalFont);
    
    DeleteObject(menuBrush);
}

// Actualiza la lógica del mapa
void Map::Update(float deltaTime, std::vector<Enemy>& currentWaveEnemies) {
    WCHAR debugMsg[256];
    swprintf_s(debugMsg, L"Map::Update - deltaTime: %.4f, Enemies: %zu, Torres: %zu\n", 
              deltaTime, currentWaveEnemies.size(), towerManager.GetTowerCount());
    OutputDebugStringW(debugMsg);
    
    // Actualizar torres
    towerManager.Update(deltaTime, projectileManager, CELL_SIZE, currentWaveEnemies);

    // Actualizar proyectiles
    projectileManager.Update(deltaTime, GetMapPixelWidth(), GetMapPixelHeight());

    // Verificar colisiones entre proyectiles y enemigos
    projectileManager.CheckCollisions(currentWaveEnemies, CELL_SIZE, economy);

    swprintf_s(debugMsg, L"Proyectiles activos: %zd\n", 
              projectileManager.GetProjectiles().size());
    OutputDebugStringW(debugMsg);
}

// Obtiene el estado de construcción actual
ConstructionState Map::GetConstructionState() const {
    return constructionState;
}

// Verifica si existe una torre en la posición indicada
bool Map::HasTower(int row, int col) const {
    return towerManager.HasTower(row, col);
}

// Construye una torre del tipo especificado en la celda seleccionada
bool Map::BuildTower(TowerType type) {
    if (selectedRow < 0 || selectedCol < 0) {
        return false;
    }
    
    // Marcar la celda como ocupada
    SetCellOccupied(selectedRow, selectedCol, true);
    
    // Construir la torre
    return towerManager.AddTower(type, selectedRow, selectedCol);
}

// Mejora la torre en la celda seleccionada
bool Map::UpgradeTower() {
    if (selectedRow < 0 || selectedCol < 0) {
        return false;
    }
    
    return towerManager.UpgradeTower(selectedRow, selectedCol);
}

// Obtiene una referencia a la economía
Economy& Map::GetEconomy() {
    return economy;
}

void Map::AddTemporaryObstacle(int row, int col) {
    if (row >= 0 && row < numRows && col >= 0 && col < numCols) {
        // Avoid adding duplicates, though std::find on vector isn't most efficient for many obstacles
        auto it = std::find(temporaryObstacles.begin(), temporaryObstacles.end(), std::make_pair(row, col));
        if (it == temporaryObstacles.end()) {
            temporaryObstacles.push_back({row, col});
        }
    }
}

void Map::ClearTemporaryObstacles() {
    temporaryObstacles.clear();
}

// No se usará RemoveTemporaryObstacle por ahora, Clear es suficiente.
void Map::RemoveTemporaryObstacle(int row, int col) {
    temporaryObstacles.erase(
        std::remove_if(temporaryObstacles.begin(), temporaryObstacles.end(), 
                       [row, col](const std::pair<int,int>& obs){ return obs.first == row && obs.second == col; }), 
        temporaryObstacles.end());
}

bool Map::IsCellTemporarilyObstructed(int row, int col) const {
    return std::find(temporaryObstacles.begin(), temporaryObstacles.end(), std::make_pair(row, col)) != temporaryObstacles.end();
}

/*
 * pathfinding a* - esta funcion es la que hace todo el trabajo
 * de encontrar el camino mas corto entre dos puntos Bv. es un dolor de huevos
 * pero es lo que hay que hacer si queremos que los enemigos no se queden
 * atascados como idiotas.
 *
 * recibe: punto inicial y final en coordenadas de grid
 * devuelve: vector con los puntos del camino, o vacio si no hay ruta
 */
std::vector<std::pair<int, int>> Map::GetPath(std::pair<int, int> startCell, std::pair<int, int> endCell) const {
    std::vector<std::pair<int, int>> path;
    if (startCell == endCell) {
        path.push_back(startCell);
        return path;
    }
    if (startCell.first < 0 || startCell.first >= numRows || startCell.second < 0 || startCell.second >= numCols ||
        endCell.first < 0 || endCell.first >= numRows || endCell.second < 0 || endCell.second >= numCols) {
        return path; // si los puntos estan fuera del mapa, que se jodan
    }

    std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openList;
    
    // necesitamos estas estructuras para no volvernos locos:
    // - costos g para cada celda (distancia desde inicio)
    // - padres de cada celda para reconstruir el camino
    // - lista cerrada para no revisar celdas mas de una vez
    std::vector<std::vector<float>> gCosts(numRows, std::vector<float>(numCols, FLT_MAX));
    std::vector<std::vector<std::pair<int,int>>> parents(numRows, std::vector<std::pair<int,int>>(numCols, {-1,-1}));
    std::vector<std::vector<bool>> closedList(numRows, std::vector<bool>(numCols, false));

    float startHCost = CalculateHeuristic(startCell.first, startCell.second, endCell.first, endCell.second);
    openList.push(AStarNode(startCell.first, startCell.second, 0.0f, startHCost, {-1,-1}));
    gCosts[startCell.first][startCell.second] = 0.0f;

    // movimientos en 8 direcciones porque somos pros
    int dr[] = {-1, 1, 0, 0, -1, -1, 1, 1}; 
    int dc[] = {0, 0, -1, 1, -1, 1, -1, 1}; 
    float moveCost[] = {1.0f, 1.0f, 1.0f, 1.0f, M_SQRT2, M_SQRT2, M_SQRT2, M_SQRT2}; 

    while(!openList.empty()) {
        AStarNode currentNode = openList.top();
        openList.pop();

        int r = currentNode.row;
        int c = currentNode.col;

        if (closedList[r][c]) {
            continue; // ya revisamos esta celda, siguiente
        }
        closedList[r][c] = true;

        if (r == endCell.first && c == endCell.second) { 
            // llegamos! ahora hay que reconstruir el camino
            std::pair<int,int> currentPathNode = endCell;
            while(currentPathNode.first != -1) {
                path.push_back(currentPathNode);
                if (currentPathNode == startCell) break;
                currentPathNode = parents[currentPathNode.first][currentPathNode.second];
                if (path.size() > numRows * numCols * 2) return {}; // por si acaso hay ciclos infinitos
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        // revisamos los 8 vecinos de la celda actual
        for (int i = 0; i < 8; ++i) {
            int nextR = r + dr[i];
            int nextC = c + dc[i];

            // verificamos que la celda sea valida y no tenga obstaculos
            if (nextR >= 0 && nextR < numRows && nextC >= 0 && nextC < numCols && 
                !grid[nextR][nextC].occupied &&                
                !IsCellTemporarilyObstructed(nextR, nextC) && 
                !grid[nextR][nextC].isConstructionSpot &&     
                !towerManager.HasTower(nextR, nextC) &&        
                !closedList[nextR][nextC]) {
                
                float tentativeGCost = gCosts[r][c] + moveCost[i];

                // si encontramos un mejor camino, actualizamos
                if (tentativeGCost < gCosts[nextR][nextC]) {
                    parents[nextR][nextC] = {r, c};
                    gCosts[nextR][nextC] = tentativeGCost;
                    float hCost = CalculateHeuristic(nextR, nextC, endCell.first, endCell.second);
                    openList.push(AStarNode(nextR, nextC, tentativeGCost, hCost, {r,c} ));
                }
            }
        }
    }
    return path; // no hay camino, que se jodan x2
}

/*
 * funcion que calcula la posicion del puente en el grid. 
 * es una shit pero funciona - basicamente toma el tamanio del mapa
 * y calcula donde empieza el puente y su altura. luego devuelve el punto
 * central de la primera columna del puente para que los enemigos sepan 
 * donde tienen que ir a joder.
 */
std::pair<int, int> Map::GetBridgeGridLocation() const {
    int bridgeActualStartCol = numCols - (numCols / 10); 
    int bridgeActualTopRow = (numRows - (numRows / 10)) / 2; 
    int bridgeActualHeight = numRows / 10;

    return std::make_pair(bridgeActualTopRow + bridgeActualHeight / 2, bridgeActualStartCol);
}

float Map::GetMapPixelWidth() const {
    return static_cast<float>(numCols * CELL_SIZE);
}

float Map::GetMapPixelHeight() const {
    return static_cast<float>(numRows * CELL_SIZE);
} 