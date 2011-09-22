//
// Objective-C cocoa wrapper for pioneer

#ifndef _SDLMain_h_
#define _SDLMain_h_

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject
@end

@interface SDLApplication : NSApplication
- (void) terminate:(id)sender;
@end

#endif /* _SDLMain_h_ */
