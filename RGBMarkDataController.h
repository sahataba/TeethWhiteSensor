//
//  RGBMarkDataController.h
//  TeethWhiteSensor
//
//  Created by Rudolf Markulin on 6/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "RGBMark.h"

@interface RGBMarkDataController : NSObject

@property (nonatomic, retain) NSMutableArray *rgbMarkDataList;

- (NSUInteger)countOfList;
- (RGBMark *)objectInListAtIndex:(NSUInteger)theIndex;
- (void)addRGBMarkWithDate:(NSDate *)date r:(int)r g:(int)g b:(int)b;
- (void)removeRGBMarkAtIndex:(NSUInteger)theIndex;

@end
