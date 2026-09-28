add_executable(fuzz_parser fuzz_parser.c)
target_link_libraries(fuzz_parser PRIVATE ubx)
target_compile_options(fuzz_parser PRIVATE -fsanitize=fuzzer,address,undefined)
target_link_options(fuzz_parser PRIVATE -fsanitize=fuzzer,address,undefined)
