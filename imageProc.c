#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <math.h>
#include <float.h>
#include <limits.h>

typedef unsigned char UC;

typedef struct{
	int x;
	int y;
} Size;
 
typedef struct{
	Size s;
	UC **val;
} Image;

typedef struct{
	Size s;
	float **val;
} ImageF;

typedef struct{
	Size s;
	int **val;
} ImageI;

typedef struct{
  UC r;
  UC g;
  UC b;
} color_t;

void imageAlloc( Size s, Image *img );
void imageAllocF( Size s, ImageF *img );
void imageAllocI( Size s, ImageI *img );
void imageFree( Image *img );
void imageFreeF( ImageF *img );
void imageFreeI( ImageI *img );
void readPNG(  Image *image, const char *filename);
void writePNG( Image *image, char *filename );


int UCcompare( const void *i, const void *j);

void imageAlloc( Size s, Image *img ){
  int x;

  img->s.x = s.x;
  img->s.y = s.y;
  img->val = (UC **)malloc( sizeof(UC *) * img->s.x );
  for (x = 0; x < img->s.x; x++) {
    img->val[x] = (UC *)malloc( sizeof(UC) * img->s.y );
  }  
}
 
void imageAllocF( Size s, ImageF *img ){
  int x;

  img->s.x = s.x;
  img->s.y = s.y;
  img->val = (float **)malloc( sizeof(float *) * img->s.x );
  for (x = 0; x < img->s.x; x++) {
    img->val[x] = (float *)malloc( sizeof(float) * img->s.y );
  }  
}

void imageAllocI( Size s, ImageI *img ){
  int x;

  img->s.x = s.x;
  img->s.y = s.y;
  img->val = (int **)malloc( sizeof(int *) * img->s.x );
  for (x = 0; x < img->s.x; x++) {
    img->val[x] = (int *)malloc( sizeof(int) * img->s.y );
  }  
}

void imageFree( Image *img ){
  int x;

  for( x=0; x<img->s.x; x++ ){
    free( img->val[x] );
  }
  free( img->val );
}

void imageFreeF( ImageF *img ){
  int x;

  for( x=0; x<img->s.x; x++ ){
    free( img->val[x] );
  }
  free( img->val );
}

void imageFreeI( ImageI *img ){
  int x;

  for( x=0; x<img->s.x; x++ ){
    free( img->val[x] );
  }
  free( img->val );
}

void imageInit( Image *img, UC val ){
  int x, y;

  for( y=0; y<img->s.y; y++ ){
    for( x=0; x<img->s.x; x++ ){
      img->val[x][y] = val;
    }
  }
}

void imageInitI( ImageI *img, UC val ){
  int x, y;

  for( y=0; y<img->s.y; y++ ){
    for( x=0; x<img->s.x; x++ ){
      img->val[x][y] = val;
    }
  }
}

void F2UC( ImageF *img1, Image *img2 ){
  Size s;
  int x, y;
  
  s.x = img1->s.x;
  s.y = img1->s.y;
  
  for( x=0; x<s.x; x++ ){
    for( y=0; y<s.y; y++ ){
      img2->val[x][y] = (UC)img1->val[x][y];
    }
  }
}

void distanceTrans( Image *img1, ImageF *img2 ){
  Size s;
  int x, y;
  int nd, n;
  int wmin, w;
  int irange;
  float *work;
  
  s.x = img1->s.x;
  s.y = img1->s.y;

  work = (float *)malloc( sizeof(float)*( s.x + s.y ) );
  
  for( y=0; y<s.y; y++ ){
    for( x=0; x<s.x; x++ ){
      img2->val[x][y] = (float)img1->val[x][y];
    }
  }

  for( y=0; y<s.y; y++ ){
    if( img2->val[0][y] != 0 ) nd = s.x;
    else                       nd = 0;
    for( x=1; x<s.x; x++ ){
      if( img2->val[x][y] != 0 ) nd++;
      else                       nd=0;
      img2->val[x][y] = nd*nd;
    }
    if( img2->val[s.x-1][y] != 0 ) nd = s.x;
    else                           nd = 0;
    for( x=s.x-2; x>=0; x-- ){
      if( img2->val[x][y] != 0 ) nd++;
      else                       nd=0;
      n = nd*nd;
      img2->val[x][y] = img2->val[x][y]<n ? img2->val[x][y] : n;
    }
  }

  for( x=0; x<s.x; x++ ){
    for( y=0; y<s.y; y++ ) work[y] = img2->val[x][y];
    for( y=0; y<s.y; y++ ){
      wmin = work[y];
      if( wmin == 0 ) continue;
      irange = (int)sqrt( (double)wmin ) + 1;
      for( n=-irange; n<=irange; n++ ){
	if( y+n<0 || y+n>s.y-1 ) w = INT_MAX;
	else w = work[y+n] + n*n;
	if( wmin > w ){
	  wmin = w;
	  img2->val[x][y] = wmin;
	}
      }
    }
  }
  
  free( work );
}

