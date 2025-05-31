#!/bin/bash

# 设置循环次数
LOOP_COUNT=8


SAT_NAME=("KISSAT" "CRYPTOMINISAT" "CADICAL" "KISSAT24" "BREAKID" "HKIS" "HKIS_PBVA" "CADICAL_BVA" "CADICAL_SCAVEL" "CADICAL_VIVINST" "MapleCaDiCaL_PPD")
SAT_ADRESS=("/home/bennywu/benny_workflow/cpp_learning/sat/kissat-master/build/kissat" \
            "cryptominisat5" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/cadical-master/build/cadical" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/sequential/kissat-sc2024/bin/kissat" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/sequential/BreakID-Kissat/bin/starexec_run_default" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/sequential/hKis/bin/starexec_run_bva" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/sequential/hKis/bin/starexec_run_pbva" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/SBVA/bin/starexec_run_sbva_cadical" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/Cadical_rel_1.5.3.Scavel/bin/starexec_run_default" \
            "/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/CaDiCaL_vivinst/bin/starexec_run_default"
            "/home/bennywu/benny_workflow/cpp_learning/sat/Sequential2023/Sequential/solvers/MapleCaDiCaL_PPD/bin/starexec_run_500_500"
            )

CNF_FOLDER="/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/output_cnf/withour_xor"

CNF_CLASS=("sat" \
            "optimize_with_k2" \
            "OptimizeVars_a_to_h" \
            "constant_bool"
)

SUM=0

# 清空输出文件（如果已存在）
TEST_FILE="/home/bennywu/benny_workflow/cpp_learning/satcoin-master/out_1k_sat.cnf"

# 循环执行指令
for ((j = 0; j < 11; j++)); do
    for((h = 0; h < 4; h++)); do
        OUTPUT_FILE="/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/result/sat/${CNF_CLASS[h]}/consume_time_${SAT_NAME[j]}.csv"
        > OUTPUT_FILE
        echo "bits, time(avg)" >> "$OUTPUT_FILE"
        for ((i = 4; i <= 10; i++)); do
            echo "Iteration $j: $i"
            echo  "${SAT_NAME[j]} : ${SAT_ADRESS[j]}"
            
            TEMP_FILE="/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/result/sat/${CNF_CLASS[h]}/temp/consume_time_${SAT_NAME[j]}_$((i))_bit.txt"
            > "$TEMP_FILE"

            CNF_FILE="$CNF_FOLDER/${CNF_CLASS[h]}/$((i))bits_left.cnf"
            SUM=0
            for((k = 0; k < 1000; k++)); do
                if [ $j -gt 3 ]; then
                    > "/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/script/temp/proof.out"
                    timeout -s SIGINT 1800s /usr/bin/time -f "$((i))_bits, %e \n" -a -o "$TEMP_FILE" \
                    ${SAT_ADRESS[j]} "$CNF_FILE" /home/bennywu/benny_workflow/cpp_learning/satcmpbrute/code/script/temp
                else
                    timeout -s SIGINT 1800s /usr/bin/time -f "$((i))_bits, %e \n" -a -o "$TEMP_FILE" \
                    ${SAT_ADRESS[j]} "$CNF_FILE" 
                fi

                TIME=$(tail -n 2 "$TEMP_FILE" | awk -F ',' '{print $2}')
                
                if [[ -z "$TIME" ]]; then
                    echo "Warning: TIME variable is empty. Skipping this iteration."
                    continue
                fi

                # 累加时间
                SUM=$(echo "$SUM + $TIME" | bc)
            done
            # RESULT_FILE="/home/bennywu/benny_workflow/cpp_learning/satcmpbrute/result/${SAT_NAME[j]}_optimize.txt"

            # echo $CNF_FILE /home/bennywu/benny_workflow/cpp_learning/satcmpbrute/script/temp
            # # 使用 time 命令记录执行时间，将结果写入文件

            AVERAGE=$(echo "$SUM / 1000" | bc -l)
            echo "$((i)), $AVERAGE" >> "$OUTPUT_FILE"
            echo "$((i)) average time is $AVERAGE"
            echo "Iteration $((i)) completed."
            echo $SUM
        done
    done
done


echo "All iterations completed. Results are saved in $OUTPUT_FILE."