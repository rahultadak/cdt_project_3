make clean
make

./sim 16 4 0 0 0 0 0 traces/val_gcc_trace_mem.txt > val_1.txt
diff -iw val_1.txt val_runs/val_1.txt

./sim 32 16 0 0 0 0 0 traces/val_perl_trace_mem.txt > val_2.txt
diff -iw val_2.txt val_runs/val_2.txt

./sim 16 4 32 2048 8 0 0 traces/val_gcc_trace_mem.txt > val_extra_1.txt
diff -iw val_extra_1.txt val_runs/val_extra_1.txt

./sim 32 8 32 1024 4 2048 8 traces/val_perl_trace_mem.txt > val_extra_2.txt
diff -iw val_extra_2.txt val_runs/val_extra_2.txt
