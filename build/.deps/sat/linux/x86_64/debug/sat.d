{
    files = {
        "build/.objs/sat/linux/x86_64/debug/src/sat/main.cpp.o",
        "build/.objs/sat/linux/x86_64/debug/src/sha256.cpp.o",
        "build/.objs/sat/linux/x86_64/debug/src/block.cpp.o"
    },
    values = {
        "/usr/bin/g++",
        {
            "-m64",
            "-L/home/bennywu/.xmake/packages/o/openssl/1.1.1-w/6c51ab6278e2479b883dffafac69fdaf/lib",
            "-lboost_program_options",
            "-lssl",
            "-lcrypto",
            "-lpthread",
            "-ldl"
        }
    }
}