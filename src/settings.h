#ifndef SETTINGS_H
#define SETTINGS_H

#include <windows.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(value) (sizeof(value) / sizeof((value)[0]))
#endif

#define MEMO_TEXT_MAX_CHARS 65536
#define DEFAULT_WINDOW_X 100
#define DEFAULT_WINDOW_Y 100
#define DEFAULT_WINDOW_WIDTH 360
#define DEFAULT_WINDOW_HEIGHT 260
#define MIN_WINDOW_WIDTH 220
#define MIN_WINDOW_HEIGHT 140
#define DEFAULT_FONT_FACE L"Meiryo"
#define DEFAULT_FONT_POINT_SIZE 10
#define MIN_FONT_POINT_SIZE 6
#define MAX_FONT_POINT_SIZE 72
#define DEFAULT_EDITOR_BACKGROUND_COLOR RGB(255, 255, 255)
#define DEFAULT_BORDER_COLOR RGB(80, 120, 200)
#define DEFAULT_TITLE_HOVER_ONLY TRUE
#define DEFAULT_ICON_MARQUEE FALSE
#define DEFAULT_ICON_MARQUEE_REVERSE FALSE
#define DEFAULT_ICON_MARQUEE_TEXT_COLOR RGB(0, 0, 0)
#define DEFAULT_ICON_MARQUEE_BACKGROUND_COLOR RGB(255, 255, 255)
#define DEFAULT_ICON_MARQUEE_FONT_FACE L"Meiryo"
#define DEFAULT_ICON_MARQUEE_SPEED 3
#define MIN_ICON_MARQUEE_SPEED 1
#define MAX_ICON_MARQUEE_SPEED 8

typedef enum DisplayMode {
    DISPLAY_MODE_NORMAL = 0,
    DISPLAY_MODE_TOPMOST = 1,
    DISPLAY_MODE_DESKTOP = 2
} DisplayMode;

typedef struct AppSettings {
    WCHAR memoText[MEMO_TEXT_MAX_CHARS];
    int x;
    int y;
    int width;
    int height;
    DisplayMode displayMode;
    BOOL topmost;
    WCHAR fontFace[LF_FACESIZE];
    int fontPointSize;
    int fontWeight;
    BOOL fontItalic;
    COLORREF editorBackgroundColor;
    COLORREF borderColor;
    BOOL titleHoverOnly;
    BOOL iconMarquee;
    BOOL iconMarqueeReverse;
    COLORREF iconMarqueeTextColor;
    COLORREF iconMarqueeBackgroundColor;
    WCHAR iconMarqueeFontFace[LF_FACESIZE];
    int iconMarqueeFontWeight;
    BOOL iconMarqueeFontItalic;
    int iconMarqueeSpeed;
    BOOL autoStart;
} AppSettings;

void InitDefaultSettings(AppSettings *settings);
BOOL LoadSettings(AppSettings *settings);
BOOL SaveSettings(const AppSettings *settings);
BOOL GetSettingsDirectoryPath(WCHAR *buffer, DWORD cchBuffer);
BOOL GetSettingsFilePath(WCHAR *buffer, DWORD cchBuffer);

#endif
