# cMemo

## 目的

Windows 10向けの軽量な常駐メモアプリを、C言語 + Win32 API で作成してください。

デスクトップ上に小さなメモウィンドウを表示し続ける、軽量なメモアプリを作りたいです。
既存の付箋アプリが使いづらかったため、自分用にできるだけシンプルなものを作ります。

C#、WinForms、WPF、Electron、Tauriなどは使わず、C言語とWin32 APIのみで実装してください。

## 開発環境

- 言語: C
- UI: Win32 API
- 対象OS: Windows 10
- コンパイラ: MinGW-w64 または MSVC
- 文字コード: Unicode対応を優先し、基本的にW系APIを使う
  - RegisterClassExW
  - CreateWindowExW
  - DefWindowProcW
  - SendMessageW
  - Shell_NotifyIconW
  - GetWindowTextW
  - SetWindowTextW
- 外部ライブラリは使わない
- DBは使わない
- 設定やメモ内容はローカルファイルに保存する
- まずはINI形式で保存してよい
- JSONパーサーなどの外部ライブラリは使わない

## 保存先

以下のフォルダに設定ファイルを保存してください。

%AppData%\cMemo\settings.ini

保存する項目:

- メモ本文
- ウィンドウ位置 X
- ウィンドウ位置 Y
- ウィンドウ幅
- ウィンドウ高さ
- 表示モード
- 最前面ON/OFF

## アプリの基本仕様

### メイン機能

- メモ用のメインウィンドウを表示する
- メインウィンドウ内に複数行入力可能なEditコントロールを配置する
- メモ本文を入力・編集できる
- アプリ終了時にメモ本文、ウィンドウ位置、サイズ、表示モードを保存する
- 次回起動時に前回の状態を復元する
- できるだけ軽量にする
- 複雑な装飾や多機能化は不要
- まずはメモ1枚だけでよい

## 表示モード

以下の表示モードを切り替えられるようにしてください。

### 1. 最前面モード

- 他の通常ウィンドウより前面に表示する
- SetWindowPos を使い、HWND_TOPMOST を指定する
- ユーザー操作で ON/OFF を切り替えられるようにする

### 2. 通常表示モード

- 通常のウィンドウとして表示する
- 他のアプリウィンドウを前面に出した場合は、そのアプリに隠れてよい
- SetWindowPos を使い、HWND_NOTOPMOST を指定する

### 3. デスクトップ常駐モード

- メモウィンドウを閉じずに、デスクトップ上に表示し続ける
- ただし、他のアプリウィンドウには普通に隠れてよい
- つまり「最前面」ではなく、通常ウィンドウとして残り続けるだけでよい
- このモードでは HWND_NOTOPMOST にする

## 最後面モードについて

「最後面」については、Windowsでは常に最後面へ固定する単純な標準APIがあるわけではないため、初期版では無理に実装しないでください。

初期版では以下の方針にしてください。

- メニュー名は「通常表示」または「デスクトップ常駐」にする
- “他アプリより背面に回る”動作は、HWND_NOTOPMOST の通常ウィンドウとして実現する
- 常時最背面固定は実装しない
- WorkerW、Progman、デスクトップ埋め込みなどの特殊実装は使わない
- 必要になれば後の版で検討する

## UI仕様

メインウィンドウ:

- 通常の枠ありウィンドウでよい
- ウィンドウ内にEditコントロールを1つ配置する
- Editコントロールは複数行入力可能にする
- ウィンドウサイズ変更時、Editコントロールも追従してリサイズする
- フォントはシステム標準でよい
- 背景色変更、透明度変更、複数メモ、Markdown対応などは不要

Editコントロールのスタイル例:

- WS_CHILD
- WS_VISIBLE
- WS_VSCROLL
- ES_LEFT
- ES_MULTILINE
- ES_AUTOVSCROLL
- ES_WANTRETURN

## 右クリックメニュー

メインウィンドウまたはEditコントロール上で右クリックしたとき、コンテキストメニューを表示してください。

メニュー項目:

- 最前面にする
- 最前面を解除
- デスクトップ常駐モード
- 通常表示
- 保存
- 終了

## タスクトレイ常駐

Shell_NotifyIconW を使ってタスクトレイに常駐できるようにしてください。

仕様:

