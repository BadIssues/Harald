#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <windows.h>
#include "config_manager.h"
#include "webview_manager.h"

typedef struct {
    int id;
    HWND browserWindow;
    BOOL isActive;
    DWORD lastActivityTime;
    WebViewInstance* webview;
} Session;

typedef struct {
    Session* sessions;
    int sessionCount;
    ConfigManager* configManager;
    WebViewManager* webviewManager;
} SessionManager;

// Fonctions de gestion des sessions
SessionManager* SessionManager_Create(ConfigManager* configManager);
void SessionManager_Destroy(SessionManager* manager);
BOOL SessionManager_StartSession(SessionManager* manager, int sessionId);
void SessionManager_StopSession(SessionManager* manager, int sessionId);
void SessionManager_HandleAntiAFK(SessionManager* manager);
void SessionManager_SyncInput(SessionManager* manager, int sourceSessionId);

#endif // SESSION_MANAGER_H 