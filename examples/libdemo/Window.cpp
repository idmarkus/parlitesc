//
// Created by eti on 2023-07-08.
//

#include "Window.hpp"

#include <SDL_keyboard.h>
#include <SDL_video.h>

namespace PLSC::Demo
{
    Window::Window(const i32 width, const i32 height, b vsync) : Window(width, height, 0, 0, vsync) { }
    Window::Window(const i32 width, const i32 height) : Window(width, height, 0, 0, false) { }
    Window::Window(const i32 width, const i32 height, const i32 x, const i32 y, b vsync) :
        m_width(width), m_height(height), m_sync(vsync)
    {
        init(x, y);
    }

    void Window::init(const i32 pos_x, const i32 pos_y)
    {
        _crashIf(SDL_Init(SDL_INIT_EVERYTHING) < 0);

        // GL #version 410 core
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // CORE/ES
        // Disable multisampling
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
#if SDL_GL_DBG > 0
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
        m_window = SDL_CreateWindow("PLSC", pos_x, pos_y, m_width, m_height,
                                    SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        _crashIf(m_window == nullptr);

        m_glContext = SDL_GL_CreateContext(m_window);
        _crashIf(m_glContext == nullptr);

        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK)
        {
            std::cout << "GLEW Init != GLEW_OK: Error: " << glewGetErrorString(glewError) << std::endl;
            _exitFailure();
        }
        (void) glGetError(); // Clear errors caused by GLEW

        _crashIf(SDL_GL_SetSwapInterval(m_sync) < 0);

        SDL_StartTextInput();
    }

    Window::~Window()
    {
        SDL_StopTextInput();
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    bool Window::sizeChanged()
    {
        i32 w, h;
        SDL_GL_GetDrawableSize(m_window, &w, &h);
        if (w != m_width || h != m_height)
        {
            m_width  = w;
            m_height = h;
            glViewport(0, 0, w, h);
            return true;
        }
        return false;
    }

    void Window::setTitle(const std::string &title) { SDL_SetWindowTitle(m_window, title.c_str()); }
    void Window::setTitle(const char * title) { SDL_SetWindowTitle(m_window, title); }

    void Window::clear()
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Window::swap() { SDL_GL_SwapWindow(m_window); }

    bool Window::keyQuit()
    {
        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT: return true;
                case SDL_KEYDOWN:
                {
                    switch (e.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                        case 'q': return true;
                    }
                }
            }
        }
        return false;
    }
} // namespace PLSC::Demo