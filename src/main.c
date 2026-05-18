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
#include <strsafe.h>

#include "resource.h"
#include "settings.h"

#define APP_TITLE L"cMemo"
#define WINDOW_CLASS_NAME L"cMemoWindow"

#define ID_EDIT 1001

#define BORDER_THICKNESS 4
#define CUSTOM_TITLE_BAR_HEIGHT 32
#define TITLE_BUTTON_WIDTH 46
#define TITLE_BUTTON_GLYPH_SIZE 10
#define TITLE_ICON_SIZE 16
#define TITLE_TEXT_LEFT_PADDING 10
#define RESIZE_HIT_THICKNESS 8
#define IDT_ICON_MARQUEE 50001
#define IDT_SAVE_NOTIFICATION 50002
#define ICON_MARQUEE_INTERVAL_MS 160
#define SAVE_NOTIFICATION_MS 1500
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

#define TRAY_ICON_UID 1
#define WMAPP_TRAYICON (WM_APP + 1)
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
static HICON g_marqueeSmallIcon = NULL;
static HICON g_marqueeBigIcon = NULL;
static int g_marqueeOffset = 0;
static UINT g_taskbarCreatedMessage = 0;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL AddTrayIcon(HWND hwnd);
static void UpdateIconMarquee(HWND hwnd);
static void StartIconMarquee(HWND hwnd);
static void StopIconMarquee(HWND hwnd);

typedef enum UiTextId {
    UI_TEXT_ALWAYS_ON_TOP = 0,
    UI_TEXT_REMOVE_ALWAYS_ON_TOP,
    UI_TEXT_DESKTOP_MODE,
    UI_TEXT_NORMAL_DISPLAY,
    UI_TEXT_DISPLAY_MENU,
    UI_TEXT_APPEARANCE_MENU,
    UI_TEXT_FONT,
    UI_TEXT_EDITOR_BACKGROUND_COLOR,
    UI_TEXT_WINDOW_COLOR,
    UI_TEXT_SHOW_TITLE_ON_HOVER_ONLY,
    UI_TEXT_ICON_SCROLL_MENU,
    UI_TEXT_SCROLL_MEMO_ON_ICON,
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
            return FALSE;
        }

        result = RegCreateKeyExW(HKEY_CURRENT_USER, RUN_KEY_PATH, 0, NULL,
                                 REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
                                 NULL, &key, NULL);
        if (result != ERROR_SUCCESS) {
            return FALSE;
        }

        result = RegSetValueExW(key, RUN_VALUE_NAME, 0, REG_SZ,
                                (const BYTE *)command,
                                (DWORD)((lstrlenW(command) + 1) *
                                        sizeof(WCHAR)));
        RegCloseKey(key);
        return result == ERROR_SUCCESS;
    }

    result = RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY_PATH, 0,
                           KEY_SET_VALUE, &key);
    if (result == ERROR_FILE_NOT_FOUND) {
        return TRUE;
    }
    if (result != ERROR_SUCCESS) {
        return FALSE;
    }

    result = RegDeleteValueW(key, RUN_VALUE_NAME);
    RegCloseKey(key);
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

    if (!g_editWindow) {
        return;
    }

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

    MoveWindow(g_editWindow, BORDER_THICKNESS,
               CUSTOM_TITLE_BAR_HEIGHT + BORDER_THICKNESS,
               width,
               height,
               TRUE);
}

static BOOL CreateEditControl(HWND hwnd)
{
    g_editWindow = CreateWindowExW(
        0,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
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

    return SaveSettings(&g_settings);
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
    x = width - (g_marqueeOffset % totalWidth);
    y = max(0, (height - textSize.cy) / 2);

    SetBkMode(memoryDc, TRANSPARENT);
    SetTextColor(memoryDc, g_settings.iconMarqueeTextColor);
    IntersectClipRect(memoryDc, 0, 0, width, height);
    TextOutW(memoryDc, x, y, text, textLength);
    if (x + textSize.cx < width) {
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
    SaveCurrentState(hwnd);
    ShowWindow(hwnd, SW_HIDE);
}

static void ExitApplication(HWND hwnd)
{
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

static void AddAppearanceMenuItems(HMENU menu)
{
    UINT titleHoverChecked = g_settings.titleHoverOnly ? MF_CHECKED : 0;

    AppendMenuW(menu, MF_STRING, IDM_FONT, UiText(UI_TEXT_FONT));
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

    AppendMenuW(menu, MF_STRING | iconMarqueeChecked,
                IDM_TOGGLE_ICON_MARQUEE,
                UiText(UI_TEXT_SCROLL_MEMO_ON_ICON));
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
    default:
        break;
    }
}

static LRESULT CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
        return CallWindowProcW(g_originalEditProc, hwnd, message, wParam, lParam);
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
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
        return 0;

    case WM_SIZE:
        ResizeEditControl(hwnd);
        InvalidateTitleBar(hwnd);
        return 0;

    case WM_MOUSEMOVE:
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
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    case WM_LBUTTONUP:
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
        if (g_pressedTitleButton != TITLE_BUTTON_NONE) {
            g_pressedTitleButton = TITLE_BUTTON_NONE;
            InvalidateTitleBar(hwnd);
        }
        return DefWindowProcW(hwnd, message, wParam, lParam);

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
        SaveCurrentState(hwnd);
        return TRUE;

    case WM_DESTROY:
        KillTimer(hwnd, IDT_ICON_MARQUEE);
        KillTimer(hwnd, IDT_SAVE_NOTIFICATION);
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

    g_instance = instance;
    InitUiLanguage();
    g_taskbarCreatedMessage = RegisterWindowMessageW(L"TaskbarCreated");

    LoadSettings(&g_settings);
    g_settings.autoStart = IsAutoStartEnabled();

    if (!RegisterMainWindowClass(instance)) {
        MessageBoxW(NULL,
                    UiText(UI_TEXT_REGISTER_CLASS_ERROR),
                    APP_TITLE,
                    MB_ICONERROR);
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
        MessageBoxW(NULL,
                    UiText(UI_TEXT_CREATE_WINDOW_ERROR),
                    APP_TITLE,
                    MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, commandShow == SW_HIDE ? SW_SHOWNORMAL : commandShow);
    UpdateWindow(hwnd);

    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return (int)message.wParam;
}
