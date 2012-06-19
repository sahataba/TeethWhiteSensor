//
//  RGBMarkDataController.m
//  TeethWhiteSensor
//
//  Created by Rudolf Markulin on 6/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "RGBMarkDataController.h"

@implementation RGBMarkDataController

@synthesize rgbMarkDataList = _rgbMarkDataList;

- (void)initializeDefaultDataList {
    NSMutableArray *sightingList = [[NSMutableArray alloc] init];
    self.rgbMarkDataList = sightingList;
    NSDate *today = [NSDate date];

    [self addRGBMarkWithDate:today r:0 g:0 b:0];
}

- (void)setRGBMarkDataList:(NSMutableArray *)newList {
    if (self.rgbMarkDataList != newList) {
        self.rgbMarkDataList = [newList mutableCopy];
    }
}

- (id)init {
    if (self = [super init]) {
        [self initializeDefaultDataList];
    }
    return self;
}

- (NSUInteger)countOfList {
    return [self.rgbMarkDataList count];
}

- (RGBMark *)objectInListAtIndex:(NSUInteger)theIndex {
    return [self.rgbMarkDataList objectAtIndex:theIndex];
}

- (void)addRGBMarkWithDate:(NSDate *)date r:(int *)r g:(int *)g b:(int *)b {
    RGBMark *mark;
    
    mark = [[RGBMark alloc] initWithDate:date r:r g:g b:b];
    NSLog(@"RGB");
    [self.rgbMarkDataList addObject:mark];
}

@end
