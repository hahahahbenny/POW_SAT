{
    files = {
        "build/.objs/test_pow/linux/x86_64/debug/src/test/test_pow.cpp.o",
        "build/.objs/test_pow/linux/x86_64/debug/src/sat/pow.cpp.o",
        "build/.objs/test_pow/linux/x86_64/debug/src/sat/cnf.cpp.o",
        "build/.objs/test_pow/linux/x86_64/debug/src/block.cpp.o",
        "build/.objs/test_pow/linux/x86_64/debug/src/sha256.cpp.o",
        "build/.objs/test_pow/linux/x86_64/debug/src/sat/format.cpp.o"
    },
    values = {
        "/usr/bin/g++",
        {
            "-m64",
            "-L/home/bennywu/.xmake/packages/o/openssl/1.1.1-w/6c51ab6278e2479b883dffafac69fdaf/lib",
            "-lssl",
            "-lcrypto",
            "-lpthread",
            "-ldl"
        }
    }
}