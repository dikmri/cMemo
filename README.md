# cMemo

Windows 10 向けの軽量な常駐メモアプリ。C と Win32 API のみで実装。

## 機能

- 1 枚のメモを編集できる通常の Win32 ウィンドウ
- 複数行 Edit コントロールでのメモ入力
- 右クリックメニューから表示モード切り替え、保存、終了
- タスクトレイ常駐
- 閉じるボタンでは終了せず、ウィンドウを非表示
- タスクトレイアイコンのダブルクリックで再表示
- 終了時にメモ本文、ウィンドウ位置、サイズ、表示モードを保存
- 起動時に GitHub Release を確認し、最新版があれば自動更新

## 保存場所

設定とメモ本文は次の INI ファイルに保存されます。

```text
%AppData%\cMemo\settings.ini
```

メモ本文は INI に保存できるように改行などをエスケープして保存します。

## 表示モード

- `Normal Display`: 通常のウィンドウとして表示します。
- `Always on Top`: `SetWindowPos` と `HWND_TOPMOST` で最前面表示にします。
- `Desktop Mode`: 初期版では特殊な WorkerW / Progman 埋め込みを行わず、`HWND_NOTOPMOST` の通常ウィンドウとして表示します。

## 既知の制限

- 常時最背面への固定は未実装です。
- WorkerW / Progman へのデスクトップ埋め込みは未実装です。
- 複数メモは未実装です。
- メモ本文は最大 65535 文字までです。

## MinGW-w64 でのビルド

```bat
windres src/app.rc -O coff -o dist/app.res
gcc src/main.c src/settings.c dist/app.res -municode -mwindows -lcomdlg32 -ldwmapi -lshell32 -lgdi32 -lwininet -o dist/cMemo.exe
```

## 実行

```bat
dist\cMemo.exe
```

起動するとメモウィンドウが表示され、タスクトレイにアイコンが追加されます。終了する場合は右クリックメニューまたはタスクトレイメニューの `Exit` を選択してください。

## ライセンス

MIT License です。詳細は [LICENSE](LICENSE) を参照してください。
