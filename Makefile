forest:		forest.cpp
		g++ -framework OpenGL -framework GLUT forest.cpp -o forest -I. -Wno-deprecated
clean:
	rm -f forest