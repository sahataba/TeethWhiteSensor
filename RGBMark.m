//
//  RGBMark.m
//  TeethWhiteSensor
//
//  Created by Rudolf Markulin on 6/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "RGBMark.h"

@implementation RGBMark

@synthesize r = _r, g = _g, b =_b, date = _date;
 
- (id)initWithDate:(NSDate *)date r:(int *)r g:(int *)g b:(int *)b
{
    self = [super init];
    if (self) {
        _r = r;
        _g = g;
        _b = b;
        _date = date;
        return self;
    }
    return nil;
}

@end
