//
// Objective-C cocoa wrapper for pioneer

#include "buildopts.h"
#import <SDL/SDL.h>
#import "SDLMain.h"
#import <sys/param.h> /* for MAXPATHLEN */
#import <unistd.h>

static int    gArgc;
static char  **gArgv;
static BOOL   gFinderLaunch;

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
- (void) applicationDidFinishLaunching: (NSNotification *) note
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
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.spacesimcentral.com/forum/viewforum.php?f=35"]];
    }
    else if ([[sender title] isEqualToString:@"Chat with the dev team"])
    {
        [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://pioneerspacesim.net/irc"]];
    }
}

- (IBAction)openFile:(id)sender
{
    if ([[sender title] isEqualToString:@"AUTHORS"])
    {
        [[NSWorkspace sharedWorkspace] openFile:@"AUTHORS.txt"];
    }
    else if ([[sender title] isEqualToString:@"Changelog"])
    {
        [[NSWorkspace sharedWorkspace] openFile:@"Changelog.txt"];
    }
    else if ([[sender title] isEqualToString:@"README"])
    {
        [[NSWorkspace sharedWorkspace] openFile:@"README.txt"];
    }
    else if ([[sender title] isEqualToString:@"Quickstart"])
    {
        [[NSWorkspace sharedWorkspace] openFile:@"Quickstart.txt"];
    }
    else if ([[sender title] isEqualToString:@"COPYING"])
    {
        [[NSWorkspace sharedWorkspace] openFile:@"COPYING.txt"];
    }
    else if ([[sender title] isEqualToString:@"fonts COPYING"])
    {
        [[NSWorkspace sharedWorkspace] openFile:@"fonts.COPYING.txt"];
    }
    else if ([[sender title] isEqualToString:@"lua COPYING"])
    {
        [[NSWorkspace sharedWorkspace] openFile:@"lua.COPYING.txt"];
    }
    else if ([[sender title] isEqualToString:@"music COPYING"])
    {
        [[NSWorkspace sharedWorkspace] openFile:@"music.COPYING.txt"];
    }
}

- (IBAction)openAboutPanel:(id)sender
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
    // Copy the arguments into a global variable
    // This is passed if we are launched by double-clicking
    if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 ) {
        gArgv = (char **) SDL_malloc(sizeof (char *) * 2);
        gArgv[0] = argv[0];
        gArgv[1] = NULL;
        gArgc = 1;
        gFinderLaunch = YES;
    } else {
        int i;
        gArgc = argc;
        gArgv = (char **) SDL_malloc(sizeof (char *) * (argc+1));
        for (i = 0; i <= argc; i++)
            gArgv[i] = argv[i];
        gFinderLaunch = NO;
    }

    NSApplicationMain (argc, argv);
    return 0;
}

