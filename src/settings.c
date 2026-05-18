#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>

#include "settings.h"

#define APP_DIRECTORY_NAME L"cMemo"
#define SETTINGS_FILE_NAME L"settings.ini"

#define SECTION_MEMO L"Memo"
#define KEY_MEMO_TEXT L"Text"

#define SECTION_WINDOW L"Window"
#define KEY_WINDOW_X L"X"
#define KEY_WINDOW_Y L"Y"
#define KEY_WINDOW_WIDTH L"Width"
#define KEY_WINDOW_HEIGHT L"Height"

#define SECTION_VIEW L"View"
#define KEY_DISPLAY_MODE L"DisplayMode"
#define KEY_TOPMOST L"TopMost"

#define SECTION_APPEARANCE L"Appearance"
#define KEY_FONT_FACE L"FontFace"
#define KEY_FONT_POINT_SIZE L"FontPointSize"
#define KEY_FONT_WEIGHT L"FontWeight"
#define KEY_FONT_ITALIC L"FontItalic"
#define KEY_BORDER_COLOR L"BorderColor"
#define KEY_TITLE_HOVER_ONLY L"TitleHoverOnly"
#define KEY_ICON_MARQUEE L"IconMarquee"
#define KEY_ICON_MARQUEE_TEXT_COLOR L"IconMarqueeTextColor"
#define KEY_ICON_MARQUEE_BACKGROUND_COLOR L"IconMarqueeBackgroundColor"
#define KEY_ICON_MARQUEE_FONT_FACE L"IconMarqueeFontFace"
#define KEY_ICON_MARQUEE_FONT_WEIGHT L"IconMarqueeFontWeight"
#define KEY_ICON_MARQUEE_FONT_ITALIC L"IconMarqueeFontItalic"
#define KEY_ICON_MARQUEE_SPEED L"IconMarqueeSpeed"

#define SECTION_STARTUP L"Startup"
#define KEY_AUTO_START L"AutoStart"

static BOOL AppendPathComponent(WCHAR *buffer, DWORD cchBuffer, LPCWSTR component)
{
    size_t bufferLength = 0;
    size_t componentLength = 0;

    if (!buffer || !component || cchBuffer == 0) {
        return FALSE;
    }

    bufferLength = lstrlenW(buffer);
    componentLength = lstrlenW(component);

    if (bufferLength > 0 &&
        buffer[bufferLength - 1] != L'\\' &&
        buffer[bufferLength - 1] != L'/') {
        if (bufferLength + 1 >= cchBuffer) {
            return FALSE;
        }
        buffer[bufferLength++] = L'\\';
        buffer[bufferLength] = L'\0';
    }

    if (bufferLength + componentLength >= cchBuffer) {
        return FALSE;
    }

    return SUCCEEDED(StringCchCatW(buffer, cchBuffer, component));
}

BOOL GetSettingsDirectoryPath(WCHAR *buffer, DWORD cchBuffer)
{
    DWORD result = 0;

    if (!buffer || cchBuffer == 0) {
        return FALSE;
    }

    result = GetEnvironmentVariableW(L"APPDATA", buffer, cchBuffer);
    if (result == 0 || result >= cchBuffer) {
        return FALSE;
    }

    return AppendPathComponent(buffer, cchBuffer, APP_DIRECTORY_NAME);
}

BOOL GetSettingsFilePath(WCHAR *buffer, DWORD cchBuffer)
{
    if (!GetSettingsDirectoryPath(buffer, cchBuffer)) {
        return FALSE;
    }

    return AppendPathComponent(buffer, cchBuffer, SETTINGS_FILE_NAME);
}

static BOOL EnsureSettingsDirectory(void)
{
    WCHAR directory[MAX_PATH];

    if (!GetSettingsDirectoryPath(directory, ARRAYSIZE(directory))) {
        return FALSE;
    }

    if (CreateDirectoryW(directory, NULL)) {
        return TRUE;
    }

    return GetLastError() == ERROR_ALREADY_EXISTS;
}

