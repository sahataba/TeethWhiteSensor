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

#include "image.h"
#include "misc.h"
#include "pnmfile.h"
#include "segment-image.h"

@interface ViewController ()

@end

@implementation ViewController

@synthesize picker = _picker;
@synthesize takePhoto = _takePhoto;
@synthesize imageView = _imageView;

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
    
    UIImage *fullImage = (UIImage *) [info objectForKey:UIImagePickerControllerOriginalImage]; 
    //UIImage *thumbImage = [fullImage imageByScalingAndCroppingForSize:CGSizeMake(44, 44)];
   
    //NSData *imageData = [NSData dataWithData:UIImagePNGRepresentation(fullImage)];
    CGImageRef inImage = fullImage.CGImage;
    printf("P3\n");
    CGContextRef cgctx = [self createARGBBitmapContextFromImage:inImage];
    
    if (cgctx == NULL) 
    { 
        // error creating context
        return;
    }
    printf("960 539\n");
    // Get image width, height. We'll use the entire image.
    size_t w = CGImageGetWidth(inImage);
    size_t h = CGImageGetHeight(inImage);
    CGRect rect = {{0,0},{w,h}}; 
    
    
    
    // Draw the image to the bitmap context. Once we draw, the memory 
    // allocated for the context for rendering will then contain the 
    // raw image data in the specified color space.
    CGContextDrawImage(cgctx, rect, inImage); 
    printf("255\n");
    
    // Now we can get a pointer to the image data associated with the bitmap
    // context.
    unsigned char* data = (unsigned char*)CGBitmapContextGetData (cgctx);
    rgb data2[(int)w * (int)h];
    if (data != NULL)
    {   
        printf("W %i", (int)w);
        printf("H %i", (int)h);
        
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                //offset locates the pixel in the data from x,y.
                //4 for 4 bytes of data per pixel, w is width of one row of data.
                int offset = 4*((w*round(j))+round(i));
                int alpha =  data[offset];
                int red = data[offset+1];
                int green = data[offset+2];
                int blue = data[offset+3];
                data[offset] = alpha;
                data[offset + 1] = alpha;
                //printf("%i %i %i ", red, green, blue);
                rgb color = {red, green, blue};
                data2[(int)(w*round(j)+round(i))] = color;
                // **** You have a pointer to the image data ****
                
                // **** Do stuff with the data here ****
            }
            printf("\n");
        }
        
    }
    
    
    
    image<rgb> *input = new image<rgb>((int)w, (int)h);
    memcpy(input->data, data, w * h * sizeof(rgb));
    //(rgb *)imPtr(input, 0, 0) -> *data;
    
    float sigma = 0.8;
    float k = 200.0;
    int min_size = 200;
	
    //printf("loading input image.\n");
    //image<rgb> *input = loadPPM(argv[4]);
	
    printf("processing\n");
    int num_ccs; 
    image<rgb> *seg = segment_image(input, sigma, k, min_size, &num_ccs); 
    //savePPM(seg, argv[5]);
    
    printf("got %d components\n", num_ccs);
    
    // When finished, release the context
    CGContextRelease(cgctx); 
    // Free image data memory for the context
    if (data)
    {
        free(data);
    }
    
    self.imageView.image = fullImage;
    
}

#pragma Button actions
- (IBAction)takePhotoTapped:(id)sender {
    if (self.navigationController == Nil) {
        printf("BB");
    }
    
    if (self.picker == nil) {   
        
        // 1) Show status
        //[SVProgressHUD showWithStatus:@"Loading picker..."];
        
        // 2) Get a concurrent queue form the system
        //dispatch_queue_t concurrentQueue =
        //dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
        
        // 3) Load picker in background
        //dispatch_async(concurrentQueue, ^{
            
            self.picker = [[UIImagePickerController alloc] init];
            self.picker.delegate = self;
            self.picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
            self.picker.allowsEditing = NO;    
            
            // 4) Present picker in main thread
            //dispatch_async(dispatch_get_main_queue(), ^{
                [self presentModalViewController:_picker animated:YES];    
            //    [SVProgressHUD dismiss];
            //});
            
        //});        
        
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


@end
