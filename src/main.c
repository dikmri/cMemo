#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <wininet.h>
#include <strsafe.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "resource.h"
#include "settings.h"

#define APP_TITLE L"cMemo"
#define WINDOW_CLASS_NAME L"cMemoWindow"
#define GITHUB_LATEST_RELEASE_API \
    L"https://api.github.com/repos/dikmri/cMemo/releases/latest"
#define UPDATE_USER_AGENT L"cMemo/" APP_VERSION_WIDE

#define ID_EDIT 1001

#define BORDER_THICKNESS 4
#define CUSTOM_TITLE_BAR_HEIGHT 32
#define TITLE_BUTTON_WIDTH 46
#define TITLE_BUTTON_GLYPH_SIZE 10
#define TITLE_ICON_SIZE 16
#define TITLE_TEXT_LEFT_PADDING 10
#define RESIZE_HIT_THICKNESS 8
#define EDIT_SCROLLBAR_WIDTH 10
#define EDIT_SCROLLBAR_GAP 4
#define EDIT_SCROLLBAR_MIN_THUMB_HEIGHT 24
#define IDT_ICON_MARQUEE 50001
#define IDT_SAVE_NOTIFICATION 50002
#define IDT_LOG_HEARTBEAT 50003
#define ICON_MARQUEE_INTERVAL_MS 160
#define SAVE_NOTIFICATION_MS 1500
#define LOG_HEARTBEAT_MS 5000
#define ICON_MARQUEE_TEXT_MAX_CHARS 512

#define IDM_TOPMOST_ON 40001
#define IDM_TOPMOST_OFF 40002
#define IDM_DESKTOP_MODE 40003
#define IDM_NORMAL_MODE 40004
#define IDM_SAVE 40005
#define IDM_EXIT 40006
#define IDM_SHOW 40007
#define IDM_TOGGLE_TOPMOST 40008
#define IDM_FONT 40009
#define IDM_BORDER_COLOR 40010
#define IDM_TOGGLE_AUTOSTART 40011
#define IDM_TOGGLE_TITLE_HOVER 40013
#define IDM_TOGGLE_ICON_MARQUEE 40014
#define IDM_ICON_MARQUEE_TEXT_COLOR 40015
#define IDM_ICON_MARQUEE_FONT 40016
#define IDM_ICON_MARQUEE_SPEED_SLOW 40017
#define IDM_ICON_MARQUEE_SPEED_NORMAL 40018
#define IDM_ICON_MARQUEE_SPEED_FAST 40019
#define IDM_ICON_MARQUEE_SPEED_FASTEST 40020
#define IDM_ICON_MARQUEE_BACKGROUND_COLOR 40021
#define IDM_EDITOR_BACKGROUND_COLOR 40022
#define IDM_TOGGLE_ICON_MARQUEE_REVERSE 40023
#define IDM_VERSION_CHECK 40024
#define IDM_OPEN_LOG_FOLDER 40025
#define IDM_FONT_SIZE_6 40030
#define IDM_FONT_SIZE_7 40031
#define IDM_FONT_SIZE_8 40032
#define IDM_FONT_SIZE_9 40033
#define IDM_FONT_SIZE_10 40034
#define IDM_FONT_SIZE_12 40035
#define IDM_FONT_SIZE_14 40036
#define IDM_FONT_SIZE_16 40037
#define IDM_FONT_SIZE_18 40038
#define IDM_FONT_SIZE_24 40039

#define TRAY_ICON_UID 1
#define WMAPP_TRAYICON (WM_APP + 1)
#define WMAPP_UPDATE_AVAILABLE (WM_APP + 2)
#define WMAPP_UPDATE_NOT_AVAILABLE (WM_APP + 3)
#define WMAPP_UPDATE_CHECK_FAILED (WM_APP + 4)
#define WMAPP_UPDATE_READY (WM_APP + 5)
#define WMAPP_UPDATE_DOWNLOAD_FAILED (WM_APP + 6)
#define RUN_KEY_PATH L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define RUN_VALUE_NAME L"cMemo"

#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif

static HINSTANCE g_instance = NULL;
static HWND g_mainWindow = NULL;
static HWND g_editWindow = NULL;
static WNDPROC g_originalEditProc = NULL;
static HFONT g_editFont = NULL;
static HBRUSH g_editBackgroundBrush = NULL;
static AppSettings g_settings;
static BOOL g_isExiting = FALSE;
static BOOL g_showSaveNotification = FALSE;
static BOOL g_trayIconAdded = FALSE;
static BOOL g_titleHovered = TRUE;
static BOOL g_clientMouseTracking = FALSE;
static BOOL g_editMouseTracking = FALSE;
static BOOL g_ncMouseTracking = FALSE;
static BOOL g_editorScrollBarVisible = FALSE;
static BOOL g_editorScrollBarDragging = FALSE;
static BOOL g_editorLayoutUpdating = FALSE;
static int g_editorScrollBarDragY = 0;
static int g_editorScrollBarDragFirstLine = 0;
static HICON g_marqueeSmallIcon = NULL;
static HICON g_marqueeBigIcon = NULL;
static int g_marqueeOffset = 0;
static UINT g_taskbarCreatedMessage = 0;
static volatile LONG g_updateCheckInProgress = 0;
static volatile LONG g_updateDownloadInProgress = 0;
static WCHAR g_availableUpdateUrl[2048];
static WCHAR g_availableUpdateVersion[32];
static WCHAR g_pendingUpdatePath[MAX_PATH];
static WCHAR g_pendingUpdateVersion[32];

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL AddTrayIcon(HWND hwnd);
static void UpdateIconMarquee(HWND hwnd);
static void StartIconMarquee(HWND hwnd);
static void StopIconMarquee(HWND hwnd);
static void RefreshEditorScrollBar(HWND hwnd, BOOL updateLayout);
static void StartUpdateCheck(HWND hwnd, BOOL manual);
static BOOL SaveCurrentState(HWND hwnd);

typedef enum UiTextId {
    UI_TEXT_ALWAYS_ON_TOP = 0,
    UI_TEXT_REMOVE_ALWAYS_ON_TOP,
    UI_TEXT_DESKTOP_MODE,
    UI_TEXT_NORMAL_DISPLAY,
    UI_TEXT_DISPLAY_MENU,
    UI_TEXT_APPEARANCE_MENU,
    UI_TEXT_FONT,
    UI_TEXT_FONT_SIZE,
    UI_TEXT_EDITOR_BACKGROUND_COLOR,
    UI_TEXT_WINDOW_COLOR,
    UI_TEXT_SHOW_TITLE_ON_HOVER_ONLY,
    UI_TEXT_ICON_SCROLL_MENU,
    UI_TEXT_SCROLL_MEMO_ON_ICON,
    UI_TEXT_REVERSE_ICON_SCROLL,
    UI_TEXT_ICON_TEXT_COLOR,
    UI_TEXT_ICON_BACKGROUND_COLOR,
    UI_TEXT_ICON_FONT,
    UI_TEXT_ICON_SCROLL_SPEED,
    UI_TEXT_SPEED_SLOW,
    UI_TEXT_SPEED_NORMAL,
    UI_TEXT_SPEED_FAST,
    UI_TEXT_SPEED_FASTEST,
    UI_TEXT_START_WITH_WINDOWS,
    UI_TEXT_STARTUP_MENU,
    UI_TEXT_SAVE,
    UI_TEXT_SAVED,
    UI_TEXT_SAVE_ERROR,
    UI_TEXT_EXIT,
    UI_TEXT_SHOW,
    UI_TEXT_HELP_MENU,
    UI_TEXT_VERSION_CHECK,
    UI_TEXT_OPEN_LOG_FOLDER,
    UI_TEXT_OPEN_LOG_FOLDER_ERROR,
    UI_TEXT_UPDATE_ALREADY_CHECKING,
    UI_TEXT_UPDATE_ALREADY_DOWNLOADING,
    UI_TEXT_UPDATE_AVAILABLE_PROMPT,
    UI_TEXT_UPDATE_NOT_AVAILABLE,
    UI_TEXT_UPDATE_CHECK_FAILED,
    UI_TEXT_UPDATE_DOWNLOAD_FAILED,
    UI_TEXT_UPDATE_RESTARTING,
    UI_TEXT_STARTUP_SETTING_ERROR,
    UI_TEXT_REGISTER_CLASS_ERROR,
    UI_TEXT_CREATE_WINDOW_ERROR,
    UI_TEXT_COUNT
} UiTextId;

typedef struct UiTextEntry {
    LPCWSTR english;
    LPCWSTR japanese;
} UiTextEntry;

static const UiTextEntry g_uiTexts[UI_TEXT_COUNT] = {
    [UI_TEXT_ALWAYS_ON_TOP] = {
        L"Always on Top",
        L"常に手前に表示"
    },
    [UI_TEXT_REMOVE_ALWAYS_ON_TOP] = {
        L"Remove Always on Top",
        L"常に手前を解除"
    },
    [UI_TEXT_DESKTOP_MODE] = {
        L"Desktop Mode",
        L"デスクトップモード"
    },
    [UI_TEXT_NORMAL_DISPLAY] = {
        L"Normal Display",
        L"通常表示"
    },
    [UI_TEXT_DISPLAY_MENU] = {
        L"Display",
        L"\u8868\u793A"
    },
    [UI_TEXT_APPEARANCE_MENU] = {
        L"Appearance",
        L"\u5916\u89B3"
    },
    [UI_TEXT_FONT] = {
        L"Font...",
        L"フォント..."
    },
    [UI_TEXT_FONT_SIZE] = {
        L"Font Size",
        L"\u30D5\u30A9\u30F3\u30C8\u30B5\u30A4\u30BA"
    },
    [UI_TEXT_EDITOR_BACKGROUND_COLOR] = {
        L"Editor Background Color...",
        L"\u5165\u529B\u6B04\u80CC\u666F\u8272..."
    },
    [UI_TEXT_WINDOW_COLOR] = {
        L"Window Color...",
        L"ウィンドウ色..."
    },
    [UI_TEXT_SHOW_TITLE_ON_HOVER_ONLY] = {
        L"Show Title on Hover Only",
        L"マウスオーバー時のみタイトルを表示"
    },
    [UI_TEXT_ICON_SCROLL_MENU] = {
        L"Icon Scroll",
        L"\u30A2\u30A4\u30B3\u30F3\u30B9\u30AF\u30ED\u30FC\u30EB"
    },
    [UI_TEXT_SCROLL_MEMO_ON_ICON] = {
        L"Scroll Memo on Icon",
        L"メモをアイコンにスクロール表示"
    },
    [UI_TEXT_REVERSE_ICON_SCROLL] = {
        L"Reverse Direction",
        L"\u9006\u65B9\u5411\u306B\u6D41\u3059"
    },
    [UI_TEXT_ICON_TEXT_COLOR] = {
        L"Icon Text Color...",
        L"アイコン文字色..."
    },
    [UI_TEXT_ICON_BACKGROUND_COLOR] = {
        L"Icon Background Color...",
        L"アイコン背景色..."
    },
    [UI_TEXT_ICON_FONT] = {
        L"Icon Font...",
        L"アイコンフォント..."
    },
    [UI_TEXT_ICON_SCROLL_SPEED] = {
        L"Icon Scroll Speed",
        L"アイコンスクロール速度"
    },
    [UI_TEXT_SPEED_SLOW] = {
        L"Slow",
        L"遅い"
    },
    [UI_TEXT_SPEED_NORMAL] = {
        L"Normal",
        L"標準"
    },
    [UI_TEXT_SPEED_FAST] = {
        L"Fast",
        L"速い"
    },
    [UI_TEXT_SPEED_FASTEST] = {
        L"Fastest",
        L"最速"
    },
    [UI_TEXT_START_WITH_WINDOWS] = {
        L"Start with Windows",
        L"Windows 起動時に開始"
    },
    [UI_TEXT_STARTUP_MENU] = {
        L"Startup",
        L"\u8D77\u52D5"
    },
    [UI_TEXT_SAVE] = {
        L"Save",
        L"保存"
    },
    [UI_TEXT_SAVED] = {
        L"Saved.",
        L"\u4FDD\u5B58\u3057\u307E\u3057\u305F\u3002"
    },
    [UI_TEXT_SAVE_ERROR] = {
        L"Failed to save.",
        L"\u4FDD\u5B58\u3067\u304D\u307E\u305B\u3093\u3067\u3057\u305F\u3002"
    },
    [UI_TEXT_EXIT] = {
        L"Exit",
        L"終了"
    },
    [UI_TEXT_SHOW] = {
        L"Show",
        L"表示"
    },
    [UI_TEXT_HELP_MENU] = {
        L"Help",
        L"ヘルプ"
    },
    [UI_TEXT_VERSION_CHECK] = {
        L"Version / Check for Updates...",
        L"バージョン / 更新の確認..."
    },
    [UI_TEXT_OPEN_LOG_FOLDER] = {
        L"Open Log Folder",
        L"ログフォルダを開く"
    },
    [UI_TEXT_OPEN_LOG_FOLDER_ERROR] = {
        L"Failed to open the log folder.",
        L"ログフォルダを開けませんでした。"
    },
    [UI_TEXT_UPDATE_ALREADY_CHECKING] = {
        L"An update check is already running.",
        L"更新確認はすでに実行中です。"
    },
    [UI_TEXT_UPDATE_ALREADY_DOWNLOADING] = {
        L"An update is already being downloaded.",
        L"更新のダウンロードはすでに実行中です。"
    },
    [UI_TEXT_UPDATE_AVAILABLE_PROMPT] = {
        L"Current version: %s\nLatest version: %s\n\nUpdate to the latest version now?",
        L"現在のバージョン: %s\n最新バージョン: %s\n\n最新版に更新しますか？"
    },
    [UI_TEXT_UPDATE_NOT_AVAILABLE] = {
        L"Current version: %s\nLatest version: %s\n\nYou are using the latest version.",
        L"現在のバージョン: %s\n最新バージョン: %s\n\n最新版を使用しています。"
    },
    [UI_TEXT_UPDATE_CHECK_FAILED] = {
        L"Current version: %s\n\nFailed to check for updates.",
        L"現在のバージョン: %s\n\n更新を確認できませんでした。"
    },
    [UI_TEXT_UPDATE_DOWNLOAD_FAILED] = {
        L"Failed to download the update.",
        L"更新をダウンロードできませんでした。"
    },
    [UI_TEXT_UPDATE_RESTARTING] = {
        L"The update is ready.\nThe app will restart now.",
        L"更新の準備ができました。\nアプリを再起動します。"
    },
    [UI_TEXT_STARTUP_SETTING_ERROR] = {
        L"Failed to update the Windows startup setting.",
        L"Windows 起動時の開始設定を更新できませんでした。"
    },
    [UI_TEXT_REGISTER_CLASS_ERROR] = {
        L"Failed to register window class.",
        L"ウィンドウクラスの登録に失敗しました。"
    },
    [UI_TEXT_CREATE_WINDOW_ERROR] = {
        L"Failed to create main window.",
        L"メインウィンドウの作成に失敗しました。"
    }
};

static BOOL g_useJapaneseUi = FALSE;

static void InitUiLanguage(void)
{
    LANGID language = GetUserDefaultUILanguage();
    g_useJapaneseUi = PRIMARYLANGID(language) == LANG_JAPANESE;
}

static LPCWSTR UiText(UiTextId id)
{
    if (id < 0 || id >= UI_TEXT_COUNT) {
        return L"";
    }

    return g_useJapaneseUi ? g_uiTexts[id].japanese : g_uiTexts[id].english;
}

typedef enum TitleButton {
    TITLE_BUTTON_NONE = 0,
    TITLE_BUTTON_MINIMIZE,
    TITLE_BUTTON_MAXIMIZE,
    TITLE_BUTTON_CLOSE
} TitleButton;

static TitleButton g_hoverTitleButton = TITLE_BUTTON_NONE;
static TitleButton g_pressedTitleButton = TITLE_BUTTON_NONE;