void InitDefaultSettings(AppSettings *settings)
{
    if (!settings) {
        return;
    }

    ZeroMemory(settings, sizeof(*settings));
    settings->x = DEFAULT_WINDOW_X;
    settings->y = DEFAULT_WINDOW_Y;
    settings->width = DEFAULT_WINDOW_WIDTH;
    settings->height = DEFAULT_WINDOW_HEIGHT;
    settings->displayMode = DISPLAY_MODE_NORMAL;
    settings->topmost = FALSE;
    StringCchCopyW(settings->fontFace, ARRAYSIZE(settings->fontFace),
                   DEFAULT_FONT_FACE);
    settings->fontPointSize = DEFAULT_FONT_POINT_SIZE;
    settings->fontWeight = FW_NORMAL;
    settings->fontItalic = FALSE;
    settings->borderColor = DEFAULT_BORDER_COLOR;
    settings->titleHoverOnly = DEFAULT_TITLE_HOVER_ONLY;
    settings->iconMarquee = DEFAULT_ICON_MARQUEE;
    settings->iconMarqueeTextColor = DEFAULT_ICON_MARQUEE_TEXT_COLOR;
    settings->iconMarqueeBackgroundColor =
        DEFAULT_ICON_MARQUEE_BACKGROUND_COLOR;
    StringCchCopyW(settings->iconMarqueeFontFace,
                   ARRAYSIZE(settings->iconMarqueeFontFace),
                   DEFAULT_ICON_MARQUEE_FONT_FACE);
    settings->iconMarqueeFontWeight = FW_BOLD;
    settings->iconMarqueeFontItalic = FALSE;
    settings->iconMarqueeSpeed = DEFAULT_ICON_MARQUEE_SPEED;
    settings->autoStart = FALSE;
}

static int ClampMinimum(int value, int minimum, int fallback)
{
    if (value < minimum) {
        return fallback;
    }
    return value;
}

static int ClampRange(int value, int minimum, int maximum, int fallback)
{
    if (value < minimum || value > maximum) {
        return fallback;
    }
    return value;
}

static DisplayMode NormalizeDisplayMode(int value)
{
    switch (value) {
    case DISPLAY_MODE_NORMAL:
    case DISPLAY_MODE_TOPMOST:
    case DISPLAY_MODE_DESKTOP:
        return (DisplayMode)value;
    default:
        return DISPLAY_MODE_NORMAL;
    }
}

static BOOL EncodeMemoText(LPCWSTR source, WCHAR **encoded)
{
    size_t sourceLength = 0;
    size_t encodedCapacity = 0;
    size_t i = 0;
    size_t j = 0;
    WCHAR *buffer = NULL;

    if (!source || !encoded) {
        return FALSE;
    }

    *encoded = NULL;
    sourceLength = lstrlenW(source);

    encodedCapacity = (sourceLength * 2) + 1;
    buffer = (WCHAR *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                encodedCapacity * sizeof(WCHAR));
    if (!buffer) {
        return FALSE;
    }

    for (i = 0; i < sourceLength && j + 1 < encodedCapacity; ++i) {
        WCHAR ch = source[i];
        switch (ch) {
        case L'\\':
            buffer[j++] = L'\\';
            buffer[j++] = L'\\';
            break;
        case L'\r':
            buffer[j++] = L'\\';
            buffer[j++] = L'r';
            break;
        case L'\n':
            buffer[j++] = L'\\';
            buffer[j++] = L'n';
            break;
        case L'\t':
            buffer[j++] = L'\\';
            buffer[j++] = L't';
            break;
        default:
            buffer[j++] = ch;
            break;
        }
    }

    buffer[j] = L'\0';
    *encoded = buffer;
    return TRUE;
}

static void DecodeMemoText(LPCWSTR source, WCHAR *destination, size_t cchDestination)
{
    size_t i = 0;
    size_t j = 0;

    if (!destination || cchDestination == 0) {
        return;
    }

    destination[0] = L'\0';
    if (!source) {
        return;
    }

    while (source[i] != L'\0' && j + 1 < cchDestination) {
        if (source[i] == L'\\' && source[i + 1] != L'\0') {
            WCHAR escaped = source[i + 1];
            switch (escaped) {
            case L'\\':
                destination[j++] = L'\\';
                i += 2;
                continue;
            case L'r':
                destination[j++] = L'\r';
                i += 2;
                continue;
            case L'n':
                destination[j++] = L'\n';
                i += 2;
                continue;
            case L't':
                destination[j++] = L'\t';
                i += 2;
                continue;
            default:
                break;
            }
        }

        destination[j++] = source[i++];
    }

    destination[j] = L'\0';
}