- 起動時にタスクトレイアイコンを追加する
- ウィンドウの閉じるボタンを押した場合、アプリを終了せずウィンドウを非表示にする
- タスクトレイアイコンをダブルクリックするとウィンドウを再表示する
- タスクトレイアイコンの右クリックでメニューを表示する
- 終了メニューからのみ完全終了する
- 完全終了時にタスクトレイアイコンを削除する

タスクトレイメニュー:

- 表示
- 最前面 ON/OFF
- 保存
- 終了

## 保存仕様

外部ライブラリを使わず、INIファイルで保存してください。

可能であれば以下のAPIを使ってください。

- GetPrivateProfileStringW
- WritePrivateProfileStringW

ただし、WritePrivateProfileString は古い互換APIなので、実装が複雑になる場合は自前でテキストファイルを読み書きしても構いません。
初期版では完成しやすさを優先してください。

## ファイル構成

以下のような構成にしてください。

- main.c
- settings.c
- settings.h
- resource.h
- app.rc
- README.md

## main.c

担当:

- WinMain
- RegisterClassExW
- CreateWindowExW
- メッセージループ
- WndProc
- メインウィンドウ作成
- Editコントロール作成
- 右クリックメニュー
- タスクトレイ処理
- 表示モード切替
- アプリ終了処理

## settings.c / settings.h

担当:

- 設定ファイルパス取得
- %AppData%\cMemo の作成
- settings.ini の読み込み
- settings.ini への保存
- 初期設定値の作成

## app.rc

担当:

- アプリケーションアイコン
- 必要であればバージョン情報

アイコンが用意できない場合は、初期版では標準アイコンを使って構いません。

## 表示モードの実装方針

enumを使って表示モードを管理してください。

例:

typedef enum DisplayMode {
    DISPLAY_MODE_NORMAL = 0,
    DISPLAY_MODE_TOPMOST = 1,
    DISPLAY_MODE_DESKTOP = 2
} DisplayMode;

表示モード変更時は、関数にまとめてください。

例:

void ApplyDisplayMode(HWND hwnd, DisplayMode mode);

処理イメージ:

- DISPLAY_MODE_TOPMOST:
  - SetWindowPos(hwnd, HWND_TOPMOST, ...)

- DISPLAY_MODE_NORMAL:
  - SetWindowPos(hwnd, HWND_NOTOPMOST, ...)

- DISPLAY_MODE_DESKTOP:
  - SetWindowPos(hwnd, HWND_NOTOPMOST, ...)
  - ウィンドウは表示したままにする
  - 特殊なデスクトップ埋め込みはしない

## 注意点

- まずは完成しやすさを優先してください
- 高度なWin32 APIや特殊なデスクトップ埋め込みは使わないでください
- 常時最背面固定のような難しい機能は、初期版では無理に実装しないでください
- 「最前面」と「通常表示」を確実に切り替えられることを優先してください
- メモ本文と位置・サイズが保存され、次回復元されることを重視してください
- エラーが起きても可能な範囲で落ちないようにしてください
- コードには保守用の簡潔なコメントを入れてください
- ビルドしてすぐ動く状態にしてください

## README.md に書いてほしいこと

- アプリ概要
- ビルド方法
- 実行方法
- 保存ファイルの場所
- 表示モードの説明
- 既知の制限
  - 最後面固定は初期版では未実装
  - WorkerW / Progman へのデスクトップ埋め込みは未実装
  - 複数メモは未実装

## ビルド方法

MinGW-w64の場合のビルドコマンド例を書いてください。

例:

windres app.rc -O coff -o app.res
gcc main.c settings.c app.res -municode -mwindows -lcomdlg32 -ldwmapi -lcomctl32 -lshell32 -lgdi32 -o cMemo.exe

MSVCの場合のビルド方法も、可能ならREADMEに簡単に書いてください。

## 最終的に期待する動作

1. cMemo.exe を起動する
2. メモウィンドウが表示される
3. 文字を入力できる
4. 右クリックメニューから最前面ON/OFFを切り替えられる
5. 閉じるボタンを押すと終了せず非表示になる
6. タスクトレイアイコンのダブルクリックで再表示できる
7. 終了メニューを押すと設定とメモ内容を保存して終了する
8. 次回起動時にメモ内容、位置、サイズ、表示モードが復元される
