#include <stdlib.h>
#include <stdio.h>
#include "../include/ui_manager.h"

#define ID_START_ALL 1001
#define ID_STOP_ALL 1002
#define ID_SESSION_BASE 2000
#define PADDING 10
#define BUTTON_HEIGHT 30
#define GROUP_HEIGHT 110
#define GROUP_WIDTH 180

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void UpdateLayout(UIManager* manager, int width, int height);

UIManager* UIManager_Create(HINSTANCE hInstance, SessionManager* sessionManager, ConfigManager* configManager) {
    UIManager* manager = (UIManager*)malloc(sizeof(UIManager));
    if (!manager) return NULL;

    manager->sessionManager = sessionManager;
    manager->configManager = configManager;

    // Enregistrement de la classe de fenetre
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "HaraldMainWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClassEx(&wc);

    // Creation de la fenetre principale
    manager->mainWindow = CreateWindowEx(
        0,
        "HaraldMainWindow",
        "Harald - Xbox Cloud Gaming Manager",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600,
        NULL,
        NULL,
        hInstance,
        manager
    );

    if (!manager->mainWindow) {
        free(manager);
        return NULL;
    }

    // Allocation des tableaux pour les controles
    manager->sessionControls = (HWND*)calloc(sessionManager->sessionCount, sizeof(HWND));
    manager->sessionGroups = (HWND*)calloc(sessionManager->sessionCount, sizeof(HWND));

    if (!manager->sessionControls || !manager->sessionGroups) {
        if (manager->sessionControls) free(manager->sessionControls);
        if (manager->sessionGroups) free(manager->sessionGroups);
        DestroyWindow(manager->mainWindow);
        free(manager);
        return NULL;
    }

    // Creation des boutons globaux
    CreateWindow(
        "BUTTON",
        "Demarrer Tout",
        WS_VISIBLE | WS_CHILD,
        PADDING, PADDING, 100, BUTTON_HEIGHT,
        manager->mainWindow,
        (HMENU)ID_START_ALL,
        hInstance,
        NULL
    );

    CreateWindow(
        "BUTTON",
        "Arreter Tout",
        WS_VISIBLE | WS_CHILD,
        PADDING * 2 + 100, PADDING, 100, BUTTON_HEIGHT,
        manager->mainWindow,
        (HMENU)ID_STOP_ALL,
        hInstance,
        NULL
    );

    // Creation des controles pour chaque session
    for (int i = 0; i < sessionManager->sessionCount; i++) {
        char sessionLabel[32];
        sprintf(sessionLabel, "Session %d", i + 1);

        // Groupe pour la session
        manager->sessionGroups[i] = CreateWindow(
            "BUTTON",
            sessionLabel,
            WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
            0, 0, GROUP_WIDTH, GROUP_HEIGHT,
            manager->mainWindow,
            NULL,
            hInstance,
            NULL
        );

        // Bouton de controle de la session
        manager->sessionControls[i] = CreateWindow(
            "BUTTON",
            "Demarrer",
            WS_VISIBLE | WS_CHILD,
            0, 0, 0, 0,
            manager->mainWindow,
            (HMENU)(INT_PTR)(ID_SESSION_BASE + i),
            hInstance,
            NULL
        );
    }

    // Initialiser la mise en page
    RECT clientRect;
    GetClientRect(manager->mainWindow, &clientRect);
    UpdateLayout(manager, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

    ShowWindow(manager->mainWindow, SW_SHOW);
    UpdateWindow(manager->mainWindow);

    return manager;
}

void UIManager_Destroy(UIManager* manager) {
    if (!manager) return;

    if (manager->sessionControls) {
        free(manager->sessionControls);
    }
    if (manager->sessionGroups) {
        free(manager->sessionGroups);
    }

    if (manager->mainWindow) {
        DestroyWindow(manager->mainWindow);
    }

    free(manager);
}

void UIManager_UpdateSessionStatus(UIManager* manager, int sessionId) {
    if (!manager || sessionId >= manager->sessionManager->sessionCount) return;

    Session* session = &manager->sessionManager->sessions[sessionId];
    SetWindowText(manager->sessionControls[sessionId],
                 session->isActive ? "Arreter" : "Demarrer");
}

void UIManager_HandleCommand(UIManager* manager, WPARAM wParam, LPARAM lParam) {
    int wmId = LOWORD(wParam);

    if (wmId == ID_START_ALL) {
        for (int i = 0; i < manager->sessionManager->sessionCount; i++) {
            SessionManager_StartSession(manager->sessionManager, i);
            UIManager_UpdateSessionStatus(manager, i);
        }
    }
    else if (wmId == ID_STOP_ALL) {
        for (int i = 0; i < manager->sessionManager->sessionCount; i++) {
            SessionManager_StopSession(manager->sessionManager, i);
            UIManager_UpdateSessionStatus(manager, i);
        }
    }
    else if (wmId >= ID_SESSION_BASE && 
             wmId < ID_SESSION_BASE + manager->sessionManager->sessionCount) {
        int sessionId = wmId - ID_SESSION_BASE;
        Session* session = &manager->sessionManager->sessions[sessionId];

        if (session->isActive) {
            SessionManager_StopSession(manager->sessionManager, sessionId);
        } else {
            SessionManager_StartSession(manager->sessionManager, sessionId);
        }
        UIManager_UpdateSessionStatus(manager, sessionId);
    }
}

static void UpdateLayout(UIManager* manager, int width, int height) {
    if (!manager) return;

    // Calculer le nombre de colonnes en fonction de la largeur
    int availableWidth = width - PADDING * 2;
    int columns = availableWidth / (GROUP_WIDTH + PADDING);
    if (columns < 1) columns = 1;
    if (columns > 4) columns = 4; // Maximum 4 colonnes

    // Calculer la largeur reelle des groupes
    int groupWidth = (availableWidth - (columns - 1) * PADDING) / columns;

    // Calculer le nombre de lignes necessaires
    int rows = (manager->sessionManager->sessionCount + columns - 1) / columns;

    // Calculer la hauteur disponible pour les groupes
    int availableHeight = height - PADDING * 2 - BUTTON_HEIGHT - PADDING;
    int groupHeight = (availableHeight - (rows - 1) * PADDING) / rows;

    // Positionner les groupes
    for (int i = 0; i < manager->sessionManager->sessionCount; i++) {
        int row = i / columns;
        int col = i % columns;
        int x = PADDING + col * (groupWidth + PADDING);
        int y = PADDING * 2 + BUTTON_HEIGHT + row * (groupHeight + PADDING);

        // Mettre a jour la position et la taille des controles
        if (manager->sessionGroups[i]) {
            SetWindowPos(manager->sessionGroups[i], NULL, x, y, groupWidth, groupHeight, SWP_NOZORDER);
        }
        if (manager->sessionControls[i]) {
            // Ajuster la taille du bouton proportionnellement
            int buttonHeight = BUTTON_HEIGHT * (groupHeight / GROUP_HEIGHT);
            if (buttonHeight < BUTTON_HEIGHT) buttonHeight = BUTTON_HEIGHT;
            SetWindowPos(manager->sessionControls[i], NULL, x + PADDING, y + groupHeight - buttonHeight - PADDING,
                        groupWidth - PADDING * 2, buttonHeight, SWP_NOZORDER);
        }
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    UIManager* manager = NULL;

    if (uMsg == WM_CREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        manager = (UIManager*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)manager);
    } else {
        manager = (UIManager*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch (uMsg) {
        case WM_COMMAND:
            if (manager) {
                UIManager_HandleCommand(manager, wParam, lParam);
            }
            break;

        case WM_SIZE:
            if (manager) {
                UpdateLayout(manager, LOWORD(lParam), HIWORD(lParam));
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
} 