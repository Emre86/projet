#include <stdio.h>
#include <stdlib.h>
#include "image.h"


int main (int argc , char **argv){

  /* On verifie le nombre d argument */

  if (argc != 3 ){
    fprintf(stderr,"Erreur sur le nombre d argument\n");
    return -1;
  }

  /* appel a la fonction de recover */
  recover(argv[1],argv[2]);
  return 0;

}
