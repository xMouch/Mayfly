#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../src/stb/stb_image_write.h"

typedef signed long long s64; 
typedef double f64; 
typedef unsigned char c8;

f64 lensq(f64 cx, f64 cy)
{
	return cx*cx + cy*cy;
}

f64 lerp(f64 v0, f64 v1, f64 t)
{
	return (1 - t) * v0 + t * v1;	
}

s64 mandel_pixel(f64 cx, f64 cy, s64 iter)
{

	f64 zx = 0;
	f64 zy = 0;
	s64 result = -1;
	
	for(s64 i = 0; i < iter; i = i + 1)
	{
		if(lensq(zx, zy) > 4.0)
		{
			result = i;
			break;
		}		
		f64 zxsq =  zx * zx - zy * zy;
		f64 zysq =  zx * zy + zx * zy;
		
		zx = zxsq + cx;
		zy = zysq + cy;
	}

	return result;
}



int main()
{
	s64 width = 1280;
	s64 height = 720;
	f64 fw = width;
	f64 fh = height;
	s64 iter = 25;

	f64 x_min = -2.1;
	f64 x_max =  1;
	f64 y_min = -1.2;
	f64 y_max = 1.2;

	c8* img = (c8*)realloc(img, width * height * 3);
	

	for(s64 y = 0; y < height; y = y + 1)
	{
		for(s64 x = 0; x < width; x = x + 1)
		{
			f64 lx = lerp(x_min, x_max, x/fw);
			f64 ly = lerp(y_min, y_max, y/fh);
			s64 n = mandel_pixel(lx, ly, iter);
			
			s64 index = (x + y * width) * 3;
			if(n == -1)
			{
				img[index] = 0;
				img[index+1] = 0;
				img[index+2] = 0;
			}
			else
			{
				img[index] = (n * 123456) % 255;
				img[index+1] = (n * 654321) % 255;
				img[index+2] = (n * 3333333) % 255;
			} 
		}
	}

    stbi_write_bmp("Cmandelb.bmp",width,height,3,img);
    return 0;
}

