# 4d-plugin-cmark-gfm

English | [日本語](README.ja.md)

Convert GitHub Flavored Markdown in 4D with [cmark-gfm](https://github.com/github/cmark-gfm)
(0.29.0.gfm.13), GitHub's fork of the CommonMark reference implementation. Output to HTML,
XML, CommonMark, man, LaTeX or plain text. cmark-gfm is linked statically into the plugin:
nothing else needs to be installed.

For a faster HTML-only converter with more syntax extensions, see
[4d-plugin-md4c](https://github.com/miyako/4d-plugin-md4c), which has the same command shape.

## Requirements

- 4D v21.1 or later
- macOS 11 or later (Apple silicon and Intel), or Windows x64

## Installation

Download the latest release from the [Releases](../../releases) page.

1. Download the `.zip` (macOS and Windows) or the notarized `.dmg` (macOS)
2. Copy `cmark-gfm.bundle` into the **Plugins** folder of your project or of your 4D application
3. Restart 4D

## Commands

### `CMark GFM Convert`

```4d
$result:=CMark GFM Convert($source {; $options})
```

| Parameter | Type | Description |
|---|---|---|
| `$source` | Text | Markdown source |
| `$options` | Object | Optional. Format, extensions and flags, see below |
| `$result` | Object | `success`, `status`, `result` and, on failure, `error` |

The command is thread-safe and can be called from preemptive processes. cmark-gfm keeps
some global parser state, so conversions run one at a time inside the plugin.

**Result object**

| Property | Type | Description |
|---|---|---|
| `success` | Boolean | `True` when the conversion succeeded |
| `status` | Integer | `0` on success; `1` internal error, `2` invalid option, `3` cmark-gfm failed |
| `result` | Text | The converted document (an HTML fragment for `html`); `""` on failure |
| `error` | Text | Present on failure only: what went wrong |

**Options**

| Property | Type | Default | Description |
|---|---|---|---|
| `format` | Text | `"html"` | `"html"`, `"xml"`, `"commonmark"`, `"man"`, `"latex"` or `"plaintext"`. Any other value is an error |
| `extensions` | Collection | all | Names of the GitHub extensions to use: `"table"`, `"strikethrough"`, `"autolink"`, `"tagfilter"`, `"tasklist"`. `[]` is plain CommonMark. An unknown name is an error |
| `width` | Integer | `0` | Wrap width for `commonmark`, `man`, `latex` and `plaintext`; `0` does not wrap |
| `unsafe` | Boolean | `False` | Render raw HTML and dangerous URLs. By default raw HTML becomes `<!-- raw HTML omitted -->` |
| `smart` | Boolean | `False` | Smart quotes, dashes and ellipses |
| `hardBreaks` | Boolean | `False` | Every line break is a hard break |
| `noBreaks` | Boolean | `False` | Line breaks are rendered as spaces |
| `sourcepos` | Boolean | `False` | Add `data-sourcepos` attributes |
| `footnotes` | Boolean | `False` | `[^label]` footnotes |
| `githubPreLang` | Boolean | `False` | `<pre lang="x">` instead of `<code class="language-x">` |
| `fullInfoString` | Boolean | `False` | Keep the rest of a code block's info string in `data-meta` |
| `liberalHTMLTag` | Boolean | `False` | Be liberal in what counts as an HTML tag |
| `strikethroughDoubleTilde` | Boolean | `False` | Strikethrough needs two tildes |
| `tablePreferStyleAttributes` | Boolean | `False` | Align table cells with `style` instead of `align` |
| `validateUTF8` | Boolean | `False` | Replace invalid UTF-8 with U+FFFD (4D text is always valid already) |

Values of the wrong type for Boolean options are ignored. The `tagfilter` extension escapes
tags such as `<script>` and `<iframe>` even with `unsafe`.

**Example**

```4d
var $result : Object
$result:=CMark GFM Convert("# Hello\n\n- [x] done"; {format: CMark GFM format HTML; smart: True})
If ($result.success)
	WA SET PAGE CONTENT(*; "WebArea"; $result.result; "")
Else
	ALERT($result.error)
End if

// Plain CommonMark, re-formatted at 72 columns
$result:=CMark GFM Convert($markdown; {format: "commonmark"; extensions: []; width: 72})
```

## Constants

| Constant | Type | Value |
|---|---|---|
| `CMark GFM format HTML` | Text | `html` |
| `CMark GFM format XML` | Text | `xml` |
| `CMark GFM format CommonMark` | Text | `commonmark` |
| `CMark GFM format man` | Text | `man` |
| `CMark GFM format LaTeX` | Text | `latex` |
| `CMark GFM format plain text` | Text | `plaintext` |

## Building from Source

### Prerequisites

- CMake 3.20+
- Xcode (macOS) or Visual Studio 2022 or later (Windows)

### Clone

```bash
git clone --recurse-submodules https://github.com/miyako/4d-plugin-cmark-gfm.git
cd 4d-plugin-cmark-gfm
```

### Build (macOS)

```bash
cmake -S cmark-gfm -B cmark-gfm/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build cmark-gfm/build --config Release
```

### Build (Windows)

```pwsh
cmake -S cmark-gfm -B cmark-gfm/build -A x64
cmake --build cmark-gfm/build --config Release
```

The plugin is written to `cmark-gfm/cmark-gfm-test/Plugins`. The plugin's CMake compiles
the cmark-gfm sources itself rather than using upstream's CMake project, whose
`cmake_minimum_required(VERSION 3.0)` CMake 4 rejects.

### Run Tests

Requires [tool4d](https://developer.4d.com/docs/Admin/cli/) (free, no license needed):

```bash
/path/to/tool4d --dataless --startup-method=test_all --project=$(pwd)/cmark-gfm/cmark-gfm-test/Project/cmark-gfm.4DProject
```

The run prints `PASS` when every test passed.

## CI/CD

This project uses the canonical generated three-workflow native-plugin pipeline from
[4d-plugin-tutorial](https://github.com/miyako/4d-plugin-tutorial):

| Workflow | Trigger | Purpose |
|---|---|---|
| `test.yml` | Push / PR / manual / reusable | Build, test and validate same-revision artifacts |
| `bump-version.yml` | Manual, default branch only | Bump `VERSION` and commit; no tag |
| `release.yml` | Manual, default branch only; no inputs | Reuse tests, sign, notarize, verify, draft and publish `vVERSION` |

### Required Secrets (for `release.yml` only)

Configure these in **Settings → Secrets and variables → Actions**:

| Secret | Description |
|---|---|
| `APPLE_DEVELOPER_ID_CERTIFICATE` | Base64-encoded Developer ID Application `.p12` including its private key |
| `APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD` | Password for the `.p12` export |
| `KEYCHAIN_PASSWORD` | Independent password for the CI runner's temporary keychain |
| `NOTARYTOOL_APPLE_ID` | Apple ID email for notarization |
| `NOTARYTOOL_TEAM_ID` | Apple Developer Team ID (10-char alphanumeric) |
| `NOTARYTOOL_PASSWORD` | Apple app-specific password, not the account password |

## License

The plugin is released under the [MIT License](LICENSE). cmark-gfm is © John MacFarlane
and contributors, BSD-2-Clause, with parts under the MIT License; see
[THIRD-PARTY-NOTICES](THIRD-PARTY-NOTICES/cmark-gfm.txt).
