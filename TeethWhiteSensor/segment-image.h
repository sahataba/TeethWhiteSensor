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
};

typedef struct
{
    image<rgb> *image;
    std::map<int, rgbb> averages;
    rgbb totalAvg; 
} SegmentResult;

static inline hsl RGBToHSL(rgbb rgb) {
	float mincolor = fminf(fminf(rgb.r, rgb.g), rgb.b);
	float maxcolor = fmaxf(fmaxf(rgb.r, rgb.g), rgb.b);
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
    
	if (rgb.r == maxcolor)
		hsl.h = (rgb.g - rgb.b)/(maxcolor - mincolor);
	else if (rgb.g == maxcolor)
		hsl.h = 2.0 + (rgb.b - rgb.r)/(maxcolor - mincolor);
	else
		hsl.h = 4.0 + (rgb.r - rgb.g)/(maxcolor - mincolor);
    
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

// random color
rgb random_rgb(){ 
  rgb c;
  double r;
  
  c.r = (uchar)random();
  c.g = (uchar)random();
  c.b = (uchar)random();

  return c;
}

// dissimilarity measure between pixels
static inline float diff(image<float> *r, image<float> *g, image<float> *b,
			 int x1, int y1, int x2, int y2) {
  return sqrt(square(imRef(r, x1, y1)-imRef(r, x2, y2)) +
	      square(imRef(g, x1, y1)-imRef(g, x2, y2)) +
	      square(imRef(b, x1, y1)-imRef(b, x2, y2)));
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
SegmentResult segment_image(image<rgb> *im, float sigma, float c, int min_size,
			  int *num_ccs) {
  int width = im->width();
  int height = im->height();

  image<float> *r = new image<float>(width, height);
  image<float> *g = new image<float>(width, height);
  image<float> *b = new image<float>(width, height);

  // smooth each color channel  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      imRef(r, x, y) = imRef(im, x, y).r;
      imRef(g, x, y) = imRef(im, x, y).g;
      imRef(b, x, y) = imRef(im, x, y).b;
    }
  }
  image<float> *smooth_r = smooth(r, sigma);
  image<float> *smooth_g = smooth(g, sigma);
  image<float> *smooth_b = smooth(b, sigma);
  delete r;
  delete g;
  delete b;
 
  // build graph
  edge *edges = new edge[width*height*4];
  int num = 0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      if (x < width-1) {
	edges[num].a = y * width + x;
	edges[num].b = y * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y);
	num++;
      }

      if (y < height-1) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + x;
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x, y+1);
	num++;
      }

      if ((x < width-1) && (y < height-1)) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y+1);
	num++;
      }

      if ((x < width-1) && (y > 0)) {
	edges[num].a = y * width + x;
	edges[num].b = (y-1) * width + (x+1);
	edges[num].w = diff(smooth_r, smooth_g, smooth_b, x, y, x+1, y-1);
	num++;
      }
    }
  }
  delete smooth_r;
  delete smooth_g;
  delete smooth_b;

  // segment
  universe *u = segment_graph(width*height, num, edges, c);
  
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
    
    printf("YYY + %i", num_ccs[0]);
    
    std::map<int, hsl> sumColors;
    
  for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
          int comp = u->find(y * width + x);
          rgb newc = imRef(im, x , y);
          rgbb newcc = {(int)newc.r, (int)newc.g, (int)newc.b, 0, 0};
          hsl newColor = RGBToHSL(newcc);

          std::map<int,hsl>::iterator i = sumColors.find (comp);
          hsl color;
          if (i == sumColors.end ()) {
              color.h = 0;
              color.s = 0;
              color.l = 0;
              sumColors.insert(std::pair<int,hsl>(comp, color));
          }
          else
          {
              color = i->second;
              i->second.h = color.h + newColor.h;
              i->second.s = color.s + newColor.s;
              i->second.l = color.l + newColor.l;
          }
      }
  }

    std::map<int, rgbb> test;
    
  
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = u->find(y * width + x);
        int size = u->size(comp); 
        std::map<int,hsl>::iterator i = sumColors.find (comp);
        
        hsl color;
        if (i == sumColors.end ()) {
        }
        else
        {
            color = i->second;
        }
        
        hsl avgHSL = {color.h/size, color.s/size, color.l/size};
        rgbb avgRGB = HSLToRGB(avgHSL);
        
        int distRG = abs(avgRGB.r - avgRGB.g);
        int distGB = abs(avgRGB.g - avgRGB.b);

        //if ( distRG < 40 && distGB < 100 && size > 200 && size < 5000) {
        if ( avgRGB.r > 90 && avgRGB.g > 90 && avgRGB.b > 90 && distRG < 30) {
            imRef(output, x, y) = imRef(im,x,y);
            //imRef(output, x, y) = colors[comp];

            rgbb a = {avgRGB.r,avgRGB.g, avgRGB.b, x, y};
            test.insert (std::pair<int,rgbb>(comp, a) );                     
        }
        else {
            imRef(output, x, y) = colors[comp];
        }
    }
  }  
    
    std::map<int,hsl>::iterator tot;
    
    float totalH = 0;
    float totalS = 0;
    float totalL = 0;
    int totalSize = 0;
    
    for ( tot=sumColors.begin() ; tot != sumColors.end(); tot++ ){
        hsl rr = tot->second;
        int comp = tot->first;
        int size = u -> size(comp);
        if (test.count(comp) > 0 ) {
            totalH = totalH + rr.h;
            totalS = totalS + rr.s;
            totalL = totalL + rr.l;
            totalSize = totalSize + size;
        }
    }
    
  delete [] colors;  
  delete u;
    
    hsl avg = {totalH/totalSize, totalS/totalSize, totalL/totalSize};
    rgbb avgRGB = HSLToRGB(avg);

    rgbb a = {avgRGB.r,avgRGB.g,avgRGB.b, 0 , 0};
    
    SegmentResult res = {output, test, a};
    return res;
}

#endif
