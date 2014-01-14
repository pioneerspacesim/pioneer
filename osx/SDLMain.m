// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

//
// Objective-C cocoa wrapper for pioneer

#include "buildopts.h"
#import <SDL/SDL.h>
#import "SDLMain.h"
#import <unistd.h>

static int    gArgc;
static char  **gArgv;

@implementation SDLApplication

// Invoked from the Quit menu item
- (void)terminate:(id)sender
{
    // Posts a SDL_QUIT event so SDL shutsdown
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
    [super terminate:sender];
}
@end

@implementation SDLMain

// Called when the internal event loop has just started running
- (void) applicationDidFinishLaunching: (__unused NSNotification *) note
{
    int status;

    // Hand off to main application code
    status = SDL_main (gArgc, gArgv);

    // We're done, thank you for playing
    exit(status);
}

// menuItem Actions

- (IBAction)openURL:(id)sender
{
    if ([[sender title] isEqualToString:@"Homepage"])
    {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://pioneerspacesim.net/"]];
    }
    else if ([[sender title] isEqualToString:@"Issue Tracker"])
    {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/pioneerspacesim/pioneer/issues"]];
    }
    else if ([[sender title] isEqualToString:@"pioneer Forums"])
    {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://pioneerspacesim.net/forum"]];
    }
    else if ([[sender title] isEqualToString:@"Chat with the dev team"])
    {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://pioneerspacesim.net/irc"]];
    }
}

- (IBAction)openAboutPanel:(__unused id)sender
{
    NSDictionary *options;
    NSImage *img;

    img = [NSImage imageNamed: @"pioneer-logo.icns"];
    options = [NSDictionary dictionaryWithObjectsAndKeys:
               img, @"ApplicationIcon",
               @"Copyright (C) 2011 - See AUTHORS.txt", @"Copyright",
               nil];

    [[NSApplication sharedApplication] orderFrontStandardAboutPanelWithOptions:options];
}

@end // SDLMain

#ifdef main
#  undef main
#endif

//
// Main application entry point
//
int main (int argc, char * argv[])
{
	// Detect if the option key is pressed on app launch
	NSUInteger flags = ([NSEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask);
	BOOL isOptionPressed = (0 != (flags & NSAlternateKeyMask));
	
    // Copy the arguments into a global variable
    // This is passed if we are launched by double-clicking
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
		// Launched from FINDER (app icon)
        gArgv = (char **) SDL_malloc(sizeof (char *) * 2);
        gArgv[0] = argv[0];
        gArgv[1] = "-mv"; // Modelviewer param when Option key is held down
		if (isOptionPressed) {
			gArgc = 2;
		} else {
			gArgc = 1;
		}
    } else {
		// Launched manually (via cmd-line)
        int i;
        gArgc = argc;
        gArgv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
        for (i = 0; i <= argc; i++)
            gArgv[i] = argv[i];
    }

    NSApplicationMain (argc, (const char **)argv);
    return 0;
}

