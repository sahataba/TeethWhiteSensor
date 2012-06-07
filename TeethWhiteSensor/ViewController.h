//
//  ViewController.h
//  TeethWhiteSensor
//
//  Created by User on 5/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ViewController : UIViewController<UINavigationControllerDelegate, UIImagePickerControllerDelegate>

@property (weak, nonatomic) IBOutlet UIButton *takePhoto;
@property (strong, nonatomic) UIImagePickerController * picker;

- (IBAction)takePhotoTapped:(id)sender;

@end
