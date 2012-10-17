#include <stdio.h>
#include <stdlib.h>
#include "image.h"

/*On recupere les bits des caracteres cache dans l'image */

char recover_texte(int red,int green,int blue){
  char c;
  int m,n,p;
  m = red & 3 ;
  n = green & 7 ;
  p = blue & 7 ;
  
  c= ( (((char)(m<<6)) | ((char)(n<<3))) | ((char)p));

  return c;
}
