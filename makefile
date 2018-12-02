maze:
	rm -rf roof && g++ src/main.cpp -x c glad/glad.c -g -F/Library/Frameworks -framework SDL2 -framework OpenGL -o roof && ./roof
