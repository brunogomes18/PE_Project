all: tinyray_base other_base

tinyray_base: tinyray_base.c
	gcc tinyray_base.c -lm -o tinyray_base 

other_base: other_base.c
	gcc other_base.c -lm -o other_base 

clean: 
	$(RM) tinyray_base other_base