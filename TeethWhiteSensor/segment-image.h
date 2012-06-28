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
    float alpha;
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

// dissimilarity measure between pixels
static inline float diff(image<rgb> *im,
			 int x1, int y1, int x2, int y2) {
    rgb c1 = imRef(im,x1,y1);
    rgb c2 = imRef(im,x2,y2);
  return sqrt(square(c1.r-c2.r) +
	      square(c1.g-c2.g) +
	      square(c1.b-c2.b));
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
SegmentResult segment_image(image<rgb> *im, float sigma, float c, int min_size,
			  int *num_ccs) {
    
    float PI2 = 2 * M_PI;
    
    NSDate *start = [NSDate date];
  int width = im->width();
  int height = im->height();

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
	edges[num].w = diff(im, x, y, x+1, y);
	num++;
      }

      if (y < height-1) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + x;
	edges[num].w = diff(im, x, y, x, y+1);
	num++;
      }

      if ((x < width-1) && (y < height-1)) {
	edges[num].a = y * width + x;
	edges[num].b = (y+1) * width + (x+1);
	edges[num].w = diff(im, x, y, x+1, y+1);
	num++;
      }

      if ((x < width-1) && (y > 0)) {
	edges[num].a = y * width + x;
	edges[num].b = (y-1) * width + (x+1);
	edges[num].w = diff(im, x, y, x+1, y-1);
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

    
    CGFloat hue;
    CGFloat saturation;
    CGFloat brightness;
    CGFloat alpha;
  for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
          int comp = u->find(y * width + x);
          rgb newc = imRef(im, x , y);
          UIColor *newcc = [UIColor colorWithRed:((int)newc.r)/255.0 green:((int)newc.g)/255.0 blue:((int)newc.b)/255.0 alpha:1.0];
          [newcc getHue:&hue saturation:&saturation brightness:&brightness alpha:&alpha];
          hsl newColor = {hue,saturation,brightness,alpha};

          std::map<int,hslCart>::iterator i = sumColors.find (comp);
          hslCart color;
          if (i == sumColors.end ()) {
              color.x = 0;
              color.y = 0;
              color.s = 0;
              color.l = 0;
              color.alpha = 1;
              sumColors.insert(std::pair<int,hslCart>(comp, color));
          }
          else
          {
              color = i->second;
              i->second.x = color.x + sinf(newColor.h * PI2);
              i->second.y = color.y + cosf(newColor.h * PI2);
              i->second.s = color.s + newColor.s;
              i->second.l = color.l + newColor.l;
              i->second.alpha = color.alpha + newColor.alpha;

          }
      }
  }

    std::map<int, hslxy> test;
    
    NSTimeInterval pass6 = [start timeIntervalSinceNow];

    CGFloat yr;
    CGFloat yg;
    CGFloat yb;
    CGFloat yalpha;
    float yellowStart = 30/360.0;
    float yellowEnd = 55/360.0;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int comp = u->find(y * width + x);
        int size = u->size(comp); 
        std::map<int,hslCart>::iterator i = sumColors.find (comp);
        
        hslCart color;
        if (i == sumColors.end ()) {
        }
        else
        {
            color = i->second;
        }

        float atanN = atan2f(color.y/size, color.x/size);
        if (atanN < 0) atanN = atanN + PI2;
        
        hsl avgHSL = {atanN/ PI2, color.s/size, color.l/size, color.alpha/size};
        
        if ( (avgHSL.h < yellowEnd && avgHSL.h > yellowStart) ||  avgHSL.l > 0.95) {

            imRef(output, x, y) = imRef(im,x,y);

            hslxy a = {avgHSL.h,avgHSL.s, avgHSL.l, avgHSL.alpha, x, y};
            test.insert (std::pair<int,hslxy>(comp, a) );                     
        }
        else {
            imRef(output, x, y) = colors[comp];
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
        hslCart rr = tot->second;
        int comp = tot->first;
        int size = u -> size(comp);
        if (test.count(comp) > 0 ) {
            totalX = totalX + rr.x;
            totalY = totalY + rr.y;
            totalS = totalS + rr.s;
            totalL = totalL + rr.l;
            totalSize = totalSize + size;
        }
    }
    
  delete [] colors;  
  delete u;
    
    
    float atanT = atan2f(totalY/totalSize, totalX/totalSize);
    if (atanT < 0) atanT = atanT + PI2;
    
    hsl avg = {atanT / PI2, totalS/totalSize, totalL/totalSize};
    
    UIColor *avgx = [UIColor colorWithHue:avg.h saturation:avg.s brightness:avg.l alpha:avg.alpha];
    /*
    CGFloat yr;
    CGFloat yg;
    CGFloat yb;
    CGFloat yalpha;*/
    [avgx getRed:&yr green:&yg blue:&yb alpha:&yalpha];
    rgbb avgRGB2 = {((int)(yr*255.0)),((int)(yg*255.0)),((int)(yb*255.0)), 0 ,0};
    
    rgbb a = {avgRGB2.r,avgRGB2.g,avgRGB2.b, 0 , 0};

    SegmentResult res = {output, test, a};
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
