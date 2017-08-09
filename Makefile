all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -std=c++11 -o sample2D Sample_GL3_2D.cpp glad.c -lGL -lao -lmpg123 -lglfw -ldl -lpthread

clean:
	rm sample2D
