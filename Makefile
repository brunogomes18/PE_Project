all: tinyray_base tinyray_times tinyray_algo tinyray_openmp

tinyray_base: tinyray_base.c
	gcc tinyray_base.c -lm -o tinyray_base 

tinyray_times: tinyray_times.c
	gcc tinyray_times.c -lm -o tinyray_times 


tinyray_algo: tinyray_algo.c
	gcc tinyray_algo.c -lm -o tinyray_algo 


tinyray_openmp: tinyray_openmp.c
	gcc tinyray_openmp.c -O3 -march=native -Wall -fopenmp -lm -o tinyray_openmp 

clean: 
	$(RM) tinyray_base tinyray_times tinyray_algo tinyray_simd tinyray_openmp