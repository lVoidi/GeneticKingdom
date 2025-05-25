// este archivo es el punto de entrada de la aplicacion, basicamente es el main xd
// aqui esta toda la logica de la ventana y el bucle principal del juego
// tambien maneja los eventos del mouse, teclado y el timer que actualiza todo

#include "framework.h"
#include "GeneticKingdom2.h"
#include "Map.h"
#include "Enemy.h"
#include "GeneticAlgorithm.h"
#include <windowsx.h> // para obtener coordenadas del mouse, porque windows es especial
#include <wingdi.h>   // para dibujar cosas feas con gdi
#include <objidl.h>   // necesario para gdi+, otro invento de windows
#include <gdiplus.h>  // gdi+ porque el gdi normal es horrible
#include <sstream>    // para debugear cuando todo explota
#include <iomanip>    // para formatear numeros en hex, que elegancia
#pragma comment(lib, "Msimg32.lib") // para transparencias, porque gdi no puede solo
#pragma comment(lib, "gdiplus.lib") // para que gdi+ funcione, obvio

using namespace Gdiplus;

#define MAX_LOADSTRING 100
#define TIMER_ID 1  
#define FPS 30 // bajado a 30 porque asi funciona mejor creo

// configuracion del algoritmo genetico
const int POPULATION_SIZE = 20;     // cuantos enemigos por oleada
const float MUTATION_RATE = 0.1f;   // probabilidad de que muten
const float CROSSOVER_RATE = 0.7f;  // probabilidad de que se reproduzcan

// variables globales porque yolo
HINSTANCE hInst;                                // la instancia de windows
WCHAR szTitle[MAX_LOADSTRING];                  // titulo de la ventana
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de la clase de la ventana
HBRUSH g_hBackgroundBrush = NULL;               // pincel para el fondo
Gdiplus::GdiplusStartupInput g_gdiplusStartupInput;
ULONG_PTR g_gdiplusToken;
Map gameMap;                                    // el mapa del juego
bool g_showConstructionInfo = false;            // mostrar menu de construccion
int g_selectedRow = -1;                         // fila seleccionada
int g_selectedCol = -1;                         // columna seleccionada
DWORD g_lastUpdateTime = 0;                     // tiempo del ultimo update
bool g_gamePaused = false;                      // si esta pausado
RECT g_menuRect = { 0 };                        // rectangulo del menu
// cosas del algoritmo genetico
GeneticAlgorithm* g_pGeneticAlgorithm = nullptr; // puntero al algoritmo genetico
std::vector<Enemy> g_currentWaveEnemies;
int g_currentWaveNumber = 0;
float g_timeSinceWaveEnd = 0.0f; // tiempo desde que termino la oleada
const float WAVE_DELAY = 3.0f; // tiempo entre oleadas

// funciones que vienen despues
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
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    Status status = GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
    if (status != Ok) {
        MessageBoxW(NULL, L"Error al inicializar GDI+", L"Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    srand(static_cast<unsigned int>(time(NULL)));

    g_hBackgroundBrush = CreateSolidBrush(RGB(14, 129, 60));

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GENETICKINGDOM2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        if (g_hBackgroundBrush) {
            DeleteObject(g_hBackgroundBrush);
        }
        GdiplusShutdown(g_gdiplusToken);
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GENETICKINGDOM2));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    if (g_pGeneticAlgorithm) {
        delete g_pGeneticAlgorithm;
        g_pGeneticAlgorithm = nullptr;
    }
    if (g_hBackgroundBrush) {
        DeleteObject(g_hBackgroundBrush);
        g_hBackgroundBrush = NULL;
    }
    
    GdiplusShutdown(g_gdiplusToken);

    return (int) msg.wParam;
}

