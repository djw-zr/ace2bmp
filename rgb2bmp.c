/*
 * *****************************************************************************
 * 45678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 *   File : rgb2bmp.c
 *
 *   This file is part of ace2bmp.
 *   Released under licence GPL-3.0-or-later.
 *   You should have received a copy of the GNU General Public License.
 *   If not, see <https://www.gnu.org/licenses/>.
 *
 *   The code in this file is based on:
 *       https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
 *
 * *****************************************************************************
 */

const int BYTES_PER_PIXEL = 4; /// alpha, red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 124;

unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

#if 0
int main ()
{
    int height = 361;
    int width = 867;
    unsigned char image[height][width][BYTES_PER_PIXEL];
    char* imageFileName = (char*) "bitmapImage.bmp";

    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            image[i][j][2] = (unsigned char) ( i * 255 / height );             ///red
            image[i][j][1] = (unsigned char) ( j * 255 / width );              ///green
            image[i][j][0] = (unsigned char) ( (i+j) * 255 / (height+width) ); ///blue
        }
    }

    generateBitmapImage((unsigned char*) image, height, width, imageFileName);
    printf("Image generated!!\n");
}
#endif

void generateBitmapImage (TextureNode *tnode, char* file_name)
{
  int ip = 0 ;
  int i, j ;
  int width  = tnode->width  ;
  int height = tnode->height ;
  int widthInBytes = width * BYTES_PER_PIXEL;
  unsigned char rb, gb, bb, ab, *pp;

  unsigned char padding[3] = {0, 0, 0};
  int paddingSize = (4 - (widthInBytes) % 4) % 4;
  int stride = (widthInBytes) + paddingSize;

      if(ip){
        printf("  Enter routine 'generateBitmapImage'\n") ;
        printf("  width = %i\n",width) ;
        printf("  height = %i\n",height) ;
        printf("  stride = %i\n",stride) ;
        printf("  paddingSize  = %i\n",paddingSize) ;
        printf("  widthInBytes = %i\n",widthInBytes) ;
      }

      FILE* imageFile = fopen(file_name, "wb");

      unsigned char* fileHeader = createBitmapFileHeader(height, stride);
      fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

      unsigned char* infoHeader = createBitmapInfoHeader(height, width);
      fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

      pp = tnode->texture[0] ;
      for(j=0; j<height; j++){
        for(i=0; i<width;i++){
          rb = pp[j*widthInBytes +i*4 + 0];
          gb = pp[j*widthInBytes +i*4 + 1];
          bb = pp[j*widthInBytes +i*4 + 2];
          ab = pp[j*widthInBytes +i*4 + 3];

          pp[j*widthInBytes +i*4 + 0] = rb;
          pp[j*widthInBytes +i*4 + 1] = gb  ;
          pp[j*widthInBytes +i*4 + 2] = bb;
          pp[j*widthInBytes +i*4 + 3] = ab  ;
#if 1
          if(ip && i==128)
              printf("  j = %i :: %3i %3i %3i %3i\n",j,
              pp[j*widthInBytes +i*4 + 0],
              pp[j*widthInBytes +i*4 + 1],
              pp[j*widthInBytes +i*4 + 2],
              pp[j*widthInBytes +i*4 + 3]
              ) ;
#endif
        }
      }

      for (i = 0; i < height; i++) {
          j = height - 1 - i ;
          fwrite(tnode->texture[0] + (j*widthInBytes), BYTES_PER_PIXEL, width, imageFile);
          fwrite(padding, 1, paddingSize, imageFile);
      }
      if(ip)printf("  Exit  routine 'generateBitmapImage'\n") ;

      fclose(imageFile);

      return ;
}

unsigned char* createBitmapFileHeader (int height, int stride)
{
  int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

  static unsigned char fileHeader[] = {
      0,0,     /// signature
      0,0,0,0, /// image file size in bytes
      0,0,0,0, /// reserved
      0,0,0,0, /// start of pixel array
  };

      fileHeader[ 0] = (unsigned char)('B');
      fileHeader[ 1] = (unsigned char)('M');
      fileHeader[ 2] = (unsigned char)(fileSize      );
      fileHeader[ 3] = (unsigned char)(fileSize >>  8);
      fileHeader[ 4] = (unsigned char)(fileSize >> 16);
      fileHeader[ 5] = (unsigned char)(fileSize >> 24);
      fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

      return fileHeader;
}

