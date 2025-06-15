#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/config_manager.h"

ConfigManager* ConfigManager_Create(void) {
    ConfigManager* manager = (ConfigManager*)malloc(sizeof(ConfigManager));
    if (!manager) return NULL;

    ConfigManager_SetDefault(manager);
    return manager;
}

void ConfigManager_Destroy(ConfigManager* manager) {
    if (!manager) return;
    
    if (manager->networkInterface) {
        free(manager->networkInterface);
    }
    if (manager->defaultGameUrl) {
        free(manager->defaultGameUrl);
    }
    
    free(manager);
}

void ConfigManager_SetDefault(ConfigManager* manager) {
    if (!manager) return;

    manager->maxSessions = 12;
    manager->antiAFKInterval = 300000; // 5 minutes en millisecondes
    manager->networkInterface = _strdup("default");
    manager->enableUI = TRUE;
    manager->defaultGameUrl = _strdup("https://www.xbox.com/fr-FR/play/games/call-of-duty-black-ops-6---pack-cross-gen/9PF528M6CRHQ");
}

BOOL ConfigManager_LoadConfig(ConfigManager* manager, const char* configFile) {
    if (!manager || !configFile) return FALSE;

    FILE* file = fopen(configFile, "r");
    if (!file) {
        // Si le fichier n'existe pas, on crée un fichier avec les valeurs par défaut
        ConfigManager_SetDefault(manager);
        return ConfigManager_SaveConfig(manager, configFile);
    }

    char line[256];
    char key[64];
    char value[192];

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;

        if (sscanf(line, "%63[^=]=%191[^\n]", key, value) == 2) {
            // Suppression des espaces
            char* k = key;
            char* v = value;
            while (*k == ' ') k++;
            while (*v == ' ') v++;
            char* end = k + strlen(k) - 1;
            while (end > k && *end == ' ') *end-- = '\0';
            end = v + strlen(v) - 1;
            while (end > v && *end == ' ') *end-- = '\0';

            if (strcmp(k, "maxSessions") == 0) {
                manager->maxSessions = atoi(v);
            } else if (strcmp(k, "antiAFKInterval") == 0) {
                manager->antiAFKInterval = atoi(v);
            } else if (strcmp(k, "networkInterface") == 0) {
                if (manager->networkInterface) free(manager->networkInterface);
                manager->networkInterface = _strdup(v);
            } else if (strcmp(k, "enableUI") == 0) {
                manager->enableUI = (strcmp(v, "true") == 0);
            } else if (strcmp(k, "defaultGameUrl") == 0) {
                if (manager->defaultGameUrl) free(manager->defaultGameUrl);
                manager->defaultGameUrl = _strdup(v);
            }
        }
    }

    fclose(file);
    return TRUE;
}

BOOL ConfigManager_SaveConfig(ConfigManager* manager, const char* configFile) {
    if (!manager || !configFile) return FALSE;

    FILE* file = fopen(configFile, "w");
    if (!file) return FALSE;

    fprintf(file, "# Configuration Harald\n\n");
    fprintf(file, "maxSessions=%d\n", manager->maxSessions);
    fprintf(file, "antiAFKInterval=%d\n", manager->antiAFKInterval);
    fprintf(file, "networkInterface=%s\n", manager->networkInterface);
    fprintf(file, "enableUI=%s\n", manager->enableUI ? "true" : "false");
    fprintf(file, "defaultGameUrl=%s\n", manager->defaultGameUrl);

    fclose(file);
    return TRUE;
} 