#include "game/game.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <iostream>
#include <cstring>
#include <GL/gl.h>
#include <GL/glx.h>
#include <csignal>


#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

// Helper to check for extension string presence.  Adapted from:
//   http://www.opengl.org/resources/features/OGLextensions/
static bool isExtensionSupported(const char *extList, const char *extension)
{
    const char *start;
    const char *where, *terminator;

    /* Extension names should not have spaces. */
    where = strchr(extension, ' ');
    if (where || *extension == '\0')
        return false;

    /* It takes a bit of care to be fool-proof about parsing the
       OpenGL extensions string. Don't be fooled by sub-strings,
       etc. */
    for (start=extList;;) {
        where = strstr(start, extension);

        if (!where)
            break;

        terminator = where + strlen(extension);

        if ( where == start || *(where - 1) == ' ' )
            if ( *terminator == ' ' || *terminator == '\0' )
                return true;

        start = terminator;
    }

    return false;
}


static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

int main() {
    const char* msg = "Hello, World";

    Display* display = XOpenDisplay(nullptr);
    int screen = DefaultScreen(display);

    static int visual_attribs[] = {
            GLX_X_RENDERABLE, True,
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DOUBLEBUFFER, True,
            GLX_SAMPLE_BUFFERS, 1,
            GLX_SAMPLES, 4,
            None
    };

    int glx_major, glx_minor;
    auto rval = glXQueryVersion(display, &glx_major, &glx_minor);
    if (!rval || ((glx_major == 1 && glx_minor < 3) || glx_major < 1)) {
        std::cout << "Invalid GLX version" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Getting matching framebuffer configs\n";

    int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, screen, visual_attribs, &fbcount);
    if (!fbc) {
        std::cout << "Failed to retrieve a framebuffer config" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Found " << fbcount << " matching FB configs.\n";

    std::cout << "Getting XVisualInfos\n";
    int best_fbc = -1;
    int worst_fbc = -1;
    int best_num_samp = -1;
    int worst_num_samp = 999;

    for (int i = 0 ; i < fbcount ; i++) {
        XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi) {
            int samp_buf, samples;
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
            glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLES, &samples);

            printf("\tMatching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d, SAMPLES = %d\n", i, vi->visualid, samp_buf, samples);

            if (best_fbc < 0 || samp_buf && samples > best_num_samp) {
                best_fbc = i;
                best_num_samp = samples;
            }

            if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp) {
                worst_fbc = i;
                worst_num_samp = samples;
            }
        }
        XFree(vi);
    }

    GLXFBConfig  bestFbc = fbc[best_fbc];

    XFree(fbc);

    XVisualInfo *vi = glXGetVisualFromFBConfig(display, bestFbc);

    std::cout << "Chosen visual ID = 0x" << std::hex << vi->visualid << "\n";

    std::cout << "Creating colormap\n";
    XSetWindowAttributes swa;
    Colormap cmap;
    swa.colormap = cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
    swa.background_pixmap = None;
    swa.border_pixel = 0;
    swa.event_mask = StructureNotifyMask;

    std::cout << "Creating Window";
    Window window = XCreateWindow(display, RootWindow(display, vi->screen), 0, 0, 800, 800, 0, vi->depth, InputOutput, vi->visual, CWBorderPixel | CWColormap | CWEventMask, &swa);
    if (!window) {
        std::cout << "Failed to create window." << std::endl;
        return EXIT_FAILURE;
    }

    XFree(vi);

    XStoreName(display, window, "My Window");

    std::cout << "Mapping Window\n";
    XMapWindow(display, window);

    const char *glxExts = glXQueryExtensionsString(display, DefaultScreen(display));

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*) "glXCreateContextAttribsARB");
    GLXContext ctx = 0;
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);

    if (!isExtensionSupported(glxExts, "GLX_ARB_create_context") || !glXCreateContextAttribsARB) {
        std::cout << "glXCreateContextAttribsARB() not found... Using old-style GLX context\n";

        ctx = glXCreateNewContext(display, bestFbc, GLX_RGBA_TYPE, 0, True);
    } else {
        int context_attribs[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
                GLX_CONTEXT_MINOR_VERSION_ARB, 6,
                None
        };

        std::cout << "Creating Context\n";

        ctx = glXCreateContextAttribsARB(display, bestFbc, 0, True, context_attribs);
        XSync(display, False);
        if (!ctxErrorOccurred && ctx) {
            std::cout << "Created GL 4.6 context\n";
        } else {
            context_attribs[1] = 1;
            context_attribs[3] = 0;
            ctxErrorOccurred = false;
            std::cout << "Failed to create GL 4.6 context... Using old-style GLX context\n";
            ctx = glXCreateContextAttribsARB(display, bestFbc, 0, True, context_attribs);
        }
    }
    XSync(display, False);

    XSetErrorHandler(oldHandler);

    if (ctxErrorOccurred || !ctx) {
        std::cout << "Failed to create an OpenGL context." << std::endl;
        return EXIT_FAILURE;
    }

    if (!glXIsDirect(display, ctx)) {
        std::cout << "Indirect GLX rendering context obtained\n";
    } else {
        std::cout << "Direct GLX rendering context obtained\n";
    }

    std::cout << "Making context current\n";
    glXMakeCurrent(display, window, ctx);

    glClearColor(0, 0.5, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glXSwapBuffers(display, window);

    glXMakeCurrent(display, 0, 0);
    glXDestroyContext(display, ctx);

    XDestroyWindow(display, window);
    XFreeColormap(display, cmap);
    XCloseDisplay(display);
    XEvent event;

    auto escape_kc = XKeysymToKeycode(display, XK_Escape);

    while (true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            auto keycode = event.xkey.keycode;
            if (keycode == escape_kc) {
                std::cout << "Closing!" << std::endl;
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}