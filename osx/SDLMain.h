//
// Objective-C cocoa wrapper for pioneer

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject
- (void) help:(id)sender;
@end

@interface SDLApplication : NSApplication
- (void) terminate:(id)sender;
@end
