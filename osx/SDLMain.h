// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See COPYING.txt for details

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

@end
