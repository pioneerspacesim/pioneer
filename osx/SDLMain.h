//
// Objective-C cocoa wrapper for pioneer

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject {
}

- (IBAction)openURL:(id)sender;
- (IBAction)openFile:(id)sender;
- (IBAction)openAboutPanel:(id)sender;

@end

@interface SDLApplication : NSApplication
- (IBAction) terminate:(id)sender;
@end
