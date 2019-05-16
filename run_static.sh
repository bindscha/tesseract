#!bin/bash

graphs="soc-lj"

algos="0" # 1 2" 

log_dir="/root/jasmina/graph_minging/sosp_logs/"
path_to_file=$1
K="3"

threads="56"
for algo in $algos; do
 if [[ $algo == 1 ]]; then
   sed -i 's/SYM [0-9]*/SYM 0/g' common_include.hpp
else
   sed -i 's/SYM [0-9]*/SYM 1/g' common_include.hpp
fi

for graph in $graphs; do
   nodes=$(cat ${path_to_file}/${graph}.ini | grep 'vertices' | awk -F '=' '{print $2}')
   for k in $K; do
      sed -i 's/define K [0-9]*/define K '"${k}"' /g' common_include.hpp
      make
      for thread in ${threads}; do
         log_file=${graph}.$k.algo_$algo.t${thread}.log
         ./tesseract -f ${path_to_file}/${graph} -d ${path_to_file}/${graph}_offsets -n ${nodes} -t ${thread} -a ${algo} 2>&1 | tee ${log_file}
   done
        
   done

done


done

