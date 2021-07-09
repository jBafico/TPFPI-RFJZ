Hola buenas, en este .txt se detallaran los pasos a seguir
para poder generar los ejecutables y luego ejecutarlos
de la forma correcta. Es de vital importancia que todos
los archivos del comprimido se mantengan juntos en el
mismo directorio a todo momento

1)Abrir la terminal y estar ubicado en el mismo directorio
en el cual se encuentran los archivos que estaban dentro
de este comprimido.

2)Ingresar a la terminal lo siguiente: make front.o
(Esto generara el .o del front)

3)Ingresar a la terminal lo siguiente: make back.o
(Esto generara el .o del back)

4)Una vez generados front.o y back.o ingresar a la
terminal lo siguiente: make all
(Esto generara el ejecutable completo llamado imdb)

5)Al ya tener el ejecutable imdb, ya puede usarlo
con su .csv, para eso ingrese lo siguiente a la
terminal: ./imdb (NOMBRE DEL .csv)
Es importante que el .csv ingresado este en el
mismo directorio que el ejecutable

Al hacer el paso anterior ya se le crearon las
4 queries que genera el programa

OPCIONAL 6)Si usted decide querer remover los .o
y el ejecutable imdb que genero el archivo puede hacerlo
escribiendo en la terminal lo siguiente: make clean
