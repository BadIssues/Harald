#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <windows.h>
#include "session_manager.h"
#include "config_manager.h"

typedef struct {
    HWND mainWindow;
    HWND* sessionControls;
    HWND* sessionGroups;    // Handles des groupbox
    SessionManager* sessionManager;
    ConfigManager* configManager;
} UIManager;

// Fonctions de gestion de l'interface utilisateur
UIManager* UIManager_Create(HINSTANCE hInstance, SessionManager* sessionManager, ConfigManager* configManager);
void UIManager_Destroy(UIManager* manager);
void UIManager_UpdateSessionStatus(UIManager* manager, int sessionId);
void UIManager_HandleCommand(UIManager* manager, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK UIManager_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // UI_MANAGER_H 