//
//  RGBMark.h
//  TeethWhiteSensor
//
//  Created by Rudolf Markulin on 6/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface RGBMark : NSObject
@property (nonatomic) int r;
@property (nonatomic) int g;
@property (nonatomic) int b;
@property (nonatomic) int s;
@property (nonatomic, strong) NSDate *date;

-(id)initWithDate:(NSDate *)date r:(int)r g:(int)g b:(int)b s:(int)s;

@end
