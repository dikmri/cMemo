# リリース手順

cMemo はタグ方式で GitHub Release を作成します。

## 通常のリリース

1. 変更をコミットして `main` に push します。
2. 最新の SemVer タグを確認します。
3. 次のバージョンタグを作成します。
4. タグを push します。

```powershell
git tag v1.0.1
git push origin v1.0.1
```

`vX.Y.Z` 形式のタグを push すると GitHub Actions が起動し、MinGW-w64 で `dist/cMemo.exe` をビルドします。
ビルド後、`cMemo-vX.Y.Z.zip` を作成して GitHub Release に添付します。

## Codex 運用ルール

ユーザーから「最新版でリリース」「最新バージョンでリリース」などの指示があった場合は、次の手順で対応します。

1. 未コミットの変更を確認します。
2. 必要なビルド/テストを実行します。
3. 変更内容に合う短い日本語メッセージでコミットします。
4. `main` に push します。
5. 既存タグから次のバージョンを判断します。
6. 新しい `vX.Y.Z` タグを作成して push します。
7. GitHub Actions の Release 作成結果を確認します。

特に指定がない場合、バグ修正や小さな改善は patch version を上げます。
