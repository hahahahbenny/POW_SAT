#include <atomic>
#include <fstream>
#include <iostream>
#include <cryptominisat5/cryptominisat.h>
#include <ostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>

std::atomic<bool> tag(false);

void Count_time(int milisecond){
    auto start = std::chrono::steady_clock::now();
    while(true){
        if(tag){
            auto end = std::chrono::steady_clock::now();
            auto last = std::chrono::duration_cast<std::chrono::seconds>(end-start);
            std::cout <<"end been " <<last.count() << "s\n" << std::endl;
            return;
        } 
        sleep(milisecond/1000);
        auto end = std::chrono::steady_clock::now();
        auto last = std::chrono::duration_cast<std::chrono::seconds>(end-start);
        std::cout <<"it's been " <<last.count() << "s\n" << std::endl;
    }
}

int main(){
    
    std::string file_name = "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/output_cnf/bitcoin_64_round.cnf";
    std::ifstream cnf_file(file_name);
    if(!cnf_file.is_open()){
        std::cerr << "无法打开cnf文件" << std::endl;
        return 1;
    }

    CMSat::SATSolver solver;
    std::vector<CMSat::Lit> clause;

    std::string line;
    std::string temp;
    while (std::getline(cnf_file, line)) {
        if(line[0] == 'c') continue;
        std::istringstream iss(line);
        if(line[0] == 'p'){
            iss >> temp;
            iss >> temp;
            iss >> temp;
            int var = std::stoi(temp);
            solver.new_vars(var);
            continue;
        }
        
        int lit;
        std::cout << line << "\n";
        while (iss >> lit) {
            std::cout << "lit == " << lit << std::endl; 
            if (lit == 0) {
                // 添加子句到求解器
                solver.add_clause(clause);
                clause.clear();
            } else if (lit > 0) {
                clause.push_back(CMSat::Lit(lit - 1, false));
            } else {
                clause.push_back(CMSat::Lit(-lit - 1, true));
            }
        }

        std::cout << "---------------------------" << std::endl;
    }
    std::thread timer = std::thread(Count_time, 2000);
    
    CMSat::lbool result = solver.solve();
    tag = true;
    timer.join();
    // 结束求解
    
    if (result == CMSat::l_True) {
        std::cout << "问题可满足！" << std::endl;
        // 输出变量的赋值
        for (int i = 0; i < solver.nVars(); ++i) {
            std::cout << "变量 " << i + 1 << " 的值为: " << solver.get_model()[i] << std::endl;
        }
    } else if (result == CMSat::l_False) {
        std::cout << "问题不可满足！" << std::endl;
    } else {
        std::cout << "求解超时或未知错误！" << std::endl;
    }

    
    
    return 0;
}