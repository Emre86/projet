#include <stdio.h>
#include <stdlib.h>
#include "image.h"

/* initialisation d un pixel avec le codage d un caractere */

pixel init_pixel(int red,int green,int blue,char carac){
  
  pixel p;
  /* On met 2 bits du caracteres dans le rouge,
      3 bits dans le vert et le bleu 
   */ 
  p.red = ( (red & (~3)) | ((carac>>6) & 3) ) ;
  p.green =( ( green & (~7)) |  ( (carac&63) >> 3 )) ;
  p.blue = ( ( blue & (~7)) | (carac&7) );

  return p;
}
