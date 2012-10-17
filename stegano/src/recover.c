#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "image.h"





int recover (char *file_image,char *file_texte){
  
  /* Definition des descripteur de fichier 
     Declaration des variables locales

   */



  FILE *lect_image,*ecrit_texte; 
  char buffer[BUFFER_LENGHT];
  char buffer1[BUFFER_LENGHT];
  int err=0;
  int largeur,hauteur,rgb;
  int r,g,b;
  char texte[1];
  int six=6;
  char ack=(char)six;

  /*On ouvre le fichier image en lecture */
  lect_image=fopen(file_image,"r"); 

  if ( lect_image == NULL){
    fprintf(stderr,"Erreur ouverture du fichier %s",file_image); 
    return -1;
  }
  
  /* On ouvre le fichier texte en ecriture */
  ecrit_texte=fopen(file_texte,"w"); 

  if ( ecrit_texte == NULL){ 
    fprintf(stderr,"Erreur ouverture du fichier %s",file_texte);
    fclose(lect_image); 
    return -1; 
  }

  
  /* On lit les en-tetes de l image */
  fgets(buffer,BUFFER_LENGHT,lect_image);

  fgets(buffer1,BUFFER_LENGHT,lect_image);

  if ( (err=fscanf(lect_image,"%d %d",&largeur,&hauteur))!=2){
     fprintf(stderr,">>>> erreur largeur hauteur\n");
  }


  if ( (err=fscanf(lect_image,"%d",&rgb))!=1){
    fprintf(stderr,">>>> erreur rgb\n");
  }
  /* On verifie si l' image est stenographie */

  fscanf(lect_image,"%d",&r);
  fscanf(lect_image,"%d",&g);
  fscanf(lect_image,"%d",&b);
  texte[0]=recover_texte(r,g,b);
  if(texte[0]!=ack){
    printf("Image non stenographié\n");
    fclose(ecrit_texte);
    fclose(lect_image);
    unlink(file_texte);
    return -1;
  }

  /* On debute les operations d extraction de texte
     jusqu'au marqueur de fin ACK */
  printf("Debut de recover\n");

  while ((fscanf(lect_image,"%d",&r)) != EOF){
    fscanf(lect_image,"%d",&g);
    fscanf(lect_image,"%d",&b);
    texte[0]=recover_texte(r,g,b);
    if (texte[0]==ack){
      break;
    }
    else{
      fwrite(texte,sizeof(char),1,ecrit_texte);
    }
  }
 


  
  fclose(lect_image);
  fclose(ecrit_texte);
  
  /* On indique que tout s'est bien passé */
  printf("Fin de recover \n");
  return 0;

}


