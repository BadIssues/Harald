#include <windows.h>
#include <stdio.h>
#include "../include/session_manager.h"
#include "../include/ui_manager.h"
#include "../include/config_manager.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialisation du gestionnaire de configuration
    ConfigManager* configManager = ConfigManager_Create();
    
    // Obtenir le chemin de l'exécutable
    char exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    
    // Construire le chemin du fichier de configuration
    char configPath[MAX_PATH];
    char* lastSlash = strrchr(exePath, '\\');
    if (lastSlash) {
        *lastSlash = '\0';
        sprintf(configPath, "%s\\..\\config\\config.ini", exePath);
    } else {
        strcpy(configPath, "config\\config.ini");
    }

    if (!ConfigManager_LoadConfig(configManager, configPath)) {
        MessageBox(NULL, "Erreur lors du chargement de la configuration", "Erreur", MB_ICONERROR);
        return 1;
    }

    // Initialisation du gestionnaire de sessions
    SessionManager* sessionManager = SessionManager_Create(configManager);
    
    // Initialisation de l'interface utilisateur
    UIManager* uiManager = UIManager_Create(hInstance, sessionManager, configManager);
    
    // Boucle principale
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Nettoyage
    UIManager_Destroy(uiManager);
    SessionManager_Destroy(sessionManager);
    ConfigManager_Destroy(configManager);

    return (int)msg.wParam;
} 