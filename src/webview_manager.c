#include <stdlib.h>
#include "../include/webview_manager.h"

// Structure pour stocker les données de création d'environnement
typedef struct {
    HWND parentWindow;
    WebViewInstance* instance;
} EnvironmentData;

// Déclarations forward des callbacks
HRESULT CALLBACK EnvironmentCreatedHandler(HRESULT result, ICoreWebView2Environment* env, void* userData);
HRESULT CALLBACK ControllerCreatedHandler(HRESULT result, ICoreWebView2Controller* controller, void* userData);

// Callback pour la création du contrôleur
HRESULT CALLBACK ControllerCreatedHandler(HRESULT result, ICoreWebView2Controller* controller, void* userData) {
    EnvironmentData* data = (EnvironmentData*)userData;
    if (FAILED(result)) {
        free(data);
        return result;
    }

    data->instance->controller = controller;
    
    // Obtenir la WebView
    controller->lpVtbl->get_CoreWebView2(controller, &data->instance->webView);
    
    // Afficher la WebView
    controller->lpVtbl->put_IsVisible(controller, TRUE);
    
    free(data);
    return S_OK;
}

// Callback pour la création de l'environnement
HRESULT CALLBACK EnvironmentCreatedHandler(HRESULT result, ICoreWebView2Environment* env, void* userData) {
    EnvironmentData* data = (EnvironmentData*)userData;
    if (FAILED(result)) {
        free(data);
        return result;
    }

    // Créer le contrôleur avec la signature correcte
    HRESULT hr = env->lpVtbl->CreateCoreWebView2Controller(env, data->parentWindow, 
        (ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*)ControllerCreatedHandler);
    
    if (FAILED(hr)) {
        free(data);
        return hr;
    }
    
    return S_OK;
}

// Fonction pour convertir une chaîne UTF-8 en UTF-16
LPWSTR ConvertToWideString(const char* str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    LPWSTR wstr = (LPWSTR)malloc(len * sizeof(WCHAR));
    if (wstr) {
        MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, len);
    }
    return wstr;
}

WebViewManager* WebViewManager_Create(int maxInstances) {
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
    if (!manager || index >= manager->instanceCount) return FALSE;

    if (manager->instances[index]) {
        WebViewManager_DestroyInstance(manager, index);
    }

    WebViewInstance* instance = (WebViewInstance*)malloc(sizeof(WebViewInstance));
    if (!instance) return FALSE;

    instance->parentWindow = parentWindow;
    instance->webView = NULL;
    instance->controller = NULL;

    // Préparer les données pour les callbacks
    EnvironmentData* data = (EnvironmentData*)malloc(sizeof(EnvironmentData));
    if (!data) {
        free(instance);
        return FALSE;
    }
    data->parentWindow = parentWindow;
    data->instance = instance;

    // Créer l'environnement WebView2 avec la signature correcte
    HRESULT hr = CreateCoreWebView2Environment(
        (ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*)EnvironmentCreatedHandler
    );

    if (FAILED(hr)) {
        free(data);
        free(instance);
        return FALSE;
    }

    manager->instances[index] = instance;
    return TRUE;
}

void WebViewManager_DestroyInstance(WebViewManager* manager, int index) {
    if (!manager || index >= manager->instanceCount || !manager->instances[index]) return;

    WebViewInstance* instance = manager->instances[index];

    if (instance->controller) {
        instance->controller->lpVtbl->Release(instance->controller);
    }
    if (instance->webView) {
        instance->webView->lpVtbl->Release(instance->webView);
    }

    free(instance);
    manager->instances[index] = NULL;
}

BOOL WebViewManager_Navigate(WebViewManager* manager, int index, const char* url) {
    if (!manager || index >= manager->instanceCount || !manager->instances[index]) return FALSE;

    WebViewInstance* instance = manager->instances[index];
    if (!instance->webView) return FALSE;

    LPWSTR wurl = ConvertToWideString(url);
    if (!wurl) return FALSE;

    HRESULT hr = instance->webView->lpVtbl->Navigate(instance->webView, wurl);
    free(wurl);
    return SUCCEEDED(hr);
}

void WebViewManager_Resize(WebViewManager* manager, int index, int width, int height) {
    if (!manager || index >= manager->instanceCount || !manager->instances[index]) return;

    WebViewInstance* instance = manager->instances[index];
    if (!instance->controller) return;

    RECT bounds = {0, 0, width, height};
    instance->controller->lpVtbl->put_Bounds(instance->controller, bounds);
}