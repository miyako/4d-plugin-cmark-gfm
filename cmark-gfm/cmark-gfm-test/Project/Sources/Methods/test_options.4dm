//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

$result:=CMark GFM Convert("\"hi\"")
ASSERT($result.result="<p>&quot;hi&quot;</p>\n")

$result:=CMark GFM Convert("\"hi\""; {smart: True})
ASSERT($result.result="<p>“hi”</p>\n")

$result:=CMark GFM Convert("a\nb")
ASSERT($result.result="<p>a\nb</p>\n")

$result:=CMark GFM Convert("a\nb"; {hardBreaks: True})
ASSERT($result.result="<p>a<br />\nb</p>\n")

$result:=CMark GFM Convert("a\nb"; {noBreaks: True})
ASSERT($result.result="<p>a b</p>\n")

$result:=CMark GFM Convert("# Hello"; {sourcepos: True})
ASSERT($result.result="<h1 data-sourcepos=\"1:1-1:7\">Hello</h1>\n")

$result:=CMark GFM Convert("x[^1]\n\n[^1]: note")
ASSERT(Position("footnote"; $result.result)=0)

$result:=CMark GFM Convert("x[^1]\n\n[^1]: note"; {footnotes: True})
ASSERT(Position("class=\"footnotes\""; $result.result)>0)

// A non-Boolean flag value is ignored
$result:=CMark GFM Convert("\"hi\""; {smart: 1})
ASSERT($result.result="<p>&quot;hi&quot;</p>\n")
