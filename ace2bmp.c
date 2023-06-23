/*
 *==============================================================================
 *345678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 *   File:  ace2bmp.c
 *
 *   Program to convert MSTS ace file to bitnap format
 *
 *   Released under licence GPL-3.0-or-later
 *
 *==============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <zlib.h>
#include <assert.h>                      // For zlib

#include "struct.h"

char  file_in[1024] ;
char  file_ot[1024] ;

/*
 *  Specify functions
 */

int  process_args(int argc, char **argv) ;
int  init_texture_node(TextureNode *texture) ;
int  load_texture(TextureNode *tnode)    ;
void generateBitmapImage (TextureNode *tnode, char* fname) ;
int  convert_texture(TextureNode *tnode) ;
int  dxt1_2_rgba(unsigned int ia[2], unsigned char *ja) ;

/*
 *==============================================================================
 *  Main program
 *==============================================================================
 */
int main(int argc, char **argv){

  TextureNode tnode ;

      process_args(argc, argv) ;

      init_texture_node(&tnode) ;

      tnode.filename = (char *)malloc(strlen(file_in)*sizeof(char)) ;
      strcpy(tnode.filename, file_in) ;

      load_texture(&tnode) ;
      convert_texture(&tnode) ;
      generateBitmapImage (&tnode, file_ot) ;

      if(l_tmp) remove(ztemp)  ;

      return 0;
}

/*
 *==============================================================================
 * Process program arguments
 *==============================================================================
 */

int process_args(int argc, char **argv){
char *fp ;

      if(argc < 2){
        printf("  Program ace2bmp needs one ir two arguments\n") ;
        printf("  ... the name of the ace input file ... \n") ;
        printf("  ... and the name of the bitmap output file\n") ;
        printf("  If the second name is missing, a suitable name"
                                             " is generated.\n") ;
        printf("  Program stopping\n") ;
        exit(0) ;
      }
      sscanf(argv[1],"%s",file_in);
      if(argc > 2){
        sscanf(argv[2],"%s",file_ot) ;
      }else{
        fp = zr_basename2(file_in) ;
        strcpy(file_ot, fp) ;
        free (fp)       ;
        strcat(file_ot,".bmp") ;
      }
//      printf("  Input file  = '%s'\n",file_in) ;
//      printf("  Output file = '%s'\n",file_ot) ;
      return 0 ;
}
#include "token.c"
#include "token_b.c"
#include "read_ace.c"
#include "rgb2bmp.c"
