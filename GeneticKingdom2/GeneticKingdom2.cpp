// GeneticKingdom2.cpp : Define el punto de entrada de la aplicación.


#include "framework.h"
#include "GeneticKingdom2.h"
#include "Map.h"
#include "Enemy.h"
#include "GeneticAlgorithm.h"
#include <windowsx.h> // Para GET_X_LPARAM, GET_Y_LPARAM
#include <wingdi.h>   // Para funciones de GDI
#include <objidl.h>   // Necesario para GDI+
#include <gdiplus.h>  // Para GDI+
#include <sstream>      // For std::wstringstream in logging
#include <iomanip>      // For std::hex in logging
#pragma comment(lib, "Msimg32.lib") // Para TransparentBlt
#pragma comment(lib, "gdiplus.lib") // Para GDI+

using namespace Gdiplus;

#define MAX_LOADSTRING 100
#define TIMER_ID 1
#define FPS 30 // Reducido de 60 a 30 fotogramas por segundo para una mejor interacción con los menús

// GA Configuration Constants
const int POPULATION_SIZE = 20;     // Example: Number of enemies per wave
const float MUTATION_RATE = 0.1f;   // Example: 10% chance of mutation
const float CROSSOVER_RATE = 0.7f;  // Example: 70% chance of crossover

// Variables globales:
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal
HBRUSH g_hBackgroundBrush = NULL;               // Pincel para el color de fondo
Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;
ULONG_PTR g_gdiplusToken;
Map gameMap;                                    // Instancia del mapa del juego
bool g_showConstructionInfo = false;            // Mostrar información de construcción
int g_selectedRow = -1;                         // Fila seleccionada para construcción
int g_selectedCol = -1;                         // Columna seleccionada para construcción
DWORD g_lastUpdateTime = 0;                     // Último tiempo de actualización
bool g_gamePaused = false;                      // Indica si el juego está pausado
RECT g_menuRect = { 0 };                        // Rect del menú actual
// Genetic Algorithm and Enemies
GeneticAlgorithm* g_pGeneticAlgorithm = nullptr; // Pointer to allow deferred construction
std::vector<Enemy> g_currentWaveEnemies;
int g_currentWaveNumber = 0;

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                HandleMouseClick(HWND hWnd, int x, int y);
void                DrawConstructionInfo(HDC hdc, int row, int col);
void                UpdateGame(float deltaTime);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    /*
        APIENTRY: Es una macro que define el punto de entrada de la aplicación. 
        wWinMain: Es la función principal de la aplicación.
        _In_ HINSTANCE hInstance: Es el identificador de la instancia de la aplicación.
        _In_opt_ HINSTANCE hPrevInstance: Es el identificador de la instancia anterior de la aplicación.
        _In_ LPWSTR    lpCmdLine: Es la línea de comandos de la aplicación.
        _In_ int       nCmdShow: Es el modo en que se mostrará la ventana principal de la aplicación.
    */

    // Evita warnings del compilador
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Inicializar GDI+
    Status status = GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
    if (status != Ok) {
        MessageBoxW(NULL, L"Error al inicializar GDI+", L"Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // Inicializar la semilla para números aleatorios
    srand(static_cast<unsigned int>(time(NULL)));

    // Crear el pincel de color verde para el fondo (#0e813c)
    g_hBackgroundBrush = CreateSolidBrush(RGB(14, 129, 60)); // RGB valores para #0e813c

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GENETICKINGDOM2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realizar la inicialización de la aplicación:
    if (!InitInstance (hInstance, nCmdShow))
    {
        // Limpieza si falla la inicialización
        if (g_hBackgroundBrush) {
            DeleteObject(g_hBackgroundBrush);
        }
        GdiplusShutdown(g_gdiplusToken);
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GENETICKINGDOM2));


    // Esto es lo que windows usa para recibir mensajes de la ventana principal, como inputs, etc
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            // Esto es lo que envia mensajes a la ventana principal
            DispatchMessage(&msg);
        }
    }

    // Liberar recursos
    if (g_pGeneticAlgorithm) {
        delete g_pGeneticAlgorithm;
        g_pGeneticAlgorithm = nullptr;
    }
    if (g_hBackgroundBrush) {
        DeleteObject(g_hBackgroundBrush);
        g_hBackgroundBrush = NULL;
    }
    
    // Cerrar GDI+
    GdiplusShutdown(g_gdiplusToken);

    return (int) msg.wParam;
}



