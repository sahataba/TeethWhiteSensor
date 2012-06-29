/*
Copyright (C) 2006 Pedro Felzenszwalb

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#ifndef SEGMENT_IMAGE
#define SEGMENT_IMAGE

#include <cstdlib>
#include <map>
#include "image.h"
#include "misc.h"
#include "filter.h"
#include "segment-graph.h"
#include "math.h"

struct rgbb
{
    int r;
    int g; 
    int b;
    int x;
    int y;
} ;

struct hsl 
{
    float h;
    float s;
    float l;
    float alpha;
};

struct hslCart 
{
    float x;
    float y;
    float s;
    float l;
    //float alpha;
};

struct hslxy 
{
    float h;
    float s;
    float l;
    float alpha;
    int x;
    int y;
};

static inline hsl RGBToHSL(int red, int green, int blue) {
	float mincolor = fminf(fminf(red, green), blue);
	float maxcolor = fmaxf(fmaxf(red, green), blue);
    hsl hsl;
    
	hsl.h = 0;
	hsl.s = 0;
	hsl.l = (maxcolor + mincolor)/2;
    
	if (maxcolor == mincolor)
		return hsl;
    
	if (hsl.l < 0.5)
		hsl.s = (maxcolor - mincolor)/(maxcolor + mincolor);
	else
		hsl.s = (maxcolor - mincolor)/(2.0 - maxcolor - mincolor);
    
	if (red == maxcolor)
		hsl.h = (green - blue)/(maxcolor - mincolor);
	else if (green == maxcolor)
		hsl.h = 2.0 + (blue - red)/(maxcolor - mincolor);
	else
		hsl.h = 4.0 + (red - green)/(maxcolor - mincolor);
    
	hsl.h /= 6;
    return hsl;
}

static inline rgbb HSLToRGB(hsl hsl) {
    rgbb rgb;
	if (hsl.s == 0) {
		rgb.r = rgb.g = rgb.b = hsl.l;
		return rgb;
	}
    
	float temp2 = 0;
    
	if (hsl.l < 0.5)
		temp2 = hsl.l*(1 + hsl.s);
	else
		temp2 = hsl.l+hsl.s-hsl.l*hsl.s;
    
	float temp1 = 2*hsl.l - temp2;
    
	float temp[3];
	temp[0] = hsl.h + 1/3.0;
	temp[1] = hsl.h;
	temp[2] = hsl.h - 1/3.0;
    
	for (int i = 0; i < 3; i++) {
		float temp3 = temp[i];
		if (temp3 < 0)
			temp3 += 1.0;
		else if (temp3 > 1)
			temp3 -= 1.0;
        
		float color = 0;
		if (6*temp3 < 1)
			color = temp1+(temp2-temp1)*6*temp3;
		else if (2*temp3 < 1)
			color = temp2;
		else if (3*temp3 < 2)
			color = temp1+(temp2 - temp1)*(4 - temp3*6);
		else
			color = temp1;
        
		switch (i) {
			case 0:
				rgb.r = color;
				break;
			case 1:
				rgb.g = color;
				break;
			case 2:
				rgb.b = color;
				break;
		}
	}
    
    return rgb;
}

typedef struct
{
    image<rgb> *image;
    std::map<int, hslxy> averages;
    rgbb totalAvg; 
} SegmentResult;


// random color
rgb random_rgb(){ 
  rgb c;
  
  c.r = (uchar)random();
  c.g = (uchar)random();
  c.b = (uchar)random();

  return c;
}

static inline void setRGB(unsigned char * im, int width, int height, int i, int j, rgb col) {
    int offset = 4*((width*j)+i);
    im[offset+1] = col.r;
    im[offset+2] = col.g;
    im[offset+3] = col.b;
}

// dissimilarity measure between pixels
static inline float diff(unsigned char * im, int width, int height, int x1,int y1, int x2, int y2) {
    int offset1 = 4*((width*y1)+x1);
    int offset2 = 4*((width*y2)+x2);
    return sqrt(square(im[offset1+1]-im[offset2+1]) +
                square(im[offset1+2]-im[offset2+2]) +
                square(im[offset1+3]-im[offset2+3]));
}

static inline float diffE(image<hsl> *im, int x1, int y1, int x2, int y2) {
    hsl hsl1 = imRef(im, x1,y1);
    hsl hsl2 = imRef(im, x2,y2);

    return sqrt(square(hsl1.h - hsl2.h) +
                square(hsl1.s - hsl2.l) +
                square(hsl1.l - hsl2.l));
}

/*
 * Segment an image
 *
 * Returns a color image representing the segmentation.
 *
 * im: image to segment.
 * sigma: to smooth the image.
 * c: constant for treshold function.
 * min_size: minimum component size (enforced by post-processing stage).
 * num_ccs: number of connected components in the segmentation.
 */
