//
//  ViewController.h
//  TeethWhiteSensor
//
//  Created by User on 5/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#import "RGBMark.h"
@class ViewController;

@protocol ViewControllerDelegate //<NSObject>
- (void)didCancel:(ViewController *)controller;
- (void)didSave:(RGBMark *)mark;
@end


#import <UIKit/UIKit.h>
#import "TableViewController.h"

@interface ViewController : UIViewController<UINavigationControllerDelegate, UIImagePickerControllerDelegate>

@property (weak, nonatomic) IBOutlet UIButton *takePhoto;
@property (weak, nonatomic) IBOutlet UIImageView *imageView;
@property (strong, nonatomic) UIImagePickerController * picker;
@property (nonatomic, weak) id <ViewControllerDelegate> delegate;
@property (strong, nonatomic) RGBMark *mark;

- (IBAction)takePhotoTapped:(id)sender;

@end
