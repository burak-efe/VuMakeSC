..\..\bin\slang\slangc.exe shader_vert.slang -target spirv -fvk-use-scalar-layout -o spirv_vert.spv
..\..\bin\slang\slangc.exe shader_frag.slang -target spirv -fvk-use-scalar-layout -o spirv_frag.spv

..\..\bin\emu-pcc\pcconvk.exe pcconvk --path . --out pipeline_cache.bin

pause