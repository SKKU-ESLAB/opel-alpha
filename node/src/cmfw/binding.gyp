{
	"targets": [
		{
			"target_name": "NIL_Comm",
			"sources": [
				"./src/crc_ccit.cc",
				"./src/comm_bt.cc",
				"./src/comm_core.cc",
				"./comm_binding.cc",
				"./NIL_Comm.cc"
			],
			"include_dirs": ["<!(node -e \"require('nan')\")", "./inc"],
			"libraries": ['-lbluetooth', '-luv', '-fpermissive'],
			"cflags": ['-std=gnu++0x']
		}
	]
}