SegmentResult segment_image(unsigned char* im, int width, int height, float sigma, float c, int min_size,
			  int *num_ccs) {
    
    float PI2 = 2 * M_PI;
    
    NSDate *start = [NSDate date];
  //int width = im->width();
  //int height = im->height();

 NSTimeInterval pass1 = [start timeIntervalSinceNow];

    NSTimeInterval pass2 = [start timeIntervalSinceNow];

 
  // build graph
  edge *edges = new edge[width*height*4];
  int num = 0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (x < width-1) {
	edges[num].a = y * width + x;
	edges[num].b = y * width + (x+1);

	edges[num].w = diff(im, width, height,x,y,x+1,y);
	num++;
      }

      if (y < height-1) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + x;
          
    edges[num].w = diff(im, width, height,x,y,x,y+1);
	num++;
      }

      if ((x < width-1) && (y < height-1)) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + (x+1);
          
    edges[num].w = diff(im, width, height,x,y,x+1,y+1);
	num++;
      }

      if ((x < width-1) && (y > 0)) {
	edges[num].a = y * width + x;
	edges[num].b = (y-1) * width + (x+1);
          
    edges[num].w = diff(im, width, height,x,y,x+1,y-1);
	num++;
      }
    }
  }
    /*
  delete smooth_r;
  delete smooth_g;
  delete smooth_b;*/
    
    NSTimeInterval pass3 = [start timeIntervalSinceNow];


  // segment
  universe *u = segment_graph(width*height, num, edges, c);
    
    NSTimeInterval pass4 = [start timeIntervalSinceNow];

  
  // post process small components
  for (int i = 0; i < num; i++) {
    int a = u->find(edges[i].a);
    int b = u->find(edges[i].b);
    if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
      u->join(a, b);
  }
  delete [] edges;
  *num_ccs = u->num_sets();

  image<rgb> *output = new image<rgb>(width, height);

  // pick random colors for each component
  rgb *colors = new rgb[width*height];
  for (int i = 0; i < width*height; i++)
    colors[i] = random_rgb();
    
    printf("COMPS: + %i", num_ccs[0]);
    
    std::map<int, hslCart> sumColors;
    
    NSTimeInterval pass5 = [start timeIntervalSinceNow];

  for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
          int comp = u->find(y * width + x);
          
          int offset = 4*((width*y)+x);

          std::map<int,hslCart>::iterator i = sumColors.find (comp);
          if (i == sumColors.end ()) {
              hslCart color = {0,0,0,0};
              sumColors.insert(std::pair<int,hslCart>(comp, color));
          }
          else
          {
              hsl newColor = RGBToHSL(im[offset+1], im[offset+2], im[offset+3]);
              float hue = newColor.h * PI2;
              i->second.x += sinf(hue);
              i->second.y += cosf(hue);
              i->second.s += newColor.s;
              i->second.l += newColor.l;
              //i->second.alpha += newColor.alpha;

          }
      }
  }

    std::map<int, hslxy> test;
    
    NSTimeInterval pass6 = [start timeIntervalSinceNow];

    float yellowStart = 25/360.0;
    float yellowEnd = 55/360.0;
    hslCart color;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = u->find(y * width + x);
        int size = u->size(comp); 
        std::map<int,hslCart>::iterator i = sumColors.find (comp);
        
        if (i == sumColors.end ()) {
        }
        else
        {
            color = i->second;
        }

        float atanN = atan2f(color.y/size, color.x/size);
        if (atanN < 0) atanN = atanN + PI2;
        
        hsl avgHSL = {atanN/ PI2, color.s/size, color.l/size/*, color.alpha/size*/};
        
        if ( (avgHSL.h < yellowEnd && avgHSL.h > yellowStart) /*||  avgHSL.l > 0.95*/) {
            hslxy a = {avgHSL.h,avgHSL.s, avgHSL.l, avgHSL.alpha, x, y};
            test.insert (std::pair<int,hslxy>(comp, a) );                     
        }
        else {
            setRGB(im, width, height, x, y, colors[comp]);
        }
    }
  }  
    
    std::map<int,hslCart>::iterator tot;
    
    float totalX = 0;
    float totalY = 0;
    float totalS = 0;
    float totalL = 0;
    int totalSize = 0;
    
    
    for ( tot=sumColors.begin() ; tot != sumColors.end(); tot++ ){
        int comp = tot->first;
        int size = u -> size(comp);
        if (test.count(comp) > 0 ) {
            totalX = totalX + tot->second.x;
            totalY = totalY + tot->second.y;
            totalS = totalS + tot->second.s;
            totalL = totalL + tot->second.l;
            totalSize = totalSize + size;
        }
    }
    
  delete [] colors;  
  delete u;
    
    
    float atanT = atan2f(totalY/totalSize, totalX/totalSize);
    if (atanT < 0) atanT = atanT + PI2;
    
    hsl avg = {atanT / PI2, totalS/totalSize, totalL/totalSize};
    
    rgbb avgRGB2 = HSLToRGB(avg);
    
    SegmentResult res = {output, test, avgRGB2};
    NSTimeInterval pass7 = [start timeIntervalSinceNow];
    
    //printf("PASS1 %f", pass1);
    printf("PASS2 %f", pass2);
    printf("PASS3 %f", pass3);
    printf("PASS4 %f", pass4);
    printf("PASS5 %f", pass5);
    printf("PASS6 %f", pass6);
    printf("PASS7 %f", pass7);


    return res;
}

#endif
