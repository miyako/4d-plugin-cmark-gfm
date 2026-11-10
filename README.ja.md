# 4d-plugin-cmark-gfm

[English](README.md) | 日本語

CommonMark のリファレンス実装を GitHub がフォークした [cmark-gfm](https://github.com/github/cmark-gfm)
（0.29.0.gfm.13）で、4D から GitHub Flavored Markdown を変換します。HTML、XML、CommonMark、man、
LaTeX、プレーンテキストに出力できます。cmark-gfm はプラグインに静的リンクされているため、
ほかに何もインストールする必要はありません。

より高速で構文拡張の多い HTML 専用のコンバーターが必要な場合は、同じ形のコマンドを持つ
[4d-plugin-md4c](https://github.com/miyako/4d-plugin-md4c) を参照してください。

## 動作環境

- 4D v21.1 以降
- macOS 11 以降（Apple シリコンおよび Intel）、または Windows x64

## インストール

[Releases](../../releases) ページから最新リリースをダウンロードします。

1. `.zip`（macOS と Windows）または公証済みの `.dmg`（macOS）をダウンロードします
2. `cmark-gfm.bundle` を、プロジェクトまたは 4D アプリケーションの **Plugins** フォルダーにコピーします
3. 4D を再起動します

## コマンド

### `CMark GFM Convert`

```4d
$result:=CMark GFM Convert($source {; $options})
```

| 引数 | 型 | 説明 |
|---|---|---|
| `$source` | テキスト | Markdown ソース |
| `$options` | オブジェクト | 省略可。形式、拡張、フラグ（下記参照） |
| `$result` | オブジェクト | `success`、`status`、`result`、失敗時は `error` |

このコマンドはスレッドセーフで、プリエンプティブプロセスから呼び出せます。cmark-gfm は
パーサーの状態の一部をグローバルに持つため、プラグイン内では変換を1つずつ実行します。

**戻り値のオブジェクト**

| プロパティ | 型 | 説明 |
|---|---|---|
| `success` | ブール | 変換に成功した場合は `True` |
| `status` | 整数 | 成功時は `0`。`1` 内部エラー、`2` 不正なオプション、`3` cmark-gfm の失敗 |
| `result` | テキスト | 変換後のドキュメント（`html` の場合は HTML の断片）。失敗時は `""` |
| `error` | テキスト | 失敗時のみ。エラーの内容 |

**オプション**

| プロパティ | 型 | デフォルト | 説明 |
|---|---|---|---|
| `format` | テキスト | `"html"` | `"html"`、`"xml"`、`"commonmark"`、`"man"`、`"latex"`、`"plaintext"`。それ以外の値はエラー |
| `extensions` | コレクション | すべて | 使用する GitHub 拡張の名前：`"table"`、`"strikethrough"`、`"autolink"`、`"tagfilter"`、`"tasklist"`。`[]` は素の CommonMark。不明な名前はエラー |
| `width` | 整数 | `0` | `commonmark`、`man`、`latex`、`plaintext` の折り返し幅。`0` は折り返さない |
| `unsafe` | ブール | `False` | 生の HTML と危険な URL を出力する。デフォルトでは生の HTML は `<!-- raw HTML omitted -->` になる |
| `smart` | ブール | `False` | スマートな引用符、ダッシュ、省略記号 |
| `hardBreaks` | ブール | `False` | すべての改行をハードブレークにする |
| `noBreaks` | ブール | `False` | 改行をスペースとして出力する |
| `sourcepos` | ブール | `False` | `data-sourcepos` 属性を付ける |
| `footnotes` | ブール | `False` | `[^label]` 脚注 |
| `githubPreLang` | ブール | `False` | `<code class="language-x">` の代わりに `<pre lang="x">` を使う |
| `fullInfoString` | ブール | `False` | コードブロックの情報文字列の残りを `data-meta` に残す |
| `liberalHTMLTag` | ブール | `False` | HTML タグの判定を緩くする |
| `strikethroughDoubleTilde` | ブール | `False` | 取り消し線にはチルダ2つが必要 |
| `tablePreferStyleAttributes` | ブール | `False` | 表のセルの揃えに `align` ではなく `style` を使う |
| `validateUTF8` | ブール | `False` | 不正な UTF-8 を U+FFFD に置き換える（4D のテキストは常に正しい UTF-8 になる） |

ブール型オプションに型の異なる値を渡した場合は無視されます。`tagfilter` 拡張は、`unsafe` を
指定しても `<script>` や `<iframe>` などのタグをエスケープします。

**例**

```4d
var $result : Object
$result:=CMark GFM Convert("# Hello\n\n- [x] done"; {format: CMark GFM format HTML; smart: True})
If ($result.success)
	WA SET PAGE CONTENT(*; "WebArea"; $result.result; "")
Else
	ALERT($result.error)
End if

// 素の CommonMark として72桁で整形し直す
$result:=CMark GFM Convert($markdown; {format: "commonmark"; extensions: []; width: 72})
```

## 定数

| 定数 | 型 | 値 |
|---|---|---|
| `CMark GFM format HTML` | テキスト | `html` |
| `CMark GFM format XML` | テキスト | `xml` |
| `CMark GFM format CommonMark` | テキスト | `commonmark` |
| `CMark GFM format man` | テキスト | `man` |
| `CMark GFM format LaTeX` | テキスト | `latex` |
| `CMark GFM format plain text` | テキスト | `plaintext` |

## ソースからのビルド

### 必要なもの

- CMake 3.20 以降
- Xcode（macOS）または Visual Studio 2022 以降（Windows）

### クローン

```bash
git clone --recurse-submodules https://github.com/miyako/4d-plugin-cmark-gfm.git
cd 4d-plugin-cmark-gfm
```

### ビルド（macOS）

```bash
cmake -S cmark-gfm -B cmark-gfm/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build cmark-gfm/build --config Release
```

### ビルド（Windows）

```pwsh
cmake -S cmark-gfm -B cmark-gfm/build -A x64
cmake --build cmark-gfm/build --config Release
```

プラグインは `cmark-gfm/cmark-gfm-test/Plugins` に出力されます。プラグインの CMake は、
upstream の CMake プロジェクトを使わずに cmark-gfm のソースを直接コンパイルします。upstream の
`cmake_minimum_required(VERSION 3.0)` は CMake 4 で受け付けられないためです。

### テストの実行

[tool4d](https://developer.4d.com/docs/Admin/cli/)（無料、ライセンス不要）が必要です。

```bash
/path/to/tool4d --dataless --startup-method=test_all --project=$(pwd)/cmark-gfm/cmark-gfm-test/Project/cmark-gfm.4DProject
```

すべてのテストに合格すると `PASS` と出力されます。

## CI/CD

このプロジェクトは、[4d-plugin-tutorial](https://github.com/miyako/4d-plugin-tutorial) から生成された、
3つのワークフローからなる標準のネイティブプラグイン用パイプラインを使用します。

| ワークフロー | トリガー | 目的 |
|---|---|---|
| `test.yml` | プッシュ / PR / 手動 / 再利用 | 同じリビジョンの成果物をビルド、テスト、検証 |
| `bump-version.yml` | 手動、デフォルトブランチのみ | `VERSION` を上げてコミット。タグなし |
| `release.yml` | 手動、デフォルトブランチのみ。入力なし | テストを再利用し、署名、公証、検証、ドラフト作成、`vVERSION` を公開 |

### 必要なシークレット（`release.yml` のみ）

**Settings → Secrets and variables → Actions** で設定します。

| シークレット | 説明 |
|---|---|
| `APPLE_DEVELOPER_ID_CERTIFICATE` | 秘密鍵を含む Developer ID Application `.p12` を Base64 エンコードしたもの |
| `APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD` | `.p12` 書き出し時のパスワード |
| `KEYCHAIN_PASSWORD` | CI ランナーの一時キーチェーン用の独立したパスワード |
| `NOTARYTOOL_APPLE_ID` | 公証に使う Apple ID のメールアドレス |
| `NOTARYTOOL_TEAM_ID` | Apple Developer のチーム ID（英数字10文字） |
| `NOTARYTOOL_PASSWORD` | アカウントのパスワードではなく、Apple の App 用パスワード |

## ライセンス

プラグインは [MIT License](LICENSE) で公開されています。cmark-gfm は © John MacFarlane および
コントリビューター、BSD-2-Clause で、一部は MIT License です。
[THIRD-PARTY-NOTICES](THIRD-PARTY-NOTICES/cmark-gfm.txt) を参照してください。
