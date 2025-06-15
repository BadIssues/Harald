#include <stdlib.h>
#include "../include/webview_manager.h"

// Variables globales pour gérer les données temporaires des callbacks
static HWND g_pendingParentWindow = NULL;
static WebViewInstance* g_pendingInstance = NULL;
static WebViewManager* g_currentManager = NULL;
static int g_currentIndex = -1;

// Déclarations forward des callbacks
HRESULT CALLBACK EnvironmentCreatedHandler(HRESULT result, ICoreWebView2Environment* env);
HRESULT CALLBACK ControllerCreatedHandler(HRESULT result, ICoreWebView2Controller* controller);

// Callback pour la création du contrôleur
HRESULT CALLBACK ControllerCreatedHandler(HRESULT result, ICoreWebView2Controller* controller) {
    if (FAILED(result) || !g_pendingInstance) {
        return result;
    }

    g_pendingInstance->controller = controller;
    
    // Obtenir la WebView
    HRESULT hr = controller->lpVtbl->get_CoreWebView2(controller, &g_pendingInstance->webView);
    if (FAILED(hr)) {
        return hr;
    }
    
    // Afficher la WebView
    controller->lpVtbl->put_IsVisible(controller, TRUE);
    
    // Définir les limites de la WebView
    if (g_pendingParentWindow) {
        RECT rect;
        GetClientRect(g_pendingParentWindow, &rect);
        controller->lpVtbl->put_Bounds(controller, rect);
    }
    
    // Nettoyer les variables globales
    g_pendingParentWindow = NULL;
    g_pendingInstance = NULL;
    g_currentManager = NULL;
    g_currentIndex = -1;
    
    return S_OK;
}

// Callback pour la création de l'environnement
HRESULT CALLBACK EnvironmentCreatedHandler(HRESULT result, ICoreWebView2Environment* env) {
    if (FAILED(result) || !g_pendingParentWindow || !g_pendingInstance) {
        return result;
    }

    // Créer le contrôleur avec la signature correcte (3 paramètres)
    HRESULT hr = env->lpVtbl->CreateCoreWebView2Controller(
        env, 
        g_pendingParentWindow, 
        (ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*)ControllerCreatedHandler
    );
    
    if (FAILED(hr)) {
        return hr;
    }
    
    return S_OK;
}

// Fonction pour convertir une chaîne UTF-8 en UTF-16
LPWSTR ConvertToWideString(const char* str) {
    if (!str) return NULL;
    
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (len <= 0) return NULL;
    
    LPWSTR wstr = (LPWSTR)malloc(len * sizeof(WCHAR));
    if (wstr) {
        MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len);
    }
    return wstr;
}

WebViewManager* WebViewManager_Create(int maxInstances) {
    if (maxInstances <= 0) return NULL;
    
    WebViewManager* manager = (WebViewManager*)malloc(sizeof(WebViewManager));
    if (!manager) return NULL;

    manager->instances = (WebViewInstance**)calloc(maxInstances, sizeof(WebViewInstance*));
    if (!manager->instances) {
        free(manager);
        return NULL;
    }

    manager->instanceCount = maxInstances;
    return manager;
}

void WebViewManager_Destroy(WebViewManager* manager) {
    if (!manager) return;

    for (int i = 0; i < manager->instanceCount; i++) {
        WebViewManager_DestroyInstance(manager, i);
    }

    free(manager->instances);
    free(manager);
}