void median( Image *img1, int n, Image *img2 ){
  int x, y;
  int xx, yy;
  int num;
  UC *work;
  Size s;

  s.x = img1->s.x;
  s.y = img1->s.y;

  
  work = (UC *)malloc( sizeof(UC) * n * n );
  
  imageInit( img2, 0 );
  
  for( y=(n-1)/2; y<s.y-(n-1)/2; y++ ){
    for( x=(n-1)/2; x<s.x-(n-1)/2; x++ ){
      num = 0;
      for( yy=-(n-1)/2; yy<=(n-1)/2; yy++ ){
	for( xx=-(n-1)/2; xx<=(n-1)/2; xx++ ){
	  work[num]=img1->val[x+xx][y+yy];
	  num++;
	}
      }
      qsort( work, n*n, sizeof(UC), UCcompare );
      img2->val[x][y] = work[(n*n-1)/2];
    }
  }

  free( work );
}

int UCcompare( const void *i, const void *j){
    return *(UC *)i - *(UC *)j;
}

void filtering( Image *img1, ImageF *mask, Image *img2 ){
  int x, y;
  int xx, yy;
  float val;
  Size s;
  Size m;

  s.x = img1->s.x;
  s.y = img1->s.y;

  m.x = mask->s.x;
  m.y = mask->s.y;

  imageInit( img2, 0 );

  for( y=m.y/2; y<s.y - m.y/2; y++ ){
    for( x=m.x/2; x<s.x - m.x/2; x++ ){
      val = 0.0;
      for( yy=-m.y/2; yy<=m.y/2; yy++ ){
	for( xx=-m.x/2; xx<=m.x/2; xx++ ){
	  val += img1->val[x+xx][y+yy] * mask->val[xx+m.x/2][yy+m.y/2];
	}
      }
      img2->val[x][y] = (UC)val;
    }
  }
}

void binalization( Image *img1, int th, Image *img2 ){
  int x, y;
  
  for( y=0; y<img1->s.y; y++ ){
    for( x=0; x<img1->s.x; x++ ){
      if( img1->val[x][y] >= th ) img2->val[x][y] = 1;
      else                        img2->val[x][y] = 0;
    }
  }
}

void multi( Image *img1, float factor, Image *img2 ){
  int x, y;

  for( y=0; y<img2->s.y; y++ ){
    for( x=0; x<img2->s.x; x++ ){
      img2->val[x][y] =  (unsigned char)(factor * img1->val[x][y]);
    }
  }
}

