#include <stdio.h>
#include <stdlib.h>
#include "image.h"


int main (int argc , char **argv){

  /* On verifie le nombre d argument */ 
  if (argc != 4 ){
    fprintf(stderr,"Erreur sur le nombre d argument\n");
    return -1;
  }
  /* on appel la fonction de stegano */

  stegano(argv[1],argv[2],argv[3]);
  return 0;

}
