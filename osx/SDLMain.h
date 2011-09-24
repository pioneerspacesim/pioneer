//
// Objective-C cocoa wrapper for pioneer

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject {
}

- (IBAction)authors:(id)sender;

@end

@interface SDLApplication : NSApplication
- (IBAction) terminate:(id)sender;
@end
