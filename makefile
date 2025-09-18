watershed: watershed.o pointcloud.o util.o bmp.o
	gcc -g watershed.o pointcloud.o util.o bmp.o -o watershed

watershed.o: watershed.c pointcloud.h util.h bmp.h
	gcc -c -g watershed.c

pointcloud.o: pointcloud.c pointcloud.h bmp.h util.h
	gcc -c -g pointcloud.c

util.o: util.c util.h
	gcc -c -g util.c

bmp.o: bmp.c bmp.h
	gcc -c -g bmp.c

clean:
	rm -f pointcloud.o util.o bmp.o watershed.o watershed