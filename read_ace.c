/*
 *==============================================================================
 *345678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 *   File:  read_ace.c
 *
 *   Part of a program to convert an MSTS ace file to bitmap format
 *
 *   Released under licence GPL-3.0-or-later
 *
 *==============================================================================
 */

int load_texture(TextureNode *tnode){

  int   i, j, k, l, m, n ;
  int   ip = 0   ;  // 0 = no printing
  int   width     ;
  int   height    ;
  int   mipmaps   ;
  int   raw_data  ;
  int   options   ;
  int   surface_format ;
  int   channel_count  ;
  int   image_count    ;
  char  signature[5]   ;
  int   icol           ;
  char  c ;

  MSfile  msfile ;
  FILE   *fp ;
  char    myname[] = "load_texture" ;

/*
 *  open_msfile reads and checks the first 16 bytes of the texture file
 *  and sets the compress flag.
 */
      if(ip){
        printf("\n  Read texture\n") ;
        printf(  "    == filename : %s\n",tnode->filename) ;
      }
/*
 *  Call open_msfile with third parameter = 1 to indicate a texture file.
 */
      l = open_msfile(tnode->filename, &msfile, 1, 0);

      if(l!=0){
        printf("\n\n  ERROR : Routine open_msfile failed to open file\n\n");
        exit(1) ;
      }
      if(ip)printf(" msfileptr  = %p\n",(void *)msfile.fp);
      fp = msfile.fp ;
/*
 *  Check file signature
 */
      for(i=0;i<4;i++) signature[i]  = (char)getc(fp);
      signature[4] = (char)'\0' ;
      if(0 != strcmp("\x01\x00\x00\x00",signature)){
        printf("  Routine %s. Wrong signature.\n", myname);
        printf("    Signature is %x %x %x %x \n",
                        signature[0],signature[1],signature[2],signature[3]);
        printf("    Program stopping ... \n");
        exit(1) ;
      }
      if(ip)printf("    Signature is %x %x %x %x \n",
                            signature[0],signature[1],signature[2],signature[3]);
/*
 *  Read basic data
 */
      options        = read_int32(fp) ;
      tnode->width   = read_int32(fp) ;
      tnode->height  = read_int32(fp) ;
      tnode->surface_format = read_int32(fp) ;
      channel_count  = read_int32(fp) ;
      width          = tnode->width  ;
      height         = tnode->height ;
      surface_format = tnode->surface_format ;
      if(ip){
        printf("    options = %8.8x \n",options);
        printf("    width   = %i \n",tnode->width);
        printf("    height  = %i \n",tnode->height);
        printf("    surface_format = %i %x\n",tnode->surface_format,
                                              tnode->surface_format);
        printf("    channel_count  = %i \n",channel_count);
      }
/*
 *  Skip unwanted data
 */
      for(i=0;i<128;i++) getc(fp) ;
/*
 *  Check for mipmaps.
 *  If present check that the image is square and each dimension
 *  is an integral power-of-two.
 */
      image_count = 1 ;
      mipmaps = options & 0x01 ;
      if(mipmaps){
        if(ip)printf("  File contains mipmaps\n");
        if(width != height){
          printf("  Texture file : tnode->name\n");
          printf("    contains mipmaps but the height and with do not match.\n") ;
          printf("    width = %i,  height = %i.\n", width, height) ;
          printf("  Routine %s exiting without loading texture.\n",myname) ;
          close_msfile(&msfile) ;
          free(tnode->name)  ;
          tnode->name = NULL ;
          return 1  ;
        }
        if((width & (width-1)) != 0){
          printf("  Texture file : %s\n",tnode->name);
          printf("    contains mipmaps but the width is not a power of 2.\n") ;
          printf("    width = %i,  height = %i\n", width, height) ;
          printf("  Routine %s exiting without loading texture\n",myname) ;
          close_msfile(&msfile) ;
          free(tnode->name)  ;
          tnode->name = NULL ;
          return 1  ;
        }
        for(i=width,image_count = 0;i>0;i=i>>1,image_count++) ;
        if(ip)printf(" image_count = %i\n",image_count) ;
      }
/*
 *  Check for raw data file
 */
      raw_data = (0x10 & options) != 0  ;
      if(raw_data && ip){
        printf("  File contains raw data\n");
      }
/*
 *  Check that surface format is supported
 */
      if(surface_format != 0x0e && surface_format != 0x10 &&
         surface_format != 0x11 && surface_format != 0x12){
        printf("  Texture file : '%s.ace',      surface format option = %x."
               "  Option not supported.\n", tnode->name, surface_format);
        printf("    options = %8.8x \n",options);
        printf("    width   = %i \n",tnode->width);
        printf("    height  = %i \n",tnode->height);
        printf("    surface_format = %i %x\n",tnode->surface_format,
                                              tnode->surface_format);
        printf("    channel_count  = %i \n",channel_count);
        printf("    raw_data       = %i \n",raw_data);
        printf("    image_count    = %i \n",image_count);
        close_msfile(&msfile) ;
        free(tnode->name)  ;
        tnode->name = NULL ;
        return 1;
      }
/*
 *  Check channel count
 */
int  size[10] ;
enum colour_type type[10] ;      //  Allow for maximum of 10

      if(channel_count>10){
        printf("  Routine %s, Channel_count (= %i) exceeds limit of 10\n",myname, (int)channel_count);
        return 1 ;
      }
/*
 *  Read channel data
 */
int   is_alpha = 0;
int   is_mask  = 0;
      for(i=0;i<channel_count;i++){
        size[i] = read_int64(fp) ;
        type[i] = read_int64(fp) ;
//        if(type[i] == colour_type(ALPHA)) is_alpha = 1 ;
//        if(type[i] == colour_type(MASK))  is_mask  = 1 ;
        if(type[i] == ALPHA) is_alpha = 1 ;
        if(type[i] == MASK)  is_mask  = 1 ;
        if(ip)printf(" Channel %i,  size = %i, type = %i\n",i,size[i],type[i]);
      }
      tnode->is_alpha = is_alpha ;
      tnode->is_mask  = is_mask  ;
      if(ip)printf(" Flags  raw_data = %i  mipmap = %i  alpha = %i  mask = %i  s_format = %2x  channels = %i,  image_count = %i, compress = %i, filename = %s\n",
           raw_data, mipmaps, is_alpha, is_mask, surface_format,channel_count,
           image_count, msfile.compress, tnode->filename);
/*
 *  Create texture pointer array
 *  This is of length one - unless mipmaps is set.
 */
//  Free any allocated memory
      if(tnode->n_textures != 0){
        for(i=0;i<tnode->n_textures;i++) free(tnode->texture[i]) ;
        free(tnode->texture)  ;
        tnode->n_textures = 0 ;
      }
//  Allocate new texture pointer array
      tnode->texture = (GLubyte **)malloc(image_count*sizeof(GLubyte **)) ;
      tnode->n_textures = image_count ;
      for(i=0;i<image_count;i++)tnode->texture[i] = NULL ;
/*
 *==============================================================================
 *   Read Raw data
 *
 *   'Raw Data' is used by MSTS to store 0x12 (Compressed Dxt1) images
 *   Dxt1 compuression uses 64 bits to store colours in a 4x4 block of pixels
 *   First 32 bits contains 2 colours stored as RGB in 5, 6 and 5 bits.
 *
 *   All cells white = 0xFF 0xFF 0xFF 0xFF 0x00 0x00 0x00 0x00
 *   All cells black = 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
 *   All cells grey  = 0x84 0x10 0x84 0x10 0x00 0x00 0x00 0x00
 *   All cells red   = 0xF8 0x00 0xF8 0x00 0x00 0x00 0x00 0x00
 *
 *   Note:  Although Ox12 uses the above scheme, the file header usually
 *          indicates that colour data is stored as 8 bytes and mask data
 *          as 1 byte.
 *
 *==============================================================================
 *
 *     Raw data starts with a table of 32-bit integers, each giving the offset
 *       in bytes from the start of the table to the start of the corresponding
 *       mipmap.
 *     If the mipmap width is 4 or larger, the data is stored as raw
 *       data following an initial 32-bit integer giving the number of raw
 *       data elements.  In most cases (surface formats 0x0e, 0x10, 0x11, 0x12,
 *       0x14, 0x16) each element is a two byte integer.  However other
 *       formats may use four byte elements (i.e C# formats: SurfaceFormat.Colour
 *       and NormalizedByte4).
 *     Coding for the 2x2 and 1x1 mipmaps is not clear.  The OR C# code says both
 *       that it is stored as a 32-bit ARGB block and as RGB data.  Coding may
 *       depend on the format (mem_format).  Here, as it is not clear, it is
 *       set to four zero bytes.  This should either correspond to black or
 *       transparent black.
 *==============================================================================
 */
/*
 *  Read table of offsets and convert to bytes in each mipmap
 */
      if(raw_data) {
/*
 *  Check raw_data supported
 */
      if(surface_format != 0x12){
        printf("    options = %8.8x \n",options);
        printf("  Texture file : '%s.ace',      surface format option = %x,  Option not supported for raw data files.\n",
                         tnode->name, surface_format);
        printf("    width   = %i \n",tnode->width);
        printf("    height  = %i \n",tnode->height);
        printf("    surface_format = %i %x\n",tnode->surface_format,
                                              tnode->surface_format);
        printf("    channel_count  = %i \n",channel_count);
        printf("    raw_data       = %i \n",raw_data);
        printf("    image_count    = %i \n",image_count);
//        return 1;
      }
      if(ip)printf("  Reading raw data\n");

int  *tex_offset = (int *)malloc(image_count*sizeof(int)) ;
int  tex_size ;

        for(i=0;i<image_count;i++){
          tex_offset[i] = read_int32(fp) ;
        }
/*
 * read mipmaps
 */
int     w = width ;
        for(i=0;i<image_count;i++,w=w>>1){
/*
 *  Allocate memory
 */
          if(w>2){
            tex_size = read_uint32(fp);
            tnode->texture[i] = (GLubyte *)malloc(tex_size*sizeof(GLubyte));
            if(ip)printf("  Mipmap image %2i, width = %3i, size = %5i,"
                         " texture[i] = %p\n",
                         i, w,  tex_size, (void *)tnode->texture[i] ) ;

            for(j=0;j<tex_size;j++){
              c = fgetc(fp) ;
              if(feof(fp)){
                printf(" EOF while reading texture data\n");
                printf("    Texture loading aborted\n") ;
                close_msfile(&msfile) ;
                free(tnode->name)  ;
                tnode->name = NULL ;
                return 1;
              }
              tnode->texture[i][j] = c ;
            }
// 2x2 and 1x1 images often missing or badly formed - so set to eight zero bytes
// LATER:  Logic to detect errors and/or make averages of 4x4 image
          }else{
            if(ip)printf("  Mipmap image %2i, width = %i. Set to eight zero bytes\n",i,w);
            tnode->texture[i] = (GLubyte *)malloc(8*sizeof(GLubyte));
#if 1
            tnode->texture[i][0] = 0x84 ;  // Grey
            tnode->texture[i][1] = 0x10 ;
            tnode->texture[i][2] = 0x84 ;
            tnode->texture[i][3] = 0x10 ;
            tnode->texture[i][4] = 0x00 ;
            tnode->texture[i][5] = 0x00 ;
            tnode->texture[i][6] = 0x00 ;
            tnode->texture[i][7] = 0x00 ;
#else
            tnode->texture[i][0] = 0xF8 ;  // Red
            tnode->texture[i][1] = 0x00 ;
            tnode->texture[i][2] = 0xF8 ;
            tnode->texture[i][3] = 0x00 ;
            tnode->texture[i][4] = 0x00 ;
            tnode->texture[i][5] = 0x00 ;
            tnode->texture[i][6] = 0x00 ;
            tnode->texture[i][7] = 0x00 ;
#endif
          }
        }
        free(tex_offset) ;
/*
 *==============================================================================
 *  Read structured data
 *
 *  This starts with a table of 32 bit offsets to each scan line for each image
 *  So skip over the table
 *==============================================================================
 */
      }else{
/*
 *  Check structured data supported
 */
      if(ip)printf("  Reading structured data\n");
      if(surface_format != 0x0e && surface_format != 0x10 && surface_format != 0x11){
        printf("  Texture file : '%s.ace',      surface format = %x,  Raw data texture not supported for structured files.\n",
                         tnode->name, surface_format);
        printf("    options = %8.8x \n",options);
        printf("    width   = %i \n",tnode->width);
        printf("    height  = %i \n",tnode->height);
        printf("    surface_format = %i %x\n",tnode->surface_format,
                                              tnode->surface_format);
        printf("    channel_count  = %i \n",channel_count);
        printf("    raw_data       = %i \n",raw_data);
        printf("    image_count    = %i \n",image_count);
        for(n=0;n<channel_count;n++)printf("  channel  %i  size = %i, type = %i\n",
                                       n,size[n],type[n]);
      }
/*
 *==============================================================================
 *  Loop over 'image_count' images
 *==============================================================================
 */
int ioff ;
        m = 1 ;
        for(i=0;i<image_count;i++){                 // Number if mipmap images
          n = tnode->height/m ;
          m = m * 2 ;
          if(ip)printf("  Image %i,  width = %i\n",i,n) ;
          for(j=0;j<n;j++){
            ioff = read_int32(fp) ;    // Offsets to each scan line
            if(ip)printf("   Line %4i, offset = %4x  %i\n",j, ioff, ioff);
          }
        }

        tnode->gl_mem_format   = GL_RGBA          ;
        tnode->gl_mem_packing  = GL_UNSIGNED_BYTE ;
/*
 *  Data can arrive as RGB, RGBA or RGB and mask, where teh colours are
 *  8-bit bytes and the mask is a 1-bit field.
 *
 *  Data is thenstored internally as RGBA, using the alpha field if present
 *  or by converting mask to transparent/solid if alpha is not present
 *  or by setting alpha to solid colour (0xFF).
 */
int   w = width, nb, h, nwords ;
        nb = 4 ;
/*
 *==============================================================================
 *  Loop over mipmaps
 *==============================================================================
 */
        for(i=0;i<image_count;i++,w=w>>1){
/*
 *  Calculate size of each image.
 *  If no mipmaps allow width and height to differ.
 */
          h  = (mipmaps ? w : height) ;
//          nwords = (w*h <4) ? 4 : w*h ;         // 1x1 mipmaps need 2x2 memory
          nwords = (w*h <8) ? 8 : w*h ;         // Ensure on 8 byte boundary
//  Possible problem calloc
          tnode->texture[i] = (GLubyte *)calloc(nb*nwords,sizeof(GLubyte));
          if(ip)printf("  Memory for image %i, nb = %i, w = %i, h = %i,"
                  " nwords = %i, size = %i, texture[i] = %p\n",
                  i,nb,w,h,nwords, nb*nwords,tnode->texture[i]);
/*
 *  Loop over the scan rows
 */
          for(j=0;j<h;j++){
            if(ip)printf("  Read image %i, row %i, channel ",i,j);
/*
 *  Initialise alpha channel (for 0x0e ony??)
 */
            for(l=0;l<w;l++){
              tnode->texture[i][(j*w+l)*nb + 3] = 0xFF ;
            }
/*
 * Loop over channels
 */
            for(k=0;k<channel_count;k++){
              if(type[k] == RED){
                icol = 0 ;
              }else if(type[k] == GREEN){
                icol = 1 ;
              }else if(type[k]== BLUE ){
                icol = 2 ;
              }else{
                icol = 3 ;
              }
              if(ip)printf(" %i, %i :: ",type[k], size[k]) ;
/*
 *   8-bit data
 *   NOTE:  getc should check for EOF.
 *   EOF has the value -1 so cannot be represented in a 'char' variable.
 *   Instead either read into an 'int' or call 'feof(fp)' after every read.
 */
              if(8 == size[k]){
                  if((type[k] == MASK) && is_alpha){
                    for(l=0;l<w;l++) getc(fp); // Skip if MASK and is_alpha
                  }else{
                    for(l=0;l<w;l++){
                      tnode->texture[i][(j*w+l)*nb + icol] = getc(fp);
                    }
                  }
/*
 *   1-bit data : assume 1 => opaque, 0 = fully transparent.
 *   The data is packed 8-bits to a character, starting at the most significant bit
 */
              }else{
                if((type[k] == MASK) && is_alpha){
                  for(l=0;l<1+(w-1)/8;l++) getc(fp);   // Skip ...
                }else{
                  for(l=0;l<1+(w-1)/8;l++){
                    c = getc(fp);
                    for(m=0;m<8;m++){
                      if( m >= w - l*8 - 1) break ;
                      if(surface_format == 0x10 && 0){
                        tnode->texture[i][(j*w + l*8 + m)*4 + 3]
//                                              = 0xff ;
                                              = ((c>>(7-m)) & 1) ? 0 : 0xff ;
                      }else{
                        tnode->texture[i][(j*w + l*8 + m)*4 + 3]
                                              = ((c>>(7-m)) & 1) ? 0xff : 0 ;
                      }
                    }
                  }
                }
              }    // 8-bit or 1-bit
            }      // Loop over channels
            if(ip)printf("\n");
          }        // Loop over rows
        }          // Loop over images
      }
      close_msfile(&msfile) ;

      return 0 ;
}