BOOL LoadSettings(AppSettings *settings)
{
    WCHAR path[MAX_PATH];
    WCHAR *encodedMemo = NULL;
    DWORD encodedMemoCch = (MEMO_TEXT_MAX_CHARS * 2) + 1;
    int legacyTopmostDefault = 0;
    int displayModeDefault = DISPLAY_MODE_NORMAL;
    int displayMode = DISPLAY_MODE_NORMAL;

    if (!settings) {
        return FALSE;
    }

    InitDefaultSettings(settings);

    if (!GetSettingsFilePath(path, ARRAYSIZE(path))) {
        return FALSE;
    }

    encodedMemo = (WCHAR *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                     encodedMemoCch * sizeof(WCHAR));
    if (encodedMemo) {
        GetPrivateProfileStringW(SECTION_MEMO, KEY_MEMO_TEXT, L"",
                                 encodedMemo, encodedMemoCch, path);
        DecodeMemoText(encodedMemo, settings->memoText, ARRAYSIZE(settings->memoText));
        HeapFree(GetProcessHeap(), 0, encodedMemo);
    }

    settings->x = GetPrivateProfileIntW(SECTION_WINDOW, KEY_WINDOW_X,
                                        settings->x, path);
    settings->y = GetPrivateProfileIntW(SECTION_WINDOW, KEY_WINDOW_Y,
                                        settings->y, path);
    settings->width = ClampMinimum(
        GetPrivateProfileIntW(SECTION_WINDOW, KEY_WINDOW_WIDTH, settings->width, path),
        MIN_WINDOW_WIDTH, DEFAULT_WINDOW_WIDTH);
    settings->height = ClampMinimum(
        GetPrivateProfileIntW(SECTION_WINDOW, KEY_WINDOW_HEIGHT, settings->height, path),
        MIN_WINDOW_HEIGHT, DEFAULT_WINDOW_HEIGHT);

    legacyTopmostDefault = GetPrivateProfileIntW(SECTION_VIEW, KEY_TOPMOST, 0, path);
    displayModeDefault = legacyTopmostDefault ? DISPLAY_MODE_TOPMOST : DISPLAY_MODE_NORMAL;
    displayMode = GetPrivateProfileIntW(SECTION_VIEW, KEY_DISPLAY_MODE,
                                        displayModeDefault, path);

    settings->displayMode = NormalizeDisplayMode(displayMode);
    settings->topmost = settings->displayMode == DISPLAY_MODE_TOPMOST;

    GetPrivateProfileStringW(SECTION_APPEARANCE, KEY_FONT_FACE,
                             DEFAULT_FONT_FACE,
                             settings->fontFace,
                             ARRAYSIZE(settings->fontFace),
                             path);
    if (settings->fontFace[0] == L'\0') {
        StringCchCopyW(settings->fontFace, ARRAYSIZE(settings->fontFace),
                       DEFAULT_FONT_FACE);
    }
    settings->fontPointSize = ClampRange(
        GetPrivateProfileIntW(SECTION_APPEARANCE, KEY_FONT_POINT_SIZE,
                              settings->fontPointSize, path),
        MIN_FONT_POINT_SIZE, MAX_FONT_POINT_SIZE, DEFAULT_FONT_POINT_SIZE);
    settings->fontWeight = ClampRange(
        GetPrivateProfileIntW(SECTION_APPEARANCE, KEY_FONT_WEIGHT,
                              settings->fontWeight, path),
        0, 1000, FW_NORMAL);
    settings->fontItalic = GetPrivateProfileIntW(SECTION_APPEARANCE,
                                                 KEY_FONT_ITALIC,
                                                 settings->fontItalic,
                                                 path) != 0;
    settings->borderColor = (COLORREF)GetPrivateProfileIntW(
        SECTION_APPEARANCE, KEY_BORDER_COLOR, (int)settings->borderColor, path);
    settings->titleHoverOnly = GetPrivateProfileIntW(
        SECTION_APPEARANCE, KEY_TITLE_HOVER_ONLY,
        settings->titleHoverOnly, path) != 0;
    settings->iconMarquee = GetPrivateProfileIntW(
        SECTION_APPEARANCE, KEY_ICON_MARQUEE,
        settings->iconMarquee, path) != 0;
    settings->iconMarqueeTextColor = (COLORREF)GetPrivateProfileIntW(
        SECTION_APPEARANCE, KEY_ICON_MARQUEE_TEXT_COLOR,
        (int)settings->iconMarqueeTextColor, path);
    settings->iconMarqueeBackgroundColor = (COLORREF)GetPrivateProfileIntW(
        SECTION_APPEARANCE, KEY_ICON_MARQUEE_BACKGROUND_COLOR,
        (int)settings->iconMarqueeBackgroundColor, path);
    GetPrivateProfileStringW(SECTION_APPEARANCE,
                             KEY_ICON_MARQUEE_FONT_FACE,
                             DEFAULT_ICON_MARQUEE_FONT_FACE,
                             settings->iconMarqueeFontFace,
                             ARRAYSIZE(settings->iconMarqueeFontFace),
                             path);
    if (settings->iconMarqueeFontFace[0] == L'\0') {
        StringCchCopyW(settings->iconMarqueeFontFace,
                       ARRAYSIZE(settings->iconMarqueeFontFace),
                       DEFAULT_ICON_MARQUEE_FONT_FACE);
    }
    settings->iconMarqueeFontWeight = ClampRange(
        GetPrivateProfileIntW(SECTION_APPEARANCE,
                              KEY_ICON_MARQUEE_FONT_WEIGHT,
                              settings->iconMarqueeFontWeight, path),
        0, 1000, FW_BOLD);
    settings->iconMarqueeFontItalic = GetPrivateProfileIntW(
        SECTION_APPEARANCE, KEY_ICON_MARQUEE_FONT_ITALIC,
        settings->iconMarqueeFontItalic, path) != 0;
    settings->iconMarqueeSpeed = ClampRange(
        GetPrivateProfileIntW(SECTION_APPEARANCE, KEY_ICON_MARQUEE_SPEED,
                              settings->iconMarqueeSpeed, path),
        MIN_ICON_MARQUEE_SPEED, MAX_ICON_MARQUEE_SPEED,
        DEFAULT_ICON_MARQUEE_SPEED);
    settings->autoStart = GetPrivateProfileIntW(SECTION_STARTUP,
                                                KEY_AUTO_START,
                                                settings->autoStart,
                                                path) != 0;
    return TRUE;
}

