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
            "-I/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/include",
            "-I/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/src/sat",
            "-isystem",
            "/home/bennywu/.xmake/packages/o/openssl/1.1.1-w/6c51ab6278e2479b883dffafac69fdaf/include"
        }
    }
}