// registra la clase de la ventana, nada interesante que ver aqui
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
    wcex.hbrBackground  = g_hBackgroundBrush;  
    wcex.lpszMenuName   = NULL; 
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// inicializa la ventana y todo lo demas
// si esto falla nada funciona xd
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; 

   int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = GetSystemMetrics(SM_CYSCREEN);

   gameMap.Initialize(screenWidth, screenHeight);

   std::pair<int, int> entryPoint = { gameMap.GetNumRows() / 2, 0 }; 
   std::pair<int, int> bridgeLocation = gameMap.GetBridgeGridLocation(); 
   
   if (g_pGeneticAlgorithm) delete g_pGeneticAlgorithm;
   g_pGeneticAlgorithm = new GeneticAlgorithm(POPULATION_SIZE, MUTATION_RATE, CROSSOVER_RATE, entryPoint, bridgeLocation, &gameMap);
   g_pGeneticAlgorithm->InitializePopulation();
   g_currentWaveEnemies = g_pGeneticAlgorithm->GetCurrentPopulation();
   g_currentWaveNumber = 1;

   HWND hWnd = CreateWindowW(
       szWindowClass, 
       szTitle, 
       WS_POPUP,
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

   SetWindowPos(
       hWnd,
       HWND_TOP,
       0, 0,
       screenWidth, screenHeight,
       SWP_FRAMECHANGED
   );

   SetTimer(hWnd, TIMER_ID, 1000 / FPS, NULL);
   
   g_lastUpdateTime = GetTickCount();

   ShowWindow(hWnd, SW_SHOWMAXIMIZED);
   UpdateWindow(hWnd);

   return TRUE;
}