void labeling( Image *img1, Image *img2, int *lmax ){
  int x, y;
  int xx, yy;
  int label;
  Size s;
  int l[4], min;
  int flag;
  int tmp, tmp2;
  int *t;
  ImageI labelTmp;
  int num;
  
  s.x = img1->s.x;
  s.y = img1->s.y;

  num = s.x * s.y;
  
  imageInit( img2, 0 );
  imageAllocI( s, &labelTmp );
  imageInitI( &labelTmp, 0 );
  
  t = (int *)malloc( sizeof(int)*num );
  for( x=0; x<num; x++ ) t[x] = 0;
  label = 0;

  for( y=1; y<s.y-1; y++ ){
    for( x=1; x<s.x-1; x++ ){
      
      if( img1->val[x][y] == 0 ){
	labelTmp.val[x][y] = 0;
	continue;
      }
      
      l[0] = labelTmp.val[x-1][y-1];
      l[1] = labelTmp.val[ x ][y-1];
      l[2] = labelTmp.val[x+1][y-1];
      l[3] = labelTmp.val[x-1][ y ];
      
      for( xx=0; xx<3; xx++ ){
	for( yy=xx+1; yy<4; yy++ ){
	  if( l[xx] > l[yy] ){
	    tmp = l[xx];
	    l[xx] = l[yy];
	    l[yy] = tmp;
	  }
	}
      }
      min = INT_MAX;
      
      if( l[0] == 0 ) flag = 0;
      else            flag = 1;
      
      for( xx=0; xx<3; xx++ ){
	if( l[xx] != l[xx+1] ) flag++;
      }
      
      for( xx=0; xx<4; xx++ ){
	if( l[xx] != 0 ){
	  min = l[xx];
	  break;
	}
      }
    
      if( flag == 0 ){
	label++;
	t[label] = label;
	labelTmp.val[x][y] = label;
      }
      else if( flag == 1 ){
	labelTmp.val[x][y] = min;
      }
      else{
	labelTmp.val[x][y] = min;
	
	for( xx=1; xx<=label; xx++ ){
	  if( xx == min ) continue;
	  for( yy=0; yy<4; yy++ ){
	    if( t[xx] == l[yy] ){
	    
	      tmp = xx;
	    
	      //	      printf("   min=%d label=%d     ", min, label);
	      while(1){
		
		//  printf("%d  ", tmp);
		if( min >= t[tmp] ) break;
		tmp2 = t[tmp];
		t[tmp] = min;
		tmp = tmp2;
	      }
	      //printf("\n\n");
	      
	      t[xx] = min;
	      
	    }
	  }
	}
      }
      
    }
  }

  for( x=0; x<label; x++ ){
    for( y=x+1; y<=label; y++ ){
      if( x==t[y] ){
	t[y] = t[x];
      }
    }
  }

  
  for( x=0, y=0; x<=label; x++ ){
    if( x==t[x] ){
      t[x] = y++;
    }
    else{
      t[x] = t[t[x]];
    }
  }
  
  *lmax = y--;
  
  for( y=0; y<s.y; y++ ){
    for( x=0; x<s.x; x++ ){
          img2->val[x][y] = (UC)t[ labelTmp.val[x][y] ];
	  //	  printf("%d   %d\n", t[ labelTmp.val[x][y] ], labelTmp.val[x][y] );
    }
  }

  free( t );
  imageFreeI( &labelTmp );
}


void smallComponentElimination( Image *img1, int th, Image *img2 ){
  int num[256];
  int x, y;
  Size s;

  s.x = img1->s.x;
  s.y = img1->s.y;

  for( x=0; x<256; x++ ){
    num[x] = 0;
  }

  for( y=0; y<s.y; y++ ){
    for( x=0; x<s.x; x++ ){
      img2->val[x][y] = img1->val[x][y];
    }
  }

  for( y=0; y<s.y; y++ ){
    for( x=0; x<s.x; x++ ){
      num[img1->val[x][y]]++;
    }
  }

  for( y=0; y<s.y; y++ ){
    for( x=0; x<s.x; x++ ){
      if( num[img1->val[x][y]] < th ){
	img2->val[x][y] = 0;
      }
    }
  }
}

void SAD( Image *img1, Image *mask, Image *img2 ){
  int x, y;
  int xx, yy;
  int sad;
  Size s;
  Size m;

  s.x = img1->s.x;
  s.y = img1->s.y;

  m.x = mask->s.x;
  m.y = mask->s.y;
  
  imageInit( img2, 0 );
  
  for( y=m.y/2; y<s.y - m.y/2; y++ ){
    for( x=m.x/2; x<s.x - m.x/2; x++ ){
      sad = 0;
      for( yy=-m.y/2; yy<m.y/2; yy++ ){
	for( xx=-m.x/2; xx<m.x/2; xx++ ){
	  sad += abs( img1->val[x+xx][y+yy] - mask->val[xx+m.x/2][yy+m.y/2] );
	}
      }
      sad /= m.x*m.y;
      //      printf("%d ", sad);
      if( sad > 255 ) sad =255;
      img2->val[x][y] = sad;
    }
  }
}

