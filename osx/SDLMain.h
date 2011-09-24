//
// Objective-C cocoa wrapper for pioneer

#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject {
}

- (IBAction)authors:(id)sender;
- (IBAction)readme:(id)sender;
- (IBAction)quickstart:(id)sender;
- (IBAction)changelog:(id)sender;
- (IBAction)copying:(id)sender;
- (IBAction)fontCopying:(id)sender;
- (IBAction)luaCopying:(id)sender;
- (IBAction)musicCopying:(id)sender;

@end

@interface SDLApplication : NSApplication
- (IBAction) terminate:(id)sender;
@end
