{
    files = {
        "src/sat/pow.cpp"
    },
    depfiles_gcc = "pow.o: src/sat/pow.cpp\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-I/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/include",
            "-isystem",
            "/home/bennywu/.xmake/packages/o/openssl/1.1.1-w/6c51ab6278e2479b883dffafac69fdaf/include"
        }
    }
}