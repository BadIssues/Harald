#include <stdlib.h>
#include <string.h>
#include "../include/session_manager.h"

SessionManager* SessionManager_Create(ConfigManager* configManager) {
    SessionManager* manager = (SessionManager*)malloc(sizeof(SessionManager));
    if (!manager) return NULL;

    manager->configManager = configManager;
    manager->sessionCount = configManager->maxSessions;
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

    WebViewManager_Destroy(manager->webviewManager);
    free(manager->sessions);
    free(manager);
}

BOOL SessionManager_StartSession(SessionManager* manager, int sessionId) {
    if (!manager || sessionId >= manager->sessionCount) return FALSE;

    Session* session = &manager->sessions[sessionId];
    if (session->isActive) return TRUE;

    // Créer une instance WebView pour cette session
    if (!WebViewManager_CreateInstance(manager->webviewManager, sessionId, session->browserWindow)) {
        return FALSE;
    }

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
    if (!manager || sessionId >= manager->sessionCount) return;

    Session* session = &manager->sessions[sessionId];
    if (!session->isActive) return;

    // Fermer la WebView
    WebViewManager_DestroyInstance(manager->webviewManager, sessionId);

    session->isActive = FALSE;
    session->lastActivityTime = 0;
}

void SessionManager_HandleAntiAFK(SessionManager* manager) {
    if (!manager) return;

    DWORD currentTime = GetTickCount();
    for (int i = 0; i < manager->sessionCount; i++) {
        Session* session = &manager->sessions[i];
        if (session->isActive) {
            DWORD timeSinceLastActivity = currentTime - session->lastActivityTime;
            if (timeSinceLastActivity >= (DWORD)manager->configManager->antiAFKInterval) {
                // Simulation d'activité pour éviter l'AFK
                // TODO: Implémenter la simulation d'activité via WebView
                session->lastActivityTime = currentTime;
            }
        }
    }
}

void SessionManager_SyncInput(SessionManager* manager, int sourceSessionId) {
    if (!manager || sourceSessionId >= manager->sessionCount) return;

    Session* sourceSession = &manager->sessions[sourceSessionId];
    if (!sourceSession->isActive) return;

    // TODO: Implémenter la synchronisation des entrées entre les WebViews
} 