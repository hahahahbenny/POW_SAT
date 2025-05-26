{
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-I/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/include",
            "-isystem",
            "/home/bennywu/.xmake/packages/o/openssl/1.1.1-w/6c51ab6278e2479b883dffafac69fdaf/include"
        }
    },
    files = {
        "src/sha256.cpp"
    },
    depfiles_gcc = "sha256.o: src/sha256.cpp  /home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/include/sha256.h\
"
}