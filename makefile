Debug:
	gcc-9 -g3 -pipe -Wall -o tp tp.cpp -lsfml-audio -lsfml-system -lstdc++

release:
	gcc-9 -O3 -o tp tp.cpp -lsfml-audio -lsfml-system -lstdc++

clean:
	rm -f $(obj) game