/*
 *  Routine to initialise texture structure
 */

int  init_texture_node(TextureNode *texture){

        texture->filename       = NULL ;
        texture->name           = NULL ;
        texture->basic          = 0 ;
        texture->needed         = 0 ;
        texture->loaded         = 0 ;
        texture->width          = 0 ;
        texture->height         = 0 ;
        texture->is_alpha       = 0 ;
        texture->is_mask        = 0 ;
        texture->alpha_is_mask  = 0 ;
        texture->needed         = 0 ;
        texture->n_textures     = 0 ;
        texture->texture        = NULL ;
        texture->surface_format = 0 ;
        texture->gl_mem_format  = 0 ;
        texture->gl_mem_packing = 0 ;
        texture->gl_tex_ref_no  = 0 ;
      return 0 ;
}
#if 0
/*
 *  char *zr_basename2(char *fname)
 *
 *  Returns a copy of fname without the extension and with all directories
 *  removed
 *
 *  zr version of basename used because of the differences between
 *  versions of the standard basename routine.  This always returns
 *  a string but it can be the null string "".
 *  The string is created using malloc.  The space should be freed
 *  when/if no longer needed.
 */

char *zr_basename2(char *fname){

char  *pz, *string1, *string2 ;
      if(fname == NULL) return strdup("") ;
      pz = strrchr(fname,'/') ;
      if(pz == NULL){
        string1 = strdup(fname) ;
      }else{
        pz++ ;
        string1 = strdup(pz) ;
      }
      pz = strrchr(string1,'.')  ;
      if(pz != NULL) *pz = '\0' ;
      string2 = strdup(string1) ;
      free(string1) ;
      return string2 ;         // malloc
}

