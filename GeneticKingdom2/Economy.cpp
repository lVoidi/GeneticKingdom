#include "framework.h"
#include "Economy.h"
#include <string>

Economy::Economy() : gold(0) {
}

Economy::~Economy() {
}

void Economy::Initialize(int initialGold) {
    gold = initialGold;
}

int Economy::GetGold() const {
    return gold;
}

void Economy::AddGold(int amount) {
    if (amount > 0) {
        gold += amount;
    }
}

bool Economy::SpendGold(int amount) {
    if (amount <= 0) {
        return true; // No hay que gastar nada
    }

    if (gold >= amount) {
        gold -= amount;
        return true;
    }

    return false; // No hay suficiente oro
}

int Economy::GetTowerCost(TowerType type) const {
    switch (type) {
    case TowerType::ARCHER:
        return ARCHER_COST;
    case TowerType::MAGE:
        return MAGE_COST;
    case TowerType::GUNNER:
        return GUNNER_COST;
    default:
        return ARCHER_COST; // Valor por defecto
    }
}

int Economy::GetUpgradeCost(int currentLevel) const {
    if (currentLevel == 1) {
        return UPGRADE_LEVEL_2_COST;
    } else if (currentLevel == 2) {
        return UPGRADE_LEVEL_3_COST;
    }
    
    return 0; // No se puede mejorar más
}

void Economy::Draw(HDC hdc, int x, int y) {
    // Configurar el color y modo de texto
    SetTextColor(hdc, RGB(255, 215, 0)); // Color dorado
    SetBkMode(hdc, TRANSPARENT);
    
    // Definir una fuente más grande para el texto
    HFONT hFont = CreateFontW(
        30,                        // Altura (30 puntos)
        0,                         // Ancho (0 = auto)
        0,                         // Ángulo de escapamiento
        0,                         // Ángulo de orientación
        FW_BOLD,                   // Peso de la fuente (negrita)
        FALSE,                     // Cursiva
        FALSE,                     // Subrayado
        FALSE,                     // Tachado
        DEFAULT_CHARSET,           // Conjunto de caracteres
        OUT_OUTLINE_PRECIS,        // Precisión de salida
        CLIP_DEFAULT_PRECIS,       // Precisión de recorte
        CLEARTYPE_QUALITY,         // Calidad
        DEFAULT_PITCH | FF_SWISS,  // Familia y paso
        L"Arial"                   // Nombre de la fuente
    );
    
    // Aplicar la fuente
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    // Preparar el texto a mostrar
    WCHAR text[64];
    swprintf_s(text, L"Monedas: %d", gold);
    
    // Dibujar el texto
    RECT textRect = { x, y, x + 300, y + 40 }; // Aumentar el tamaño del rectángulo
    DrawTextW(hdc, text, -1, &textRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
    
    // Restaurar la fuente anterior
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
} 