/*
 *==============================================================================
 *345678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 *   File:  structure.h
 *
 *   Part of a program to convert an MSTS ace file to bitmap format
 *
 *   Released under licence GPL-3.0-or-later
 *
 *==============================================================================
 */

char *zr_basename2(char *fname) ;

//  Temporary file for zlib

char ztemp[] = "ztempXXXXXX" ;
int  l_tmp = 0 ;
FILE *fp_tmp   ;


enum colour_type{
    MASK  = 2,
    RED   = 3,
    GREEN = 4,
    BLUE  = 5,
    ALPHA = 6 } ;

#define GL_RGBA					0x1908
#define GL_UNSIGNED_BYTE			0x1401
typedef unsigned int    uint      ;
typedef unsigned int	GLenum    ;
typedef unsigned char	GLboolean ;
typedef unsigned int	GLbitfield;
typedef void		GLvoid;
typedef signed char	GLbyte;		/* 1-byte signed */
typedef short		GLshort;	/* 2-byte signed */
typedef int		GLint;		/* 4-byte signed */
typedef unsigned char	GLubyte;	/* 1-byte unsigned */
typedef unsigned short	GLushort;	/* 2-byte unsigned */
typedef unsigned int	GLuint;		/* 4-byte unsigned */
typedef int		GLsizei;	/* 4-byte signed */
typedef float		GLfloat;	/* single precision float */
typedef float		GLclampf;	/* single precision float in [0,1] */
typedef double		GLdouble;	/* double precision float */
typedef double		GLclampd;	/* double precision float in [0,1] */

char    eof_mark[] = "******Z" ;   // Use to flag end-of-file in text files.


/**
 *  MSblock structure.  MSTS files are structured as a series of
 *  binary or text blocks, each of which can contain a mixture
 *  of data and more blocks.  The MSblock structure is used to
 *  keep track of the current position and level in the
 *  overall structure.  For binary files it also keeps the
 *  position of the end byte of the current block at each level.
 */


typedef struct msblock {
  char *token_name  ;              /// Text Name    (Text Files)
  int  itoken       ;              /// Token Number (Binary Files)
  int  flags        ;
  int  l_label      ;
  int  length       ;
  char *label       ;
  int  byte_end     ;
} MSblock  ;

/*
 *  MSfile structure used to save data used while reading each
 *  MSTS format file whether text or binary
 */

typedef struct msfile {              // MSTS files
  FILE *fp           ;                 // Standard file pointer
  char *filename     ;                 // File name
  int  unicode       ;                 // Unicode file
  int  ascii         ;                 // Ascii file
  int  compress      ;                 // Compressed file (gzip?)
  int  text          ;                 // Text file
  int  binary        ;                 // Binary file
  int  world         ;                 // World file
  int  texture       ;                 // Texture file
  char *token_unused ;                 // Token from text file awaiting use
  MSblock level[20]  ;                 // Information on level
} MSfile ;



/*
 *==============================================================================
 *  TextureNode  Structure
 *  This stores information on the texture image
 *
 *  NOTE:  The MSTS ACE file surface format often differs from the normal
 *         Microsoft convention
 *==============================================================================
 */

typedef struct texturenode {
  struct texturenode *next            ;  //  Pointer to the next node
  char               *name            ;  //  Name without '.ace'
  char               *filename        ;  //  Full filename
  int                basic            ;  //  = 1 if alwauys needed
  int                needed           ;  //  True if needed for graphics
  int                loaded           ;  //  True if loaded into graphics card
  int                width            ;  //  Width of base texture
  int                height           ;  //  Height of base texture
  int                is_alpha         ;  //  True if file has 8-bit alpha channel
  int                is_mask          ;  //  True if file has 1-bit mask channel
  int                alpha_is_mask    ;  //  True if alpha used as mask
  int                n_textures       ;  //  Number of mipmaps (1 if base only)
  GLubyte            **texture        ;  //  Pointers to mipmaps in memory
  int                surface_format   ;  //  MSTS ACE file 'surface format'
  int                surface_format_orig  ;  //  Original
  GLenum             gl_mem_format    ;  //  GL_RGB, GL_RGBA etc
  GLenum             gl_mem_packing   ;  //  GL_UNSIGNED_BYTE etc
  GLuint             gl_tex_ref_no    ;  //  Output from glGenTexture (openGL)
} TextureNode ;

