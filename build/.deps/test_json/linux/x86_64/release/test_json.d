{
    files = {
        "build/.objs/test_json/linux/x86_64/release/src/sha256.cpp.o",
        "build/.objs/test_json/linux/x86_64/release/src/block.cpp.o"
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