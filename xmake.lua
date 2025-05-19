add_includedirs("$(projectdir)/include")
add_requires("openssl", {configs = {system = true}})
add_requires("cryptominisat5", {configs = {system = true}})
add_packages("openssl")
add_rules("mode.release", "mode.debug")

target("sat")
    set_kind("binary")
    add_links("boost_program_options")
    add_files("src/sat/*.cpp")  
    add_files("src/*.cpp")
    add_packages("boost")

target("brute")
    set_kind("binary")
    add_files("src/*.cpp")
    add_files("src/brute/*.cpp")

-- test example
target("test_sha256")
    set_kind("binary")
    add_files("src/test/test_sha256.cpp")
    add_files("src/*.cpp")

target("test_simple")
    add_includedirs("$(projectdir)/src/sat")
    set_kind("binary")
    add_files("src/test/sim_round.cpp")
    add_files("src/*.cpp")


target("test_solver")
    add_packages("cryptominisat5")
    add_files("src/test/test_for_read.cpp")
    set_kind("binary")

target("test_less_than")
    add_includedirs("$(projectdir)/src/sat/")
    add_packages("cryptominisat5")
    add_files("src/test/test_less_than.cpp")
    set_kind("binary")

target("test_truth_table")
    add_files("src/test/test_truth_table.cpp")
    set_kind("binary")

