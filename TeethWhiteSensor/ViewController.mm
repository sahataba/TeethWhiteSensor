//
//  ViewController.m
//  TeethWhiteSensor
//
//  Created by User on 5/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ViewController.h"
#import "SVProgressHUD.h"
#import "UIKit/UIKit.h"
#import "UIImage+AverageColor.h"

#include "image.h"
#include "misc.h"
#include "segment-image.h"

@interface ViewController ()

@end

@implementation ViewController

@synthesize picker = _picker;
@synthesize takePhoto = _takePhoto;
@synthesize imageView = _imageView;
@synthesize delegate;
@synthesize mark;

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}

#pragma mark UIImagePickerControllerDelegate

- (void)imagePickerControllerDidCancel:(UIImagePickerController *)picker {
    [self dismissModalViewControllerAnimated:YES];
}

- (void)imagePickerController:(UIImagePickerController *)picker didFinishPickingMediaWithInfo:(NSDictionary *)info {
    [self dismissModalViewControllerAnimated:YES];
    
    UIImage *fullImage = (UIImage *) [info objectForKey:UIImagePickerControllerEditedImage]; 
    //UIImage *thumbImage = [fullImage imageByScalingAndCroppingForSize:CGSizeMake(44, 44)];
    
    CGImageRef inImage = fullImage.CGImage;
    CGContextRef cgctx = [self createARGBBitmapContextFromImage:inImage];
    
    if (cgctx == NULL) 
    { 
        // error creating context
        return;
    }
    
    // Get image width, height. We'll use the entire image.
    size_t w = CGImageGetWidth(inImage);
    size_t h = CGImageGetHeight(inImage);
    CGRect rect = {{0,0},{w,h}}; 
    
    // Draw the image to the bitmap context. Once we draw, the memory 
    // allocated for the context for rendering will then contain the 
    // raw image data in the specified color space.
    CGContextDrawImage(cgctx, rect, inImage); 
    
    // Now we can get a pointer to the image data associated with the bitmap
    // context.
    unsigned char* data = (unsigned char*)CGBitmapContextGetData (cgctx);
    
    float sigma = 0.8;
    float k =100;
    int min_size = 200;
    
    int num_ccs; 
    SegmentResult res = segment_image(data, w, h, sigma, k, min_size, &num_ccs); 
    std::map<int, rgbb> averages = res.averages;
    
    // When finished, release the context
    CGContextRelease(cgctx); 
    // Free image data memory for the context
    if (data)
    {
        //free(data);
    }
    
    fullImage = [self createUIImage : data : inImage];
    
    NSString *myWatermarkText = @"Watermark";
    UIImage *watermarkedImage = nil;
    
    UIGraphicsBeginImageContext(fullImage.size);
    [fullImage drawAtPoint: CGPointZero];
    
    std::map<int,rgbb>::iterator it;
    for ( it=averages.begin() ; it != averages.end(); it++ ){
        rgbb r = (*it).second;
        myWatermarkText = [NSString stringWithFormat : @"[%i]", r.r];
        [myWatermarkText drawAtPoint: CGPointMake(r.x, r.y) withFont: [UIFont systemFontOfSize: 12]];
    }
    
    myWatermarkText = [NSString stringWithFormat : @"[%i %i %i]", res.totalAvg.r,res.totalAvg.g,res.totalAvg.b ];
    [myWatermarkText drawAtPoint: CGPointMake(0, 0) withFont: [UIFont systemFontOfSize: 18]];
    
    watermarkedImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    self.imageView.image = watermarkedImage;
    rgbb total = res.totalAvg;
    RGBMark *tmp = [[RGBMark alloc] initWithDate:[NSDate date] r:total.r g:total.g b:total.b];
    mark = tmp;
}

