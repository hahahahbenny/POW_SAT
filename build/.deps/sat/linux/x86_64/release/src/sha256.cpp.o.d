{
    files = {
        "src/sha256.cpp"
    },
    depfiles_gcc = "sha256.o: src/sha256.cpp  /home/bennywu/benny_workflow/cpp_learning/satcmpbrute/include/sha256.h\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-I/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/include"
        }
    }
}