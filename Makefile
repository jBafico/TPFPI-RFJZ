COMPILER=gcc
OUTPUT_FILE=imdb
BACK_FILES= imdbTAD.c
FRONT_FILES= imdbMain.c
HEADER_FILES= imdbTAD.h
FLAGS=-pedantic -std=c99 -Wall -fsanitize=address

all: back.o front.o
	$(COMPILER) -o $(OUTPUT_FILE) front.o back.o $(FLAGS)

front.o: $(FRONT_FILES) $(HEADER_FILES)
	$(COMPILER) -c $(FRONT_FILES) $(HEADER_FILES) -o front

back.o: $(BACK_FILES) $(HEADER_FILES)
	$(COMPILER) -c $(BACK_FILES) $(HEADER_FILES) -o back

clean:
	rm -r $(OUTPUT_FILE) *.o
