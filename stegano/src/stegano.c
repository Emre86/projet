#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "image.h"





int stegano (char *file_image1,char *file_image2,char *file_texte){
 
  /* Definition des descripteur de fichier 
     Declaration des variables locales

   */

  FILE *lect_image,*lect_texte,*ecrit_image; 
  char buffer[BUFFER_LENGHT];
  char buffer1[BUFFER_LENGHT];
  pixel *tableau;
  int err=0;
  int flag=0;
  int i=0;
  int j=0;
  int largeur=DEFAULT,hauteur=DEFAULT,rgb=DEFAULT;
  int r;
  int g;
  int b;
  char carac;
  int six=6;
  char pointeur_null=(char)((int)NULL);
  struct stat sts;
  long unsigned l;
  int taille_texte,taille_image;
  char ack=(char)six;

 /*On recupere la taille du texte */   
  if ( stat (file_texte, & sts) != 0) { 
    fprintf (stderr, "%s : erreur\n",file_texte);
    exit (1); 
  } 
   
  l=sts.st_size;
  taille_texte=(int)l;

  /*On ouvre l image source a steganographier */
  lect_image=fopen(file_image1,"r"); 

  if ( lect_image == NULL){
    fprintf(stderr,"Erreur ouverture du fichier %s",file_image1); 
    return -1;
  }

  /*On ouvre l image destinataire */

  ecrit_image=fopen(file_image2,"w"); 

  if ( ecrit_image == NULL){ 
    fprintf(stderr,"Erreur ouverture du fichier %s",file_image2);
    fclose(lect_image); 
    return -1; 
  }

  /* On ouvre le fichier texte */
  lect_texte=fopen(file_texte,"r");

  if ( lect_texte == NULL ){ 
    fprintf(stderr,"Erreur ouverture du fichier %s",file_texte);
    fclose(lect_image); 
    fclose(ecrit_image);
    return -1; 
  }

  /* On recupere les en-tete de l image source */

  fgets(buffer,BUFFER_LENGHT,lect_image);

  fgets(buffer1,BUFFER_LENGHT,lect_image);

  if ( (err=fscanf(lect_image,"%d %d",&largeur,&hauteur))!=2){
     fprintf(stderr,">>>> erreur largeur hauteur\n");
  }
  taille_image=largeur*hauteur;
  
  /* on verifie si l image peux accueillir le texte */
  if (taille_image-1 < taille_texte){
    fprintf(stderr,"Fichier texte trop volumineux\n");
    fclose(lect_image);
    fclose(lect_texte);
    fclose(ecrit_image);
    unlink(file_image2);
    return -1;
  }


  if ( (err=fscanf(lect_image,"%d",&rgb))!=1){
    fprintf(stderr,">>>> erreur rgb\n");
  }
 
  /* On alloue la taille necessaire pour recuperer l image produite */
  tableau=(pixel *)malloc(largeur*hauteur*sizeof(pixel));
  
  /*On indique le debut des operations */
  printf("Debut de la stenographie\n");

  /* On lie le premier pixel et on fixe le marqueur de debut ACK
     pour signifier le faite que cette image est stenographie */

  fscanf(lect_image,"%d",&r);
  fscanf(lect_image,"%d",&g);
  fscanf(lect_image,"%d",&b);
  tableau[i]=init_pixel(0,0,0,ack);
  i++;

  /*On remplie le tableau temporaire de l image 
    en fixant le marqueur de fin equivalent au marqueur de debut ACK 
    lorque nous sommes en fin de texte */ 


  while ((fscanf(lect_image,"%d",&r)) != EOF){
    fscanf(lect_image,"%d",&g);
    fscanf(lect_image,"%d",&b);
    
    if (!feof(lect_texte)){
  
      fread(&carac,sizeof(char),1,lect_texte);
  
      tableau[i]=init_pixel(r,g,b,carac);
  
    }
    else{
      if (flag == 0){
	tableau[i]=init_pixel(0,0,0,ack);
	flag=1;
      }
      else{
      carac=pointeur_null;
      tableau[i]=init_pixel(r,g,b,carac);
      }
    }
  
    i++;

  }
 
 
  /* On recopie la nouvelle image */
  

  fprintf(ecrit_image,"%s",buffer);
  fprintf(ecrit_image,"%s",buffer1);
  fprintf(ecrit_image,"%d %d\n",largeur,hauteur);
  fprintf(ecrit_image,"%d\n",rgb);

  while(j < i){
    
    fprintf(ecrit_image,"%d\n",tableau[j].red);
    fprintf(ecrit_image,"%d\n",tableau[j].green);
    fprintf(ecrit_image,"%d\n",tableau[j].blue);
    j++;
  }
	  

 
  free(tableau);
  fclose(lect_image);
  fclose(lect_texte);
  fclose(ecrit_image);

  /* On signifie que tout s est bien passé*/
  printf("Fin de stenographie\n");
  return 0;

}


