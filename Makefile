all: tinyray_base tinyray_times tinyray_algo tinyray_openmp tinyray_opt1 tinyray_opt2 tinyray_opt3 tinyray_cuda_base

tinyray_base: tinyray_base.c
	gcc tinyray_base.c -lm -o tinyray_base 

tinyray_opt1: tinyray_inline.c
	gcc tinyray_inline.c -lm -o tinyray_opt1

tinyray_opt2: tinyray_inline_2.c
	gcc tinyray_inline_2.c -lm -o tinyray_opt2

tinyray_opt3: tinyray_inline_3.c
	gcc tinyray_inline_3.c -lm -o tinyray_opt3

tinyray_times: tinyray_times.c
	gcc tinyray_times.c -lm -o tinyray_times 


tinyray_algo: tinyray_algo.c
	gcc tinyray_algo.c -lm -o tinyray_algo 


tinyray_openmp: tinyray_openmp.c
	gcc tinyray_openmp.c -O3 -march=native -Wall -fopenmp -lm -o tinyray_openmp 

tinyray_cuda_base: tinyray_cuda_base.cu
	nvcc -O3 --expt-relaxed-constexpr tinyray_cuda_base.cu -o tinyray_cuda_base 

clean: 
	$(RM) tinyray_base tinyray_times tinyray_algo tinyray_simd tinyray_openmp tinyray_cuda_base
