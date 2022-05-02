all: tinyray_base tinyray_times

tinyray_base: tinyray_base.c
	gcc tinyray_base.c -lm -o tinyray_base 

tinyray_times: tinyray_times.c
	gcc tinyray_times.c -lm -o tinyray_times 

clean: 
	$(RM) tinyray_base tinyray_times