//
// Objective-C cocoa wrapper for pioneer

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

// Set the working directory to the Resources folder in the app bundle
- (void) setupWorkingDirectory:(BOOL)shouldChdir
{
    if (shouldChdir)
    {
        char parentdir[MAXPATHLEN];
        CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
        if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)parentdir, MAXPATHLEN)) {
            chdir(parentdir);   /* chdir to the binary app's parent */
        }
        CFRelease(url);
        CFRelease(url2);
    }
    
    NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
    [[NSFileManager defaultManager] changeCurrentDirectoryPath:resourcePath];
}

// Called when the internal event loop has just started running
- (void) applicationDidFinishLaunching: (NSNotification *) note
{
    int status;

    [self setupWorkingDirectory:gFinderLaunch];

    // Hand off to main application code
    status = SDL_main (gArgc, gArgv);

    // We're done, thank you for playing
    exit(status);
}

- (IBAction)wwwHome:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://pioneerspacesim.net/"]])
    {
        NSLog(@"ERROR: Unable to open URL http://pioneerspacesim.net/\n");
    }
}

- (IBAction)wwwIssueTracker:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/pioneerspacesim/pioneer/issues"]])
    {
        NSLog(@"ERROR: Unable to open URL https://github.com/pioneerspacesim/pioneer/issues\n");
    }
}

- (IBAction)wwwForums:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.spacesimcentral.com/forum/viewforum.php?f=35"]])
    {
        NSLog(@"ERROR: Unable to open URL http://www.spacesimcentral.com/forum/viewforum.php?f=35\n");
    }
}

- (IBAction)wwwChat:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://pioneerspacesim.net/irc"]])
    {
        NSLog(@"ERROR: Unable to open URL http://pioneerspacesim.net/irc\n");
    }
}

- (IBAction)authors:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openFile:@"AUTHORS.txt"])
    {
        NSLog(@"ERROR: Unable to open AUTHORS.TXT file\n");
    }
}

- (IBAction)readme:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openFile:@"README.txt"])
    {
        NSLog(@"ERROR: Unable to open README.TXT file\n");
    }
}

- (IBAction)quickstart:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openFile:@"Quickstart.txt"])
    {
        NSLog(@"ERROR: Unable to open Quickstart.TXT file\n");
    }
}

- (IBAction)changelog:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openFile:@"Changelog.txt"])
    {
        NSLog(@"ERROR: Unable to open Changelog.TXT file\n");
    }
}

- (IBAction)copying:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openFile:@"COPYING.txt"])
    {
        NSLog(@"ERROR: Unable to open COPYING.TXT file\n");
    }
}

- (IBAction)fontCopying:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openFile:@"fonts.COPYING.txt"])
    {
        NSLog(@"ERROR: Unable to open fonts.COPYING.TXT file\n");
    }
}

- (IBAction)luaCopying:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openFile:@"lua.COPYING.txt"])
    {
        NSLog(@"ERROR: Unable to open lua.COPYING.TXT file\n");
    }
}

- (IBAction)musicCopying:(id)sender {
    if (![[NSWorkspace sharedWorkspace] openFile:@"music.COPYING.txt"])
    {
        NSLog(@"ERROR: Unable to open music.COPYING.TXT file\n");
    }
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

