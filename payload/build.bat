del r5.dll
rd /s /q obj
make -j 4
remove_compiler_padding.exe r5.dll 1> remove_comp_padding_results.txt 2> remove_comp_padding_errors.txt