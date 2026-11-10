/*
 cmark-gfm: GitHub Flavored Markdown for 4D, using cmark-gfm (https://github.com/github/cmark-gfm).

 cmark-gfm keeps one piece of global mutable state: the table of special inline characters.
 Attaching an extension to a parser adds its characters ('~' for strikethrough, ...) and
 freeing the parser removes them again, so two conversions running at the same time in
 preemptive processes could make each other miss an extension. Conversions are therefore
 serialized with one lock. The command never calls the 4D runtime while holding it.
*/

#include "cmark-gfm-4dplugin.h"
#include "4d-helpers.h"

#include "cmark-gfm.h"
#include "cmark-gfm-extension_api.h"
#include "cmark-gfm-core-extensions.h"

#include <cstdlib>
#include <iterator>
#include <mutex>
#include <string>
#include <vector>

namespace {

std::mutex cmark_lock;

struct Flag {
    const char *key;
    unsigned value;
};

const Flag option_flags[] = {
    {"sourcepos", CMARK_OPT_SOURCEPOS},
    {"hardBreaks", CMARK_OPT_HARDBREAKS},
    {"unsafe", CMARK_OPT_UNSAFE},
    {"noBreaks", CMARK_OPT_NOBREAKS},
    {"validateUTF8", CMARK_OPT_VALIDATE_UTF8},
    {"smart", CMARK_OPT_SMART},
    {"githubPreLang", CMARK_OPT_GITHUB_PRE_LANG},
    {"liberalHTMLTag", CMARK_OPT_LIBERAL_HTML_TAG},
    {"footnotes", CMARK_OPT_FOOTNOTES},
    {"strikethroughDoubleTilde", CMARK_OPT_STRIKETHROUGH_DOUBLE_TILDE},
    {"tablePreferStyleAttributes", CMARK_OPT_TABLE_PREFER_STYLE_ATTRIBUTES},
    {"fullInfoString", CMARK_OPT_FULL_INFO_STRING},
};

// The GitHub extensions shipped with cmark-gfm; all of them are used unless the caller
// passes an `extensions` collection.
const char *const core_extensions[] = {"table", "strikethrough", "autolink", "tagfilter", "tasklist"};

enum Format { HTML, XML, COMMONMARK, MAN, LATEX, PLAINTEXT };

bool parse_format(const std::string &s, Format &f) {
    if (s == "html") f = HTML;
    else if (s == "xml") f = XML;
    else if (s == "commonmark") f = COMMONMARK;
    else if (s == "man") f = MAN;
    else if (s == "latex") f = LATEX;
    else if (s == "plaintext") f = PLAINTEXT;
    else return false;
    return true;
}

// Reads options.extensions: true when the option is absent or a collection of known names.
bool read_extensions(PA_ObjectRef options, std::vector<std::string> &names, std::string &error) {
    names.assign(std::begin(core_extensions), std::end(core_extensions));
    PA_Variable v;
    if (!ob_get_var(options, "extensions", v)) return true;
    if (PA_GetVariableKind(v) != eVK_Collection) {
        error = "extensions must be a collection of text";
        return false;
    }
    PA_CollectionRef c = PA_GetCollectionVariable(v);
    names.clear();
    PA_long32 n = c ? PA_GetCollectionLength(c) : 0;
    for (PA_long32 i = 0; i < n; ++i) {
        PA_Variable e = PA_GetCollectionElement(c, i);
        if (PA_GetVariableKind(e) != eVK_Unistring) {
            error = "extensions must be a collection of text";
            return false;
        }
        PA_Unistring u = PA_GetStringVariable(e);
        names.push_back(unistring_utf8(&u));
    }
    return true;
}

struct Result {
    PA_long32 status = 0;
    std::string text;
    std::string error;
};

// Parses and renders without touching the 4D runtime.
Result convert(const std::string &source, Format format, int options, int width,
               const std::vector<std::string> &extensions) {
    Result r;
    std::lock_guard<std::mutex> guard(cmark_lock);
    cmark_gfm_core_extensions_ensure_registered();

    cmark_parser *parser = cmark_parser_new(options);
    if (!parser) {
        r.status = 3;
        r.error = "cmark-gfm could not create a parser";
        return r;
    }
    for (const std::string &name : extensions) {
        cmark_syntax_extension *ext = cmark_find_syntax_extension(name.c_str());
        if (!ext) {
            r.status = 2;
            r.error = "unknown extension \"" + name + "\"";
            cmark_parser_free(parser);
            return r;
        }
        cmark_parser_attach_syntax_extension(parser, ext);
    }

    cmark_parser_feed(parser, source.data(), source.size());
    cmark_node *doc = cmark_parser_finish(parser);
    if (!doc) {
        r.status = 3;
        r.error = "cmark-gfm could not parse the source";
        cmark_parser_free(parser);
        return r;
    }

    char *out = nullptr;
    switch (format) {
        case HTML: out = cmark_render_html(doc, options, cmark_parser_get_syntax_extensions(parser)); break;
        case XML: out = cmark_render_xml(doc, options); break;
        case COMMONMARK: out = cmark_render_commonmark(doc, options, width); break;
        case MAN: out = cmark_render_man(doc, options, width); break;
        case LATEX: out = cmark_render_latex(doc, options, width); break;
        case PLAINTEXT: out = cmark_render_plaintext(doc, options, width); break;
    }
    if (out) {
        r.text = out;
        cmark_get_default_mem_allocator()->free(out);
    } else {
        r.status = 3;
        r.error = "cmark-gfm could not render the document";
    }
    cmark_node_free(doc);
    cmark_parser_free(parser);
    return r;
}

}  // namespace

void PluginMain(PA_long32 selector, PA_PluginParameters params) {
    try {
        switch (selector) {
            case 1: CMark_GFM_Convert(params); break;
        }
    } catch (...) {
        // A command that failed half way still owes 4D its result.
        try {
            if (selector == 1) return_failure(params, 1, "internal error");
        } catch (...) {
        }
    }
}

// CMark GFM Convert(source : Text {; options : Object}) : Object
static void CMark_GFM_Convert(PA_PluginParameters params) {
    std::string source = unistring_utf8(PA_GetStringParameter(params, 1));
    PA_ObjectRef options = PA_GetObjectParameter(params, 2);

    Format format = HTML;
    std::string s;
    if (ob_get_s(options, "format", s) && !parse_format(s, format)) {
        return_failure(params, 2, "unknown format \"" + s +
            "\"; use \"html\", \"xml\", \"commonmark\", \"man\", \"latex\" or \"plaintext\"");
        return;
    }

    unsigned flags = CMARK_OPT_DEFAULT;
    for (const Flag &f : option_flags) ob_get_flag(options, f.key, f.value, flags);

    int width = 0;
    double w;
    if (ob_get_n(options, "width", w)) {
        if (w < 0 || w > 100000) {
            return_failure(params, 2, "width must be between 0 and 100000");
            return;
        }
        width = int(w);
    }

    std::vector<std::string> extensions;
    std::string error;
    if (!read_extensions(options, extensions, error)) {
        return_failure(params, 2, error);
        return;
    }

    Result r = convert(source, format, int(flags), width, extensions);
    if (r.status != 0) {
        return_failure(params, r.status, r.error);
        return;
    }
    return_success(params, r.text.data(), r.text.size());
}