static BOOL WriteIntValue(LPCWSTR path, LPCWSTR section, LPCWSTR key, int value)
{
    WCHAR buffer[32];

    if (FAILED(StringCchPrintfW(buffer, ARRAYSIZE(buffer), L"%d", value))) {
        return FALSE;
    }

    return WritePrivateProfileStringW(section, key, buffer, path);
}

BOOL SaveSettings(const AppSettings *settings)
{
    WCHAR path[MAX_PATH];
    WCHAR *encodedMemo = NULL;
    BOOL ok = TRUE;

    if (!settings) {
        return FALSE;
    }

    if (!EnsureSettingsDirectory()) {
        return FALSE;
    }

    if (!GetSettingsFilePath(path, ARRAYSIZE(path))) {
        return FALSE;
    }

    if (!EncodeMemoText(settings->memoText, &encodedMemo)) {
        return FALSE;
    }

    ok = ok && WritePrivateProfileStringW(SECTION_MEMO, KEY_MEMO_TEXT,
                                          encodedMemo, path);
    ok = ok && WriteIntValue(path, SECTION_WINDOW, KEY_WINDOW_X, settings->x);
    ok = ok && WriteIntValue(path, SECTION_WINDOW, KEY_WINDOW_Y, settings->y);
    ok = ok && WriteIntValue(path, SECTION_WINDOW, KEY_WINDOW_WIDTH, settings->width);
    ok = ok && WriteIntValue(path, SECTION_WINDOW, KEY_WINDOW_HEIGHT, settings->height);
    ok = ok && WriteIntValue(path, SECTION_VIEW, KEY_DISPLAY_MODE,
                             (int)settings->displayMode);
    ok = ok && WriteIntValue(path, SECTION_VIEW, KEY_TOPMOST,
                             settings->displayMode == DISPLAY_MODE_TOPMOST ? 1 : 0);
    ok = ok && WritePrivateProfileStringW(SECTION_APPEARANCE,
                                          KEY_FONT_FACE,
                                          settings->fontFace,
                                          path);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE, KEY_FONT_POINT_SIZE,
                             settings->fontPointSize);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE, KEY_FONT_WEIGHT,
                             settings->fontWeight);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE, KEY_FONT_ITALIC,
                             settings->fontItalic ? 1 : 0);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE, KEY_BORDER_COLOR,
                             (int)settings->borderColor);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE, KEY_TITLE_HOVER_ONLY,
                             settings->titleHoverOnly ? 1 : 0);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE, KEY_ICON_MARQUEE,
                             settings->iconMarquee ? 1 : 0);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE,
                             KEY_ICON_MARQUEE_TEXT_COLOR,
                             (int)settings->iconMarqueeTextColor);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE,
                             KEY_ICON_MARQUEE_BACKGROUND_COLOR,
                             (int)settings->iconMarqueeBackgroundColor);
    ok = ok && WritePrivateProfileStringW(SECTION_APPEARANCE,
                                          KEY_ICON_MARQUEE_FONT_FACE,
                                          settings->iconMarqueeFontFace,
                                          path);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE,
                             KEY_ICON_MARQUEE_FONT_WEIGHT,
                             settings->iconMarqueeFontWeight);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE,
                             KEY_ICON_MARQUEE_FONT_ITALIC,
                             settings->iconMarqueeFontItalic ? 1 : 0);
    ok = ok && WriteIntValue(path, SECTION_APPEARANCE,
                             KEY_ICON_MARQUEE_SPEED,
                             settings->iconMarqueeSpeed);
    ok = ok && WriteIntValue(path, SECTION_STARTUP, KEY_AUTO_START,
                             settings->autoStart ? 1 : 0);

    WritePrivateProfileStringW(NULL, NULL, NULL, path);
    HeapFree(GetProcessHeap(), 0, encodedMemo);
    return ok;
}
