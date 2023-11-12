#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "imageProc.c"

int main( int argc, char **argv )
{
  Image img;

  readPNG( &img, "Cat.png");;
  writePNG( &img, "Cat-output.png");

}
