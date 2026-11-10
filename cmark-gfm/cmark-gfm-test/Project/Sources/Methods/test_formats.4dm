//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

$result:=CMark GFM Convert("# Hello"; {format: CMark GFM format HTML})
ASSERT($result.result="<h1>Hello</h1>\n")

$result:=CMark GFM Convert("# Hello"; {format: CMark GFM format XML})
ASSERT(Position("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"; $result.result)=1)
ASSERT(Position("<heading level=\"1\">"; $result.result)>0)

$result:=CMark GFM Convert("# Hello *x*"; {format: CMark GFM format CommonMark})
ASSERT($result.result="# Hello *x*\n")

$result:=CMark GFM Convert("# Hello *x*"; {format: CMark GFM format plain text})
ASSERT($result.result="Hello x\n")

$result:=CMark GFM Convert("# Hello"; {format: CMark GFM format LaTeX})
ASSERT($result.result="\\section{Hello}\n")

$result:=CMark GFM Convert("# Hello"; {format: CMark GFM format man})
ASSERT($result.result=".SH\nHello\n")

// width wraps the text renderers
$result:=CMark GFM Convert("one two three four five"; {format: "commonmark"; width: 10})
ASSERT($result.result="one two\nthree four\nfive\n")