static HICON LoadAppIcon(int width, int height)
{
    UINT flags = LR_SHARED;
    HICON icon = NULL;

    if (width == 0 || height == 0) {
        flags |= LR_DEFAULTSIZE;
    }

    icon = (HICON)LoadImageW(g_instance,
                             MAKEINTRESOURCEW(IDI_APPICON),
                             IMAGE_ICON,
                             width,
                             height,
                             flags);
    if (!icon) {
        icon = LoadIconW(NULL, IDI_APPLICATION);
    }

    return icon;
}

static int ClampInt(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

static BOOL IsCursorOverWindow(HWND hwnd)
{
    POINT point;
    RECT windowRect;

    if (!IsWindow(hwnd)) {
        return FALSE;
    }
    if (!GetCursorPos(&point) || !GetWindowRect(hwnd, &windowRect)) {
        return FALSE;
    }

    return PtInRect(&windowRect, point);
}

static void TrackMouseLeave(HWND hwnd, BOOL nonClient, BOOL *trackingFlag)
{
    TRACKMOUSEEVENT trackMouse;

    if (!IsWindow(hwnd) || !trackingFlag || *trackingFlag) {
        return;
    }

    ZeroMemory(&trackMouse, sizeof(trackMouse));
    trackMouse.cbSize = sizeof(trackMouse);
    trackMouse.dwFlags = TME_LEAVE;
    if (nonClient) {
        trackMouse.dwFlags |= TME_NONCLIENT;
    }
    trackMouse.hwndTrack = hwnd;

    if (TrackMouseEvent(&trackMouse)) {
        *trackingFlag = TRUE;
    }
}

static COLORREF GetReadableTitleTextColor(COLORREF backgroundColor)
{
    int red = GetRValue(backgroundColor);
    int green = GetGValue(backgroundColor);
    int blue = GetBValue(backgroundColor);
    int luminance = (red * 299 + green * 587 + blue * 114) / 1000;

    return luminance >= 140 ? RGB(0, 0, 0) : RGB(255, 255, 255);
}

static COLORREF BlendColor(COLORREF first, COLORREF second, int secondPercent)
{
    int firstPercent = 100 - secondPercent;
    int red = (GetRValue(first) * firstPercent +
               GetRValue(second) * secondPercent) / 100;
    int green = (GetGValue(first) * firstPercent +
                 GetGValue(second) * secondPercent) / 100;
    int blue = (GetBValue(first) * firstPercent +
                GetBValue(second) * secondPercent) / 100;

    return RGB(ClampInt(red, 0, 255),
               ClampInt(green, 0, 255),
               ClampInt(blue, 0, 255));
}

static BOOL IsTitleBarVisible(void)
{
    return !g_settings.titleHoverOnly || g_titleHovered;
}

static void GetTitleBarRect(HWND hwnd, RECT *rect)
{
    if (!rect) {
        return;
    }

    GetClientRect(hwnd, rect);
    rect->bottom = rect->top + CUSTOM_TITLE_BAR_HEIGHT;
    if (rect->bottom < rect->top) {
        rect->bottom = rect->top;
    }
}

static void InvalidateTitleBar(HWND hwnd)
{
    RECT titleRect;

    if (!IsWindow(hwnd)) {
        return;
    }

    GetTitleBarRect(hwnd, &titleRect);
    InvalidateRect(hwnd, &titleRect, TRUE);
}

static void ApplyTitleBarAppearance(HWND hwnd)
{
    if (!IsWindow(hwnd)) {
        return;
    }

    InvalidateTitleBar(hwnd);
}

static void SetTitleHoverState(HWND hwnd, BOOL hovered)
{
    if (!g_settings.titleHoverOnly) {
        hovered = TRUE;
    }

    if (g_titleHovered == hovered) {
        return;
    }

    g_titleHovered = hovered;
    ApplyTitleBarAppearance(hwnd);
}

static void GetTitleButtonRect(HWND hwnd, TitleButton button, RECT *rect)
{
    RECT clientRect;
    int right = 0;
    int index = 0;

    if (!rect) {
        return;
    }

    SetRectEmpty(rect);
    GetClientRect(hwnd, &clientRect);
    right = clientRect.right;

    switch (button) {
    case TITLE_BUTTON_CLOSE:
        index = 0;
        break;
    case TITLE_BUTTON_MAXIMIZE:
        index = 1;
        break;
    case TITLE_BUTTON_MINIMIZE:
        index = 2;
        break;
    default:
        return;
    }

    rect->left = right - (TITLE_BUTTON_WIDTH * (index + 1));
    rect->right = right - (TITLE_BUTTON_WIDTH * index);
    rect->top = 0;
    rect->bottom = CUSTOM_TITLE_BAR_HEIGHT;
}

static TitleButton HitTestTitleButton(HWND hwnd, int x, int y)
{
    POINT point;
    RECT buttonRect;

    if (!IsTitleBarVisible()) {
        return TITLE_BUTTON_NONE;
    }

    point.x = x;
    point.y = y;

    GetTitleButtonRect(hwnd, TITLE_BUTTON_CLOSE, &buttonRect);
    if (PtInRect(&buttonRect, point)) {
        return TITLE_BUTTON_CLOSE;
    }

    GetTitleButtonRect(hwnd, TITLE_BUTTON_MAXIMIZE, &buttonRect);
    if (PtInRect(&buttonRect, point)) {
        return TITLE_BUTTON_MAXIMIZE;
    }

    GetTitleButtonRect(hwnd, TITLE_BUTTON_MINIMIZE, &buttonRect);
    if (PtInRect(&buttonRect, point)) {
        return TITLE_BUTTON_MINIMIZE;
    }

    return TITLE_BUTTON_NONE;
}

static BOOL UpdateTitleButtonHover(HWND hwnd, int x, int y)
{
    TitleButton hoveredButton = HitTestTitleButton(hwnd, x, y);

    if (g_hoverTitleButton == hoveredButton) {
        return FALSE;
    }

    g_hoverTitleButton = hoveredButton;
    InvalidateTitleBar(hwnd);
    return TRUE;
}

static void DrawTitleButtonGlyph(HDC hdc, const RECT *rect, TitleButton button,
                                 COLORREF glyphColor)
{
    HPEN pen = CreatePen(PS_SOLID, 2, glyphColor);
    HPEN oldPen = NULL;
    int centerX = 0;
    int centerY = 0;
    int half = TITLE_BUTTON_GLYPH_SIZE / 2;

    if (!pen || !rect) {
        if (pen) {
            DeleteObject(pen);
        }
        return;
    }

    oldPen = (HPEN)SelectObject(hdc, pen);
    centerX = rect->left + (rect->right - rect->left) / 2;
    centerY = rect->top + (rect->bottom - rect->top) / 2;

    switch (button) {
    case TITLE_BUTTON_MINIMIZE:
        MoveToEx(hdc, centerX - half, centerY + half / 2, NULL);
        LineTo(hdc, centerX + half, centerY + half / 2);
        break;
    case TITLE_BUTTON_MAXIMIZE:
        if (IsZoomed(g_mainWindow)) {
            Rectangle(hdc, centerX - half + 3, centerY - half,
                      centerX + half + 3, centerY + half);
            Rectangle(hdc, centerX - half - 1, centerY - half + 4,
                      centerX + half - 1, centerY + half + 4);
        } else {
            Rectangle(hdc, centerX - half, centerY - half,
                      centerX + half, centerY + half);
        }
        break;
    case TITLE_BUTTON_CLOSE:
        MoveToEx(hdc, centerX - half, centerY - half, NULL);
        LineTo(hdc, centerX + half, centerY + half);
        MoveToEx(hdc, centerX + half, centerY - half, NULL);
        LineTo(hdc, centerX - half, centerY + half);
        break;
    default:
        break;
    }

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

static void DrawTitleButton(HWND hwnd, HDC hdc, TitleButton button,
                            COLORREF titleTextColor)
{
    RECT buttonRect;
    HBRUSH brush = NULL;
    COLORREF fillColor = g_settings.borderColor;
    COLORREF glyphColor = titleTextColor;

    GetTitleButtonRect(hwnd, button, &buttonRect);
    if (IsRectEmpty(&buttonRect)) {
        return;
    }

    if (g_hoverTitleButton == button || g_pressedTitleButton == button) {
        if (button == TITLE_BUTTON_CLOSE) {
            fillColor = g_pressedTitleButton == button
                            ? RGB(150, 0, 0)
                            : RGB(210, 45, 45);
            glyphColor = RGB(255, 255, 255);
        } else {
            int red = min(255, GetRValue(fillColor) + 28);
            int green = min(255, GetGValue(fillColor) + 28);
            int blue = min(255, GetBValue(fillColor) + 28);
            fillColor = RGB(red, green, blue);
        }
    }

    brush = CreateSolidBrush(fillColor);
    if (brush) {
        FillRect(hdc, &buttonRect, brush);
        DeleteObject(brush);
    }

    DrawTitleButtonGlyph(hdc, &buttonRect, button, glyphColor);
}

static void DrawCustomTitleBar(HWND hwnd, HDC hdc)
{
    RECT titleRect;
    RECT textRect;
    HBRUSH titleBrush = NULL;
    HICON icon = NULL;
    LPCWSTR titleText = g_showSaveNotification ? UiText(UI_TEXT_SAVED) : APP_TITLE;
    COLORREF textColor = GetReadableTitleTextColor(g_settings.borderColor);
    int oldBkMode = 0;
    COLORREF oldTextColor = 0;
    int iconY = 0;

    GetTitleBarRect(hwnd, &titleRect);

    titleBrush = CreateSolidBrush(g_settings.borderColor);
    if (titleBrush) {
        FillRect(hdc, &titleRect, titleBrush);
        DeleteObject(titleBrush);
    }

    if (!IsTitleBarVisible()) {
        return;
    }

    iconY = titleRect.top + (CUSTOM_TITLE_BAR_HEIGHT - TITLE_ICON_SIZE) / 2;
    icon = LoadAppIcon(TITLE_ICON_SIZE, TITLE_ICON_SIZE);
    if (icon) {
        DrawIconEx(hdc,
                   titleRect.left + TITLE_TEXT_LEFT_PADDING,
                   iconY,
                   icon,
                   TITLE_ICON_SIZE,
                   TITLE_ICON_SIZE,
                   0,
                   NULL,
                   DI_NORMAL);
    }

    textRect = titleRect;
    textRect.left += TITLE_TEXT_LEFT_PADDING + TITLE_ICON_SIZE + 8;
    textRect.right -= TITLE_BUTTON_WIDTH * 3;

    oldBkMode = SetBkMode(hdc, TRANSPARENT);
    oldTextColor = SetTextColor(hdc, textColor);
    DrawTextW(hdc, titleText, -1, &textRect,
              DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_END_ELLIPSIS);
    SetTextColor(hdc, oldTextColor);
    SetBkMode(hdc, oldBkMode);

    DrawTitleButton(hwnd, hdc, TITLE_BUTTON_MINIMIZE, textColor);
    DrawTitleButton(hwnd, hdc, TITLE_BUTTON_MAXIMIZE, textColor);
    DrawTitleButton(hwnd, hdc, TITLE_BUTTON_CLOSE, textColor);
}

static void PerformTitleButtonAction(HWND hwnd, TitleButton button)
{
    switch (button) {
    case TITLE_BUTTON_MINIMIZE:
        ShowWindow(hwnd, SW_MINIMIZE);
        break;
    case TITLE_BUTTON_MAXIMIZE:
        ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
        break;
    case TITLE_BUTTON_CLOSE:
        SendMessageW(hwnd, WM_CLOSE, 0, 0);
        break;
    default:
        break;
    }
}

static LRESULT HitTestCustomWindow(HWND hwnd, LPARAM lParam)
{
    POINT screenPoint;
    POINT clientPoint;
    RECT windowRect;
    RECT clientRect;
    int resizeBorder = RESIZE_HIT_THICKNESS;
    BOOL onLeft = FALSE;
    BOOL onRight = FALSE;
    BOOL onTop = FALSE;
    BOOL onBottom = FALSE;

    screenPoint.x = GET_X_LPARAM(lParam);
    screenPoint.y = GET_Y_LPARAM(lParam);

    if (!GetWindowRect(hwnd, &windowRect)) {
        return HTCLIENT;
    }

    if (!IsZoomed(hwnd)) {
        onLeft = screenPoint.x >= windowRect.left &&
                 screenPoint.x < windowRect.left + resizeBorder;
        onRight = screenPoint.x < windowRect.right &&
                  screenPoint.x >= windowRect.right - resizeBorder;
        onTop = screenPoint.y >= windowRect.top &&
                screenPoint.y < windowRect.top + resizeBorder;
        onBottom = screenPoint.y < windowRect.bottom &&
                   screenPoint.y >= windowRect.bottom - resizeBorder;

        if (onTop && onLeft) {
            return HTTOPLEFT;
        }
        if (onTop && onRight) {
            return HTTOPRIGHT;
        }
        if (onBottom && onLeft) {
            return HTBOTTOMLEFT;
        }
        if (onBottom && onRight) {
            return HTBOTTOMRIGHT;
        }
        if (onLeft) {
            return HTLEFT;
        }
        if (onRight) {
            return HTRIGHT;
        }
        if (onTop) {
            return HTTOP;
        }
        if (onBottom) {
            return HTBOTTOM;
        }
    }

    clientPoint = screenPoint;
    ScreenToClient(hwnd, &clientPoint);
    GetClientRect(hwnd, &clientRect);

    if (clientPoint.y >= clientRect.top &&
        clientPoint.y < clientRect.top + CUSTOM_TITLE_BAR_HEIGHT) {
        if (HitTestTitleButton(hwnd, clientPoint.x, clientPoint.y) !=
            TITLE_BUTTON_NONE) {
            return HTCLIENT;
        }
        return HTCAPTION;
    }

    return HTCLIENT;
}

static void BuildCurrentLogFont(HWND hwnd, LOGFONTW *logFont)
{
    HDC hdc = NULL;
    int dpiY = 96;

    if (!logFont) {
        return;
    }

    ZeroMemory(logFont, sizeof(*logFont));

    hdc = GetDC(hwnd);
    if (hdc) {
        dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(hwnd, hdc);
    }
    if (dpiY <= 0) {
        dpiY = 96;
    }

    logFont->lfHeight = -MulDiv(g_settings.fontPointSize, dpiY, 72);
    logFont->lfWeight = g_settings.fontWeight;
    logFont->lfItalic = (BYTE)(g_settings.fontItalic ? TRUE : FALSE);
    logFont->lfCharSet = DEFAULT_CHARSET;
    logFont->lfQuality = CLEARTYPE_QUALITY;
    StringCchCopyW(logFont->lfFaceName,
                   ARRAYSIZE(logFont->lfFaceName),
                   g_settings.fontFace);
}

static void ApplyEditorFont(void)
{
    LOGFONTW logFont;
    HFONT newFont = NULL;
    HFONT oldFont = NULL;

    if (!g_editWindow) {
        return;
    }

    BuildCurrentLogFont(g_editWindow, &logFont);
    newFont = CreateFontIndirectW(&logFont);
    if (!newFont) {
        SendMessageW(g_editWindow, WM_SETFONT,
                     (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        return;
    }

    oldFont = g_editFont;
    g_editFont = newFont;
    SendMessageW(g_editWindow, WM_SETFONT, (WPARAM)g_editFont, TRUE);
    RefreshEditorScrollBar(g_mainWindow, TRUE);

    if (oldFont) {
        DeleteObject(oldFont);
    }
}

static HBRUSH GetEditorBackgroundBrush(void)
{
    if (!g_editBackgroundBrush) {
        g_editBackgroundBrush =
            CreateSolidBrush(g_settings.editorBackgroundColor);
    }

    if (!g_editBackgroundBrush) {
        return (HBRUSH)GetStockObject(WHITE_BRUSH);
    }

    return g_editBackgroundBrush;
}

static void ApplyEditorBackgroundColor(void)
{
    if (g_editBackgroundBrush) {
        DeleteObject(g_editBackgroundBrush);
        g_editBackgroundBrush = NULL;
    }

    g_editBackgroundBrush = CreateSolidBrush(g_settings.editorBackgroundColor);
    if (g_editWindow) {
        InvalidateRect(g_editWindow, NULL, TRUE);
    }
}

static BOOL ChooseEditorFont(HWND owner)
{
    CHOOSEFONTW chooseFont;
    LOGFONTW logFont;
    int selectedPointSize = g_settings.fontPointSize;

    BuildCurrentLogFont(owner, &logFont);
    ZeroMemory(&chooseFont, sizeof(chooseFont));
    chooseFont.lStructSize = sizeof(chooseFont);
    chooseFont.hwndOwner = owner;
    chooseFont.lpLogFont = &logFont;
    chooseFont.iPointSize = g_settings.fontPointSize * 10;
    chooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST;

    if (!ChooseFontW(&chooseFont)) {
        return FALSE;
    }

    if (chooseFont.iPointSize > 0) {
        selectedPointSize = (chooseFont.iPointSize + 5) / 10;
    }

    if (logFont.lfFaceName[0] != L'\0') {
        StringCchCopyW(g_settings.fontFace,
                       ARRAYSIZE(g_settings.fontFace),
                       logFont.lfFaceName);
    }
    g_settings.fontPointSize = ClampInt(selectedPointSize,
                                        MIN_FONT_POINT_SIZE,
                                        MAX_FONT_POINT_SIZE);
    g_settings.fontWeight = logFont.lfWeight;
    g_settings.fontItalic = logFont.lfItalic != 0;
    ApplyEditorFont();
    return TRUE;
}

static int GetFontSizeFromCommand(int commandId)
{
    switch (commandId) {
    case IDM_FONT_SIZE_6:
        return 6;
    case IDM_FONT_SIZE_7:
        return 7;
    case IDM_FONT_SIZE_8:
        return 8;
    case IDM_FONT_SIZE_9:
        return 9;
    case IDM_FONT_SIZE_10:
        return 10;
    case IDM_FONT_SIZE_12:
        return 12;
    case IDM_FONT_SIZE_14:
        return 14;
    case IDM_FONT_SIZE_16:
        return 16;
    case IDM_FONT_SIZE_18:
        return 18;
    case IDM_FONT_SIZE_24:
        return 24;
    default:
        return 0;
    }
}

static void SetEditorFontPointSize(int pointSize)
{
    g_settings.fontPointSize = ClampInt(pointSize,
                                        MIN_FONT_POINT_SIZE,
                                        MAX_FONT_POINT_SIZE);
    ApplyEditorFont();
}

static BOOL ChooseEditorBackgroundColor(HWND owner)
{
    static COLORREF customColors[16];
    CHOOSECOLORW chooseColor;

    ZeroMemory(&chooseColor, sizeof(chooseColor));
    chooseColor.lStructSize = sizeof(chooseColor);
    chooseColor.hwndOwner = owner;
    chooseColor.rgbResult = g_settings.editorBackgroundColor;
    chooseColor.lpCustColors = customColors;
    chooseColor.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (!ChooseColorW(&chooseColor)) {
        return FALSE;
    }

    g_settings.editorBackgroundColor = chooseColor.rgbResult;
    ApplyEditorBackgroundColor();
    return TRUE;
}

static void ApplyWindowBorderColor(HWND hwnd)
{
    COLORREF borderColor = g_settings.borderColor;

    if (!IsWindow(hwnd)) {
        return;
    }

    DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR,
                          &borderColor, sizeof(borderColor));
    InvalidateRect(hwnd, NULL, TRUE);
}

static BOOL ChooseWindowBorderColor(HWND owner)
{
    static COLORREF customColors[16];
    CHOOSECOLORW chooseColor;

    ZeroMemory(&chooseColor, sizeof(chooseColor));
    chooseColor.lStructSize = sizeof(chooseColor);
    chooseColor.hwndOwner = owner;
    chooseColor.rgbResult = g_settings.borderColor;
    chooseColor.lpCustColors = customColors;
    chooseColor.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (!ChooseColorW(&chooseColor)) {
        return FALSE;
    }

    g_settings.borderColor = chooseColor.rgbResult;
    ApplyWindowBorderColor(owner);
    ApplyTitleBarAppearance(owner);
    if (g_settings.iconMarquee) {
        UpdateIconMarquee(owner);
    }
    return TRUE;
}

static BOOL ChooseIconMarqueeTextColor(HWND owner)
{
    static COLORREF customColors[16];
    CHOOSECOLORW chooseColor;

    ZeroMemory(&chooseColor, sizeof(chooseColor));
    chooseColor.lStructSize = sizeof(chooseColor);
    chooseColor.hwndOwner = owner;
    chooseColor.rgbResult = g_settings.iconMarqueeTextColor;
    chooseColor.lpCustColors = customColors;
    chooseColor.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (!ChooseColorW(&chooseColor)) {
        return FALSE;
    }

    g_settings.iconMarqueeTextColor = chooseColor.rgbResult;
    if (g_settings.iconMarquee) {
        UpdateIconMarquee(owner);
    }
    return TRUE;
}

static BOOL ChooseIconMarqueeBackgroundColor(HWND owner)
{
    static COLORREF customColors[16];
    CHOOSECOLORW chooseColor;

    ZeroMemory(&chooseColor, sizeof(chooseColor));
    chooseColor.lStructSize = sizeof(chooseColor);
    chooseColor.hwndOwner = owner;
    chooseColor.rgbResult = g_settings.iconMarqueeBackgroundColor;
    chooseColor.lpCustColors = customColors;
    chooseColor.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (!ChooseColorW(&chooseColor)) {
        return FALSE;
    }

    g_settings.iconMarqueeBackgroundColor = chooseColor.rgbResult;
    if (g_settings.iconMarquee) {
        UpdateIconMarquee(owner);
    }
    return TRUE;
}

static BOOL ChooseIconMarqueeFont(HWND owner)
{
    CHOOSEFONTW chooseFont;
    LOGFONTW logFont;
    HDC hdc = NULL;
    int dpiY = 96;

    ZeroMemory(&logFont, sizeof(logFont));
    hdc = GetDC(owner);
    if (hdc) {
        dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(owner, hdc);
    }
    if (dpiY <= 0) {
        dpiY = 96;
    }

    logFont.lfHeight = -MulDiv(DEFAULT_FONT_POINT_SIZE, dpiY, 72);
    logFont.lfWeight = g_settings.iconMarqueeFontWeight;
    logFont.lfItalic = (BYTE)(g_settings.iconMarqueeFontItalic ? TRUE : FALSE);
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfQuality = CLEARTYPE_QUALITY;
    StringCchCopyW(logFont.lfFaceName,
                   ARRAYSIZE(logFont.lfFaceName),
                   g_settings.iconMarqueeFontFace);

    ZeroMemory(&chooseFont, sizeof(chooseFont));
    chooseFont.lStructSize = sizeof(chooseFont);
    chooseFont.hwndOwner = owner;
    chooseFont.lpLogFont = &logFont;
    chooseFont.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT |
                       CF_FORCEFONTEXIST | CF_NOSIZESEL;

    if (!ChooseFontW(&chooseFont)) {
        return FALSE;
    }

    if (logFont.lfFaceName[0] != L'\0') {
        StringCchCopyW(g_settings.iconMarqueeFontFace,
                       ARRAYSIZE(g_settings.iconMarqueeFontFace),
                       logFont.lfFaceName);
    }
    g_settings.iconMarqueeFontWeight = ClampInt(logFont.lfWeight, 0, 1000);
    g_settings.iconMarqueeFontItalic = logFont.lfItalic != 0;

    if (g_settings.iconMarquee) {
        UpdateIconMarquee(owner);
    }
    return TRUE;
}

static BOOL GetRunCommand(WCHAR *buffer, DWORD cchBuffer)
{
    WCHAR modulePath[MAX_PATH];
    DWORD length = 0;

    if (!buffer || cchBuffer == 0) {
        return FALSE;
    }

    length = GetModuleFileNameW(NULL, modulePath, ARRAYSIZE(modulePath));
    if (length == 0 || length >= ARRAYSIZE(modulePath)) {
        return FALSE;
    }

    return SUCCEEDED(StringCchPrintfW(buffer, cchBuffer,
                                      L"\"%s\"", modulePath));
}

static BOOL ReadUrlToMemory(LPCWSTR url, BYTE **data, DWORD *dataSize,
                            DWORD maxBytes)
{
    HINTERNET internet = NULL;
    HINTERNET request = NULL;
    BYTE *buffer = NULL;
    DWORD capacity = 0;
    DWORD size = 0;
    BOOL ok = FALSE;
    static const WCHAR headers[] =
        L"Accept: application/vnd.github+json, application/octet-stream\r\n"
        L"X-GitHub-Api-Version: 2022-11-28\r\n";

    if (!url || !data || !dataSize || maxBytes == 0) {
        return FALSE;
    }

    *data = NULL;
    *dataSize = 0;

    internet = InternetOpenW(UPDATE_USER_AGENT,
                             INTERNET_OPEN_TYPE_PRECONFIG,
                             NULL,
                             NULL,
                             0);
    if (!internet) {
        goto cleanup;
    }

    request = InternetOpenUrlW(internet,
                               url,
                               headers,
                               (DWORD)(sizeof(headers) / sizeof(headers[0]) - 1),
                               INTERNET_FLAG_RELOAD |
                                   INTERNET_FLAG_NO_CACHE_WRITE |
                                   INTERNET_FLAG_NO_UI,
                               0);
    if (!request) {
        goto cleanup;
    }

    capacity = 8192;
    buffer = (BYTE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                               capacity + 1);
    if (!buffer) {
        goto cleanup;
    }

    for (;;) {
        BYTE chunk[4096];
        DWORD bytesRead = 0;

        if (!InternetReadFile(request, chunk, sizeof(chunk), &bytesRead)) {
            goto cleanup;
        }
        if (bytesRead == 0) {
            break;
        }
        if (size + bytesRead > maxBytes) {
            goto cleanup;
        }
        if (size + bytesRead + 1 > capacity) {
            DWORD newCapacity = max(capacity * 2, size + bytesRead + 1);
            BYTE *newBuffer = (BYTE *)HeapReAlloc(GetProcessHeap(),
                                                  HEAP_ZERO_MEMORY,
                                                  buffer,
                                                  newCapacity + 1);
            if (!newBuffer) {
                goto cleanup;
            }
            buffer = newBuffer;
            capacity = newCapacity;
        }
        CopyMemory(buffer + size, chunk, bytesRead);
        size += bytesRead;
    }

    buffer[size] = '\0';
    *data = buffer;
    *dataSize = size;
    buffer = NULL;
    ok = TRUE;

cleanup:
    if (buffer) {
        HeapFree(GetProcessHeap(), 0, buffer);
    }
    if (request) {
        InternetCloseHandle(request);
    }
    if (internet) {
        InternetCloseHandle(internet);
    }
    return ok;
}

static BOOL DownloadUrlToFile(LPCWSTR url, LPCWSTR path, DWORD maxBytes)
{
    BYTE *data = NULL;
    DWORD dataSize = 0;
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD written = 0;
    BOOL ok = FALSE;

    LogInfo(L"Downloading update asset. target=%s", path);
    if (!ReadUrlToMemory(url, &data, &dataSize, maxBytes)) {
        LogWarn(L"Failed to read update URL.");
        return FALSE;
    }

    file = CreateFileW(path,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (file == INVALID_HANDLE_VALUE) {
        LogLastError(L"Create update file");
        goto cleanup;
    }

    ok = WriteFile(file, data, dataSize, &written, NULL) &&
         written == dataSize;
    if (!ok) {
        LogLastError(L"Write update file");
    } else {
        LogInfo(L"Downloaded update asset. bytes=%lu", dataSize);
    }

cleanup:
    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
    }
    if (data) {
        HeapFree(GetProcessHeap(), 0, data);
    }
    if (!ok) {
        DeleteFileW(path);
    }
    return ok;
}

static BOOL ExtractJsonStringValue(const char *start, char *buffer,
                                   size_t bufferSize)
{
    const char *cursor = start;
    size_t writeIndex = 0;

    if (!start || !buffer || bufferSize == 0) {
        return FALSE;
    }

    buffer[0] = '\0';
    while (*cursor && *cursor != '"') {
        ++cursor;
    }
    if (*cursor != '"') {
        return FALSE;
    }
    ++cursor;

    while (*cursor && *cursor != '"') {
        if (*cursor == '\\' && cursor[1]) {
            ++cursor;
        }
        if (writeIndex + 1 >= bufferSize) {
            return FALSE;
        }
        buffer[writeIndex++] = *cursor++;
    }

    if (*cursor != '"') {
        return FALSE;
    }

    buffer[writeIndex] = '\0';
    return TRUE;
}

static BOOL FindJsonStringValue(const char *json, const char *key,
                                char *buffer, size_t bufferSize)
{
    const char *keyPosition = NULL;
    const char *colon = NULL;
    const char *value = NULL;

    if (!json || !key || !buffer || bufferSize == 0) {
        return FALSE;
    }

    keyPosition = strstr(json, key);
    if (!keyPosition) {
        return FALSE;
    }

    colon = strchr(keyPosition, ':');
    if (!colon) {
        return FALSE;
    }

    value = colon + 1;
    while (*value == ' ' || *value == '\t' ||
           *value == '\r' || *value == '\n') {
        ++value;
    }

    return ExtractJsonStringValue(value, buffer, bufferSize);
}

static BOOL FindReleaseExeDownloadUrl(const char *json, char *buffer,
                                      size_t bufferSize)
{
    const char *cursor = json;

    if (!json || !buffer || bufferSize == 0) {
        return FALSE;
    }

    buffer[0] = '\0';
    while ((cursor = strstr(cursor, "\"browser_download_url\"")) != NULL) {
        const char *colon = strchr(cursor, ':');
        const char *value = colon ? colon + 1 : NULL;
        char url[2048];

        if (!value) {
            return FALSE;
        }
        while (*value == ' ' || *value == '\t' ||
               *value == '\r' || *value == '\n') {
            ++value;
        }
        if (ExtractJsonStringValue(value, url, sizeof(url)) &&
            strstr(url, "cMemo-") &&
            strstr(url, ".exe")) {
            return SUCCEEDED(StringCchCopyA(buffer, bufferSize, url));
        }
        ++cursor;
    }

    return FALSE;
}

static BOOL Utf8ToWideString(const char *source, WCHAR *destination,
                             int cchDestination)
{
    int written = 0;

    if (!source || !destination || cchDestination <= 0) {
        return FALSE;
    }

    written = MultiByteToWideChar(CP_UTF8,
                                  0,
                                  source,
                                  -1,
                                  destination,
                                  cchDestination);
    return written > 0;
}

static void NormalizeVersionString(const char *source, char *destination,
                                   size_t destinationSize)
{
    if (!source || !destination || destinationSize == 0) {
        return;
    }

    if (source[0] == 'v' || source[0] == 'V') {
        ++source;
    }
    StringCchCopyA(destination, destinationSize, source);
}

static int CompareVersionStrings(const char *latest, const char *current)
{
    char latestNormalized[32];
    char currentNormalized[32];
    int latestParts[3] = {0, 0, 0};
    int currentParts[3] = {0, 0, 0};
    char *context = NULL;
    char *token = NULL;
    int i = 0;

    NormalizeVersionString(latest, latestNormalized,
                           sizeof(latestNormalized));
    NormalizeVersionString(current, currentNormalized,
                           sizeof(currentNormalized));

    token = strtok_s(latestNormalized, ".", &context);
    while (token && i < 3) {
        latestParts[i++] = atoi(token);
        token = strtok_s(NULL, ".", &context);
    }

    context = NULL;
    i = 0;
    token = strtok_s(currentNormalized, ".", &context);
    while (token && i < 3) {
        currentParts[i++] = atoi(token);
        token = strtok_s(NULL, ".", &context);
    }

    for (i = 0; i < 3; ++i) {
        if (latestParts[i] > currentParts[i]) {
            return 1;
        }
        if (latestParts[i] < currentParts[i]) {
            return -1;
        }
    }
    return 0;
}

static BOOL IsDownloadedUpdateExecutable(LPCWSTR path)
{
    HANDLE file = INVALID_HANDLE_VALUE;
    BYTE signature[2];
    DWORD bytesRead = 0;
    BOOL ok = FALSE;

    file = CreateFileW(path,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (file == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    ok = ReadFile(file, signature, sizeof(signature), &bytesRead, NULL) &&
         bytesRead == sizeof(signature) &&
         signature[0] == 'M' &&
         signature[1] == 'Z';
    CloseHandle(file);
    return ok;
}

static BOOL WidePathToBatchPath(LPCWSTR source, CHAR *destination,
                                size_t cchDestination)
{
    CHAR converted[MAX_PATH * 4];
    int convertedLength = 0;
    size_t writeIndex = 0;
    int i = 0;

    if (!source || !destination || cchDestination == 0) {
        return FALSE;
    }

    destination[0] = '\0';
    convertedLength = WideCharToMultiByte(CP_ACP,
                                          0,
                                          source,
                                          -1,
                                          converted,
                                          sizeof(converted),
                                          NULL,
                                          NULL);
    if (convertedLength <= 0) {
        return FALSE;
    }

    for (i = 0; converted[i] != '\0'; ++i) {
        if (converted[i] == '\r' || converted[i] == '\n') {
            return FALSE;
        }
        if (converted[i] == '%') {
            if (writeIndex + 2 >= cchDestination) {
                return FALSE;
            }
            destination[writeIndex++] = '%';
            destination[writeIndex++] = '%';
        } else {
            if (writeIndex + 1 >= cchDestination) {
                return FALSE;
            }
            destination[writeIndex++] = converted[i];
        }
    }

    destination[writeIndex] = '\0';
    return TRUE;
}

static BOOL WriteUpdaterScript(LPCWSTR updateExePath, LPCWSTR targetExePath,
                               WCHAR *scriptPath, DWORD cchScriptPath)
{
    WCHAR tempPath[MAX_PATH];
    CHAR updatePathA[MAX_PATH * 4];
    CHAR targetPathA[MAX_PATH * 4];
    CHAR scriptPathA[MAX_PATH * 4];
    CHAR script[2048];
    HANDLE file = INVALID_HANDLE_VALUE;
    DWORD written = 0;
    size_t scriptLength = 0;

    if (!updateExePath || !targetExePath || !scriptPath ||
        cchScriptPath == 0) {
        return FALSE;
    }

    if (GetTempPathW(ARRAYSIZE(tempPath), tempPath) == 0) {
        return FALSE;
    }

    if (FAILED(StringCchPrintfW(scriptPath, cchScriptPath,
                                L"%scMemo-update.cmd", tempPath))) {
        return FALSE;
    }

    if (!WidePathToBatchPath(updateExePath, updatePathA,
                             ARRAYSIZE(updatePathA)) ||
        !WidePathToBatchPath(targetExePath, targetPathA,
                             ARRAYSIZE(targetPathA)) ||
        !WidePathToBatchPath(scriptPath, scriptPathA,
                             ARRAYSIZE(scriptPathA))) {
        return FALSE;
    }

    if (FAILED(StringCchPrintfA(
        script,
        ARRAYSIZE(script),
        "@echo off\r\n"
        "set COUNT=0\r\n"
        ":retry\r\n"
        "set /a COUNT+=1\r\n"
        "copy /Y \"%s\" \"%s\" >nul 2>nul\r\n"
        "if errorlevel 1 (\r\n"
        "  if %%COUNT%% GEQ 60 exit /b 1\r\n"
        "  timeout /t 1 /nobreak >nul\r\n"
        "  goto retry\r\n"
        ")\r\n"
        "start \"\" \"%s\"\r\n"
        "del \"%s\" >nul 2>nul\r\n"
        "del \"%s\" >nul 2>nul\r\n",
        updatePathA,
        targetPathA,
        targetPathA,
        updatePathA,
        scriptPathA))) {
        return FALSE;
    }
    scriptLength = strlen(script);

    file = CreateFileW(scriptPath,
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (file == INVALID_HANDLE_VALUE) {
        DeleteFileW(scriptPath);
        return FALSE;
    }

    if (!WriteFile(file, script, (DWORD)scriptLength, &written, NULL) ||
        written != (DWORD)scriptLength) {
        CloseHandle(file);
        DeleteFileW(scriptPath);
        return FALSE;
    }

    CloseHandle(file);
    return TRUE;
}

static BOOL LaunchUpdaterAndExit(HWND hwnd)
{
    WCHAR modulePath[MAX_PATH];
    WCHAR scriptPath[MAX_PATH];
    WCHAR commandLine[MAX_PATH * 2];
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInfo;

    if (GetModuleFileNameW(NULL, modulePath, ARRAYSIZE(modulePath)) == 0) {
        LogLastError(L"GetModuleFileName for updater");
        return FALSE;
    }

    if (!WriteUpdaterScript(g_pendingUpdatePath, modulePath,
                            scriptPath, ARRAYSIZE(scriptPath))) {
        LogError(L"Failed to write updater script.");
        return FALSE;
    }

    if (FAILED(StringCchPrintfW(commandLine, ARRAYSIZE(commandLine),
                                L"cmd.exe /c \"\"%s\"\"",
                                scriptPath))) {
        DeleteFileW(scriptPath);
        return FALSE;
    }

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_HIDE;
    ZeroMemory(&processInfo, sizeof(processInfo));

    if (!CreateProcessW(NULL,
                        commandLine,
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_NO_WINDOW,
                        NULL,
                        NULL,
                        &startupInfo,
                        &processInfo)) {
        LogLastError(L"Create updater process");
        DeleteFileW(scriptPath);
        return FALSE;
    }

    LogInfo(L"Updater launched. version=%s update=%s target=%s",
            g_pendingUpdateVersion,
            g_pendingUpdatePath,
            modulePath);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    g_isExiting = TRUE;
    SaveCurrentState(hwnd);
    DestroyWindow(hwnd);
    return TRUE;
}

typedef struct UpdateCheckContext {
    HWND hwnd;
    BOOL manual;
} UpdateCheckContext;

typedef struct UpdateDownloadContext {
    HWND hwnd;
    BOOL manual;
    WCHAR downloadUrl[2048];
    WCHAR latestVersion[32];
} UpdateDownloadContext;

static DWORD WINAPI UpdateCheckThreadProc(LPVOID parameter)
{
    UpdateCheckContext *context = (UpdateCheckContext *)parameter;
    HWND hwnd = context ? context->hwnd : NULL;
    BOOL manual = context ? context->manual : FALSE;
    BYTE *jsonBytes = NULL;
    DWORD jsonSize = 0;
    char latestTag[64];
    char downloadUrl[2048];
    char latestVersion[64];
    WCHAR downloadUrlW[2048];

    UNREFERENCED_PARAMETER(jsonSize);

    if (context) {
        HeapFree(GetProcessHeap(), 0, context);
    }

    if (!ReadUrlToMemory(GITHUB_LATEST_RELEASE_API,
                         &jsonBytes,
                         &jsonSize,
                         1024 * 1024)) {
        LogWarn(L"Update check failed while reading latest release.");
        if (manual && IsWindow(hwnd)) {
            PostMessageW(hwnd, WMAPP_UPDATE_CHECK_FAILED, 0, 0);
        }
        InterlockedExchange(&g_updateCheckInProgress, 0);
        return 0;
    }

    if (!FindJsonStringValue((const char *)jsonBytes,
                             "\"tag_name\"",
                             latestTag,
                             sizeof(latestTag)) ||
        !FindReleaseExeDownloadUrl((const char *)jsonBytes,
                                   downloadUrl,
                                   sizeof(downloadUrl))) {
        HeapFree(GetProcessHeap(), 0, jsonBytes);
        LogWarn(L"Update check failed while parsing latest release.");
        if (manual && IsWindow(hwnd)) {
            PostMessageW(hwnd, WMAPP_UPDATE_CHECK_FAILED, 0, 0);
        }
        InterlockedExchange(&g_updateCheckInProgress, 0);
        return 0;
    }
    HeapFree(GetProcessHeap(), 0, jsonBytes);

    NormalizeVersionString(latestTag, latestVersion, sizeof(latestVersion));
    if (CompareVersionStrings(latestVersion, APP_VERSION_STRING) <= 0) {
        LogInfo(L"Update check completed. current=%S latest=%S update=none",
                APP_VERSION_STRING,
                latestVersion);
        if (Utf8ToWideString(latestVersion,
                             g_availableUpdateVersion,
                             ARRAYSIZE(g_availableUpdateVersion)) &&
            manual && IsWindow(hwnd)) {
            PostMessageW(hwnd, WMAPP_UPDATE_NOT_AVAILABLE, 0, 0);
        }
        InterlockedExchange(&g_updateCheckInProgress, 0);
        return 0;
    }

    if (!Utf8ToWideString(downloadUrl, downloadUrlW,
                          ARRAYSIZE(downloadUrlW))) {
        LogWarn(L"Update check failed while converting download URL.");
        if (manual && IsWindow(hwnd)) {
            PostMessageW(hwnd, WMAPP_UPDATE_CHECK_FAILED, 0, 0);
        }
        InterlockedExchange(&g_updateCheckInProgress, 0);
        return 0;
    }

    StringCchCopyW(g_availableUpdateUrl, ARRAYSIZE(g_availableUpdateUrl),
                   downloadUrlW);
    if (!Utf8ToWideString(latestVersion,
                          g_availableUpdateVersion,
                          ARRAYSIZE(g_availableUpdateVersion))) {
        g_availableUpdateVersion[0] = L'\0';
    }

    InterlockedExchange(&g_updateCheckInProgress, 0);
    LogInfo(L"Update available. current=%S latest=%s manual=%d",
            APP_VERSION_STRING,
            g_availableUpdateVersion,
            manual);
    if (IsWindow(hwnd)) {
        PostMessageW(hwnd, WMAPP_UPDATE_AVAILABLE, manual ? 1 : 0, 0);
    }
    return 0;
}

static DWORD WINAPI UpdateDownloadThreadProc(LPVOID parameter)
{
    UpdateDownloadContext *context = (UpdateDownloadContext *)parameter;
    HWND hwnd = context ? context->hwnd : NULL;
    BOOL manual = context ? context->manual : FALSE;
    WCHAR downloadUrl[2048];
    WCHAR latestVersion[32];
    WCHAR tempPath[MAX_PATH];
    WCHAR updatePath[MAX_PATH];

    if (!context) {
        InterlockedExchange(&g_updateDownloadInProgress, 0);
        return 0;
    }

    StringCchCopyW(downloadUrl, ARRAYSIZE(downloadUrl), context->downloadUrl);
    StringCchCopyW(latestVersion, ARRAYSIZE(latestVersion),
                   context->latestVersion);
    HeapFree(GetProcessHeap(), 0, context);

    if (GetTempPathW(ARRAYSIZE(tempPath), tempPath) == 0 ||
        FAILED(StringCchPrintfW(updatePath, ARRAYSIZE(updatePath),
                                L"%scMemo-update-%s.exe",
                                tempPath,
                                latestVersion))) {
        LogWarn(L"Failed to build temporary update path.");
        if (manual && IsWindow(hwnd)) {
            PostMessageW(hwnd, WMAPP_UPDATE_DOWNLOAD_FAILED, 1, 0);
        }
        InterlockedExchange(&g_updateDownloadInProgress, 0);
        return 0;
    }

    if (!DownloadUrlToFile(downloadUrl, updatePath, 64 * 1024 * 1024) ||
        !IsDownloadedUpdateExecutable(updatePath)) {
        LogWarn(L"Update download or validation failed. version=%s",
                latestVersion);
        DeleteFileW(updatePath);
        if (manual && IsWindow(hwnd)) {
            PostMessageW(hwnd, WMAPP_UPDATE_DOWNLOAD_FAILED, 1, 0);
        }
        InterlockedExchange(&g_updateDownloadInProgress, 0);
        return 0;
    }

    StringCchCopyW(g_pendingUpdatePath, ARRAYSIZE(g_pendingUpdatePath),
                   updatePath);
    StringCchCopyW(g_pendingUpdateVersion, ARRAYSIZE(g_pendingUpdateVersion),
                   latestVersion);

    InterlockedExchange(&g_updateDownloadInProgress, 0);
    LogInfo(L"Update downloaded and validated. version=%s path=%s",
            latestVersion,
            updatePath);
    if (IsWindow(hwnd)) {
        PostMessageW(hwnd, WMAPP_UPDATE_READY, manual ? 1 : 0, 0);
    }
    return 0;
}

static void StartUpdateDownload(HWND hwnd, BOOL manual)
{
    HANDLE thread = NULL;
    UpdateDownloadContext *context = NULL;

    if (!IsWindow(hwnd)) {
        return;
    }

    if (InterlockedCompareExchange(&g_updateDownloadInProgress, 1, 0) != 0) {
        LogInfo(L"Update download skipped because another download is running.");
        if (manual) {
            MessageBoxW(hwnd, UiText(UI_TEXT_UPDATE_ALREADY_DOWNLOADING),
                        APP_TITLE, MB_OK | MB_ICONINFORMATION);
        }
        return;
    }

    context = (UpdateDownloadContext *)HeapAlloc(GetProcessHeap(),
                                                 HEAP_ZERO_MEMORY,
                                                 sizeof(*context));
    if (!context) {
        InterlockedExchange(&g_updateDownloadInProgress, 0);
        LogError(L"Failed to allocate update download context.");
        if (manual) {
            MessageBoxW(hwnd, UiText(UI_TEXT_UPDATE_DOWNLOAD_FAILED),
                        APP_TITLE, MB_OK | MB_ICONERROR);
        }
        return;
    }

    context->hwnd = hwnd;
    context->manual = manual;
    StringCchCopyW(context->downloadUrl, ARRAYSIZE(context->downloadUrl),
                   g_availableUpdateUrl);
    StringCchCopyW(context->latestVersion, ARRAYSIZE(context->latestVersion),
                   g_availableUpdateVersion);

    thread = CreateThread(NULL, 0, UpdateDownloadThreadProc, context, 0, NULL);
    if (thread) {
        CloseHandle(thread);
        LogInfo(L"Update download started. version=%s manual=%d",
                g_availableUpdateVersion,
                manual);
    } else {
        HeapFree(GetProcessHeap(), 0, context);
        InterlockedExchange(&g_updateDownloadInProgress, 0);
        LogLastError(L"Create update download thread");
        if (manual) {
            MessageBoxW(hwnd, UiText(UI_TEXT_UPDATE_DOWNLOAD_FAILED),
                        APP_TITLE, MB_OK | MB_ICONERROR);
        }
    }
}

static void StartUpdateCheck(HWND hwnd, BOOL manual)
{
    HANDLE thread = NULL;
    UpdateCheckContext *context = NULL;
    WCHAR message[256];

    if (!IsWindow(hwnd)) {
        return;
    }

    if (InterlockedCompareExchange(&g_updateCheckInProgress, 1, 0) != 0) {
        LogInfo(L"Update check skipped because another check is running.");
        if (manual) {
            MessageBoxW(hwnd, UiText(UI_TEXT_UPDATE_ALREADY_CHECKING),
                        APP_TITLE, MB_OK | MB_ICONINFORMATION);
        }
        return;
    }

    context = (UpdateCheckContext *)HeapAlloc(GetProcessHeap(),
                                             HEAP_ZERO_MEMORY,
                                             sizeof(*context));
    if (!context) {
        InterlockedExchange(&g_updateCheckInProgress, 0);
        LogError(L"Failed to allocate update check context.");
        if (manual) {
            if (SUCCEEDED(StringCchPrintfW(message, ARRAYSIZE(message),
                                          UiText(UI_TEXT_UPDATE_CHECK_FAILED),
                                          APP_VERSION_WIDE))) {
                MessageBoxW(hwnd, message, APP_TITLE, MB_OK | MB_ICONERROR);
            }
        }
        return;
    }

    context->hwnd = hwnd;
    context->manual = manual;
    thread = CreateThread(NULL, 0, UpdateCheckThreadProc, context, 0, NULL);
    if (thread) {
        CloseHandle(thread);
        LogInfo(L"Update check started. manual=%d", manual);
    } else {
        HeapFree(GetProcessHeap(), 0, context);
        InterlockedExchange(&g_updateCheckInProgress, 0);
        LogLastError(L"Create update check thread");
        if (manual) {
            if (SUCCEEDED(StringCchPrintfW(message, ARRAYSIZE(message),
                                          UiText(UI_TEXT_UPDATE_CHECK_FAILED),
                                          APP_VERSION_WIDE))) {
                MessageBoxW(hwnd, message, APP_TITLE, MB_OK | MB_ICONERROR);
            }
        }
    }
}

static BOOL IsAutoStartEnabled(void)
{
    HKEY key = NULL;
    WCHAR value[1024];
    DWORD valueType = 0;
    DWORD valueSize = sizeof(value);
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY_PATH, 0,
                                KEY_QUERY_VALUE, &key);

    if (result != ERROR_SUCCESS) {
        return FALSE;
    }

    result = RegQueryValueExW(key, RUN_VALUE_NAME, NULL, &valueType,
                              (LPBYTE)value, &valueSize);
    RegCloseKey(key);

    return result == ERROR_SUCCESS &&
           (valueType == REG_SZ || valueType == REG_EXPAND_SZ);
}

static BOOL SetAutoStartEnabled(BOOL enabled)
{
    HKEY key = NULL;
    LONG result = ERROR_SUCCESS;

    if (enabled) {
        WCHAR command[1024];

        if (!GetRunCommand(command, ARRAYSIZE(command))) {
            LogError(L"Failed to build auto start command.");
            return FALSE;
        }

        result = RegCreateKeyExW(HKEY_CURRENT_USER, RUN_KEY_PATH, 0, NULL,
                                 REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                                 NULL, &key, NULL);
        if (result != ERROR_SUCCESS) {
            LogWarn(L"Failed to open auto start registry key. result=%ld",
                    result);
            return FALSE;
        }

        result = RegSetValueExW(key, RUN_VALUE_NAME, 0, REG_SZ,
                                (const BYTE *)command,
                                (DWORD)((lstrlenW(command) + 1) *
                                        sizeof(WCHAR)));
        RegCloseKey(key);
        if (result == ERROR_SUCCESS) {
            LogInfo(L"Auto start enabled.");
        } else {
            LogWarn(L"Failed to enable auto start. result=%ld", result);
        }
        return result == ERROR_SUCCESS;
    }

    result = RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY_PATH, 0,
                           KEY_SET_VALUE, &key);
    if (result == ERROR_FILE_NOT_FOUND) {
        LogInfo(L"Auto start registry key was not found while disabling.");
        return TRUE;
    }
    if (result != ERROR_SUCCESS) {
        LogWarn(L"Failed to open auto start registry key for delete. result=%ld",
                result);
        return FALSE;
    }

    result = RegDeleteValueW(key, RUN_VALUE_NAME);
    RegCloseKey(key);
    if (result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND) {
        LogInfo(L"Auto start disabled.");
    } else {
        LogWarn(L"Failed to disable auto start. result=%ld", result);
    }
    return result == ERROR_SUCCESS || result == ERROR_FILE_NOT_FOUND;
}

static void ApplyDisplayMode(HWND hwnd, DisplayMode mode)
{
    HWND insertAfter = mode == DISPLAY_MODE_TOPMOST ? HWND_TOPMOST : HWND_NOTOPMOST;

    if (!IsWindow(hwnd)) {
        return;
    }

    SetWindowPos(hwnd, insertAfter, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

    g_settings.displayMode = mode;
    g_settings.topmost = mode == DISPLAY_MODE_TOPMOST;
}

static void ResizeEditControl(HWND hwnd)
{
    RECT clientRect;
    int width = 0;
    int height = 0;
    int editorWidth = 0;

    if (!g_editWindow) {
        return;
    }

    g_editorLayoutUpdating = TRUE;
    GetClientRect(hwnd, &clientRect);
    width = clientRect.right - clientRect.left - (BORDER_THICKNESS * 2);
    height = clientRect.bottom - clientRect.top -
             CUSTOM_TITLE_BAR_HEIGHT - (BORDER_THICKNESS * 2);
    if (width < 0) {
        width = 0;
    }
    if (height < 0) {
        height = 0;
    }

    editorWidth = width;
    if (g_editorScrollBarVisible) {
        editorWidth -= EDIT_SCROLLBAR_WIDTH + EDIT_SCROLLBAR_GAP;
        if (editorWidth < 0) {
            editorWidth = 0;
        }
    }

    MoveWindow(g_editWindow, BORDER_THICKNESS,
               CUSTOM_TITLE_BAR_HEIGHT + BORDER_THICKNESS,
               editorWidth,
               height,
               TRUE);
    g_editorLayoutUpdating = FALSE;
    RefreshEditorScrollBar(hwnd, TRUE);
}

static void GetEditorAreaRect(HWND hwnd, RECT *rect)
{
    RECT clientRect;

    if (!rect) {
        return;
    }

    SetRectEmpty(rect);
    if (!IsWindow(hwnd)) {
        return;
    }

    GetClientRect(hwnd, &clientRect);
    rect->left = BORDER_THICKNESS;
    rect->top = CUSTOM_TITLE_BAR_HEIGHT + BORDER_THICKNESS;
    rect->right = max(rect->left,
                      clientRect.right - BORDER_THICKNESS);
    rect->bottom = max(rect->top,
                       clientRect.bottom - BORDER_THICKNESS);
}

static void GetEditorScrollBarRect(HWND hwnd, RECT *rect)
{
    RECT areaRect;

    if (!rect) {
        return;
    }

    GetEditorAreaRect(hwnd, &areaRect);
    *rect = areaRect;
    rect->left = max(rect->left, rect->right - EDIT_SCROLLBAR_WIDTH);
}

static int GetEditorLineHeight(void)
{
    HDC hdc = NULL;
    HFONT oldFont = NULL;
    TEXTMETRICW metrics;
    int lineHeight = 16;

    if (!g_editWindow) {
        return lineHeight;
    }

    hdc = GetDC(g_editWindow);
    if (!hdc) {
        return lineHeight;
    }

    if (g_editFont) {
        oldFont = (HFONT)SelectObject(hdc, g_editFont);
    }

    ZeroMemory(&metrics, sizeof(metrics));
    if (GetTextMetricsW(hdc, &metrics)) {
        lineHeight = max(1, metrics.tmHeight + metrics.tmExternalLeading);
    }

    if (oldFont) {
        SelectObject(hdc, oldFont);
    }
    ReleaseDC(g_editWindow, hdc);
    return lineHeight;
}

static int GetEditorVisibleLineCount(void)
{
    RECT editRect;
    int lineHeight = GetEditorLineHeight();
    int height = 0;

    if (!g_editWindow || lineHeight <= 0) {
        return 1;
    }

    GetClientRect(g_editWindow, &editRect);
    height = editRect.bottom - editRect.top;
    return max(1, height / lineHeight);
}

static int GetEditorLineCount(void)
{
    if (!g_editWindow) {
        return 1;
    }

    return max(1, (int)SendMessageW(g_editWindow, EM_GETLINECOUNT, 0, 0));
}

static int GetEditorFirstVisibleLine(void)
{
    if (!g_editWindow) {
        return 0;
    }

    return max(0, (int)SendMessageW(g_editWindow,
                                    EM_GETFIRSTVISIBLELINE, 0, 0));
}

static int GetEditorMaxFirstVisibleLine(void)
{
    return max(0, GetEditorLineCount() - GetEditorVisibleLineCount());
}

static BOOL IsEditorScrollBarNeeded(void)
{
    return GetEditorMaxFirstVisibleLine() > 0;
}

static void InvalidateEditorScrollBar(HWND hwnd)
{
    RECT barRect;

    if (!IsWindow(hwnd)) {
        return;
    }

    GetEditorScrollBarRect(hwnd, &barRect);
    InvalidateRect(hwnd, &barRect, TRUE);
}

static BOOL GetEditorScrollThumbRect(HWND hwnd, RECT *thumbRect)
{
    RECT barRect;
    int lineCount = GetEditorLineCount();
    int visibleLines = GetEditorVisibleLineCount();
    int maxFirstLine = GetEditorMaxFirstVisibleLine();
    int firstLine = min(GetEditorFirstVisibleLine(), maxFirstLine);
    int trackHeight = 0;
    int thumbHeight = 0;
    int thumbTop = 0;

    if (!thumbRect || !g_editorScrollBarVisible || maxFirstLine <= 0) {
        if (thumbRect) {
            SetRectEmpty(thumbRect);
        }
        return FALSE;
    }

    GetEditorScrollBarRect(hwnd, &barRect);
    InflateRect(&barRect, -2, -4);
    trackHeight = max(1, barRect.bottom - barRect.top);
    thumbHeight = max(EDIT_SCROLLBAR_MIN_THUMB_HEIGHT,
                      MulDiv(trackHeight, visibleLines, lineCount));
    thumbHeight = min(thumbHeight, trackHeight);
    thumbTop = barRect.top;
    if (trackHeight > thumbHeight) {
        thumbTop += MulDiv(trackHeight - thumbHeight,
                           firstLine,
                           maxFirstLine);
    }

    SetRect(thumbRect,
            barRect.left,
            thumbTop,
            barRect.right,
            thumbTop + thumbHeight);
    return TRUE;
}

static void RefreshEditorScrollBar(HWND hwnd, BOOL updateLayout)
{
    BOOL needed = IsEditorScrollBarNeeded();

    if (needed != g_editorScrollBarVisible) {
        g_editorScrollBarVisible = needed;
        if (updateLayout && !g_editorLayoutUpdating) {
            ResizeEditControl(hwnd);
            return;
        }
    }

    InvalidateEditorScrollBar(hwnd);
}

static void ScrollEditorToFirstVisibleLine(HWND hwnd, int targetFirstLine)
{
    int currentFirstLine = GetEditorFirstVisibleLine();
    int maxFirstLine = GetEditorMaxFirstVisibleLine();
    int newFirstLine = ClampInt(targetFirstLine, 0, maxFirstLine);
    int delta = newFirstLine - currentFirstLine;

    if (!g_editWindow || delta == 0) {
        RefreshEditorScrollBar(hwnd, FALSE);
        return;
    }

    SendMessageW(g_editWindow, EM_LINESCROLL, 0, delta);
    RefreshEditorScrollBar(hwnd, FALSE);
}

static BOOL IsPointInEditorScrollBar(HWND hwnd, int x, int y)
{
    RECT barRect;
    POINT point;

    if (!g_editorScrollBarVisible) {
        return FALSE;
    }

    point.x = x;
    point.y = y;
    GetEditorScrollBarRect(hwnd, &barRect);
    return PtInRect(&barRect, point);
}

static BOOL BeginEditorScrollBarDrag(HWND hwnd, int x, int y)
{
    RECT thumbRect;
    POINT point;

    if (!GetEditorScrollThumbRect(hwnd, &thumbRect)) {
        return FALSE;
    }

    point.x = x;
    point.y = y;
    if (!PtInRect(&thumbRect, point)) {
        return FALSE;
    }

    g_editorScrollBarDragging = TRUE;
    g_editorScrollBarDragY = y;
    g_editorScrollBarDragFirstLine = GetEditorFirstVisibleLine();
    SetCapture(hwnd);
    return TRUE;
}

static void PageEditorFromScrollBarClick(HWND hwnd, int y)
{
    RECT thumbRect;
    int firstLine = GetEditorFirstVisibleLine();
    int visibleLines = GetEditorVisibleLineCount();

    if (!GetEditorScrollThumbRect(hwnd, &thumbRect)) {
        return;
    }

    if (y < thumbRect.top) {
        ScrollEditorToFirstVisibleLine(hwnd, firstLine - visibleLines);
    } else if (y > thumbRect.bottom) {
        ScrollEditorToFirstVisibleLine(hwnd, firstLine + visibleLines);
    }
}

static void UpdateEditorScrollBarDrag(HWND hwnd, int y)
{
    RECT barRect;
    RECT thumbRect;
    int trackHeight = 0;
    int thumbHeight = 0;
    int availableHeight = 0;
    int maxFirstLine = GetEditorMaxFirstVisibleLine();
    int deltaY = y - g_editorScrollBarDragY;
    int deltaLines = 0;

    if (!g_editorScrollBarDragging ||
        !GetEditorScrollThumbRect(hwnd, &thumbRect)) {
        return;
    }

    GetEditorScrollBarRect(hwnd, &barRect);
    InflateRect(&barRect, -2, -4);
    trackHeight = max(1, barRect.bottom - barRect.top);
    thumbHeight = max(1, thumbRect.bottom - thumbRect.top);
    availableHeight = max(1, trackHeight - thumbHeight);
    deltaLines = MulDiv(deltaY, maxFirstLine, availableHeight);

    ScrollEditorToFirstVisibleLine(hwnd,
                                   g_editorScrollBarDragFirstLine +
                                       deltaLines);
}

static void EndEditorScrollBarDrag(HWND hwnd)
{
    if (!g_editorScrollBarDragging) {
        return;
    }

    g_editorScrollBarDragging = FALSE;
    if (GetCapture() == hwnd) {
        ReleaseCapture();
    }
    RefreshEditorScrollBar(hwnd, FALSE);
}

static void DrawEditorScrollBar(HWND hwnd, HDC hdc)
{
    RECT barRect;
    RECT trackRect;
    RECT thumbRect;
    HBRUSH trackBrush = NULL;
    HBRUSH thumbBrush = NULL;
    HPEN trackPen = NULL;
    HPEN thumbPen = NULL;
    HGDIOBJ oldBrush = NULL;
    HGDIOBJ oldPen = NULL;
    COLORREF trackColor = BlendColor(g_settings.editorBackgroundColor,
                                     g_settings.borderColor, 12);
    COLORREF thumbColor = BlendColor(g_settings.borderColor,
                                     RGB(255, 255, 255), 18);
    COLORREF thumbLineColor = BlendColor(g_settings.borderColor,
                                         RGB(0, 0, 0), 8);

    if (!g_editorScrollBarVisible ||
        !GetEditorScrollThumbRect(hwnd, &thumbRect)) {
        return;
    }

    GetEditorScrollBarRect(hwnd, &barRect);
    trackRect = barRect;
    InflateRect(&trackRect, -3, -4);

    trackBrush = CreateSolidBrush(trackColor);
    trackPen = CreatePen(PS_SOLID, 1, trackColor);
    thumbBrush = CreateSolidBrush(thumbColor);
    thumbPen = CreatePen(PS_SOLID, 1, thumbLineColor);
    if (!trackBrush || !trackPen || !thumbBrush || !thumbPen) {
        goto cleanup;
    }

    oldBrush = SelectObject(hdc, trackBrush);
    oldPen = SelectObject(hdc, trackPen);
    RoundRect(hdc,
              trackRect.left,
              trackRect.top,
              trackRect.right,
              trackRect.bottom,
              EDIT_SCROLLBAR_WIDTH,
              EDIT_SCROLLBAR_WIDTH);

    SelectObject(hdc, thumbBrush);
    SelectObject(hdc, thumbPen);
    RoundRect(hdc,
              thumbRect.left,
              thumbRect.top,
              thumbRect.right,
              thumbRect.bottom,
              EDIT_SCROLLBAR_WIDTH,
              EDIT_SCROLLBAR_WIDTH);

cleanup:
    if (oldBrush) {
        SelectObject(hdc, oldBrush);
    }
    if (oldPen) {
        SelectObject(hdc, oldPen);
    }
    if (trackBrush) {
        DeleteObject(trackBrush);
    }
    if (trackPen) {
        DeleteObject(trackPen);
    }
    if (thumbBrush) {
        DeleteObject(thumbBrush);
    }
    if (thumbPen) {
        DeleteObject(thumbPen);
    }
}

static BOOL CreateEditControl(HWND hwnd)
{
    g_editWindow = CreateWindowExW(
        0,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE |
            ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL |
            ES_WANTRETURN | ES_NOHIDESEL,
        0,
        0,
        0,
        0,
        hwnd,
        (HMENU)(INT_PTR)ID_EDIT,
        g_instance,
        NULL);

    if (!g_editWindow) {
        return FALSE;
    }

    ApplyEditorFont();
    ApplyEditorBackgroundColor();
    SendMessageW(g_editWindow, EM_LIMITTEXT, MEMO_TEXT_MAX_CHARS - 1, 0);
    SetWindowTextW(g_editWindow, g_settings.memoText);

    g_originalEditProc = (WNDPROC)SetWindowLongPtrW(
        g_editWindow, GWLP_WNDPROC, (LONG_PTR)EditProc);

    ResizeEditControl(hwnd);
    return TRUE;
}

static BOOL SaveCurrentState(HWND hwnd)
{
    RECT windowRect;

    if (g_editWindow) {
        GetWindowTextW(g_editWindow, g_settings.memoText,
                       ARRAYSIZE(g_settings.memoText));
    }

    if (IsWindow(hwnd) && !IsIconic(hwnd) && GetWindowRect(hwnd, &windowRect)) {
        g_settings.x = windowRect.left;
        g_settings.y = windowRect.top;
        g_settings.width = windowRect.right - windowRect.left;
        g_settings.height = windowRect.bottom - windowRect.top;
    }

    if (g_settings.width < MIN_WINDOW_WIDTH) {
        g_settings.width = DEFAULT_WINDOW_WIDTH;
    }
    if (g_settings.height < MIN_WINDOW_HEIGHT) {
        g_settings.height = DEFAULT_WINDOW_HEIGHT;
    }

    if (SaveSettings(&g_settings)) {
        LogInfo(L"Settings saved. memoChars=%d width=%d height=%d mode=%d",
                lstrlenW(g_settings.memoText),
                g_settings.width,
                g_settings.height,
                (int)g_settings.displayMode);
        return TRUE;
    }

    LogError(L"Failed to save settings.");
    return FALSE;
}

static void ShowSaveNotification(HWND hwnd)
{
    if (!IsWindow(hwnd)) {
        return;
    }

    g_showSaveNotification = TRUE;
    g_titleHovered = TRUE;
    SetTimer(hwnd, IDT_SAVE_NOTIFICATION, SAVE_NOTIFICATION_MS, NULL);
    ApplyTitleBarAppearance(hwnd);
}

static void FinishSaveNotification(HWND hwnd)
{
    KillTimer(hwnd, IDT_SAVE_NOTIFICATION);
    g_showSaveNotification = FALSE;
    if (g_settings.titleHoverOnly && !IsCursorOverWindow(hwnd)) {
        g_titleHovered = FALSE;
    }
    ApplyTitleBarAppearance(hwnd);
}

static void SaveCurrentStateWithNotification(HWND hwnd)
{
    if (SaveCurrentState(hwnd)) {
        if (IsWindowVisible(hwnd)) {
            ShowSaveNotification(hwnd);
        } else {
            MessageBoxW(hwnd, UiText(UI_TEXT_SAVED), APP_TITLE, MB_OK);
        }
    } else {
        MessageBoxW(hwnd, UiText(UI_TEXT_SAVE_ERROR), APP_TITLE, MB_ICONERROR);
    }
}

static void DestroyMarqueeIcons(void)
{
    if (g_marqueeSmallIcon) {
        DestroyIcon(g_marqueeSmallIcon);
        g_marqueeSmallIcon = NULL;
    }
    if (g_marqueeBigIcon) {
        DestroyIcon(g_marqueeBigIcon);
        g_marqueeBigIcon = NULL;
    }
}

static void GetMarqueeText(WCHAR *buffer, size_t cchBuffer)
{
    WCHAR source[ICON_MARQUEE_TEXT_MAX_CHARS];
    size_t sourceLength = 0;
    size_t readIndex = 0;
    size_t writeIndex = 0;
    BOOL previousWasSpace = TRUE;

    if (!buffer || cchBuffer == 0) {
        return;
    }

    buffer[0] = L'\0';
    source[0] = L'\0';

    if (g_editWindow) {
        GetWindowTextW(g_editWindow, source, ARRAYSIZE(source));
    } else {
        StringCchCopyW(source, ARRAYSIZE(source), g_settings.memoText);
    }

    sourceLength = lstrlenW(source);
    for (readIndex = 0;
         readIndex < sourceLength && writeIndex + 1 < cchBuffer;
         ++readIndex) {
        WCHAR ch = source[readIndex];
        BOOL isSpace = ch == L'\r' || ch == L'\n' ||
                       ch == L'\t' || ch == L' ';

        if (isSpace) {
            if (!previousWasSpace && writeIndex + 1 < cchBuffer) {
                buffer[writeIndex++] = L' ';
            }
            previousWasSpace = TRUE;
            continue;
        }

        buffer[writeIndex++] = ch;
        previousWasSpace = FALSE;
    }

    while (writeIndex > 0 && buffer[writeIndex - 1] == L' ') {
        --writeIndex;
    }
    buffer[writeIndex] = L'\0';

    if (buffer[0] == L'\0') {
        StringCchCopyW(buffer, cchBuffer, APP_TITLE);
    }
}

static HBITMAP CreateOpaqueMaskBitmap(int width, int height)
{
    DWORD stride = ((DWORD)width + 31) / 32 * 4;
    DWORD maskBytes = stride * (DWORD)height;
    BYTE *maskBits = NULL;
    HBITMAP maskBitmap = NULL;

    maskBits = (BYTE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, maskBytes);
    if (!maskBits) {
        return NULL;
    }

    maskBitmap = CreateBitmap(width, height, 1, 1, maskBits);
    HeapFree(GetProcessHeap(), 0, maskBits);
    return maskBitmap;
}

static HICON CreateMarqueeIcon(int width, int height, LPCWSTR text)
{
    HDC screenDc = NULL;
    HDC memoryDc = NULL;
    HBITMAP colorBitmap = NULL;
    HBITMAP maskBitmap = NULL;
    HBITMAP oldBitmap = NULL;
    HBRUSH backgroundBrush = NULL;
    HFONT font = NULL;
    HFONT oldFont = NULL;
    ICONINFO iconInfo;
    HICON icon = NULL;
    RECT iconRect;
    SIZE textSize;
    int fontHeight = 0;
    int verticalPadding = 0;
    int textLength = 0;
    int gap = 0;
    int totalWidth = 0;
    int x = 0;
    int y = 0;

    if (width <= 0 || height <= 0 || !text) {
        return NULL;
    }

    screenDc = GetDC(NULL);
    if (!screenDc) {
        return NULL;
    }

    memoryDc = CreateCompatibleDC(screenDc);
    colorBitmap = CreateCompatibleBitmap(screenDc, width, height);
    maskBitmap = CreateOpaqueMaskBitmap(width, height);
    if (!memoryDc || !colorBitmap || !maskBitmap) {
        goto cleanup;
    }

    oldBitmap = (HBITMAP)SelectObject(memoryDc, colorBitmap);
    SetRect(&iconRect, 0, 0, width, height);

    backgroundBrush = CreateSolidBrush(g_settings.iconMarqueeBackgroundColor);
    if (backgroundBrush) {
        FillRect(memoryDc, &iconRect, backgroundBrush);
    }

    textLength = lstrlenW(text);
    verticalPadding = max(1, width / 64);
    fontHeight = max(8, height - (verticalPadding * 2));
    while (fontHeight >= 6) {
        font = CreateFontW(-fontHeight,
                           0,
                           0,
                           0,
                           g_settings.iconMarqueeFontWeight,
                           (DWORD)(g_settings.iconMarqueeFontItalic ? TRUE : FALSE),
                           FALSE,
                           FALSE,
                           DEFAULT_CHARSET,
                           OUT_DEFAULT_PRECIS,
                           CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY,
                           DEFAULT_PITCH | FF_DONTCARE,
                           g_settings.iconMarqueeFontFace);
        if (!font) {
            break;
        }

        oldFont = (HFONT)SelectObject(memoryDc, font);
        ZeroMemory(&textSize, sizeof(textSize));
        GetTextExtentPoint32W(memoryDc, text, textLength, &textSize);
        if (textSize.cy <= height - (verticalPadding * 2) ||
            fontHeight == 6) {
            break;
        }

        SelectObject(memoryDc, oldFont);
        oldFont = NULL;
        DeleteObject(font);
        font = NULL;
        fontHeight -= 1;
    }

    gap = max(width / 2, 8);
    totalWidth = max(1, textSize.cx + gap);
    if (g_settings.iconMarqueeReverse) {
        x = (g_marqueeOffset % totalWidth) - textSize.cx;
    } else {
        x = width - (g_marqueeOffset % totalWidth);
    }
    y = max(0, (height - textSize.cy) / 2);

    SetBkMode(memoryDc, TRANSPARENT);
    SetTextColor(memoryDc, g_settings.iconMarqueeTextColor);
    IntersectClipRect(memoryDc, 0, 0, width, height);
    TextOutW(memoryDc, x, y, text, textLength);
    if (g_settings.iconMarqueeReverse) {
        if (x > 0) {
            TextOutW(memoryDc, x - totalWidth, y, text, textLength);
        }
    } else if (x + textSize.cx < width) {
        TextOutW(memoryDc, x + totalWidth, y, text, textLength);
    }

    ZeroMemory(&iconInfo, sizeof(iconInfo));
    iconInfo.fIcon = TRUE;
    iconInfo.hbmColor = colorBitmap;
    iconInfo.hbmMask = maskBitmap;
    icon = CreateIconIndirect(&iconInfo);

cleanup:
    if (oldFont) {
        SelectObject(memoryDc, oldFont);
    }
    if (font) {
        DeleteObject(font);
    }
    if (oldBitmap) {
        SelectObject(memoryDc, oldBitmap);
    }
    if (backgroundBrush) {
        DeleteObject(backgroundBrush);
    }
    if (maskBitmap) {
        DeleteObject(maskBitmap);
    }
    if (colorBitmap) {
        DeleteObject(colorBitmap);
    }
    if (memoryDc) {
        DeleteDC(memoryDc);
    }
    if (screenDc) {
        ReleaseDC(NULL, screenDc);
    }

    return icon;
}

static BOOL SetTrayIcon(HWND hwnd, HICON icon)
{
    NOTIFYICONDATAW notifyData;
    HICON effectiveIcon = icon ? icon :
        LoadAppIcon(GetSystemMetrics(SM_CXSMICON),
                    GetSystemMetrics(SM_CYSMICON));
    DWORD operation = g_trayIconAdded ? NIM_MODIFY : NIM_ADD;

    ZeroMemory(&notifyData, sizeof(notifyData));
    notifyData.cbSize = sizeof(notifyData);
    notifyData.hWnd = hwnd;
    notifyData.uID = TRAY_ICON_UID;
    notifyData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    notifyData.uCallbackMessage = WMAPP_TRAYICON;
    notifyData.hIcon = effectiveIcon;
    StringCchCopyW(notifyData.szTip, ARRAYSIZE(notifyData.szTip), APP_TITLE);

    if (!Shell_NotifyIconW(operation, &notifyData)) {
        if (operation == NIM_MODIFY &&
            Shell_NotifyIconW(NIM_ADD, &notifyData)) {
            g_trayIconAdded = TRUE;
            return TRUE;
        }
        LogLastError(L"Shell_NotifyIcon");
        return FALSE;
    }

    g_trayIconAdded = TRUE;
    return TRUE;
}

static BOOL AddTrayIcon(HWND hwnd)
{
    return SetTrayIcon(hwnd, NULL);
}

static void RestoreDefaultIcons(HWND hwnd)
{
    SendMessageW(hwnd, WM_SETICON, ICON_SMALL,
                 (LPARAM)LoadAppIcon(GetSystemMetrics(SM_CXSMICON),
                                      GetSystemMetrics(SM_CYSMICON)));
    SendMessageW(hwnd, WM_SETICON, ICON_BIG,
                 (LPARAM)LoadAppIcon(GetSystemMetrics(SM_CXICON),
                                      GetSystemMetrics(SM_CYICON)));
    AddTrayIcon(hwnd);
    DestroyMarqueeIcons();
}

static void UpdateIconMarquee(HWND hwnd)
{
    WCHAR text[ICON_MARQUEE_TEXT_MAX_CHARS];
    HICON newSmallIcon = NULL;
    HICON newBigIcon = NULL;
    HICON oldSmallIcon = NULL;
    HICON oldBigIcon = NULL;

    if (!IsWindow(hwnd) || !g_settings.iconMarquee) {
        return;
    }

    GetMarqueeText(text, ARRAYSIZE(text));

    newSmallIcon = CreateMarqueeIcon(GetSystemMetrics(SM_CXSMICON),
                                     GetSystemMetrics(SM_CYSMICON),
                                     text);
    newBigIcon = CreateMarqueeIcon(GetSystemMetrics(SM_CXICON),
                                   GetSystemMetrics(SM_CYICON),
                                   text);
    if (!newSmallIcon || !newBigIcon) {
        if (newSmallIcon) {
            DestroyIcon(newSmallIcon);
        }
        if (newBigIcon) {
            DestroyIcon(newBigIcon);
        }
        return;
    }

    oldSmallIcon = g_marqueeSmallIcon;
    oldBigIcon = g_marqueeBigIcon;
    g_marqueeSmallIcon = newSmallIcon;
    g_marqueeBigIcon = newBigIcon;

    SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_marqueeSmallIcon);
    SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_marqueeBigIcon);
    SetTrayIcon(hwnd, g_marqueeSmallIcon);

    if (oldSmallIcon) {
        DestroyIcon(oldSmallIcon);
    }
    if (oldBigIcon) {
        DestroyIcon(oldBigIcon);
    }

    g_marqueeOffset += ClampInt(g_settings.iconMarqueeSpeed,
                                MIN_ICON_MARQUEE_SPEED,
                                MAX_ICON_MARQUEE_SPEED);
}

static void StartIconMarquee(HWND hwnd)
{
    if (!IsWindow(hwnd)) {
        return;
    }

    g_marqueeOffset = 0;
    UpdateIconMarquee(hwnd);
    SetTimer(hwnd, IDT_ICON_MARQUEE, ICON_MARQUEE_INTERVAL_MS, NULL);
}

static void StopIconMarquee(HWND hwnd)
{
    if (!IsWindow(hwnd)) {
        return;
    }

    KillTimer(hwnd, IDT_ICON_MARQUEE);
    g_marqueeOffset = 0;
    RestoreDefaultIcons(hwnd);
}

static void RemoveTrayIcon(HWND hwnd)
{
    NOTIFYICONDATAW notifyData;

    if (!g_trayIconAdded) {
        return;
    }

    ZeroMemory(&notifyData, sizeof(notifyData));
    notifyData.cbSize = sizeof(notifyData);
    notifyData.hWnd = hwnd;
    notifyData.uID = TRAY_ICON_UID;
    Shell_NotifyIconW(NIM_DELETE, &notifyData);
    g_trayIconAdded = FALSE;
}

static void ShowMainWindow(HWND hwnd)
{
    ShowWindow(hwnd, SW_SHOWNORMAL);
    ApplyDisplayMode(hwnd, g_settings.displayMode);
    g_titleHovered = !g_settings.titleHoverOnly || IsCursorOverWindow(hwnd);
    ApplyTitleBarAppearance(hwnd);
    SetForegroundWindow(hwnd);
    if (g_editWindow) {
        SetFocus(g_editWindow);
    }
}

static void HideMainWindow(HWND hwnd)
{
    LogInfo(L"Main window hidden.");
    SaveCurrentState(hwnd);
    ShowWindow(hwnd, SW_HIDE);
}

static void ExitApplication(HWND hwnd)
{
    LogInfo(L"Exit requested.");
    g_isExiting = TRUE;
    SaveCurrentState(hwnd);
    DestroyWindow(hwnd);
}

static void AddDisplayMenuItems(HMENU menu)
{
    UINT topmostChecked = g_settings.displayMode == DISPLAY_MODE_TOPMOST ? MF_CHECKED : 0;
    UINT desktopChecked = g_settings.displayMode == DISPLAY_MODE_DESKTOP ? MF_CHECKED : 0;
    UINT normalChecked = g_settings.displayMode == DISPLAY_MODE_NORMAL ? MF_CHECKED : 0;

    AppendMenuW(menu, MF_STRING | topmostChecked,
                IDM_TOPMOST_ON, UiText(UI_TEXT_ALWAYS_ON_TOP));
    AppendMenuW(menu, MF_STRING,
                IDM_TOPMOST_OFF, UiText(UI_TEXT_REMOVE_ALWAYS_ON_TOP));
    AppendMenuW(menu, MF_STRING | desktopChecked,
                IDM_DESKTOP_MODE, UiText(UI_TEXT_DESKTOP_MODE));
    AppendMenuW(menu, MF_STRING | normalChecked,
                IDM_NORMAL_MODE, UiText(UI_TEXT_NORMAL_DISPLAY));
}

static void AddIconMarqueeSpeedMenu(HMENU menu)
{
    HMENU speedMenu = CreatePopupMenu();
    int speed = ClampInt(g_settings.iconMarqueeSpeed,
                         MIN_ICON_MARQUEE_SPEED,
                         MAX_ICON_MARQUEE_SPEED);

    if (!speedMenu) {
        return;
    }

    AppendMenuW(speedMenu, MF_STRING | (speed == 1 ? MF_CHECKED : 0),
                IDM_ICON_MARQUEE_SPEED_SLOW, UiText(UI_TEXT_SPEED_SLOW));
    AppendMenuW(speedMenu, MF_STRING | (speed == 3 ? MF_CHECKED : 0),
                IDM_ICON_MARQUEE_SPEED_NORMAL, UiText(UI_TEXT_SPEED_NORMAL));
    AppendMenuW(speedMenu, MF_STRING | (speed == 5 ? MF_CHECKED : 0),
                IDM_ICON_MARQUEE_SPEED_FAST, UiText(UI_TEXT_SPEED_FAST));
    AppendMenuW(speedMenu, MF_STRING | (speed == 8 ? MF_CHECKED : 0),
                IDM_ICON_MARQUEE_SPEED_FASTEST, UiText(UI_TEXT_SPEED_FASTEST));

    AppendMenuW(menu, MF_POPUP, (UINT_PTR)speedMenu,
                UiText(UI_TEXT_ICON_SCROLL_SPEED));
}

static void AppendFontSizeItem(HMENU menu, int pointSize, UINT commandId)
{
    WCHAR label[16];
    UINT checked = g_settings.fontPointSize == pointSize ? MF_CHECKED : 0;

    if (FAILED(StringCchPrintfW(label, ARRAYSIZE(label),
                                L"%d pt", pointSize))) {
        return;
    }

    AppendMenuW(menu, MF_STRING | checked, commandId, label);
}

static void AddFontSizeMenu(HMENU menu)
{
    HMENU fontSizeMenu = CreatePopupMenu();

    if (!fontSizeMenu) {
        return;
    }

    AppendFontSizeItem(fontSizeMenu, 6, IDM_FONT_SIZE_6);
    AppendFontSizeItem(fontSizeMenu, 7, IDM_FONT_SIZE_7);
    AppendFontSizeItem(fontSizeMenu, 8, IDM_FONT_SIZE_8);
    AppendFontSizeItem(fontSizeMenu, 9, IDM_FONT_SIZE_9);
    AppendFontSizeItem(fontSizeMenu, 10, IDM_FONT_SIZE_10);
    AppendFontSizeItem(fontSizeMenu, 12, IDM_FONT_SIZE_12);
    AppendFontSizeItem(fontSizeMenu, 14, IDM_FONT_SIZE_14);
    AppendFontSizeItem(fontSizeMenu, 16, IDM_FONT_SIZE_16);
    AppendFontSizeItem(fontSizeMenu, 18, IDM_FONT_SIZE_18);
    AppendFontSizeItem(fontSizeMenu, 24, IDM_FONT_SIZE_24);

    AppendMenuW(menu, MF_POPUP, (UINT_PTR)fontSizeMenu,
                UiText(UI_TEXT_FONT_SIZE));
}

static void AddAppearanceMenuItems(HMENU menu)
{
    UINT titleHoverChecked = g_settings.titleHoverOnly ? MF_CHECKED : 0;

    AppendMenuW(menu, MF_STRING, IDM_FONT, UiText(UI_TEXT_FONT));
    AddFontSizeMenu(menu);
    AppendMenuW(menu, MF_STRING, IDM_EDITOR_BACKGROUND_COLOR,
                UiText(UI_TEXT_EDITOR_BACKGROUND_COLOR));
    AppendMenuW(menu, MF_STRING, IDM_BORDER_COLOR,
                UiText(UI_TEXT_WINDOW_COLOR));
    AppendMenuW(menu, MF_STRING | titleHoverChecked,
                IDM_TOGGLE_TITLE_HOVER,
                UiText(UI_TEXT_SHOW_TITLE_ON_HOVER_ONLY));
}

static void AddIconMarqueeMenuItems(HMENU menu)
{
    UINT iconMarqueeChecked = g_settings.iconMarquee ? MF_CHECKED : 0;
    UINT iconMarqueeReverseChecked =
        g_settings.iconMarqueeReverse ? MF_CHECKED : 0;

    AppendMenuW(menu, MF_STRING | iconMarqueeChecked,
                IDM_TOGGLE_ICON_MARQUEE,
                UiText(UI_TEXT_SCROLL_MEMO_ON_ICON));
    AppendMenuW(menu, MF_STRING | iconMarqueeReverseChecked,
                IDM_TOGGLE_ICON_MARQUEE_REVERSE,
                UiText(UI_TEXT_REVERSE_ICON_SCROLL));
    AppendMenuW(menu, MF_STRING, IDM_ICON_MARQUEE_TEXT_COLOR,
                UiText(UI_TEXT_ICON_TEXT_COLOR));
    AppendMenuW(menu, MF_STRING, IDM_ICON_MARQUEE_BACKGROUND_COLOR,
                UiText(UI_TEXT_ICON_BACKGROUND_COLOR));
    AppendMenuW(menu, MF_STRING, IDM_ICON_MARQUEE_FONT,
                UiText(UI_TEXT_ICON_FONT));
    AddIconMarqueeSpeedMenu(menu);
}

static void AddStartupMenuItems(HMENU menu)
{
    UINT autoStartChecked = g_settings.autoStart ? MF_CHECKED : 0;

    AppendMenuW(menu, MF_STRING | autoStartChecked,
                IDM_TOGGLE_AUTOSTART, UiText(UI_TEXT_START_WITH_WINDOWS));
}

static void AddHelpMenuItems(HMENU menu)
{
    AppendMenuW(menu, MF_STRING, IDM_VERSION_CHECK,
                UiText(UI_TEXT_VERSION_CHECK));
    AppendMenuW(menu, MF_STRING, IDM_OPEN_LOG_FOLDER,
                UiText(UI_TEXT_OPEN_LOG_FOLDER));
}

static void AppendItemsAsSubMenu(HMENU menu, UiTextId labelId,
                                 void (*addItems)(HMENU))
{
    HMENU subMenu = CreatePopupMenu();

    if (!subMenu) {
        addItems(menu);
        return;
    }

    addItems(subMenu);
    if (!AppendMenuW(menu, MF_POPUP, (UINT_PTR)subMenu, UiText(labelId))) {
        DestroyMenu(subMenu);
    }
}

static void ShowContextMenu(HWND owner, int x, int y)
{
    HMENU menu = CreatePopupMenu();
    int command = 0;

    if (!menu) {
        return;
    }

    if (x == -1 && y == -1) {
        POINT point;
        GetCursorPos(&point);
        x = point.x;
        y = point.y;
    }

    AppendItemsAsSubMenu(menu, UI_TEXT_DISPLAY_MENU, AddDisplayMenuItems);
    AppendItemsAsSubMenu(menu, UI_TEXT_APPEARANCE_MENU, AddAppearanceMenuItems);
    AppendItemsAsSubMenu(menu, UI_TEXT_ICON_SCROLL_MENU,
                         AddIconMarqueeMenuItems);
    AppendItemsAsSubMenu(menu, UI_TEXT_STARTUP_MENU, AddStartupMenuItems);
    AppendItemsAsSubMenu(menu, UI_TEXT_HELP_MENU, AddHelpMenuItems);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, IDM_SAVE, UiText(UI_TEXT_SAVE));
    AppendMenuW(menu, MF_STRING, IDM_EXIT, UiText(UI_TEXT_EXIT));

    SetForegroundWindow(owner);
    command = TrackPopupMenu(menu,
                             TPM_RIGHTBUTTON | TPM_RETURNCMD,
                             x,
                             y,
                             0,
                             owner,
                             NULL);
    DestroyMenu(menu);

    if (command != 0) {
        SendMessageW(owner, WM_COMMAND, MAKEWPARAM(command, 0), 0);
    }

    PostMessageW(owner, WM_NULL, 0, 0);
}

static void ShowTrayMenu(HWND owner)
{
    HMENU menu = CreatePopupMenu();
    POINT point;
    int command = 0;

    if (!menu) {
        return;
    }

    GetCursorPos(&point);
    AppendMenuW(menu, MF_STRING, IDM_SHOW, UiText(UI_TEXT_SHOW));
    AppendItemsAsSubMenu(menu, UI_TEXT_DISPLAY_MENU, AddDisplayMenuItems);
    AppendItemsAsSubMenu(menu, UI_TEXT_APPEARANCE_MENU, AddAppearanceMenuItems);
    AppendItemsAsSubMenu(menu, UI_TEXT_ICON_SCROLL_MENU,
                         AddIconMarqueeMenuItems);
    AppendItemsAsSubMenu(menu, UI_TEXT_STARTUP_MENU, AddStartupMenuItems);
    AppendItemsAsSubMenu(menu, UI_TEXT_HELP_MENU, AddHelpMenuItems);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, IDM_SAVE, UiText(UI_TEXT_SAVE));
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, IDM_EXIT, UiText(UI_TEXT_EXIT));

    SetForegroundWindow(owner);
    command = TrackPopupMenu(menu,
                             TPM_RIGHTBUTTON | TPM_RETURNCMD,
                             point.x,
                             point.y,
                             0,
                             owner,
                             NULL);
    DestroyMenu(menu);

    if (command != 0) {
        SendMessageW(owner, WM_COMMAND, MAKEWPARAM(command, 0), 0);
    }

    PostMessageW(owner, WM_NULL, 0, 0);
}

