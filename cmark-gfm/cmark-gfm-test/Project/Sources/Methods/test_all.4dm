//%attributes = {"invisible":true}
If (Application info.headless)
	
	test_convert
	test_extensions
	test_options
	test_formats
	test_errors
	
	LOG EVENT(Into system standard outputs; "PASS"; Information message)
	
End if 
