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

// menuItem Actions
- (IBAction)wwwHome:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://pioneerspacesim.net/"]];
}

- (IBAction)wwwIssueTracker:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"https://github.com/pioneerspacesim/pioneer/issues"]];
}

- (IBAction)wwwForums:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://www.spacesimcentral.com/forum/viewforum.php?f=35"]];
}

- (IBAction)wwwChat:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://pioneerspacesim.net/irc"]];
}

- (IBAction)authors:(id)sender {
    [[NSWorkspace sharedWorkspace] openFile:@"AUTHORS.txt"];
}

- (IBAction)readme:(id)sender {
    [[NSWorkspace sharedWorkspace] openFile:@"README.txt"];
}

- (IBAction)quickstart:(id)sender {
    [[NSWorkspace sharedWorkspace] openFile:@"Quickstart.txt"];
}

- (IBAction)changelog:(id)sender {
    [[NSWorkspace sharedWorkspace] openFile:@"Changelog.txt"];
}

- (IBAction)copying:(id)sender {
    [[NSWorkspace sharedWorkspace] openFile:@"COPYING.txt"];
}

- (IBAction)fontCopying:(id)sender {
    [[NSWorkspace sharedWorkspace] openFile:@"fonts.COPYING.txt"];
}

- (IBAction)luaCopying:(id)sender {
    [[NSWorkspace sharedWorkspace] openFile:@"lua.COPYING.txt"];
}

- (IBAction)musicCopying:(id)sender {
    [[NSWorkspace sharedWorkspace] openFile:@"music.COPYING.txt"];
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