unsigned char* createBitmapInfoHeader (int height, int width)
{
  static unsigned char infoHeader[] = {
#if 1
    0,0,0,0, /// header size
    0,0,0,0, /// image width
    0,0,0,0, /// image height
    0,0,     /// number of color planes
    0,0,     /// bits per pixel
    3,0,0,0, /// Format (bitfield = use bitfields | no compression)
    0,0,0,0, /// Image raw size (32 bytes)
    0x13, 0x0B, 0,0, /// Horizontal print resolution (2835 = 72dpi * 39.3701)
    0x13, 0x0B, 0,0, /// Vertical print resolution (2835 = 72dpi * 39.3701)
    0,0,0,0, // Colors in palette (none)
    0,0,0,0, // Important colors (0 = all)
#if 0
    0,0,0xFF,0, // R bitmask (FF000000)
    0,0xFF,0,0, // G bitmask (00FF0000)
    0xFF,0,0,0, // B bitmask (0000FF00)
    0,0,0,0xFF, // A bitmask (000000FF)
#else
    0xFF,0,0,0, // R bitmask (FF000000)
    0,0xFF,0,0, // G bitmask (00FF0000)
    0,0,0xFF,0, // B bitmask (0000FF00)
    0,0,0,0xFF, // A bitmask (000000FF)
#endif
    0x42, 0x47, 0x52, 0x73, // sRGB color space
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0, // Unused R, G, B entries for color space
    0,0,0,0, // Unused Gamma X entry for color space
    0,0,0,0, // Unused Gamma Y entry for color space
    0,0,0,0, // Unused Gamma Z entry for color space

    0,0,0,0, // Unknown
    0,0,0,0, // Unknown
    0,0,0,0, // Unknown
    0,0,0,0  // Unknown
#else
    0x7c, 0x00, 0x00, 0x00, // DIB header size (124 bytes)
    0x04, 0x00, 0x00, 0x00, // Width (4px)
    0x02, 0x00, 0x00, 0x00, // Height (2px)
    0x01, 0x00,             // Planes (1)
    0x20, 0x00,             // Bits per pixel (32)
    0x03, 0x00, 0x00, 0x00, // Format (bitfield = use bitfields | no compression)
    0x20, 0x00, 0x00, 0x00, // Image raw size (32 bytes)
    0x13, 0x0B, 0x00, 0x00, // Horizontal print resolution (2835 = 72dpi * 39.3701)
    0x13, 0x0B, 0x00, 0x00, // Vertical print resolution (2835 = 72dpi * 39.3701)
    0x00, 0x00, 0x00, 0x00, // Colors in palette (none)
    0x00, 0x00, 0x00, 0x00, // Important colors (0 = all)
#if 1
    0x00, 0x00, 0xFF, 0x00, // R bitmask (00FF0000)
    0x00, 0xFF, 0x00, 0x00, // G bitmask (0000FF00)
    0xFF, 0x00, 0x00, 0x00, // B bitmask (000000FF)
    0x00, 0x00, 0x00, 0xFF, // A bitmask (FF000000)
#else
    0xFF, 0x00, 0x00, 0x00, // R bitmask (00FF0000)
    0x00, 0xFF, 0x00, 0x00, // G bitmask (0000FF00)
    0x00, 0x00, 0xFF, 0x00, // B bitmask (000000FF)
    0x00, 0x00, 0x00, 0xFF, // A bitmask (FF000000)
#endif
    0x42, 0x47, 0x52, 0x73, // sRGB color space
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Unused R, G, B entries for color space
    0x00, 0x00, 0x00, 0x00, // Unused Gamma X entry for color space
    0x00, 0x00, 0x00, 0x00, // Unused Gamma Y entry for color space
    0x00, 0x00, 0x00, 0x00, // Unused Gamma Z entry for color space

    0x00, 0x00, 0x00, 0x00, // Unknown
    0x00, 0x00, 0x00, 0x00, // Unknown
    0x00, 0x00, 0x00, 0x00, // Unknown
    0x00, 0x00, 0x00, 0x00, // Unknown

#endif
  };

      infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
      infoHeader[ 4] = (unsigned char)(width      );
      infoHeader[ 5] = (unsigned char)(width >>  8);
      infoHeader[ 6] = (unsigned char)(width >> 16);
      infoHeader[ 7] = (unsigned char)(width >> 24);
      infoHeader[ 8] = (unsigned char)(height      );
      infoHeader[ 9] = (unsigned char)(height >>  8);
      infoHeader[10] = (unsigned char)(height >> 16);
      infoHeader[11] = (unsigned char)(height >> 24);
      infoHeader[12] = (unsigned char)(1);
      infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

      return infoHeader;
}
