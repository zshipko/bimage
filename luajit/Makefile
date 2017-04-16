
bimage:
	@echo "local ffi = require'ffi'" > bimage.lua
	@echo "ffi.cdef[[" >> bimage.lua
	@cat ../src/bimage.h | sed '/^#/ d' | sed '/^\// d' | sed '/\\$$/ d' | sed '/^$/ d' >> bimage.lua
	@echo "]]" >> bimage.lua
	@cat _bimage.lua >> bimage.lua