static ATOM RegisterMainWindowClass(HINSTANCE instance)
{
    WNDCLASSEXW windowClass;

    ZeroMemory(&windowClass, sizeof(windowClass));
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadAppIcon(GetSystemMetrics(SM_CXICON),
                                    GetSystemMetrics(SM_CYICON));
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    windowClass.hIconSm = LoadAppIcon(GetSystemMetrics(SM_CXSMICON),
                                      GetSystemMetrics(SM_CYSMICON));

    return RegisterClassExW(&windowClass);
}

static void HandleCommand(HWND hwnd, int commandId)
{
    LogInfo(L"Command received. id=%d", commandId);
    switch (commandId) {
    case IDM_TOPMOST_ON:
        ApplyDisplayMode(hwnd, DISPLAY_MODE_TOPMOST);
        SaveCurrentState(hwnd);
        break;
    case IDM_TOPMOST_OFF:
    case IDM_NORMAL_MODE:
        ApplyDisplayMode(hwnd, DISPLAY_MODE_NORMAL);
        SaveCurrentState(hwnd);
        break;
    case IDM_DESKTOP_MODE:
        ApplyDisplayMode(hwnd, DISPLAY_MODE_DESKTOP);
        SaveCurrentState(hwnd);
        break;
    case IDM_SAVE:
        SaveCurrentStateWithNotification(hwnd);
        break;
    case IDM_EXIT:
        ExitApplication(hwnd);
        break;
    case IDM_SHOW:
        ShowMainWindow(hwnd);
        break;
    case IDM_TOGGLE_TOPMOST:
        ApplyDisplayMode(hwnd,
                         g_settings.displayMode == DISPLAY_MODE_TOPMOST
                             ? DISPLAY_MODE_NORMAL
                             : DISPLAY_MODE_TOPMOST);
        SaveCurrentState(hwnd);
        break;
    case IDM_FONT:
        if (ChooseEditorFont(hwnd)) {
            SaveCurrentState(hwnd);
        }
        break;
    case IDM_FONT_SIZE_6:
    case IDM_FONT_SIZE_7:
    case IDM_FONT_SIZE_8:
    case IDM_FONT_SIZE_9:
    case IDM_FONT_SIZE_10:
    case IDM_FONT_SIZE_12:
    case IDM_FONT_SIZE_14:
    case IDM_FONT_SIZE_16:
    case IDM_FONT_SIZE_18:
    case IDM_FONT_SIZE_24:
        SetEditorFontPointSize(GetFontSizeFromCommand(commandId));
        SaveCurrentState(hwnd);
        break;
    case IDM_EDITOR_BACKGROUND_COLOR:
        if (ChooseEditorBackgroundColor(hwnd)) {
            SaveCurrentState(hwnd);
        }
        break;
    case IDM_BORDER_COLOR:
        if (ChooseWindowBorderColor(hwnd)) {
            SaveCurrentState(hwnd);
        }
        break;
    case IDM_TOGGLE_TITLE_HOVER:
        g_settings.titleHoverOnly = !g_settings.titleHoverOnly;
        g_titleHovered = !g_settings.titleHoverOnly || IsCursorOverWindow(hwnd);
        ApplyTitleBarAppearance(hwnd);
        SaveCurrentState(hwnd);
        break;
    case IDM_TOGGLE_ICON_MARQUEE:
        g_settings.iconMarquee = !g_settings.iconMarquee;
        if (g_settings.iconMarquee) {
            StartIconMarquee(hwnd);
        } else {
            StopIconMarquee(hwnd);
        }
        SaveCurrentState(hwnd);
        break;
    case IDM_TOGGLE_ICON_MARQUEE_REVERSE:
        g_settings.iconMarqueeReverse = !g_settings.iconMarqueeReverse;
        g_marqueeOffset = 0;
        if (g_settings.iconMarquee) {
            UpdateIconMarquee(hwnd);
        }
        SaveCurrentState(hwnd);
        break;
    case IDM_ICON_MARQUEE_TEXT_COLOR:
        if (ChooseIconMarqueeTextColor(hwnd)) {
            SaveCurrentState(hwnd);
        }
        break;
    case IDM_ICON_MARQUEE_BACKGROUND_COLOR:
        if (ChooseIconMarqueeBackgroundColor(hwnd)) {
            SaveCurrentState(hwnd);
        }
        break;
    case IDM_ICON_MARQUEE_FONT:
        if (ChooseIconMarqueeFont(hwnd)) {
            SaveCurrentState(hwnd);
        }
        break;
    case IDM_ICON_MARQUEE_SPEED_SLOW:
    case IDM_ICON_MARQUEE_SPEED_NORMAL:
    case IDM_ICON_MARQUEE_SPEED_FAST:
    case IDM_ICON_MARQUEE_SPEED_FASTEST:
        switch (commandId) {
        case IDM_ICON_MARQUEE_SPEED_SLOW:
            g_settings.iconMarqueeSpeed = 1;
            break;
        case IDM_ICON_MARQUEE_SPEED_NORMAL:
            g_settings.iconMarqueeSpeed = 3;
            break;
        case IDM_ICON_MARQUEE_SPEED_FAST:
            g_settings.iconMarqueeSpeed = 5;
            break;
        case IDM_ICON_MARQUEE_SPEED_FASTEST:
            g_settings.iconMarqueeSpeed = 8;
            break;
        default:
            break;
        }
        if (g_settings.iconMarquee) {
            UpdateIconMarquee(hwnd);
        }
        SaveCurrentState(hwnd);
        break;
    case IDM_TOGGLE_AUTOSTART:
    {
        BOOL enableAutoStart = !g_settings.autoStart;
        if (SetAutoStartEnabled(enableAutoStart)) {
            g_settings.autoStart = enableAutoStart;
            SaveCurrentState(hwnd);
        } else {
            MessageBoxW(hwnd,
                        UiText(UI_TEXT_STARTUP_SETTING_ERROR),
                        APP_TITLE,
                        MB_ICONERROR);
        }
        break;
    }
    case IDM_VERSION_CHECK:
        StartUpdateCheck(hwnd, TRUE);
        break;
    case IDM_OPEN_LOG_FOLDER:
        if (!LogOpenFolder(hwnd)) {
            LogWarn(L"Failed to open log folder from menu.");
            MessageBoxW(hwnd,
                        UiText(UI_TEXT_OPEN_LOG_FOLDER_ERROR),
                        APP_TITLE,
                        MB_OK | MB_ICONERROR);
        }
        break;
    default:
        break;
    }
}