// PNG_COLOR_TYPE_RGBのみに対応
void readPNG( Image *img, const char *filename) {
  int i, x, y;
  int num;
  int flag;
  png_colorp palette;
  png_structp png = NULL;
  png_infop info = NULL;
  png_bytep row;
  png_bytepp rows;
  png_byte sig_bytes[8];

  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    perror(filename);
    exit(-1);
  }

  if (fread(sig_bytes, sizeof(sig_bytes), 1, fp) != 1) {
    printf("file read error\n");
    exit(-1);
  }
  if (png_sig_cmp(sig_bytes, 0, sizeof(sig_bytes))) {
    printf("png cmp error\n");
    exit(-1);
  }
  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png == NULL) {
    printf("png_create_read_struct error\n");
    exit(-1);
  }
  info = png_create_info_struct(png);
  if (info == NULL) {
    printf("png_create_info error\n");
    exit(-1);
  }
  if (setjmp(png_jmpbuf(png))) {
    printf("png_jmpbuf error\n");
    exit(-1);
  }
  png_init_io(png, fp);
  png_set_sig_bytes(png, sizeof(sig_bytes));
  png_read_png(png, info, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_STRIP_16, NULL);
  img->s.x = png_get_image_width(png, info);
  img->s.y = png_get_image_height(png, info);
  rows = png_get_rows(png, info);

  img->val = (unsigned char **)malloc( sizeof(unsigned char *) * img->s.x );
  for (x = 0; x < img->s.x; x++) {
    img->val[x] = (unsigned char *)malloc( sizeof(unsigned char) * img->s.y );
  }

  flag = png_get_color_type(png, info);
 if(flag == PNG_COLOR_TYPE_PALETTE){
 	printf("PNG file format error\n");
 		exit(-1);
 	}
 	
  	for (y = 0; y < img->s.y; y++) {
      row = rows[y];
      for (x = 0; x < img->s.x; x++) {
        img->val[x][y] = *row++;
      	if( flag == PNG_COLOR_TYPE_GRAY_ALPHA ||
      		flag == PNG_COLOR_TYPE_RGB ||
      		flag == PNG_COLOR_TYPE_RGB_ALPHA )
      	    *row++;
      	if( flag == PNG_COLOR_TYPE_RGB ||
      		flag == PNG_COLOR_TYPE_RGB_ALPHA )
      	    *row++;
      	if( flag == PNG_COLOR_TYPE_RGB_ALPHA )
      	    *row++;
      }
    }
}

// PNG_COLOR_TYPE_RGBのみに対応
void writePNG( Image *img, char *filename )
{
  FILE *fp;
  png_byte **pngImage;
  png_structp png_ptr;
  png_infop info_ptr;
  int       flag;
  int       x, y, p;

  fp = fopen( filename, "wb" );

  pngImage = (png_byte **)malloc( sizeof(png_byte *) * img->s.y );
  for( y=0; y<img->s.y; y++ ){
    pngImage[y] = (png_byte *)malloc( sizeof(png_byte) * img->s.x * 3 );
  }
  
  for (y=0; y<img->s.y; y++){
    for (x=0; x<img->s.x; x++){
      for( p=0; p<3; p++ ){
	pngImage[y][p+x*3] = (png_byte)img->val[x][y];
      }
    }
  }
  
  png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
  info_ptr = png_create_info_struct( png_ptr );
  png_init_io( png_ptr, fp );
  png_set_IHDR(png_ptr, info_ptr, img->s.x, img->s.y, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info( png_ptr, info_ptr );
  png_write_image( png_ptr, pngImage );
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct( &png_ptr, &info_ptr );

  for( y=0; y<img->s.y; y++ ){
    free( pngImage[y] );
  }
  free( pngImage );
  
  fclose ( fp );
}

