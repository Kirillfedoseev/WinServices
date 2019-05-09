Launch the system:
1) Launch server.exe 
2) Write to register version of plugin (it will throw exception, if register will be empty, see on "Change plugin version")
3) Launch client.exe with two arguments:
	1)server_ip: 127.0.0.1 
	2)DLL GUID: {00000000-0000-0000-0200-000000000000} or 
				{00000000-0000-0000-0100-000000000000} 
Example: > client.exe 127.0.0.1 {00000000-0000-0000-0100-000000000000} 

The should be like "copyrigth"

Change plugin version:
1) Launch supplier.exe with argument:
	a) plugin_name: Plugin1_2.dll or Plugin1_1.dll
Example: > Supplier.exe Plugin1_2.dll

this action will change the register in address:
	HKEY_CURRENT_USER\Software\PluginSupplier\Plugin Name
	
After this changing the register, the server will use another version of plugin without restart
