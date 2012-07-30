//
//  Mark.h
//  TeethWhiteSensor
//
//  Created by Rudolf Markulin on 7/5/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreData/CoreData.h>


@interface Mark : NSManagedObject

@property (nonatomic, retain) NSNumber * blue;
@property (nonatomic, retain) NSDate * date;
@property (nonatomic, retain) NSNumber * green;
@property (nonatomic, retain) NSNumber * red;
@property (nonatomic, retain) NSNumber * h;
@property (nonatomic, retain) NSNumber * s;
@property (nonatomic, retain) NSNumber * l;


@end
