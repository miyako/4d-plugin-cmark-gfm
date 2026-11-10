//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

// All GitHub extensions are on by default
$result:=CMark GFM Convert("~~x~~")
ASSERT($result.result="<p><del>x</del></p>\n")

$result:=CMark GFM Convert("www.example.com")
ASSERT($result.result="<p><a href=\"http://www.example.com\">www.example.com</a></p>\n")

$result:=CMark GFM Convert("| a |\n|---|\n| 1 |")
ASSERT($result.result="<table>\n<thead>\n<tr>\n<th>a</th>\n</tr>\n</thead>\n<tbody>\n<tr>\n<td>1</td>\n</tr>\n</tbody>\n</table>\n")

$result:=CMark GFM Convert("- [x] done")
ASSERT($result.result="<ul>\n<li><input type=\"checkbox\" checked=\"\" disabled=\"\" /> done</li>\n</ul>\n")

// tagfilter escapes dangerous tags even in unsafe mode
$result:=CMark GFM Convert("<script>x</script>"; {unsafe: True})
ASSERT($result.result="&lt;script>x&lt;/script>\n")

// An empty collection is plain CommonMark
$result:=CMark GFM Convert("~~x~~"; {extensions: []})
ASSERT($result.result="<p>~~x~~</p>\n")

$result:=CMark GFM Convert("www.example.com"; {extensions: []})
ASSERT($result.result="<p>www.example.com</p>\n")

// Choose extensions one by one
$result:=CMark GFM Convert("~~x~~ www.example.com"; {extensions: ["strikethrough"]})
ASSERT($result.result="<p><del>x</del> www.example.com</p>\n")

// The special character added by an extension does not leak into the next conversion
$result:=CMark GFM Convert("~~x~~"; {extensions: ["strikethrough"]})
ASSERT($result.result="<p><del>x</del></p>\n")
$result:=CMark GFM Convert("~~x~~"; {extensions: []})
ASSERT($result.result="<p>~~x~~</p>\n")
$result:=CMark GFM Convert("~~x~~")
ASSERT($result.result="<p><del>x</del></p>\n")
