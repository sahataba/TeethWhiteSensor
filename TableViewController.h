//
//  TableViewControllerViewController.h
//  TeethWhiteSensor
//
//  Created by Rudolf Markulin on 6/18/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TableViewController.h"
#include "ViewController.h"

@class RGBMarkDataController;
@interface TableViewController : UITableViewController<ViewControllerDelegate>

@property (strong, nonatomic) RGBMarkDataController *dataController;

@end
