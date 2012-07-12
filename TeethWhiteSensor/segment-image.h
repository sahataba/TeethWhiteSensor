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


typedef struct
{
    std::map<int, rgbb> averages;
    rgbb totalAvg; 
} SegmentResult;

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

static inline float diff(unsigned char * im, int width, int height, int x1,int y1, int x2, int y2) {
    int offset1 = 4*((width*y1)+x1);
    int offset2 = 4*((width*y2)+x2);
    return sqrt(square(im[offset1+1]-im[offset2+1]) +
                square(im[offset1+2]-im[offset2+2]) +
                square(im[offset1+3]-im[offset2+3]));
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
    //unsigned char* compact = (unsigned char*)malloc(4 * width * height);
    //float PI2 = 2 * M_PI;
    
    NSDate *start = [NSDate date];
    
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
    
    universe *u = segment_graph(width*height, num, edges, c);
    
    for (int i = 0; i < num; i++) {
        int a = u->find(edges[i].a);
        int b = u->find(edges[i].b);
        if ((a != b) && ((u->size(a) < min_size) || (u->size(b) < min_size)))
            u->join(a, b);
    }
    delete [] edges;
    *num_ccs = u->num_sets();
    
    rgb *colors = new rgb[width*height];
    for (int i = 0; i < width*height; i++)
        colors[i] = random_rgb();
    
    printf("COMPS: %i", num_ccs[0]);
    
    std::map<int, rgbb> sumColors;
    
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int comp = u->find(y * width + x);
            int offset = 4*((width*y)+x);
            
            std::map<int,rgbb>::iterator i = sumColors.find (comp);
            if (i == sumColors.end ()) {
                rgbb color = {0,0,0,0,0};
                sumColors.insert(std::pair<int,rgbb>(comp, color));
            }
            else
            {
                i->second.r += im[offset + 1];
                i->second.g += im[offset + 2];
                i->second.b += im[offset + 3];
            }
        }
    }
    
    std::map<int, rgbb> test;
    
    float yellowStart = 30/360.0;
    float yellowEnd = 55/360.0;
    rgbb color;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int comp = u->find(y * width + x);
            int size = u->size(comp); 
            std::map<int,rgbb>::iterator i = sumColors.find (comp);
            
            if (i == sumColors.end ()) {
            }
            else
            {
                color = i->second;
            }
            float h,s,b,a;
            UIColor *col = [UIColor colorWithRed:(color.r/size)/255.0 green:(color.g/size)/255.0 blue:(color.b/size)/255.0 alpha:1.0];
            [col getHue:&h saturation:&s brightness:&b alpha:&a];
            if (h < yellowEnd && h > yellowStart) {
                rgbb a = {color.r/size,color.g/size, color.b/size, x, y};
                test.insert (std::pair<int,rgbb>(comp, a) );   
            }
            else {
                rgb bela = {0,0,0};
                setRGB(im, width, height, x, y, bela);
            }
        }
    }  
    
    std::map<int,rgbb>::iterator tot;
    
    int totalR = 0;
    int totalG = 0;
    int totalB = 0;
    int totalSize = 0;
    
    
    for ( tot=sumColors.begin() ; tot != sumColors.end(); tot++ ){
        int comp = tot->first;
        int size = u -> size(comp);
        if (test.count(comp) > 0 ) {
            totalR += tot->second.r;
            totalG += tot->second.g;
            totalB += tot->second.b;
            totalSize += size;
        }
    }
    
    delete [] colors;  
    delete u;
    
    rgbb tt = {totalR/totalSize, totalG/totalSize, totalB/totalSize};
    
    SegmentResult res = {test, tt};
    NSTimeInterval pass1 = [start timeIntervalSinceNow];
    NSLog(@"PASS: %f", pass1);
    return res;
}

#endif
