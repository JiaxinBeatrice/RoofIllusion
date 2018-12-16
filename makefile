maze:
	rm -rf roof && g++ src/main.cpp -x c glad/glad.c -g -F/Library/Frameworks -framework SDL2 -framework OpenGL -framework SFML -o roof -lsfml-system -lsfml-audio && ./roof