BOOL WebViewManager_CreateInstance(WebViewManager* manager, int index, HWND parentWindow) {
    if (!manager || index < 0 || index >= manager->instanceCount || !parentWindow) {
        return FALSE;
    }

    // Détruire l'instance existante si elle existe
    if (manager->instances[index]) {
        WebViewManager_DestroyInstance(manager, index);
    }

    WebViewInstance* instance = (WebViewInstance*)malloc(sizeof(WebViewInstance));
    if (!instance) return FALSE;

    instance->parentWindow = parentWindow;
    instance->webView = NULL;
    instance->controller = NULL;

    // Configurer les variables globales pour les callbacks
    g_pendingParentWindow = parentWindow;
    g_pendingInstance = instance;
    g_currentManager = manager;
    g_currentIndex = index;

    // Créer l'environnement WebView2 avec la signature correcte (1 paramètre)
    HRESULT hr = CreateCoreWebView2Environment(
        (ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*)EnvironmentCreatedHandler
    );

    if (FAILED(hr)) {
        // Nettoyer en cas d'échec
        g_pendingParentWindow = NULL;
        g_pendingInstance = NULL;
        g_currentManager = NULL;
        g_currentIndex = -1;
        free(instance);
        return FALSE;
    }

    manager->instances[index] = instance;
    return TRUE;
}

void WebViewManager_DestroyInstance(WebViewManager* manager, int index) {
    if (!manager || index < 0 || index >= manager->instanceCount || !manager->instances[index]) {
        return;
    }

    WebViewInstance* instance = manager->instances[index];

    // Libérer les ressources COM dans l'ordre inverse
    if (instance->webView) {
        instance->webView->lpVtbl->Release(instance->webView);
        instance->webView = NULL;
    }
    
    if (instance->controller) {
        instance->controller->lpVtbl->Release(instance->controller);
        instance->controller = NULL;
    }

    free(instance);
    manager->instances[index] = NULL;
}

BOOL WebViewManager_Navigate(WebViewManager* manager, int index, const char* url) {
    if (!manager || index < 0 || index >= manager->instanceCount || 
        !manager->instances[index] || !url) {
        return FALSE;
    }

    WebViewInstance* instance = manager->instances[index];
    if (!instance->webView) return FALSE;

    LPWSTR wurl = ConvertToWideString(url);
    if (!wurl) return FALSE;

    HRESULT hr = instance->webView->lpVtbl->Navigate(instance->webView, wurl);
    free(wurl);
    
    return SUCCEEDED(hr);
}

void WebViewManager_Resize(WebViewManager* manager, int index, int width, int height) {
    if (!manager || index < 0 || index >= manager->instanceCount || 
        !manager->instances[index] || width < 0 || height < 0) {
        return;
    }

    WebViewInstance* instance = manager->instances[index];
    if (!instance->controller) return;

    RECT bounds = {0, 0, width, height};
    instance->controller->lpVtbl->put_Bounds(instance->controller, bounds);
}

// Fonction utilitaire pour vérifier si une instance est prête
BOOL WebViewManager_IsInstanceReady(WebViewManager* manager, int index) {
    if (!manager || index < 0 || index >= manager->instanceCount || 
        !manager->instances[index]) {
        return FALSE;
    }
    
    WebViewInstance* instance = manager->instances[index];
    return (instance->webView != NULL && instance->controller != NULL);
}

// Fonction pour obtenir l'état d'une instance
BOOL WebViewManager_GetInstanceState(WebViewManager* manager, int index, BOOL* isVisible) {
    if (!manager || index < 0 || index >= manager->instanceCount || 
        !manager->instances[index] || !isVisible) {
        return FALSE;
    }
    
    WebViewInstance* instance = manager->instances[index];
    if (!instance->controller) return FALSE;
    
    HRESULT hr = instance->controller->lpVtbl->get_IsVisible(instance->controller, isVisible);
    return SUCCEEDED(hr);
}

// Fonction pour définir la visibilité d'une instance
BOOL WebViewManager_SetInstanceVisibility(WebViewManager* manager, int index, BOOL visible) {
    if (!manager || index < 0 || index >= manager->instanceCount || 
        !manager->instances[index]) {
        return FALSE;
    }
    
    WebViewInstance* instance = manager->instances[index];
    if (!instance->controller) return FALSE;
    
    HRESULT hr = instance->controller->lpVtbl->put_IsVisible(instance->controller, visible);
    return SUCCEEDED(hr);
}