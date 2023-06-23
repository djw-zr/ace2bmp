/*
 * *****************************************************************************
 * 45678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 *   File : token.c
 *
 *   This file is part of ace2bmp.
 *   Released under licence GPL-3.0-or-later.
 *   You should have received a copy of the GNU General Public License.
 *   If not, see <https://www.gnu.org/licenses/>.
 *
 * *****************************************************************************
 */

#define CHUNK 16384                      //  For zlib

/*
 * *****************************************************************************
 *
 *  zlib inflation/decompression
 *
 *  Decompress from file 'source' to file 'dest' until stream ends or EOF.
 *  inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
 *  allocated for processing, Z_DATA_ERROR if the deflate data is
 *  invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
 *  the version of the library linked do not match, or Z_ERRNO if there
 *  is an error reading or writing the files.
 *
 *  The zr program uses routine zlib_uncompress (below) which calls this
 *  routine to uncompress the MSTS binary files.
 *
 *  The routine requires file zlib.h to compile and the program then
 *  needs to be linked to a matching version of the z library (libz).
 *
 *  This is based on a program on the zlib web pages
 * *****************************************************************************
 */
int inf(FILE *source, FILE *dest)
{
  int ret;
  unsigned have;
  z_stream strm;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

    /* allocate inflate state */
      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;
      strm.avail_in = 0;
      strm.next_in = Z_NULL;
      ret = inflateInit(&strm);
      if (ret != Z_OK)
        return ret;

/* decompress until deflate stream ends or end of file */
      do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
          (void)inflateEnd(&strm);
          return Z_ERRNO;
        }
        if (strm.avail_in == 0)
          break;
        strm.next_in = in;

/* run inflate() on input until output buffer not full */
        do {
          strm.avail_out = CHUNK;
          strm.next_out = out;
          ret = inflate(&strm, Z_NO_FLUSH);
          assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
          switch (ret) {
          case Z_NEED_DICT:
              (void)inflateEnd(&strm);
              ret = Z_DATA_ERROR;
              return ret;
          case Z_DATA_ERROR:
          case Z_MEM_ERROR:
              (void)inflateEnd(&strm);
              return ret;
          }
          have = CHUNK - strm.avail_out;
          if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
              (void)inflateEnd(&strm);
              return Z_ERRNO;
          }
        } while (strm.avail_out == 0);

/* done when inflate() says it's done */
      } while (ret != Z_STREAM_END);

/* clean up and return */
      (void)inflateEnd(&strm);
      return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
      fputs("zpipe: ", stderr);
      switch (ret) {
      case Z_ERRNO:
          if (ferror(stdin))
              fputs("error reading stdin\n", stderr);
          if (ferror(stdout))
              fputs("error writing stdout\n", stderr);
          break;
      case Z_STREAM_ERROR:
          fputs("invalid compression level\n", stderr);
          break;
      case Z_DATA_ERROR:
          fputs("invalid or incomplete deflate data\n", stderr);
          break;
      case Z_MEM_ERROR:
          fputs("out of memory\n", stderr);
          break;
      case Z_VERSION_ERROR:
          fputs("zlib version mismatch!\n", stderr);
      }
}

/*
 * *****************************************************************************
 *  Uncompress zlib format file
 *
 *  This routine uncompresses the file to a temporary file and then changes
 *  the stream in the structure 'msfile' to point to the start of the
 *  uncompressed file.
 *
 *   A better method would be to use internal buffers but this means either
 *   modifying all the getc(fp) calls in this file, so that they read from
 *   the the input stream of the buffer, or having separate token subroutine
 *   for compressed files.
 *
 *   Check : It might only be necessary to change the calls in ms_token.
 * *****************************************************************************
 */

