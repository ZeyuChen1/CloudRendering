//
//  AppDelegate.m
//  CloudRendering
//
//  Created by Zeyu Chen on 2022-03-19.
//

#import "AppDelegate.h"

@interface AppDelegate ()


@end

@implementation AppDelegate
{
    NSWindow* window;
    RendererView* rendererView;
}


- (id) init {
    self = [super init];
    // set up window
    window = [[NSWindow alloc] init];
    // set window stype
    [window setStyleMask:NSWindowStyleMaskResizable |
                         NSWindowStyleMaskClosable |
                         NSWindowStyleMaskMiniaturizable |
                         NSWindowStyleMaskTitled |
                         NSWindowStyleMaskFullSizeContentView];
    // set minimum size
    NSSize defaultWindowSize = CGSizeMake(1024, 768);
    NSRect windowFrame = { .origin = CGPointMake(0.0f, 0.0f), .size = defaultWindowSize };
    // set initial position and size
    [window setFrame:windowFrame display:YES];
    
    rendererView = [[RendererView alloc] init];
    
    window.contentView = rendererView;
    
    return self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    [window makeKeyAndOrderFront:self];
    [rendererView beginRendering];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return NO;
}


@end
