//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

// Basic conversion; options may be omitted, HTML is the default format
$result:=CMark GFM Convert("# Hello")
ASSERT($result.success=True)
ASSERT($result.status=0)
ASSERT($result.result="<h1>Hello</h1>\n")
ASSERT($result.error=Null)

// Non-ASCII text, including a character outside the BMP (surrogate pair)
$result:=CMark GFM Convert("*日本語* 🎉"; {})
ASSERT($result.success)
ASSERT($result.result="<p><em>日本語</em> 🎉</p>\n")

// Empty source
$result:=CMark GFM Convert("")
ASSERT($result.success)
ASSERT($result.result="")

// Raw HTML is omitted unless unsafe is set
$result:=CMark GFM Convert("<b>x</b>")
ASSERT($result.result="<p><!-- raw HTML omitted -->x<!-- raw HTML omitted --></p>\n")

$result:=CMark GFM Convert("<b>x</b>"; {unsafe: True})
ASSERT($result.result="<p><b>x</b></p>\n")
