#!/bin/bash


# ARGUMENT PARSING
dbg=false
mbits=12
break_on_error=false

while getopts dm:b flag
do 
    case "$flag" in
        d) dbg=true;;
        m) mbits=${OPTARG};;
        b) break_on_error=true;;
    esac
done

if [ "$dbg" = true ]; then
    export DBG=1
fi

echo -e "Building binaries...\n"
make

echo -e "\nRunning LZW Unit Tests..."

file_count=0
num_correct=0
total_compression_increase=0
total_time_increase=0

for filename in tests/test_cases/*; do
    file_count=$((file_count + 1))
    original_size=$(wc -c < "$filename")

    # First calculate the benchmarks.
    start_time=$(perl -MTime::HiRes=time -e 'printf "%.9f\n", time')
    compress -c -b $mbits < "$filename" > temp.BUILTIN.COMPRESS.Z
    end_time=$(perl -MTime::HiRes=time -e 'printf "%.9f\n", time')
    builtin_compression_time=$(echo "($end_time - $start_time) * 1000" | bc -q -l)

    start_time=$(perl -MTime::HiRes=time -e 'printf "%.9f\n", time')
    uncompress -c < "temp.BUILTIN.COMPRESS.Z" > "temp.BUILTIN.OUT"
    end_time=$(perl -MTime::HiRes=time -e 'printf "%.9f\n", time')
    builtin_decompression_time=$(echo "($end_time - $start_time) * 1000" | bc -q -l)

    builtin_total_time=$(echo "($builtin_compression_time + $builtin_decompression_time)" | bc -q -l)
    builtin_compressed_size=$(wc -c < "temp.BUILTIN.COMPRESS.Z")
    builtin_compression_ratio=$(echo "scale=2; $original_size / $builtin_compressed_size" | bc -q)

    rm "temp.BUILTIN.COMPRESS.Z"
    rm "temp.BUILTIN.OUT"

    # Next, we use the custom implementation
    start_time=$(perl -MTime::HiRes=time -e 'printf "%.9f\n", time')
    ./compress "-m $mbits" < "$filename" > "temp.CUSTOM.COMPRESS"
    end_time=$(perl -MTime::HiRes=time -e 'printf "%.9f\n", time')
    custom_compression_time=$(echo "($end_time - $start_time) * 1000" | bc -q -l)

    start_time=$(perl -MTime::HiRes=time -e 'printf "%.9f\n", time')
    ./decompress < "temp.CUSTOM.COMPRESS" > "temp.CUSTOM.OUT"
    end_time=$(perl -MTime::HiRes=time -e 'printf "%.9f\n", time')
    custom_decompression_time=$(echo "($end_time - $start_time) * 1000" | bc -q -l)

    custom_total_time=$(echo "($custom_compression_time + $custom_decompression_time)" | bc -q -l)
    custom_compressed_size=$(wc -c < "temp.CUSTOM.COMPRESS")
    custom_compression_ratio=$(echo "scale=2; $original_size / $custom_compressed_size" | bc -q)

    compression_increase=$(echo "($custom_compression_ratio - $builtin_compression_ratio) / $builtin_compression_ratio" | bc -q -l)
    time_increase=$(echo "($custom_total_time - $builtin_total_time) / $builtin_total_time" | bc -q -l)
    total_compression_increase=$(echo "$total_compression_increase + $compression_increase" | bc -q -l)
    total_time_increase=$(echo "$total_time_increase + $time_increase" | bc -q -l)

    if cmp "temp.CUSTOM.OUT" "$filename"; then
        echo -e "\n\033[1mFile: $(basename "$filename")\033[0m"
        echo -e "Status: \033[1;32mSuccess\033[0m"
        echo -e "\033[33mCompression Ratio\033[0m: $(echo $custom_compression_ratio | sed 's/^\./0./')     \033[33mBuiltin Compression Ratio\033[0m: $(echo $builtin_compression_ratio | sed 's/^\./0./')"
        echo -e "\033[33mTime\033[0m: $custom_total_time ms       \033[33mBuiltin Time\033[0m: $builtin_total_time ms\n"
        num_correct=$((num_correct + 1))
    else
        echo -e "\n$(basename "$filename") \033[1mStatus:\033[0m \033[1;31mFailure\033[0m\n"
        if [ "$break_on_error" = true ]; then
            rm "temp.CUSTOM.COMPRESS"
            rm "temp.CUSTOM.OUT"
            break
        fi
    fi
    rm "temp.CUSTOM.COMPRESS"
    rm "temp.CUSTOM.OUT"

    if [ "$dbg" = true ]; then
        if diff "DBG.compress" "DBG.decompress"; then
            echo -e "\033[1;32mString tables match\033[0m"
        fi
    fi
done
echo -e "\033[1mAGGREGATE RESULTS\033[0m"
echo -e "====================================="
echo -e "\033[1mAccuracy\033[0m: $(echo "scale=4; $num_correct / $file_count * 100" | bc -q)%"
echo -e "\033[1mAverage Compression Increase\033[0m: $(echo "scale=4; $total_compression_increase / $file_count * 100" | bc -q)%"
echo -e "\033[1mAverage Time Increase\033[0m: $(echo "scale=4; $total_time_increase / $file_count * 100" | bc -q)%"
