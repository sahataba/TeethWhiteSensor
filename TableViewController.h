//
//  TableViewControllerViewController.h
//  TeethWhiteSensor
//
//  Created by Rudolf Markulin on 6/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TableViewController.h"
@class RGBMarkDataController;
@interface TableViewController : UITableViewController

@property (strong, nonatomic) RGBMarkDataController *dataController;

@end
