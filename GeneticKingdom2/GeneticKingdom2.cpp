// GeneticKingdom2.cpp : Define el punto de entrada de la aplicación.


#include "framework.h"
#include "GeneticKingdom2.h"
#include "Map.h"

#define MAX_LOADSTRING 100

// Variables globales:
HINSTANCE hInst;                                // instancia actual
WCHAR szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal
HBRUSH g_hBackgroundBrush;                      // Pincel para el color de fondo
Map gameMap;                                    // Objeto del mapa del juego

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Crear el pincel de color verde para el fondo (#0e813c)
    g_hBackgroundBrush = CreateSolidBrush(RGB(14, 129, 60)); // RGB valores para #0e813c

    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GENETICKINGDOM2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realizar la inicialización de la aplicación:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GENETICKINGDOM2));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Liberar recursos
    DeleteObject(g_hBackgroundBrush);

    return (int) msg.wParam;
}



//
//  FUNCIÓN: MyRegisterClass()
//
//  PROPÓSITO: Registra la clase de ventana.
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
   hInst = hInstance; // Almacenar identificador de instancia en una variable global

   // Obtener dimensiones de la pantalla
   int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = GetSystemMetrics(SM_CYSCREEN);

   // Inicializar el mapa con las dimensiones de la pantalla
   gameMap.Initialize(screenWidth, screenHeight);

   // Crear ventana sin bordes ni decoraciones y asegurar que está en pantalla completa
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
            
            // Pintar todo el fondo de verde
            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect(hdc, &rc, g_hBackgroundBrush);
            
            // Dibujar la cuadrícula del mapa
            gameMap.Draw(hdc);
            
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_KEYDOWN:
        // Permitir salir con la tecla ESC
        if (wParam == VK_ESCAPE)
        {
            DestroyWindow(hWnd);
        }
        break;
    // Procesar mensajes para evitar el redimensionamiento
    case WM_GETMINMAXINFO:
        {
            // Esto evita que la ventana se redimensione
            LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
            lpMMI->ptMaxSize.x = GetSystemMetrics(SM_CXSCREEN);
            lpMMI->ptMaxSize.y = GetSystemMetrics(SM_CYSCREEN);
            lpMMI->ptMaxPosition.x = 0;
            lpMMI->ptMaxPosition.y = 0;
            lpMMI->ptMinTrackSize.x = GetSystemMetrics(SM_CXSCREEN);
            lpMMI->ptMinTrackSize.y = GetSystemMetrics(SM_CYSCREEN);
            lpMMI->ptMaxTrackSize.x = GetSystemMetrics(SM_CXSCREEN);
            lpMMI->ptMaxTrackSize.y = GetSystemMetrics(SM_CYSCREEN);
        }
        break;
    case WM_ERASEBKGND:
        // No borrar el fondo aquí, se hará en WM_PAINT
        return TRUE;
    case WM_DESTROY:
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
