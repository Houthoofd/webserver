all:
	g++ -o run.exe *.cpp src/*.cpp -Iincludes -lws2_32