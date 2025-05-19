{
    files = {
        "build/.objs/test_sha256/linux/x86_64/debug/src/test/test_sha256.cpp.o",
        "build/.objs/test_sha256/linux/x86_64/debug/src/sha256.cpp.o",
        "build/.objs/test_sha256/linux/x86_64/debug/src/block.cpp.o"
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