//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: Registra la clase de ventana.
//
//  ATOM: Es una macro que define el tipo de dato para las clases de ventanas.
//  HINSTANCE: Es el identificador de la instancia de la aplicación.
//  MyRegisterClass: Es el nombre de la función que registra la clase de ventana.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GENETICKINGDOM2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = g_hBackgroundBrush;  // Usar el pincel de color verde
    wcex.lpszMenuName   = NULL; // Eliminar el menú
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCIÓN: InitInstance(HINSTANCE, int)
//
//   PROPÓSITO: Guarda el identificador de instancia y crea la ventana principal
//
//   COMENTARIOS:
//
//        En esta función, se guarda el identificador de instancia en una variable común y
//        se crea y muestra la ventana principal del programa.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; 

   int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = GetSystemMetrics(SM_CYSCREEN);

   gameMap.Initialize(screenWidth, screenHeight);

   // Initialize Genetic Algorithm
   // Assuming GetEntryPoint() and GetBridgeGridLocation() are available in Map class
   // For now, using common assumptions for entry point.
   std::pair<int, int> entryPoint = { gameMap.GetNumRows() / 2, 0 }; 
   std::pair<int, int> bridgeLocation = gameMap.GetBridgeGridLocation(); 
   
   if (g_pGeneticAlgorithm) delete g_pGeneticAlgorithm; // Should be null here, but good practice
   g_pGeneticAlgorithm = new GeneticAlgorithm(POPULATION_SIZE, MUTATION_RATE, CROSSOVER_RATE, entryPoint, bridgeLocation, &gameMap);
   g_pGeneticAlgorithm->InitializePopulation();
   g_currentWaveEnemies = g_pGeneticAlgorithm->GetCurrentPopulation();
   g_currentWaveNumber = 1;

   HWND hWnd = CreateWindowW(
       szWindowClass, 
       szTitle, 
       WS_POPUP, // Sólo WS_POPUP para una ventana sin bordes, menús o decoraciones
       0, 0, 
       screenWidth, screenHeight, 
       NULL, NULL, 
       hInstance, 
       NULL
   );

   if (!hWnd)
   {
      return FALSE;
   }

   // Forzar el modo de pantalla completa
   SetWindowPos(
       hWnd,
       HWND_TOP,
       0, 0,
       screenWidth, screenHeight,
       SWP_FRAMECHANGED
   );

   // Establecer temporizadores:
   // Uno para la lógica del juego
   SetTimer(hWnd, TIMER_ID, 1000 / FPS, NULL);
   
   g_lastUpdateTime = GetTickCount();

   ShowWindow(hWnd, SW_SHOWMAXIMIZED); // Usar SW_SHOWMAXIMIZED para asegurar pantalla completa
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCIÓN: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PROPÓSITO: Procesa mensajes de la ventana principal.
//
//  WM_COMMAND  - procesar el menú de aplicaciones
//  WM_PAINT    - Pintar la ventana principal
//  WM_DESTROY  - publicar un mensaje de salida y volver
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        // Inicialización de GDI+
        Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
        // Aquí puedes inicializar otros elementos si es necesario
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Analizar las selecciones de menú:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            
            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            HBRUSH hbrBackground = CreateSolidBrush(RGB(14, 129, 60)); // Verde #0e813c
            FillRect(hdcMem, &clientRect, hbrBackground);
            DeleteObject(hbrBackground);

            // DIBUJAR EL MAPA Y TODOS SUS COMPONENTES EN EL BACK BUFFER
            gameMap.Draw(hdcMem); 

            std::wstringstream wss_paint;
            wss_paint << L"WM_PAINT - Drawing enemies (Wave " << g_currentWaveNumber << L") - Count: " << g_currentWaveEnemies.size() << L"\n";
            for (size_t i = 0; i < g_currentWaveEnemies.size(); ++i) {
                const Enemy& enemy_to_draw = g_currentWaveEnemies[i]; // Use a different variable name
                wss_paint << L"  Draw Enemy[" << i << L"] ID: " << std::hex << &enemy_to_draw
                          << L", Health: " << enemy_to_draw.GetHealth() << L"/" << enemy_to_draw.GetMaxHealth()
                          << L", IsActive: " << (enemy_to_draw.IsActive() ? L"Yes" : L"No")
                          << L", IsAlive: " << (enemy_to_draw.IsAlive() ? L"Yes" : L"No")
                          << L", X: " << enemy_to_draw.GetX() << L", Y: " << enemy_to_draw.GetY() << L"\n";
            }
            OutputDebugStringW(wss_paint.str().c_str());

            // Dibujar enemigos de la oleada actual
            for (const Enemy& enemy_to_draw_loop : g_currentWaveEnemies) { // Use a different variable name
                enemy_to_draw_loop.Draw(hdcMem);
            }

            BitBlt(hdc, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_LBUTTONDOWN:
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            gameMap.HandleClick(xPos, yPos);
            InvalidateRect(hWnd, NULL, FALSE); 
        }
        break;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(hWnd);
        }
        else if (wParam == 'M') // Tecla M para añadir oro (para pruebas)
        {
            gameMap.GetEconomy().AddGold(100);
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    case WM_ERASEBKGND:
        return TRUE; 
    case WM_TIMER:
        if (wParam == TIMER_ID) {
            DWORD currentTime = GetTickCount();
            float deltaTime = (currentTime - g_lastUpdateTime) / 1000.0f;
            // Limitar deltaTime a un máximo para evitar saltos grandes si hay lag o pausas
            if (deltaTime > 0.1f) deltaTime = 0.1f;
            g_lastUpdateTime = currentTime;
            
            if (!g_gamePaused) {
                UpdateGame(deltaTime);
            }
            
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    case WM_DESTROY:
        // Finalización de GDI+
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        KillTimer(hWnd, TIMER_ID); // Detener el timer al destruir la ventana
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Controlador de mensajes del cuadro Acerca de.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Maneja el clic del mouse para seleccionar un punto de construcción
void HandleMouseClick(HWND hWnd, int x, int y) {
    // Utilizar el nuevo método HandleClick del mapa
    gameMap.HandleClick(x, y);
    
    // Forzar repintado para mostrar/ocultar información o menús
    InvalidateRect(hWnd, NULL, FALSE);
}

// Dibuja información sobre el punto de construcción seleccionado
void DrawConstructionInfo(HDC hdc, int row, int col) {
    // Esta función ya no se usa directamente, el menú de construcción
    // se maneja dentro de Map::Draw a través de DrawConstructionMenu
}

// Actualiza el juego con el tiempo transcurrido
void UpdateGame(float deltaTime) {
    // Actualizar la lógica del mapa (torres, proyectiles)
    gameMap.Update(deltaTime, g_currentWaveEnemies); // Pass current wave to map update

    // Actualizar enemigos
    bool anyActiveEnemies = false;
    std::wstringstream wss_ug_loop;
    wss_ug_loop << L"UpdateGame Loop (Pre-EnemyUpdate) - Wave: " << g_currentWaveNumber << L"\n";

    for (Enemy& enemy : g_currentWaveEnemies) {
        wss_ug_loop << L"  Enemy ID: " << std::hex << &enemy 
                    << L", Health: " << enemy.GetHealth()
                    << L"/" << enemy.GetMaxHealth()
                    << L", IsActive: " << (enemy.IsActive() ? L"Yes" : L"No") 
                    << L", IsAlive: " << (enemy.IsAlive() ? L"Yes" : L"No") 
                    << L", X: " << enemy.GetX() << L", Y: " << enemy.GetY();
        if (enemy.IsActive()) {
            enemy.Update(deltaTime);
            anyActiveEnemies = true;
            wss_ug_loop << L", Action: Called Update()\n";
        } else {
            wss_ug_loop << L", Action: Skipped Update()\n";
        }
    }
    OutputDebugStringW(wss_ug_loop.str().c_str());

    // Wave end and GA logic
    if (!anyActiveEnemies && g_pGeneticAlgorithm) { // Wave is over
        std::wstringstream wss_ga_start;
        wss_ga_start << L"UpdateGame: All enemies inactive. Starting GA for next wave (current wave " << g_currentWaveNumber << L").\n";
        OutputDebugStringW(wss_ga_start.str().c_str());
        // Ensure bridgeLocation is correctly fetched for fitness calculation
        std::pair<int, int> bridgeGridLoc = gameMap.GetBridgeGridLocation();
        float mapPixelWidth = gameMap.GetMapPixelWidth();
        float mapPixelHeight = gameMap.GetMapPixelHeight();

        for (Enemy& enemy : g_currentWaveEnemies) {
            // Fitness is calculated using the enemy's own timeAlive and whether it reached the bridge
            enemy.CalculateFitness(bridgeGridLoc, mapPixelWidth, mapPixelHeight, 
                                   enemy.GetTimeAlive(), enemy.HasReachedBridge());
        }

        g_pGeneticAlgorithm->SetCurrentPopulation(g_currentWaveEnemies);
        
        std::wstringstream wss_ga_pre_gen;
        wss_ga_pre_gen << L"UpdateGame: Population set for GA. Generating new generation...\n";
        OutputDebugStringW(wss_ga_pre_gen.str().c_str());

        g_currentWaveEnemies = g_pGeneticAlgorithm->GenerateNewGeneration();
        g_currentWaveNumber++;
        
        std::wstringstream wss_ga_post_gen;
        wss_ga_post_gen << L"UpdateGame: New wave " << g_currentWaveNumber << L" generated with " << g_currentWaveEnemies.size() << L" enemies.\n";
        for (const auto& en : g_currentWaveEnemies) {
            wss_ga_post_gen << L"  NewGen Enemy ID: " << std::hex << &en << L", Health: " << en.GetHealth() 
                            << L"/" << en.GetMaxHealth()
                            << L", IsActive: " << (en.IsActive() ? L"Yes" : L"No") << L", X: " << en.GetX() << L", Y: " << en.GetY() << L"\n";
        }
        OutputDebugStringW(wss_ga_post_gen.str().c_str());
    }
}
