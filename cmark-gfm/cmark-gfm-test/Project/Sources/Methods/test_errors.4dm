//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

$result:=CMark GFM Convert("# x"; {format: "docx"})
ASSERT($result.success=False)
ASSERT($result.status=2)
ASSERT($result.result="")
ASSERT(Position("docx"; $result.error)>0)

$result:=CMark GFM Convert("# x"; {extensions: ["table"; "emoji"]})
ASSERT($result.success=False)
ASSERT($result.status=2)
ASSERT(Position("emoji"; $result.error)>0)

$result:=CMark GFM Convert("# x"; {extensions: "table"})
ASSERT($result.success=False)
ASSERT($result.status=2)

$result:=CMark GFM Convert("# x"; {extensions: [1]})
ASSERT($result.success=False)
ASSERT($result.status=2)

$result:=CMark GFM Convert("# x"; {width: -1})
ASSERT($result.success=False)
ASSERT($result.status=2)