int zlib_uncompress(MSfile *msfile, int uncompressed_size, int ip)
{
  int   iret ;
  char  tmp_file[] = "unziplib.tmp"    ;
  char  myname[] = "zlib_uncompress" ;
  FILE  *fp_out,  *fp_in ;

      if(ip){
        printf("  Enter %s\n",myname) ;
        printf("  File = %s\n",msfile->filename) ;
        printf("  uncompressed_size (scaled?) = %i\n",uncompressed_size) ;
      }

      fp_in = msfile->fp ;
      if(ip)printf(" Z1\n");
/*
 *  Open 'temporary' file.
 *
 *  This presently uses mkstemp and fdopen, to open a suitable
 *  temporary file.  The alternative is the original code which
 *  was different for WINDOWS and GNU/linux.
 *
 *  With mkstemp, the character array needs to end in 'XXXXXX'. This
 *  section will be overwritten by the routine to give a unique name.
 *  The file descriptor then uses the Windows form "wb+".  GNU/linux
 *  ignores the 'b'.
 *
 *  With GNU/linux - the file is a simple byte stream
 *                 = usually it will be kept in memory
 *  With Microsoft - the "b" flag has to be defined to prevent text conversion
 *                 - setting up an in-memory file is messy
 *
 */
#if 1
int  fd = mkstemp(ztemp) ;
      fp_out =  fdopen(fd,"wb+") ;
      l_tmp  = 1      ;
      fp_tmp = fp_out ;
#else
#ifdef WINDOWS
      fp_out = fopen(ziptmp,"wb+") ;
      l_tmp  = 1      ;
      fp_tmp = fp_out ;
#else
      fp_out = tmpfile() ; //  Should work for Microsoft as well as GNU/Linux
      l_tmp = 0 ;
#endif
#endif
      if(ip)printf(" Z2  %p\n",(void *)fp_out);
/*
 *  Decompress
 */
      if(ip)printf(" Z3\n");
      iret = inf(fp_in, fp_out);
      if(ip)printf(" Z4  iret = %i\n",iret);
      if (iret != Z_OK) zerr(iret);
/*
 * Close original file and replace file pointer by new file
 */
      if(ip)printf(" Z5\n");
      fclose(fp_in) ;
      if(ip)printf(" Z6\n");
      msfile->fp = fp_out ;
/*
 *  Rewind ready for further processing
 */
      rewind(msfile->fp) ;

      return 0;
}

void init_msfile(MSfile *msfile){
  int      k ;
  MSblock  *block ;
      msfile->fp        = NULL ;
      msfile->filename  = NULL ;
      msfile->unicode   = 0    ;
      msfile->ascii     = 0    ;
      msfile->compress  = 0    ;
      msfile->text      = 0    ;
      msfile->binary    = 0    ;
      msfile->world     = 0    ;
      msfile->texture   = 0    ;
      for(k=0;k<20;k++){
        block = &(msfile->level[k]) ;
        block->token_name = NULL ;
        block->label      = NULL ;
        block->itoken     = 0    ;
        block->flags      = 0    ;
        block->l_label    = 0    ;
        block->length     = 0    ;
        block->byte_end   = 0    ;
      }
      return ;
}

/*
 * *****************************************************************************
 *   Open MSTS format file
 *
 *   MSTS files start with a MSTS header.
 *   * If it is a text file using unicode the file starts with the characters
 *       0xff and 0xfe, the order showing whether the file is big-endian or
 *       littel-endian.  If it is not unicode these characters are missing.
 *   * The file then has an 16 byte header (16 * 2 bytes if unicode)
 *   * The header should start  "SIMISA@".  If the file is compressed the
 *     next (last) character is 'F', otherwise it is '@'.
 *   * If compressed the next four characters give the size of the
 *     uncompressed file as a four byte 'int'.  This is needed so the
 *     program can provide a large enough buffer to contain the
 *     uncompressed file before further processing.  This is followed by
 *     four bytes '@@@@'.
 *   * Compression uses the zlib library.  In the standard form the core
 *     compressed region is surrounded by a small zlib container which
 *     contains parity information.  In the gzip form it is surrounded by a
 *     gzip container which contains the original filename and other data.
 *   * If compressed the remainder of the file needs to be uncompressed
 *     (decompressed?)
 *
 *   * The file (the uncompressed version if compressed) then contains a
 *     16-byte sub-header (16 * 2 bytes if unicode)
 *       If character 6 is a 'w', it is a world file.
 *        "     "     7 is a 't', it is a text file
 *        "     "     7 is a 'b', it is a binary file
 *
 *   Some files containing tokens are pure ascii files without a header
 *   (see comments in 'tsection_db.c').  To allow these to be read with
 *   the same token routines is also used to flag possible text files.
 *   So    texture =  1  :: texture file
 *                 = -1  :: if not an MSTS file assume ascii
 *                 =  0  :: normal MSTS file.
 * *****************************************************************************
 */