// procesa todos los mensajes de windows
// es como un switch gigante que maneja todo lo que pasa en la ventana
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        Gdiplus::GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
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

            HBRUSH hbrBackground = CreateSolidBrush(RGB(14, 129, 60));
            FillRect(hdcMem, &clientRect, hbrBackground);
            DeleteObject(hbrBackground);

            gameMap.Draw(hdcMem); 

            std::wstringstream wss_paint;
            wss_paint << L"WM_PAINT - Drawing enemies (Wave " << g_currentWaveNumber << L") - Count: " << g_currentWaveEnemies.size() << L"\n";
            for (size_t i = 0; i < g_currentWaveEnemies.size(); ++i) {
                const Enemy& enemy_to_draw = g_currentWaveEnemies[i];
                wss_paint << L"  Draw Enemy[" << i << L"] ID: " << std::hex << &enemy_to_draw
                          << L", Health: " << enemy_to_draw.GetHealth() << L"/" << enemy_to_draw.GetMaxHealth()
                          << L", IsActive: " << (enemy_to_draw.IsActive() ? L"Yes" : L"No")
                          << L", IsAlive: " << (enemy_to_draw.IsAlive() ? L"Yes" : L"No")
                          << L", X: " << enemy_to_draw.GetX() << L", Y: " << enemy_to_draw.GetY() << L"\n";
            }
            OutputDebugStringW(wss_paint.str().c_str());

            for (const Enemy& enemy_to_draw_loop : g_currentWaveEnemies) {
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
        else if (wParam == 'M')
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
            if (deltaTime > 0.1f) deltaTime = 0.1f;
            g_lastUpdateTime = currentTime;
            
            if (!g_gamePaused) {
                UpdateGame(deltaTime);
            }
            
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    case WM_DESTROY:
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        KillTimer(hWnd, TIMER_ID);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// maneja el dialogo de "acerca de"
// nadie lo usa pero ahi esta xd
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

// maneja los clicks del mouse
// basicamente le dice al mapa "oye, clickearon aqui"
void HandleMouseClick(HWND hWnd, int x, int y) {
    gameMap.HandleClick(x, y);
    InvalidateRect(hWnd, NULL, FALSE);
}

// dibuja info de construccion
// ya no se usa pero lo dejamos por si acaso :V
void DrawConstructionInfo(HDC hdc, int row, int col) {
}

// actualiza todo el juego
// esto es el cerebro del juego, si falla todo se rompe xd
void UpdateGame(float deltaTime) {
    gameMap.Update(deltaTime, g_currentWaveEnemies);

    bool anyActiveEnemies = false;
    bool anyUnspawnedEnemies = false;
    int aliveEnemies = 0;
    int deadEnemies = 0;
    int reachedBridge = 0;
    
    std::wstringstream wss_ug_loop;
    wss_ug_loop << L"UpdateGame Loop - Wave: " << g_currentWaveNumber << L", Enemies: " << g_currentWaveEnemies.size() << L"\n";

    for (Enemy& enemy : g_currentWaveEnemies) {
        bool wasActive = enemy.IsActive();
        bool wasAlive = enemy.IsAlive();
        bool hasSpawned = enemy.HasSpawned();
        
        wss_ug_loop << L"  Enemy ID: " << std::hex << &enemy 
                    << L", Type: " << static_cast<int>(enemy.GetType())
                    << L", Health: " << enemy.GetHealth() << L"/" << enemy.GetMaxHealth()
                    << L", IsActive: " << (wasActive ? L"Yes" : L"No") 
                    << L", IsAlive: " << (wasAlive ? L"Yes" : L"No")
                    << L", HasSpawned: " << (hasSpawned ? L"Yes" : L"No");
        
        enemy.Update(deltaTime);
        
        bool isActiveAfter = enemy.IsActive();
        bool isAliveAfter = enemy.IsAlive();
        bool hasSpawnedAfter = enemy.HasSpawned();
        
        if (!hasSpawnedAfter) {
            anyUnspawnedEnemies = true;
            wss_ug_loop << L", Action: Waiting to spawn (delay remaining)\n";
        } else if (isActiveAfter && isAliveAfter) {
            anyActiveEnemies = true;
            aliveEnemies++;
            wss_ug_loop << L", Action: Updated (active & alive)\n";
        } else if (!isAliveAfter) {
            deadEnemies++;
            wss_ug_loop << L", Action: Dead\n";
        } else if (enemy.HasReachedBridge()) {
            reachedBridge++;
            wss_ug_loop << L", Action: Reached bridge\n";
        } else {
            wss_ug_loop << L", Action: Inactive\n";
        }
    }
    
    wss_ug_loop << L"Summary - Active: " << anyActiveEnemies << L", Unspawned: " << anyUnspawnedEnemies 
                << L", Alive: " << aliveEnemies << L", Dead: " << deadEnemies << L", ReachedBridge: " << reachedBridge;
    OutputDebugStringW(wss_ug_loop.str().c_str());

    bool waveIsOver = !anyActiveEnemies && !anyUnspawnedEnemies;
    
    if (waveIsOver) {
        g_timeSinceWaveEnd += deltaTime;
        
        if (g_timeSinceWaveEnd >= WAVE_DELAY && g_pGeneticAlgorithm) {
            std::wstringstream wss_ga_start;
            wss_ga_start << L"UpdateGame: Wave " << g_currentWaveNumber << L" ended. Starting GA for next wave.\n";
            wss_ga_start << L"  Final stats - Alive: " << aliveEnemies << L", Dead: " << deadEnemies << L", ReachedBridge: " << reachedBridge << L"\n";
            OutputDebugStringW(wss_ga_start.str().c_str());
            
            std::pair<int, int> bridgeGridLoc = gameMap.GetBridgeGridLocation();
            float mapPixelWidth = gameMap.GetMapPixelWidth();
            float mapPixelHeight = gameMap.GetMapPixelHeight();

            for (Enemy& enemy : g_currentWaveEnemies) {
                enemy.CalculateFitness(bridgeGridLoc, mapPixelWidth, mapPixelHeight, 
                                       enemy.GetTimeAlive(), enemy.HasReachedBridge());
            }

            g_pGeneticAlgorithm->SetCurrentPopulation(g_currentWaveEnemies);
            g_pGeneticAlgorithm->EvaluateFitness(0.0f, reachedBridge > 0);
            g_pGeneticAlgorithm->SelectParents();
            g_pGeneticAlgorithm->CrossoverAndMutate();
            
            g_currentWaveEnemies = g_pGeneticAlgorithm->GenerateNewGeneration();
            g_currentWaveNumber++;
            g_timeSinceWaveEnd = 0.0f;
            
            std::wstringstream wss_ga_post_gen;
            wss_ga_post_gen << L"UpdateGame: New wave " << g_currentWaveNumber << L" generated with " << g_currentWaveEnemies.size() << L" enemies.\n";
            OutputDebugStringW(wss_ga_post_gen.str().c_str());
        }
    } else {
        g_timeSinceWaveEnd = 0.0f;
    }
}