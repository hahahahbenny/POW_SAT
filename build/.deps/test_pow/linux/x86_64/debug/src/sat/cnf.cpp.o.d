{
    depfiles_gcc = "cnf.o: src/sat/cnf.cpp  /home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/include/sat/cnf.h  /home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/include/sat/format.hh\
",
    files = {
        "src/sat/cnf.cpp"
    },
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