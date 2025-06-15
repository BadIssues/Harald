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

// Fonctions de gestion des WebView
WebViewManager* WebViewManager_Create(int maxInstances);
void WebViewManager_Destroy(WebViewManager* manager);
BOOL WebViewManager_CreateInstance(WebViewManager* manager, int index, HWND parentWindow);
void WebViewManager_DestroyInstance(WebViewManager* manager, int index);
BOOL WebViewManager_Navigate(WebViewManager* manager, int index, const char* url);
void WebViewManager_Resize(WebViewManager* manager, int index, int width, int height);

#endif // WEBVIEW_MANAGER_H 