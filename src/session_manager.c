#include <stdlib.h>
#include <string.h>
#include "../include/session_manager.h"

SessionManager* SessionManager_Create(ConfigManager* configManager) {
    if (!configManager) return NULL;
    
    SessionManager* manager = (SessionManager*)malloc(sizeof(SessionManager));
    if (!manager) return NULL;

    manager->configManager = configManager;
    manager->sessionCount = configManager->maxSessions;
    
    // Valider le nombre de sessions
    if (manager->sessionCount <= 0 || manager->sessionCount > 50) {
        manager->sessionCount = 12; // Valeur par défaut sécurisée
    }
    
    manager->sessions = (Session*)calloc(manager->sessionCount, sizeof(Session));
    manager->webviewManager = WebViewManager_Create(manager->sessionCount);

    if (!manager->sessions || !manager->webviewManager) {
        if (manager->sessions) free(manager->sessions);
        if (manager->webviewManager) WebViewManager_Destroy(manager->webviewManager);
        free(manager);
        return NULL;
    }

    // Initialisation des sessions
    for (int i = 0; i < manager->sessionCount; i++) {
        manager->sessions[i].id = i;
        manager->sessions[i].isActive = FALSE;
        manager->sessions[i].browserWindow = NULL;
        manager->sessions[i].lastActivityTime = 0;
        manager->sessions[i].webview = NULL;
    }

    return manager;
}

void SessionManager_Destroy(SessionManager* manager) {
    if (!manager) return;

    // Arrêt de toutes les sessions actives
    for (int i = 0; i < manager->sessionCount; i++) {
        if (manager->sessions[i].isActive) {
            SessionManager_StopSession(manager, i);
        }
    }

    if (manager->webviewManager) {
        WebViewManager_Destroy(manager->webviewManager);
    }
    
    if (manager->sessions) {
        free(manager->sessions);
    }
    
    free(manager);
}

BOOL SessionManager_StartSession(SessionManager* manager, int sessionId) {
    if (!manager || sessionId < 0 || sessionId >= manager->sessionCount) {
        return FALSE;
    }

    Session* session = &manager->sessions[sessionId];
    if (session->isActive) {
        return TRUE; // Déjà active
    }

    // Vérifier que nous avons une URL valide
    if (!manager->configManager->defaultGameUrl || 
        strlen(manager->configManager->defaultGameUrl) == 0) {
        return FALSE;
    }

    // Pour cette version, nous créons une fenêtre temporaire pour la WebView
    // Dans une version complète, cette fenêtre devrait être fournie par l'UI
    if (!session->browserWindow) {
        session->browserWindow = CreateWindow(
            "STATIC", 
            "",
            WS_CHILD | WS_VISIBLE,
            0, 0, 800, 600,
            GetDesktopWindow(), // Fenêtre parent temporaire
            NULL,
            GetModuleHandle(NULL),
            NULL
        );
        
        if (!session->browserWindow) {
            return FALSE;
        }
    }

    // Créer une instance WebView pour cette session
    if (!WebViewManager_CreateInstance(manager->webviewManager, sessionId, session->browserWindow)) {
        return FALSE;
    }

    // Attendre un peu que la WebView soit initialisée
    // Note: Dans une vraie application, il faudrait utiliser un système d'événements
    Sleep(100);

    // Naviguer vers l'URL du jeu
    if (!WebViewManager_Navigate(manager->webviewManager, sessionId, manager->configManager->defaultGameUrl)) {
        WebViewManager_DestroyInstance(manager->webviewManager, sessionId);
        return FALSE;
    }

    session->isActive = TRUE;
    session->lastActivityTime = GetTickCount();
    
    return TRUE;
}

void SessionManager_StopSession(SessionManager* manager, int sessionId) {
    if (!manager || sessionId < 0 || sessionId >= manager->sessionCount) {
        return;
    }

    Session* session = &manager->sessions[sessionId];
    if (!session->isActive) {
        return; // Déjà inactive
    }

    // Fermer la WebView
    WebViewManager_DestroyInstance(manager->webviewManager, sessionId);

    // Fermer la fenêtre du navigateur si elle existe
    if (session->browserWindow) {
        DestroyWindow(session->browserWindow);
        session->browserWindow = NULL;
    }

    session->isActive = FALSE;
    session->lastActivityTime = 0;
    session->webview = NULL;
}

void SessionManager_HandleAntiAFK(SessionManager* manager) {
    if (!manager) return;

    DWORD currentTime = GetTickCount();
    
    for (int i = 0; i < manager->sessionCount; i++) {
        Session* session = &manager->sessions[i];
        
        if (!session->isActive) continue;
        
        // Vérifier si la session est encore valide
        if (!WebViewManager_IsInstanceReady(manager->webviewManager, i)) {
            // La WebView n'est plus valide, marquer la session comme inactive
            session->isActive = FALSE;
            session->lastActivityTime = 0;
            continue;
        }
        
        DWORD timeSinceLastActivity = currentTime - session->lastActivityTime;
        
        if (timeSinceLastActivity >= (DWORD)manager->configManager->antiAFKInterval) {
            // Simulation d'activité pour éviter l'AFK
            // Pour l'instant, on simule juste en mettant à jour le temps
            // Dans une vraie implémentation, on pourrait :
            // - Envoyer des événements de souris/clavier
            // - Recharger la page
            // - Exécuter du JavaScript pour simuler l'activité
            
            session->lastActivityTime = currentTime;
            
            // Optionnel : forcer la visibilité de la WebView
            WebViewManager_SetInstanceVisibility(manager->webviewManager, i, TRUE);
        }
    }
}

void SessionManager_SyncInput(SessionManager* manager, int sourceSessionId) {
    if (!manager || sourceSessionId < 0 || sourceSessionId >= manager->sessionCount) {
        return;
    }

    Session* sourceSession = &manager->sessions[sourceSessionId];
    if (!sourceSession->isActive) return;

    // TODO: Implémenter la synchronisation des entrées entre les WebViews
    // Cela nécessiterait :
    // 1. Capturer les événements d'entrée de la session source
    // 2. Les rediffuser vers toutes les autres sessions actives
    // 3. Utiliser l'API WebView2 pour injecter des événements JavaScript
    
    // Pour l'instant, on met juste à jour le temps d'activité des autres sessions
    DWORD currentTime = GetTickCount();
    for (int i = 0; i < manager->sessionCount; i++) {
        if (i != sourceSessionId && manager->sessions[i].isActive) {
            manager->sessions[i].lastActivityTime = currentTime;
        }
    }
}

// Fonction utilitaire pour obtenir le statut d'une session
BOOL SessionManager_GetSessionStatus(SessionManager* manager, int sessionId, BOOL* isActive, BOOL* isReady) {
    if (!manager || sessionId < 0 || sessionId >= manager->sessionCount || !isActive || !isReady) {
        return FALSE;
    }
    
    Session* session = &manager->sessions[sessionId];
    *isActive = session->isActive;
    *isReady = session->isActive && WebViewManager_IsInstanceReady(manager->webviewManager, sessionId);
    
    return TRUE;
}

// Fonction pour redimensionner la WebView d'une session
BOOL SessionManager_ResizeSession(SessionManager* manager, int sessionId, int width, int height) {
    if (!manager || sessionId < 0 || sessionId >= manager->sessionCount) {
        return FALSE;
    }
    
    Session* session = &manager->sessions[sessionId];
    if (!session->isActive) return FALSE;
    
    WebViewManager_Resize(manager->webviewManager, sessionId, width, height);
    return TRUE;
}