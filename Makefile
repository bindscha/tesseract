.PHONY:all clean

ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

all:tesseract libtesseract# motif_counting  k-clique #triangle_count#clique_ooc_dfs motif_counting triangle_count triangle_dfs
#motif_counting_bfs motif_counting clique_ooc_dfs clique_ooc triangle_dfs k-clique
INCLUDES=utils.cpp graph.cpp
FLAGS=g++ -D_FILE_OFFSET_BITS=64  -D_FILE_OFFSET_BITS=64 -fcilkplus -lcilkrts -DCILK -pthread -fms-extensions -ggdb3 -std=c++11 -pthread -fopenmp -U_FORTIFY_SOURCE -O3 -fno-stack-protector
#triangle_counting tc_ooc tc_ooc_pthread clique_ooc clique_ooc_dfs triangle_dfs

libtesseract: libtesseract.h libtesseract.cpp
	 $(FLAGS) $(INCLUDES) -shared -fPIC -g libtesseract.h libtesseract.cpp -o libtesseract.so

install: libtesseract
	 install -d $(DESTDIR)$(PREFIX)/lib/
	 install -m 644 libtesseract.so $(DESTDIR)$(PREFIX)/lib/
	 install -d $(DESTDIR)$(PREFIX)/include/
	 install -m 644 libtesseract.h $(DESTDIR)$(PREFIX)/include/

triangle_counting: *.hpp triangleCounting.cpp
	 $(FLAGS) $(INCLUDES) triangleCounting.cpp -o triangle_counting

tesseract: *.hpp *.cpp tesseract.cpp
	 $(FLAGS) $(INCLUDES) tesseract.cpp -o tesseract

triangle_count: *.hpp triangleCounting.cpp
	 $(FLAGS) $(INCLUDES) triangleCounting.cpp -o triangle_count


motif_counting: *.hpp $(INCLUDES) motif_counting.cpp
	 $(FLAGS) $(INCLUDES) motif_counting.cpp -o motif_counting
motif_counting_bfs: motif_counting_BFS.cpp
	 $(FLAGS) $(INCLUDES) motif_counting_BFS.cpp -o motif_counting_bfs

tc_ooc: triangle_counting_ooc.cpp
	$(FLAGS) $(INCLUDES) triangle_counting_ooc.cpp -o tc_ooc


#tc_ooc_pthread: tc_smart_ooc.cpp
#	$(FLAGS) $(INCLUDES) triangle_counting_ooc.cpp -o tc_ooc_pthread

triangle_dfs: triangle_dfs.cpp
	$(FLAGS) $(INCLUDES) triangle_dfs.cpp -o triangle_dfs


clique_ooc: clique_ooc.cpp
	$(FLAGS) $(INCLUDES) clique_ooc.cpp -o clique_ooc

clique_ooc_dfs: clique_ooc_DFS.cpp
	$(FLAGS) $(INCLUDES) clique_ooc_DFS.cpp  -o clique_ooc_dfs


k-clique: *.hpp k-clique.cpp
	$(FLAGS) $(INCLUDES) k-clique.cpp  -o k-clique

clean:
	rm -f tesseract #motif_counting_bfs motif_counting triangle_dfs clique_ooc clique_ooc_dfs k-clique triangle_count
#triangle_counting tc_ooc tc_ooc_pthread

