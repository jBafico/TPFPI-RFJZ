COMPILER=gcc
OUTPUT_FILE=imdb
BACK_FILES= imdbTAD.c
FRONT_FILES= imdbMain.c
HEADER_FILES= imdbTAD.h
FLAGS_COMPL=-pedantic -std=c99 -Wall
FLAGS_LINK= -fsanitize=address

all: back.o front.o
	$(COMPILER) -o $(OUTPUT_FILE) front.o back.o $(FLAGS_LINK)

front.o: $(FRONT_FILES) $(HEADER_FILES)
	$(COMPILER) -c $(FRONT_FILES) $(FLAGS_COMPL) -o front.o

back.o: $(BACK_FILES) $(HEADER_FILES)
	$(COMPILER) -c $(BACK_FILES) $(FLAGS_COMPL) -o back.o

clean:
	rm -r $(OUTPUT_FILE) *.o