#endif
/*
 *   Routine to convert texture data stored in DXT1 format to RGBA byte format
 *
 *   In the ace files with the surface format flag equal to 0X12 each mipmap is
 *   stored as a single vector.  The vector is made up of 8-byte sections, each
 *   of which describes the colours in 4 by 4 set of pixels.
 *
 *   This routines unpacks the formats and replaces each mipmap by the
 *   equivalent data a simple RGBA array with each colour stored in a simgle
 *   byte.  As the lower order bits are redundent a record is kept of the
 *   original surface format (variable surface_format_orig) which can be used
 *   later to specify the internal format to be used by OpenGL.
 */

int convert_texture(TextureNode *tnode){

int           ip = 0,  // DEBUG
              ii, jj,  // indices of pixel block
              iz, jz,  // sub-indices
              w4,
              m, n,
              width  ,
              height ,
              n_textures ;
GLubyte       *t1 = NULL,   //  Initial texture array
              *t2 = NULL,   //  Final texture array
              *tp ;
int      r, g, b, a ;
GLubyte  r0, g0, b0, a0 ;
unsigned int  *uip ;
unsigned char  uc[64],
              *ucp ;
char          my_name[] = "convert_texture" ;

      if(ip)printf("  Enter routine %s  %p\n",my_name,(void *)tnode) ;
      r0 = g0 = b0 = a0 = 0xFF ;

      tnode->surface_format_orig = tnode->surface_format ;
      if(0x12 != tnode->surface_format)return 0 ;

      tnode->surface_format = 0 ;

      if(ip)printf("  Convert texture %s\n",tnode->name) ;

      width      = tnode->width  ;
      height     = tnode->height ;
      n_textures = tnode->n_textures ;

      for(m=0;m<n_textures;m++){
        t1 = tnode->texture[m] ;
        if(width>2){
          t2 = (GLubyte *)malloc(width*height*4*sizeof(GLubyte)) ;
        }else{
          t2 = (GLubyte *)malloc(16*sizeof(GLubyte)) ;
        }

        w4 = 4*width ;
        uip = (unsigned int  *)t1 ;

        if(width>2){
          for(jj=0;jj<height;jj=jj+4){
            for(ii=0;ii<width;ii=ii+4,uip=uip+2){
              ucp = uc ;
              dxt1_2_rgba(uip, ucp) ;
              tp = t2 + jj*w4 + ii*4;
              for(jz=0;jz<4;jz++){
                for(iz=0;iz<4;iz++){
                  *tp++ = *ucp++ ;
                  *tp++ = *ucp++ ;
                  *tp++ = *ucp++ ;
                  *tp++ = *ucp++ ;
                }
                tp = tp -16 + w4 ;
              }
              if(ip){
                printf(" ===================================\n") ;
                printf(" ii = %i, jj = %i\n",ii,jj) ;
                for(jz=0;jz<4;jz++){
                  for(iz=0;iz<4;iz++){
int                 k = ((jj+jz)*width + ii + iz)*4 ;
                    printf("     %i %i %i %i\n",t2[k],t2[k+1],t2[k+2],t2[k+3]) ;
                  }
                }
              }
            }
          }
/*
 *  Average the 4x4 Mipmap
 */
          if(width == 4){
            r = g = b = a = n = 0 ;
            tp = t2 ;
              for(jz=0;jz<4;jz++){
                for(iz=0;iz<4;iz++){
                  r0 = *tp++  ;
                  g0 = *tp++  ;
                  b0 = *tp++  ;
                  a0 = *tp++  ;
                  if(a0 != 0){
                    r = r0 + r ;
                    g = g0 + g ;
                    b = b0 + b ;
                    n = n + 1  ;
                  }
                }
                tp = tp -16 + w4 ;
              }
              if(n>0){
                r0 = r/n ;
                g0 = g/n ;
                b0 = b/n ;
                a0 = 0xFF ;
              }else{
                r0 = g0 = b0 = a0 = 0xFF ;
              }
          }
        }else{
/*
 *  For 2x2 and 1x1 Mipmaps use the average of the 4x4 Mipmap.
 */
          for(ii=0,tp=t2;ii<4;ii++){
            *tp++ = r0 ;
            *tp++ = g0 ;
            *tp++ = b0 ;
            *tp++ = a0 ;
          }
        }
        free(tnode->texture[m]) ;   //  Replace old texture array
        tnode->texture[m] = t2 ;

        width  = width  >> 1 ;       //  Divide by 2
        height = height >> 1 ;
      }

      if(ip)printf("  Exit  routine %s\n",my_name) ;

      return 0;
}


