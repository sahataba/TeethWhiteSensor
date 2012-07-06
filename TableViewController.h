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
@interface TableViewController : UITableViewController<ViewControllerDelegate, NSFetchedResultsControllerDelegate> {
@private
    NSPersistentStoreCoordinator *persistentStoreCoordinator;
    NSFetchedResultsController *fetchedResultsController;
    NSManagedObjectContext *managedObjectContext;
    NSManagedObjectModel *managedObjectModel;
}

@property (nonatomic, retain, readonly) NSPersistentStoreCoordinator *persistentStoreCoordinator;
@property (nonatomic, retain) NSFetchedResultsController *fetchedResultsController;
@property (nonatomic, retain, readonly) NSManagedObjectModel *managedObjectModel;
@property (nonatomic, retain) NSManagedObjectContext *managedObjectContext;

- (NSString *)applicationDocumentsDirectory;


@end
