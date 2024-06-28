.data
var1:	.word	1
var2:	.word	2
var3:	.word	0x100
	.text
main:
	and $18, $18, $0
	la $8, var1
	and $10, $10, $0
func1:
	and $11, $11, $0