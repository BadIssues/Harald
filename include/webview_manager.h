#ifndef WEBVIEW_MANAGER_H
#define WEBVIEW_MANAGER_H

#include <windows.h>
#include "../lib/WebView2.h"

typedef struct {
    ICoreWebView2* webView;
    ICoreWebView2Controller* controller;
    HWND parentWindow;
} WebViewInstance;

typedef struct {
    WebViewInstance** instances;
    int instanceCount;
} WebViewManager;

// Fonctions principales de gestion des WebView
WebViewManager* WebViewManager_Create(int maxInstances);
void WebViewManager_Destroy(WebViewManager* manager);
BOOL WebViewManager_CreateInstance(WebViewManager* manager, int index, HWND parentWindow);
void WebViewManager_DestroyInstance(WebViewManager* manager, int index);
BOOL WebViewManager_Navigate(WebViewManager* manager, int index, const char* url);
void WebViewManager_Resize(WebViewManager* manager, int index, int width, int height);

// Fonctions utilitaires supplémentaires
BOOL WebViewManager_IsInstanceReady(WebViewManager* manager, int index);
BOOL WebViewManager_GetInstanceState(WebViewManager* manager, int index, BOOL* isVisible);
BOOL WebViewManager_SetInstanceVisibility(WebViewManager* manager, int index, BOOL visible);

// Fonction utilitaire pour convertir les chaînes
LPWSTR ConvertToWideString(const char* str);

#endif // WEBVIEW_MANAGER_H