#pragma Button actions
- (IBAction)takePhotoTapped:(id)sender {
    if (self.navigationController == Nil) {
    }
    
    if (self.picker == nil) {   
        
        // 1) Show status
        [SVProgressHUD showWithStatus:@"Loading picker..."];
        
        // 2) Get a concurrent queue form the system
        dispatch_queue_t concurrentQueue =
        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        
        // 3) Load picker in background
        dispatch_async(concurrentQueue, ^{
            
            self.picker = [[UIImagePickerController alloc] init];
            self.picker.delegate = self;
            self.picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
            self.picker.allowsEditing = YES;    
            
            // 4) Present picker in main thread
            dispatch_async(dispatch_get_main_queue(), ^{
                [self presentModalViewController:_picker animated:YES];    
                [SVProgressHUD dismiss];
            });
            
        });        
        
    }  else {        
        [self presentModalViewController:_picker animated:YES];    
    }
}

#pragma segment
- (CGContextRef) createARGBBitmapContextFromImage:(CGImageRef) inImage {
    
	CGContextRef    context = NULL;
	CGColorSpaceRef colorSpace;
	void *          bitmapData;
	int             bitmapByteCount;
	int             bitmapBytesPerRow;
    
	// Get image width, height. We'll use the entire image.
	size_t pixelsWide = CGImageGetWidth(inImage);
	size_t pixelsHigh = CGImageGetHeight(inImage);
    
	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	bitmapBytesPerRow   = (pixelsWide * 4);
	bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);
    
	// Use the generic RGB color space.
    colorSpace = CGColorSpaceCreateDeviceRGB();
	//colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	if (colorSpace == NULL)
	{
		fprintf(stderr, "Error allocating color space\n");
		return NULL;
	}
    
	// Allocate memory for image data. This is the destination in memory
	// where any drawing to the bitmap context will be rendered.
	bitmapData = malloc( bitmapByteCount );
	if (bitmapData == NULL)
	{
		fprintf (stderr, "Memory not allocated!");
		CGColorSpaceRelease( colorSpace );
		return NULL;
	}
    
	// Create the bitmap context. We want pre-multiplied ARGB, 8-bits
	// per component. Regardless of what the source image format is
	// (CMYK, Grayscale, and so on) it will be converted over to the format
	// specified here by CGBitmapContextCreate.
	context = CGBitmapContextCreate (bitmapData,
									 pixelsWide,
									 pixelsHigh,
									 8,      // bits per component
									 bitmapBytesPerRow,
									 colorSpace,
									 kCGImageAlphaPremultipliedFirst);
	if (context == NULL)
	{
		free (bitmapData);
		fprintf (stderr, "Context not created!");
	}
    
	// Make sure and release colorspace before returning
	CGColorSpaceRelease( colorSpace );
    
	return context;
}

-  (UIImage*) createUIImage:(unsigned char*) rawData:(CGImageRef) imageRef {
    
	size_t pixelsWide = CGImageGetWidth(imageRef);
	size_t pixelsHigh = CGImageGetHeight(imageRef);
    
	// Declare the number of bytes per row. Each pixel in the bitmap in this
	// example is represented by 4 bytes; 8 bits each of red, green, blue, and
	// alpha.
	int bitmapBytesPerRow   = (pixelsWide * 4);
	int bitmapByteCount     = (bitmapBytesPerRow * pixelsHigh);
    
    CGContextRef ctx = CGBitmapContextCreate(rawData,
                                             CGImageGetWidth( imageRef ),
                                             CGImageGetHeight( imageRef ),
                                             8,
                                             bitmapBytesPerRow,
                                             CGImageGetColorSpace( imageRef ),
                                             kCGImageAlphaPremultipliedFirst ); 
    
    imageRef = CGBitmapContextCreateImage (ctx);
    UIImage* rawImage = [UIImage imageWithCGImage:imageRef];  
    
    CGContextRelease(ctx);  
    
    free(rawData);
    return rawImage;
}

- (IBAction)done:(id)sender {
    [[self presentingViewController] dismissModalViewControllerAnimated:YES];
    [self.delegate didSave:self.mark];
}

- (UIImage *)imageWithColor:(UIColor *)color size:(float)size{
    CGRect rect = CGRectMake(0.0f, 0.0f, size, 1.0f);
    UIGraphicsBeginImageContext(rect.size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    CGContextSetFillColorWithColor(context, [color CGColor]);
    CGContextFillRect(context, rect);
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return image;
}

@end
