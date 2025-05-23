#!/bin/bash

# 设置循环次数
LOOP_COUNT=8


SAT_NAME=("KISSAT" "CRYPTOMINISAT" "CADICAL" "KISSAT24" "BREAKID" "HKIS" "HKIS_PBVA" "CADICAL_BVA" "CADICAL_SCAVEL" "CADICAL_VIVINST")
SAT_ADRESS=("/home/bennywu/benny_workflow/cpp_learning/sat/kissat-master/build/kissat" \
            "cryptominisat5" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/cadical-master/build/cadical" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/sequential/kissat-sc2024/bin/kissat" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/sequential/BreakID-Kissat/bin" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/sequential/hKis/bin/starexec_run_bva" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/sequential/hKis/bin/starexec_run_pbva" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/SBVA/bin/starexec_run_sbva_cadical" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/Cadical_rel_1.5.3.Scavel/bin/starexec_run_default" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/CaDiCaL_vivinst/bin/starexec_run_default"
            )

# 设置求解器
KISSAT="/home/bennywu/benny_workflow/cpp_learning/sat/kissat-master/build/kissat"
CRYPTOMINISAT="cryptominisat5"
CADICAL="/home/bennywu/benny_workflow/cpp_learning/sat/cadical-master/build/cadical"
BREAKID="/home/bennywu/benny_workflow/cpp_learning/sat/sequential/BreakID-Kissat/bin/starexec_run_default"
KISSAT24="/home/bennywu/benny_workflow/cpp_learning/sat/sequential/kissat-sc2024/bin/kissat"
HKIS="/home/bennywu/benny_workflow/cpp_learning/sat/sequential/hKis/bin/starexec_run_bva"
HKIS_PBVA="/home/bennywu/benny_workflow/cpp_learning/sat/sequential/hKis/bin/starexec_run_pbva"
CADICAL_BVA="/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/SBVA/bin/starexec_run_sbva_cadical"
CADICAL_SCAVEL="/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/Cadical_rel_1.5.3.Scavel/bin/starexec_run_default"
CADICAL_VIVINST="/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/CaDiCaL_vivinst/bin/starexec_run_default"

SAT=$CADICAL_VIVINST

# 清空输出文件（如果已存在）
TEST_FILE="/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/output_cnf/bitcoin_test8bit.cnf"

# 循环执行指令
for ((j = 0; j < 10; j++)); do
    OUTPUT_FILE="../result/sat/consume_time_${SAT_NAME[j]}_withoutxor.csv"
    > "$OUTPUT_FILE"
    for ((i = 8; i <= LOOP_COUNT; i++)); do
        echo "Iteration $j: $i"

        echo  "${SAT_NAME[j]} : ${SAT_ADRESS[j]}"

        > "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/script/temp/proof.out"

        CNF_FILE="/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/output_cnf/withour_xor/sat/random$((i))bit.cnf"
        RESULT_FILE="/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/result/${SAT_NAME[j]}.txt"
        
        # echo $CNF_FILE /home/bennywu/benny_workflow/cpp_learning/satcmpbrute/script/temp

        if [ $j -gt 3 ]; then
            /usr/bin/time -f "$((i))_bits, %e \n" -a -o "$OUTPUT_FILE" \
            ${SAT_ADRESS[j]} "$CNF_FILE" /home/bennywu/benny_workflow/cpp_learning/satcmpbrute/script/temp > "$RESULT_FILE"
        else
            /usr/bin/time -f "$((i))_bits, %e \n" -a -o "$OUTPUT_FILE" \
            ${SAT_ADRESS[j]} "$CNF_FILE" > "$RESULT_FILE"
        fi
        # # 使用 time 命令记录执行时间，将结果写入文件

        echo "Iteration $((i)) completed."
    done
done


echo "All iterations completed. Results are saved in $OUTPUT_FILE."