/*
 * Subroutine to convert packed Dxt1 texture block into rgba bytes
 *
 *  Textures in memory are stored as four bytes per pixel, in which
 *  increasing byte number corresponds to red, green, blue and alpha
 */

int  dxt1_2_rgba(unsigned int ia[2], unsigned char *ja){

  int           i, j ;
  unsigned int  c0, c1, kk, ll ;
  unsigned int  r[4], g[4], b[4], a[4] ;

        c0 = ia[0] & 0xFFFF ;          // First 2 bytes
        c1 = (ia[0]>>16) & 0xFFFF ;    // Next 2 bytes

/*
 *  In the dtx1 format, the first two 16-bit integers contain red, green
 *  and blue values stored in bits 0:4, 5:10 and 11:15.  Here the colours
 *  are extracted and placed in integers with their most significan bit
 *  in bit-7.  Bits 0:7 of the four colours then reflect the original
 *  rgb bytes but with the loss of the low order bits dropped by the dxt1
 *  scheme.
 */
        b[0] = c0 &   0x1f ; b[0] = b[0] << 3 ;
        g[0] = c0 &  0x7e0 ; g[0] = g[0] >> 3 ;
        r[0] = c0 & 0xf800 ; r[0] = r[0] >> 8 ;
        a[0] = 0xffff ;

        b[1] = c1 &   0x1f ; b[1] = b[1] << 3 ;
        g[1] = c1 &  0x7e0 ; g[1] = g[1] >> 3 ;
        r[1] = c1 & 0xf800 ; r[1] = r[1] >> 8 ;
        a[1] = 0xffff ;
/*
 *  Two sets of equations are then used to define a third and fourth colours
 *  used by the dxt1 scheme.  The + 1 in the equations is included to round
 *  up the fraction 2/3, when it occurs, to the next highest integer.
 */
        if(c0>c1){
          r[2] = (r[0]*2 + r[1] + 1)/3 ;
          g[2] = (g[0]*2 + g[1] + 1)/3 ;
          b[2] = (b[0]*2 + b[1] + 1)/3 ;
          a[2] = 0xffff ;
          r[3] = (r[0] + 2*r[1] + 1)/3 ;
          g[3] = (g[0] + 2*g[1] + 1)/3 ;
          b[3] = (b[0] + 2*b[1] + 1)/3 ;
          a[3] = 0xffff ;
        }else{
          r[2] = (r[0] + r[1])/2 ;
          g[2] = (g[0] + g[1])/2 ;
          b[2] = (b[0] + b[1])/2 ;
          a[2] = 0xffff ;
          r[3] = 0 ;               // Transparent - no colour
          g[3] = 0 ;
          b[3] = 0 ;
          a[3] = 0x0000 ;
        }
/*
 *   Finally use the array of 2-bit indices in the second pair of 16-bit
 *   words to define the rgba values of a 4 by 4 block of pixels.
 */
        for(i=0;i<4;i++){
          for(j=0;j<4;j++){
            kk = ia[1] & 0x3 ;     // Extract lowest two bits
            ll = (i*4 + j)*4 ;
            if(1){
            ja[ll  ] = r[kk] ;
            ja[ll+1] = g[kk] ;
            ja[ll+2] = b[kk] ;
            ja[ll+3] = a[kk] ;
            }else{
            ja[ll  ] = a[kk] ;
            ja[ll+1] = b[kk] ;
            ja[ll+2] = g[kk] ;
            ja[ll+3] = r[kk] ;
            }
            ia[1]    = ia[1]>>2 ;  // Shift down two bits
          }
        }
        return 0 ;
}