int open_msfile(char *filename, MSfile *msfile, int texture, int iprint){

  int ip = 0 ;
  int i, iret ;
  unsigned int m, n ;
  int  unicode ;
  char buffer[32] ;
  char string[33] ;
  FILE *fp ;
/*
 *  Initialise
 */
      iprint = iprint || ip ;
//      init_token() ;
/*
 * Open File
 * Use 'rb'  'b' is needed for Windows, ignored by linux.
 */
      fp = fopen(filename,"rb");
      if(fp==NULL){
        printf(" File not found.  File = %s\n",filename);
        return 1 ;
      }
      init_msfile(msfile) ;           // Initialise the data structure

      msfile->filename = (char *)malloc((strlen(filename)+1)*sizeof(char)) ;
      strcpy(msfile->filename,filename) ;
/*
 *   Initialise flags
 */
      msfile->fp = fp ;
      msfile->unicode = msfile->ascii = msfile->compress = msfile->text
                                      = msfile->binary   = msfile->world = 0 ;
      if(1 == texture)msfile->texture = texture   ;
      msfile->token_unused = NULL ;    // Used when reading text files
/*
 *  Test for two-character unicode.  MSTS files, including the binary files,
 *  represent text and names either using ascii or two character unicode
 *  characters.  These routines assume that the second character of each
 *  two character uncode is a binary zero and so can be ignored.
 */
      buffer[0] = getc(fp);
      buffer[1] = getc(fp);
      unicode = (char)buffer[0]==(char)0xff
                     && (char)buffer[1]==(char)0xfe ;
      msfile->unicode = unicode ;
      msfile->ascii = unicode ? 0 : 1 ;
/*
 *  Read first MSTS header
 *  Some unicode files start with blanks (!)
 */
      if(unicode){
        for(i=0;i<256;i++){
          buffer[0] = getc(fp) ; getc(fp) ;
          if('S' == buffer[0])break;
        }
        for(i=1;i<16;i++){
          buffer[i] = getc(fp); getc(fp) ;
        }
      }else{
        for(i=2;i<16;i++){
          buffer[i] = getc(fp);
        }
      }
//      printf(" Routine open_msfile :: file   = %s\n",filename) ;
//      printf(" Routine open_msfile :: buffer = %s\n",buffer) ;
/*
 *  Check for compress
 */
      if(0 == strncmp(buffer,"SIMISA@F",8)){       // Compressed
#if 0
        printf(" +++ Compressed file: %s\n", filename) ;
#endif
        msfile->compress = 1 ;
        for(i=0;i<32;i++) string[i] = buffer[i] ;
        string[32] = '\0' ;
/*
 *  Read size of file
 *  File is little-endian so each following byte has a value 256 times larger
 */
        n = 0 ;
        m = 1 ;
        for(i=8;i<12;i++){
          n = n + m*(buffer[i] & 0xff) ;
          m = m*256 ;
          if(iprint)printf(" BB i, m, n = %i %i :: %i %i \n",i,
                                        (unsigned char)buffer[i], m,n);
        }

        if(iprint)printf(" CC msfile->pr = %p  n = %i\n",(void *)msfile->fp,n);
        iret = zlib_uncompress(msfile,n,iprint);
        fp = msfile->fp;
        if(iprint)printf(" DD msfile->pr = %p\n",(void *)msfile->fp);

        if(iret != 0){
          printf("/n  Error uncompressing file %s\n",msfile->filename);
          printf(  "  Program Stopping ...\n");
          exit(1) ;
        }
      }else if(0 != strncmp(buffer,"SIMISA@@",8)){
//  Possible Ascii/text file
        if(-1 == texture){
          rewind(fp);
          if(unicode){ getc(fp) ; getc(fp) ; }
          msfile->text = 1 ;                      // Give it a chance
          return 0 ;
// Error
        }else{
          printf("ERROR 1 : file header not recognised\n");
          printf("        :  File = %s\n",filename) ;
          strncpy(string,buffer,8); string[8] = 0;
          printf("  Header reads : %s\n",string);
          return 1;
        }
      }
//      printf(" BBBB iprint = %i\n",iprint);
/*
 *  Texture file
 */
      if(1 == texture){
/*
 * Print summary line
 */
      if(iprint){
        printf("\n  file       = %s\n",filename);
        printf("    unicode  = %i,",msfile->unicode);
        printf(  "  ascii    = %i,",msfile->ascii);
        printf(  "  compress = %i\n",msfile->compress);
        if(msfile->unicode)
          printf(" File is little-endian UTF-16 Unicode text"
                                  " with CRLF terminators.\n");
        if(msfile->ascii) printf(" File is text file\n");
      }
        return 0 ;
      }
/*
 *  Read second MSTS header
 */
//      printf(" CCCC iprint = %i\n",iprint);
      if(unicode){
        for(i=16;i<32;i++){
          buffer[i] = getc(fp); getc(fp) ;
        }
      }else{
        for(i=16;i<32;i++){
          buffer[i] = getc(fp);
        }
      }
//      printf(" DDDD iprint = %i\n",iprint);
//      for(i=0;i<32;i++)printf(" %x",buffer[i]);
//      printf("\n") ;
/*
 *  Check for normal text or binary file
 */
      if('t' == buffer[23]){
        msfile->text = 1 ;
      }else if('b' == buffer[23]){
        msfile->binary = 1;
      }else{
        printf("ERROR 2 : file header not recognised\n");
        strncpy(string,buffer,32); string[32] = '\0';
        printf("  Header reads : %s\n",string);
        return 2;
      }
/*
 *  Check for world file
 */
      if('w' == buffer[21]){
        msfile->world = 1 ;
      }
//      printf(" EEEE iprint = %i\n",iprint);
/*
 * Print summary line
 */
      if(iprint){
        printf("\n  file       = %s\n",filename);
        printf("    unicode  = %i,",msfile->unicode);
        printf(  "  ascii    = %i,",msfile->ascii);
        printf(  "  compress = %i,",msfile->compress);
        printf(  "  text     = %i,",msfile->text);
        printf(  "  binary   = %i,",msfile->binary);
        printf(  "  world    = %i,",msfile->world);
        printf(" ms file ptr = %p\n",(void *)fp) ;

        if(msfile->unicode)
          printf(" File is little-endian UTF-16 Unicode text"
                                  " with CRLF terminators.\n");
        if(msfile->ascii) printf(" File is text file\n");
      }

      return 0;
}
/*
 * *****************************************************************************
 * Close msfile
 *
 * "if NULL" statements used here and elsehwere to keep valgrind happy.
 * It should be possible to remove them.
 * *****************************************************************************
 */
int close_msfile(MSfile *msfile)
{
#if 0
  int i ;
      for(i=0;i<20;i++){
        if(NULL != msfile->level[i].token_name)
                                 free(msfile->level[i].token_name) ;
        if(NULL != msfile->level[i].label)
                                 free(msfile->level[i].label) ;
      }
      if(msfile->token_unused)free(msfile->token_unused) ;
      if(msfile->filename)free(msfile->filename) ;
#endif
      fclose(msfile->fp) ;
      return 0;
}

/**
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
