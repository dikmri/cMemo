# リリース手順

cMemo はタグ方式で GitHub Release を作成します。

## バージョン番号の基準

cMemo は SemVer に近い `MAJOR.MINOR.PATCH` 形式でバージョンを管理します。

- `MAJOR`: 互換性を壊す変更や、既存ユーザーの使い方が大きく変わる変更で上げます。
- `MINOR`: 互換性を保ったまま、機能追加や比較的大きな改善を行う場合に上げます。
- `PATCH`: バグ修正、小さな改善、文言修正、軽微なUI調整で上げます。

例:

- `2.0.0`: 設定形式の大幅変更、複数メモ化に伴う保存方式の根本変更など。
- `1.2.0`: 検索機能、インストーラー対応、Microsoft Store 用パッケージ対応など。
- `1.1.3`: 表示崩れ修正、メニュー文言修正、更新確認の挙動修正など。

## 通常のリリース

1. 必要に応じて `src/resource.h` の `APP_VERSION_*` を更新します。
2. 必要なビルド/テストを実行します。
3. 変更をコミットして `main` に push します。
4. 最新の SemVer タグを確認します。
5. 次のバージョンタグを作成して push します。

```powershell
git tag v1.0.1
git push origin v1.0.1
```

`vX.Y.Z` 形式のタグを push すると GitHub Actions が起動し、MinGW-w64 で `dist/cMemo.exe` をビルドします。
ビルド後、`cMemo-vX.Y.Z.zip` と `cMemo-vX.Y.Z.exe` を作成して GitHub Release に添付します。
自動更新機能は GitHub Release の `cMemo-vX.Y.Z.exe` を参照するため、タグと `APP_VERSION_STRING` は必ず一致させます。
Release notes には、前回タグからの変更点を箇条書きで記載します。
バージョン番号だけを更新するコミットは変更点から除外します。

## Codex 運用ルール

ユーザーから「最新版でリリース」「最新バージョンでリリース」などの指示があった場合は、次の手順で対応します。

1. 未コミットの変更を確認します。
2. 既存タグから次のバージョンを判断します。
3. `src/resource.h` の `APP_VERSION_*` を次のバージョンへ更新します。
4. 必要なビルド/テストを実行します。
5. 変更内容に合う短い日本語メッセージでコミットします。
6. `main` に push します。
7. 新しい `vX.Y.Z` タグを作成して push します。
8. GitHub Actions の Release 作成結果を確認します。
9. Release notes に前回バージョンからの変更点が箇条書きで入っていることを確認します。

特に指定がない場合、バグ修正や小さな改善は patch version を上げます。