static LRESULT CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL refreshAfter = FALSE;
    LRESULT result = 0;

    switch (message) {
    case WM_CHAR:
        if (wParam == 1) {
            SendMessageW(hwnd, EM_SETSEL, 0, -1);
            return 0;
        }
        if (wParam == 19) {
            SaveCurrentStateWithNotification(g_mainWindow);
            return 0;
        }
        refreshAfter = TRUE;
        break;
    case WM_KEYUP:
    case WM_LBUTTONUP:
    case WM_PASTE:
    case WM_CUT:
    case WM_CLEAR:
    case WM_UNDO:
    case WM_SETTEXT:
        refreshAfter = TRUE;
        break;
    case WM_MOUSEWHEEL:
        if (g_editorScrollBarVisible) {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            ScrollEditorToFirstVisibleLine(
                g_mainWindow,
                GetEditorFirstVisibleLine() - (delta / WHEEL_DELTA) * 3);
            return 0;
        }
        refreshAfter = TRUE;
        break;
    case WM_MOUSEMOVE:
        TrackMouseLeave(hwnd, FALSE, &g_editMouseTracking);
        SetTitleHoverState(g_mainWindow, TRUE);
        break;
    case WM_MOUSELEAVE:
        g_editMouseTracking = FALSE;
        if (!IsCursorOverWindow(g_mainWindow)) {
            SetTitleHoverState(g_mainWindow, FALSE);
        }
        break;
    case WM_CONTEXTMENU:
        ShowContextMenu(g_mainWindow, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    default:
        break;
    }

    if (g_originalEditProc) {
        result = CallWindowProcW(g_originalEditProc, hwnd, message,
                                 wParam, lParam);
    } else {
        result = DefWindowProcW(hwnd, message, wParam, lParam);
    }

    if (refreshAfter) {
        RefreshEditorScrollBar(g_mainWindow, TRUE);
    }

    return result;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (g_taskbarCreatedMessage != 0 && message == g_taskbarCreatedMessage) {
        if (g_settings.iconMarquee) {
            UpdateIconMarquee(hwnd);
        } else {
            AddTrayIcon(hwnd);
        }
        return 0;
    }

    if (message == WMAPP_UPDATE_AVAILABLE) {
        WCHAR updateMessage[512];
        BOOL manual = wParam != 0;

        if (!manual) {
            StartUpdateDownload(hwnd, FALSE);
            return 0;
        }

        if (FAILED(StringCchPrintfW(updateMessage,
                                    ARRAYSIZE(updateMessage),
                                    UiText(UI_TEXT_UPDATE_AVAILABLE_PROMPT),
                                    APP_VERSION_WIDE,
                                    g_availableUpdateVersion))) {
            StringCchCopyW(updateMessage, ARRAYSIZE(updateMessage),
                           UiText(UI_TEXT_UPDATE_DOWNLOAD_FAILED));
        }
        if (MessageBoxW(hwnd,
                        updateMessage,
                        APP_TITLE,
                        MB_YESNO | MB_ICONQUESTION) == IDYES) {
            StartUpdateDownload(hwnd, TRUE);
        }
        return 0;
    }

    if (message == WMAPP_UPDATE_NOT_AVAILABLE) {
        WCHAR updateMessage[512];
        if (FAILED(StringCchPrintfW(updateMessage,
                                    ARRAYSIZE(updateMessage),
                                    UiText(UI_TEXT_UPDATE_NOT_AVAILABLE),
                                    APP_VERSION_WIDE,
                                    g_availableUpdateVersion))) {
            StringCchCopyW(updateMessage, ARRAYSIZE(updateMessage),
                           APP_VERSION_WIDE);
        }
        MessageBoxW(hwnd, updateMessage, APP_TITLE,
                    MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (message == WMAPP_UPDATE_CHECK_FAILED) {
        WCHAR updateMessage[512];
        if (FAILED(StringCchPrintfW(updateMessage,
                                    ARRAYSIZE(updateMessage),
                                    UiText(UI_TEXT_UPDATE_CHECK_FAILED),
                                    APP_VERSION_WIDE))) {
            StringCchCopyW(updateMessage, ARRAYSIZE(updateMessage),
                           UiText(UI_TEXT_UPDATE_CHECK_FAILED));
        }
        MessageBoxW(hwnd, updateMessage, APP_TITLE, MB_OK | MB_ICONERROR);
        return 0;
    }

    if (message == WMAPP_UPDATE_READY) {
        if (wParam != 0) {
            MessageBoxW(hwnd, UiText(UI_TEXT_UPDATE_RESTARTING), APP_TITLE,
                        MB_OK | MB_ICONINFORMATION);
        }
        if (!LaunchUpdaterAndExit(hwnd)) {
            if (wParam != 0) {
                MessageBoxW(hwnd, UiText(UI_TEXT_UPDATE_DOWNLOAD_FAILED),
                            APP_TITLE, MB_OK | MB_ICONERROR);
            }
        }
        return 0;
    }

    if (message == WMAPP_UPDATE_DOWNLOAD_FAILED) {
        if (wParam != 0) {
            MessageBoxW(hwnd, UiText(UI_TEXT_UPDATE_DOWNLOAD_FAILED),
                        APP_TITLE, MB_OK | MB_ICONERROR);
        }
        return 0;
    }

    switch (message) {
    case WM_NCCALCSIZE:
        return 0;

    case WM_NCPAINT:
        return 0;

    case WM_NCACTIVATE:
        return TRUE;

    case WM_NCHITTEST:
        return HitTestCustomWindow(hwnd, lParam);

    case WM_CREATE:
        g_mainWindow = hwnd;
        if (!CreateEditControl(hwnd)) {
            LogError(L"Failed to create edit control.");
            return -1;
        }
        AddTrayIcon(hwnd);
        ApplyDisplayMode(hwnd, g_settings.displayMode);
        ApplyWindowBorderColor(hwnd);
        g_titleHovered = !g_settings.titleHoverOnly || IsCursorOverWindow(hwnd);
        ApplyTitleBarAppearance(hwnd);
        if (g_settings.iconMarquee) {
            StartIconMarquee(hwnd);
        }
        SetTimer(hwnd, IDT_LOG_HEARTBEAT, LOG_HEARTBEAT_MS, NULL);
        LogInfo(L"Main window created.");
        StartUpdateCheck(hwnd, FALSE);
        return 0;

    case WM_SIZE:
        ResizeEditControl(hwnd);
        InvalidateTitleBar(hwnd);
        return 0;

    case WM_MOUSEMOVE:
        if (g_editorScrollBarDragging) {
            UpdateEditorScrollBarDrag(hwnd, GET_Y_LPARAM(lParam));
            return 0;
        }
        TrackMouseLeave(hwnd, FALSE, &g_clientMouseTracking);
        SetTitleHoverState(hwnd, TRUE);
        UpdateTitleButtonHover(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSELEAVE:
        g_clientMouseTracking = FALSE;
        if (g_hoverTitleButton != TITLE_BUTTON_NONE) {
            g_hoverTitleButton = TITLE_BUTTON_NONE;
            InvalidateTitleBar(hwnd);
        }
        if (!IsCursorOverWindow(hwnd)) {
            SetTitleHoverState(hwnd, FALSE);
        }
        return 0;

    case WM_NCMOUSEMOVE:
        TrackMouseLeave(hwnd, TRUE, &g_ncMouseTracking);
        if (g_hoverTitleButton != TITLE_BUTTON_NONE) {
            g_hoverTitleButton = TITLE_BUTTON_NONE;
            InvalidateTitleBar(hwnd);
        }
        SetTitleHoverState(hwnd, TRUE);
        return DefWindowProcW(hwnd, message, wParam, lParam);

    case WM_NCMOUSELEAVE:
        g_ncMouseTracking = FALSE;
        if (g_hoverTitleButton != TITLE_BUTTON_NONE) {
            g_hoverTitleButton = TITLE_BUTTON_NONE;
            InvalidateTitleBar(hwnd);
        }
        if (!IsCursorOverWindow(hwnd)) {
            SetTitleHoverState(hwnd, FALSE);
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);

    case WM_LBUTTONDOWN:
    {
        TitleButton button = HitTestTitleButton(hwnd,
                                                GET_X_LPARAM(lParam),
                                                GET_Y_LPARAM(lParam));
        if (button != TITLE_BUTTON_NONE) {
            g_pressedTitleButton = button;
            SetCapture(hwnd);
            InvalidateTitleBar(hwnd);
            return 0;
        }
        if (BeginEditorScrollBarDrag(hwnd,
                                     GET_X_LPARAM(lParam),
                                     GET_Y_LPARAM(lParam))) {
            return 0;
        }
        if (IsPointInEditorScrollBar(hwnd,
                                     GET_X_LPARAM(lParam),
                                     GET_Y_LPARAM(lParam))) {
            PageEditorFromScrollBarClick(hwnd, GET_Y_LPARAM(lParam));
            return 0;
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    case WM_LBUTTONUP:
        if (g_editorScrollBarDragging) {
            EndEditorScrollBarDrag(hwnd);
            return 0;
        }
        if (g_pressedTitleButton != TITLE_BUTTON_NONE) {
            TitleButton pressedButton = g_pressedTitleButton;
            TitleButton releasedButton = HitTestTitleButton(hwnd,
                                                            GET_X_LPARAM(lParam),
                                                            GET_Y_LPARAM(lParam));
            g_pressedTitleButton = TITLE_BUTTON_NONE;
            if (GetCapture() == hwnd) {
                ReleaseCapture();
            }
            InvalidateTitleBar(hwnd);
            if (pressedButton == releasedButton) {
                PerformTitleButtonAction(hwnd, pressedButton);
            }
            return 0;
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);

    case WM_CAPTURECHANGED:
        if (g_editorScrollBarDragging) {
            g_editorScrollBarDragging = FALSE;
            RefreshEditorScrollBar(hwnd, FALSE);
        }
        if (g_pressedTitleButton != TITLE_BUTTON_NONE) {
            g_pressedTitleButton = TITLE_BUTTON_NONE;
            InvalidateTitleBar(hwnd);
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);

    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int firstLine = GetEditorFirstVisibleLine();
        int scrollLines = 3;

        if (g_editorScrollBarVisible) {
            ScrollEditorToFirstVisibleLine(
                hwnd,
                firstLine - (delta / WHEEL_DELTA) * scrollLines);
            return 0;
        }
        break;
    }

    case WM_NCRBUTTONUP:
        if (wParam == HTCAPTION) {
            ShowContextMenu(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);

    case WM_DWMCOMPOSITIONCHANGED:
    case WM_SETTINGCHANGE:
        ApplyWindowBorderColor(hwnd);
        ApplyTitleBarAppearance(hwnd);
        return 0;

    case WM_TIMER:
        if (wParam == IDT_ICON_MARQUEE) {
            UpdateIconMarquee(hwnd);
            return 0;
        }
        if (wParam == IDT_SAVE_NOTIFICATION) {
            FinishSaveNotification(hwnd);
            return 0;
        }
        if (wParam == IDT_LOG_HEARTBEAT) {
            LogHeartbeat();
            return 0;
        }
        break;

    case WM_CTLCOLOREDIT:
        if ((HWND)lParam == g_editWindow) {
            HDC editDc = (HDC)wParam;
            SetBkColor(editDc, g_settings.editorBackgroundColor);
            SetTextColor(editDc,
                         GetReadableTitleTextColor(
                             g_settings.editorBackgroundColor));
            return (LRESULT)GetEditorBackgroundBrush();
        }
        break;

    case WM_ERASEBKGND:
    {
        RECT clientRect;
        HBRUSH brush = CreateSolidBrush(g_settings.borderColor);
        if (!brush) {
            return DefWindowProcW(hwnd, message, wParam, lParam);
        }
        GetClientRect(hwnd, &clientRect);
        FillRect((HDC)wParam, &clientRect, brush);
        DeleteObject(brush);
        return 1;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC hdc = BeginPaint(hwnd, &paint);
        if (hdc) {
            DrawCustomTitleBar(hwnd, hdc);
            DrawEditorScrollBar(hwnd, hdc);
        }
        EndPaint(hwnd, &paint);
        return 0;
    }

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO *minMaxInfo = (MINMAXINFO *)lParam;
        minMaxInfo->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
        minMaxInfo->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
        return 0;
    }

    case WM_COMMAND:
        HandleCommand(hwnd, LOWORD(wParam));
        return 0;

    case WM_CONTEXTMENU:
        ShowContextMenu(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WMAPP_TRAYICON:
        if (wParam != TRAY_ICON_UID) {
            return 0;
        }
        switch (lParam) {
        case WM_LBUTTONDBLCLK:
            ShowMainWindow(hwnd);
            return 0;
        case WM_RBUTTONUP:
        case WM_CONTEXTMENU:
            ShowTrayMenu(hwnd);
            return 0;
        default:
            return 0;
        }

    case WM_CLOSE:
        if (g_isExiting) {
            DestroyWindow(hwnd);
        } else {
            HideMainWindow(hwnd);
        }
        return 0;

    case WM_QUERYENDSESSION:
        LogInfo(L"WM_QUERYENDSESSION received.");
        SaveCurrentState(hwnd);
        return TRUE;

    case WM_DESTROY:
        KillTimer(hwnd, IDT_ICON_MARQUEE);
        KillTimer(hwnd, IDT_SAVE_NOTIFICATION);
        KillTimer(hwnd, IDT_LOG_HEARTBEAT);
        RemoveTrayIcon(hwnd);
        DestroyMarqueeIcons();
        if (g_editFont) {
            DeleteObject(g_editFont);
            g_editFont = NULL;
        }
        if (g_editBackgroundBrush) {
            DeleteObject(g_editBackgroundBrush);
            g_editBackgroundBrush = NULL;
        }
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

int WINAPI wWinMain(HINSTANCE instance,
                    HINSTANCE previousInstance,
                    PWSTR commandLine,
                    int commandShow)
{
    HWND hwnd;
    MSG message;
    DWORD windowStyle = WS_POPUP | WS_THICKFRAME |
                        WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
                        WS_CLIPCHILDREN;

    UNREFERENCED_PARAMETER(previousInstance);
    UNREFERENCED_PARAMETER(commandLine);

    LogInitialize();
    SetUnhandledExceptionFilter(LogUnhandledExceptionFilter);
    LogInfo(L"Application starting. version=%s", APP_VERSION_WIDE);

    g_instance = instance;
    InitUiLanguage();
    g_taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");

    if (LoadSettings(&g_settings)) {
        LogInfo(L"Settings loaded.");
    } else {
        LogWarn(L"Settings could not be loaded. Defaults are used.");
    }
    g_settings.autoStart = IsAutoStartEnabled();

    if (!RegisterMainWindowClass(instance)) {
        LogLastError(L"Register main window class");
        MessageBoxW(NULL,
                    UiText(UI_TEXT_REGISTER_CLASS_ERROR),
                    APP_TITLE,
                    MB_ICONERROR);
        LogShutdown();
        return 1;
    }

    hwnd = CreateWindowExW(WS_EX_APPWINDOW,
                           WINDOW_CLASS_NAME,
                           APP_TITLE,
                           windowStyle,
                           g_settings.x,
                           g_settings.y,
                           g_settings.width,
                           g_settings.height,
                           NULL,
                           NULL,
                           instance,
                           NULL);

    if (!hwnd) {
        LogLastError(L"Create main window");
        MessageBoxW(NULL,
                    UiText(UI_TEXT_CREATE_WINDOW_ERROR),
                    APP_TITLE,
                    MB_ICONERROR);
        LogShutdown();
        return 1;
    }

    ShowWindow(hwnd, commandShow == SW_HIDE ? SW_SHOWNORMAL : commandShow);
    UpdateWindow(hwnd);

    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    LogInfo(L"Application exiting. code=%d", (int)message.wParam);
    LogShutdown();
    return (int)message.wParam;
}
