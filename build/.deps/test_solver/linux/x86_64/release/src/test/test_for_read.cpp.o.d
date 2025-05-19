{
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-I/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/include",
            "-isystem",
            "/home/bennywu/.xmake/packages/o/openssl/1.1.1-w/6c51ab6278e2479b883dffafac69fdaf/include"
        }
    },
    files = {
        "src/test/test_for_read.cpp"
    },
    depfiles_gcc = "test_for_read.o: src/test/test_for_read.cpp\
"
}