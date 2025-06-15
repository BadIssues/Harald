#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <windows.h>

typedef struct {
    int maxSessions;
    int antiAFKInterval;
    char* networkInterface;
    BOOL enableUI;
    char* defaultGameUrl;    // URL du jeu par défaut
    // Autres paramètres de configuration
} ConfigManager;

// Fonctions de gestion de la configuration
ConfigManager* ConfigManager_Create(void);
void ConfigManager_Destroy(ConfigManager* manager);
BOOL ConfigManager_LoadConfig(ConfigManager* manager, const char* configFile);
BOOL ConfigManager_SaveConfig(ConfigManager* manager, const char* configFile);
void ConfigManager_SetDefault(ConfigManager* manager);

#endif // CONFIG_MANAGER_H 