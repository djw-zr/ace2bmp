/*
 * *****************************************************************************
 * 45678901234567890123456789012345678901234567890123456789012345678901234567890
 *
 *   File : token_b.c
 *
 *   Released under licence GPL-3.0-or-later.
 *   You should have received a copy of the GNU General Public License.
 *   If not, see <https://www.gnu.org/licenses/>.
 *
 *    Subroutines equivalent to those of token.c but for binary files.
 *
 * *****************************************************************************
 */

/*
 *   Binary read subroutines
 */


int  read_int32(FILE *fp){

int  i ;
union u_tag {
  int  m ;
  char ca[4] ;
}  u ;
//char  myname[] = "read_int32";

      u.m = 0 ;
      for(i=0;i<4;i++){
       u.ca[i] = getc(fp) ;
      }
      return u.m ;
}

uint  read_uint32(FILE *fp){

int  i ;
union u_tag {
  uint m ;
  char ca[4] ;
}  u ;
//char  myname[] = "read_uint32";

      u.m = 0 ;
      for(i=0;i<4;i++){
       u.ca[i] = getc(fp) ;
//      printf(" read_uint16 i= %i, ca = %c,  m = %X\n",i,u.ca[i],u.m) ;
      }
      return u.m ;
}

int  read_int64(FILE *fp){

int  i ;
union u_tag {
  uint m ;
  char ca[4] ;
}  u ;
//char  myname[] = "read_int64";

      u.m = 0 ;
      for(i=0;i<4;i++){
       u.ca[i] = getc(fp) ;
      }
      for(i=0;i<4;i++)getc(fp) ;
      return u.m ;
}
