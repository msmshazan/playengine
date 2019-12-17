
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <shellscalingapi.h>
#undef NOMINMAX
#include <intrin.h>
#include <stdint.h>
#include <bgfx/c99/bgfx.h>
#define ZPL_IMPLEMENTATION
#include "zpl.h"
#include "lodepng.cpp"
#include "imgui.cpp"
#include "imgui_widgets.cpp"
#include "imgui_draw.cpp"
#include "imgui_demo.cpp"
#include "imgui_impl_win32.cpp"

typedef union
Vec2
{
    struct
    {
        float X,Y;
    };

    float Elements[2];

#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif

}
Vec2;

typedef union
Vec3
{
    struct
    {
        float X,Y,Z;
    };
    struct
    {
        float R,G,B;
    };
    struct
    {
        Vec2 XY;
        float Ignored0_;
    };
    struct
    {
        float Ignored1_;
        Vec2 YZ;
    };

    struct
    {
        Vec2 UV;
        float Ignored2_;
    };

    struct
    {
        float Ignored3_;
        Vec2 VW;
    };
    
    float Elements[3];
    
#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif

}
Vec3;

typedef union
Vec4
{
    struct
    {
        union
        {
            Vec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };

        float W;
    };
    struct
    {
        union
        {
            Vec3 RGB;
            struct
            {
                float R, G, B;
            };
        };

        float A;
    };

    struct
    {
        Vec2 XY;
        float Ignored0_;
        float Ignored1_;
    };

    struct
    {
        float Ignored2_;
        Vec2 YZ;
        float Ignored3_;
    };

    struct
    {
        float Ignored4_;
        float Ignored5_;
        Vec2 ZW;
    };
    float Elements[4];

#ifdef __cplusplus
    inline float &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif

}
Vec4;

typedef struct
Rect
{
    int X;
    int Y;
    int Width;
    int Height;
}
Rect;

typedef struct
Entity
{
    int64_t id;
}
Entity;

typedef struct
GameState
{
    Entity* Entities;
    int EntityCount;
}
GameState;

typedef struct
DrawCommand
{
    int64_t id;
}
DrawCommand;

typedef struct
Buffer
{
    void* data;
    size_t size;
}
Buffer;

typedef struct
Effect
{
    bgfx_program_handle_t ShaderProgram;
    bgfx_shader_handle_t VertexShader;
    bgfx_shader_handle_t PixelShader;
    bgfx_transform_t ViewTransform;
}
Effect;

typedef struct
Texture
{
    bgfx_texture_handle_t BGFXtexture;
    uint32_t Width;
    uint32_t Height;
    uint32_t BPP;
    Buffer PixelBuffer;
}
Texture;

typedef struct
VirtualTexture
{
    Texture* Texture;
    Rect Rect;
}
VirtualTexture;

ZPL_TABLE( , VTexHT, VTex, VirtualTexture)

typedef struct
RendererState
{
    DrawCommand* DrawCommands;
    int DrawCommandCount;
}
RendererState;

typedef struct
Animation
{
    int CurrentFrame;
    int MaxFrame;
    char* Name;
    float DuerationPerFrame;
}
Animation;

typedef struct
AppContext
{
    float dt;
    int width;
    int height;
    int TitleBarHeight;
    int MinMaxCloseButtonWidth;
    int MinMaxCloseButtonPadding;
    UINT NCMouseButton;
    LPARAM NCMousePos;
    void* Win32MessageProcFiber;
    void* Win32MainThreadFiber;
    bool Running;
    bool sizingloop;
    bool redraw;
    bool resizebuffer;
    Vec2 MousePos;
    uint32_t WindowColor;
    GameState GameState;
}
AppContext;


Vec2 AddVec2(Vec2 L , Vec2 R)
{
    Vec2 Result;
    Result.X = L.X + R.X;
    Result.Y = L.Y + R.Y;
    return Result;
}

Vec3 AddVec3(Vec3 L , Vec3 R)
{
    Vec3 Result;
    Result.X = L.X + R.X;
    Result.Y = L.Y + R.Y;
    Result.Z = L.Z + R.Z;
    return Result;
}

Vec4 AddVec4(Vec4 L , Vec4 R)
{
    Vec4 Result;
    Result.X = L.X + R.X;
    Result.Y = L.Y + R.Y;
    Result.Z = L.Z + R.Z;
    Result.W = L.W + R.W;
    return Result;
}
Vec2 SubVec2(Vec2 L , Vec2 R)
{
    Vec2 Result;
    Result.X = L.X - R.X;
    Result.Y = L.Y - R.Y;
    return Result;
}

Vec3 SubVec3(Vec3 L , Vec3 R)
{
    Vec3 Result;
    Result.X = L.X - R.X;
    Result.Y = L.Y - R.Y;
    Result.Z = L.Z - R.Z;
    return Result;
}

Vec4 SubVec4(Vec4 L , Vec4 R)
{
    Vec4 Result;
    Result.X = L.X - R.X;
    Result.Y = L.Y - R.Y;
    Result.Z = L.Z - R.Z;
    Result.W = L.W - R.W;
    return Result;
}


Buffer ReadEntireFile(const char* filename)
{
    Buffer Result = {};
    HANDLE FileHandle = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    LARGE_INTEGER size;
    GetFileSizeEx(FileHandle, &size);
    Result.size = size.QuadPart;
    Result.data = (void *) calloc(Result.size,1);
    ReadFile(FileHandle,(LPVOID)Result.data,Result.size,(LPDWORD)&Result.size,NULL);
    CloseHandle(FileHandle);
    return Result;
}

Effect LoadShader(const char* vsfile,const char* psfile)
{
    Effect Result = {};
    Buffer vsdata = ReadEntireFile(vsfile);
    Buffer psdata = ReadEntireFile(psfile);
    Result.VertexShader = bgfx_create_shader(bgfx_make_ref(vsdata.data,vsdata.size));
    Result.PixelShader = bgfx_create_shader(bgfx_make_ref(psdata.data,psdata.size));
    Result.ShaderProgram = bgfx_create_program(Result.VertexShader,Result.PixelShader,true);
    return Result;
}

Texture LoadPNG(const char* file)
{
    Texture Result = {};
    Buffer File =  ReadEntireFile(file);
    Result.BPP = 4;
    lodepng_decode_memory((unsigned char **)(&Result.PixelBuffer.data),&Result.Width,&Result.Height,(const unsigned char*)File.data,File.size,LCT_RGBA,32 / Result.BPP);
    Result.PixelBuffer.size = Result.Width*Result.Height*Result.BPP;
    uint32_t* pixelbase = (uint32_t *)(Result.PixelBuffer.data );
    Result.BGFXtexture = bgfx_create_texture_2d(Result.Width,Result.Height,false,1,BGFX_TEXTURE_FORMAT_RGBA8,BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT,bgfx_make_ref(Result.PixelBuffer.data,Result.PixelBuffer.size));
    return Result;
}

zpl_json_object ParseJSON(const char* file,bool hascomments)
{
    zpl_json_object Result ={};
    zpl_u8 Error;
    Buffer File =  ReadEntireFile(file);
    zpl_json_parse(&Result,File.size,(char *const)(File.data),zpl_heap(),hascomments,&Error);
    return Result;
}

zpl_json_object* SearchJSON(const char* Param , zpl_json_object *obj)
{
    return zpl_json_find(obj,Param,false);
}

void Win32EnableHIDPISupport()
{
    if(!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
    {
        if(!SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE))
        {
            SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
        }
    }
}


LRESULT CALLBACK Win32WindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    AppContext* App = (AppContext *)GetWindowLongPtr(hwnd,GWLP_USERDATA); 
    LRESULT result = {};
    if(ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
    {
        return true;
    }
    switch(uMsg)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
        break;
        case WM_NCCALCSIZE:
        {

        }
        break;
        case WM_SIZE:
        {
            if(App)
            {
                App->resizebuffer = true;
                int newwidth = LOWORD(lParam) ;
                int newheight =HIWORD(lParam) ;
                    
                {
                    if(newwidth != App->width)
                    {
                        App->width = newwidth;
                    }
                    if(newheight != App->height)
                    {
                        App->height = newheight;
                    }
                }
            }
            
        }
        break;
                
        case WM_CREATE:
        {

        }
        break;
        case WM_WINDOWPOSCHANGING:
        {
            if(App)
            {
                WINDOWPOS WPos = *((WINDOWPOS *) lParam);
                App->resizebuffer = true;
                
            }
        }
        break;
        case WM_TIMER:
        {
            if(wParam==1)
            {
                SwitchToFiber(App->Win32MainThreadFiber);
            }
        }
        break;
        case WM_ENTERSIZEMOVE:
        case WM_ENTERMENULOOP:
        {
            SetTimer(hwnd,1,1,NULL);
        }
        break;
        case WM_EXITSIZEMOVE:
        case WM_EXITMENULOOP:
        {
            KillTimer(hwnd, 1);
        }
        break;
        case WM_NCMOUSEMOVE:
        {
            if(App)
            {
                if(App->NCMouseButton == WM_NCLBUTTONDOWN)
                {
                    if(GET_X_LPARAM(App->NCMousePos) != GET_X_LPARAM(lParam) ||
                       GET_Y_LPARAM(App->NCMousePos) != GET_Y_LPARAM(lParam))
                    {
                        DefWindowProc(hwnd,WM_NCLBUTTONDOWN,HTCAPTION,App->NCMousePos);
                        App->NCMouseButton = 0;
                    }

                }
            }
        }
        break;
        case WM_MOUSEMOVE:
        {
            if(App)
            {
                if(App->NCMouseButton == WM_NCLBUTTONDOWN)
                {
                    if(GET_X_LPARAM(App->NCMousePos) != GET_X_LPARAM(lParam) ||
                       GET_Y_LPARAM(App->NCMousePos) != GET_Y_LPARAM(lParam))
                    {
                        DefWindowProc(hwnd,WM_NCLBUTTONDOWN,HTCAPTION,App->NCMousePos);
                        App->NCMouseButton = 0;
                    }

                }

                App->MousePos.X = GET_X_LPARAM(lParam);
                App->MousePos.Y = GET_Y_LPARAM(lParam);
            }
        }
        break;
#if 0
        case WM_NCLBUTTONDOWN:
        {
            if(wParam == HTCLOSE || wParam == HTMINBUTTON || wParam == HTMAXBUTTON)
            {
                if(wParam == HTCLOSE)
                {
                    SendMessage(hwnd,WM_CLOSE,NULL,NULL); 
                }
                if(wParam == HTMINBUTTON)
                {
                    ShowWindow(hwnd,SW_MINIMIZE); 
                }
                if(wParam == HTMAXBUTTON)
                {
                    WINDOWPLACEMENT WndPlcmnt={};
                    WndPlcmnt.length = sizeof(WINDOWPLACEMENT);
                    GetWindowPlacement(hwnd,&WndPlcmnt);
                    if(WndPlcmnt.showCmd == SW_MAXIMIZE)
                    {
                        ShowWindow(hwnd,SW_RESTORE);                    
                    }
                    else
                    {
                        ShowWindow(hwnd,SW_MAXIMIZE);
                    }
                }
                return 0;
            }
        
            if(wParam == HTCAPTION && App)
            {
                App->NCMouseButton = uMsg;
                App->NCMousePos = lParam; 
                return 0;
            }

        }
        break;
        case WM_NCLBUTTONDBLCLK:
        {
            if(wParam == HTCLOSE || wParam == HTMINBUTTON || wParam == HTMAXBUTTON)
            {
                if(wParam == HTCLOSE)
                {
                    SendMessage(hwnd,WM_CLOSE,NULL,NULL); 
                }
                if(wParam == HTMINBUTTON)
                {
                    ShowWindow(hwnd,SW_MINIMIZE); 
                }
                if(wParam == HTMAXBUTTON)
                {
                    WINDOWPLACEMENT WndPlcmnt={};
                    WndPlcmnt.length = sizeof(WINDOWPLACEMENT);
                    GetWindowPlacement(hwnd,&WndPlcmnt);
                    if(WndPlcmnt.showCmd == SW_MAXIMIZE)
                    {
                        ShowWindow(hwnd,SW_RESTORE);                    
                    }
                    else
                    {
                        ShowWindow(hwnd,SW_MAXIMIZE);
                    }
                }
                return 0;
            }
        }
        break;
        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO mmi = (LPMINMAXINFO ) lParam;
            HMONITOR monitor = MonitorFromWindow(hwnd,MONITOR_DEFAULTTONEAREST);
            MONITORINFOEX monitorinfo = {};
            monitorinfo.cbSize = sizeof(MONITORINFOEX);
            GetMonitorInfo(monitor,&monitorinfo);
            RECT rcWorkArea = monitorinfo.rcWork;
            RECT rcMonitorArea = monitorinfo.rcMonitor;
            mmi->ptMaxPosition.x = rcWorkArea.left - rcMonitorArea.left;
            mmi->ptMaxPosition.y = rcWorkArea.top - rcMonitorArea.top;
            mmi->ptMaxSize.x = rcWorkArea.right - rcMonitorArea.left;
            mmi->ptMaxSize.y = rcWorkArea.bottom - rcMonitorArea.top;
            mmi->ptMaxPosition.x = mmi->ptMaxPosition.x < 0 ? -mmi->ptMaxPosition.x : mmi->ptMaxPosition.x ;
            mmi->ptMaxPosition.y = mmi->ptMaxPosition.y < 0 ? -mmi->ptMaxPosition.y : mmi->ptMaxPosition.y ;
            mmi->ptMaxSize.x = mmi->ptMaxSize.x < 0 ? -mmi->ptMaxSize.x : mmi->ptMaxSize.x ;
            mmi->ptMaxSize.y = mmi->ptMaxSize.y < 0 ? -mmi->ptMaxSize.y : mmi->ptMaxSize.y ;
            mmi->ptMaxSize.x -= 1;
            //mmi->ptMaxSize.y += 8;
            return 0;
        }
        break;
        case WM_NCHITTEST:
        {
            if(App)
            {
                // Acquire the window rect
                RECT WindowRect={};
                GetWindowRect(hwnd, &WindowRect);
                int offset = App->MinMaxCloseButtonPadding;
                int caption_height = App->TitleBarHeight;
                int buttonwidth = App->MinMaxCloseButtonWidth;
                POINT CursorPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                result = HTNOWHERE;
                if (CursorPos.x > WindowRect.left && CursorPos.x < WindowRect.right)
                {
                    if (CursorPos.y < WindowRect.top + caption_height)
                    {
                        result = HTCAPTION;
                        if(CursorPos.x > (WindowRect.right - buttonwidth*3)  ) result = HTMINBUTTON;  
                        if(CursorPos.x > (WindowRect.right - buttonwidth*2)  ) result = HTMAXBUTTON;  
                        if(CursorPos.x > (WindowRect.right - buttonwidth*1)  ) result = HTCLOSE;  
                    }
                }
            
                if (CursorPos.x > WindowRect.left && CursorPos.x < WindowRect.right)
                {
                    if (CursorPos.y < WindowRect.top + offset)
                    {
                        result = HTTOP;
                    }
                    else if (CursorPos.y > WindowRect.bottom - offset)
                    {
                        result = HTBOTTOM;
                    }
                }
                if (CursorPos.y > WindowRect.top && CursorPos.y < WindowRect.bottom)
                {
                    if (CursorPos.x < WindowRect.left + offset)
                    {
                        result = HTLEFT;
                    }
                    else if (CursorPos.x > WindowRect.right - offset)
                    {
                        result = HTRIGHT;
                    }
                }

                if (CursorPos.y < WindowRect.top + offset && CursorPos.x < WindowRect.left + offset)
                {
                    result = HTTOPLEFT;
                }
                if (CursorPos.y < WindowRect.top + offset && CursorPos.x > WindowRect.right - offset) result = HTTOPRIGHT;
                if (CursorPos.y > WindowRect.bottom - offset && CursorPos.x > WindowRect.right - offset) result = HTBOTTOMRIGHT;
                if (CursorPos.y > WindowRect.bottom - offset && CursorPos.x < WindowRect.left + offset) result = HTBOTTOMLEFT;

                if (result != HTNOWHERE) return result;
            }
        }
        break;
#endif
        default:
        {
        }
        break;
        
    }
    result = DefWindowProc( hwnd, uMsg, wParam, lParam);
    return result;
}

void CALLBACK Win32MessageProc(LPVOID lpFiberParameter)
{
    AppContext* App = (AppContext *)lpFiberParameter;
    MSG WindowMessage = {};
    for(;;)
    {
        while(PeekMessage(&WindowMessage,NULL,0,0,PM_REMOVE))
        {                    
            switch(WindowMessage.message)
            {
                case WM_QUIT:
                {
                    App->Running = false;
                }
                break;
                default:
                {
                    TranslateMessage(&WindowMessage);
                    DispatchMessageW(&WindowMessage);
                }
                break;
            }
        }
        SwitchToFiber(App->Win32MainThreadFiber);
    }

}

void CALLBACK Win32FallbackMessageProc(LPVOID lpFiberParameter)
{
    AppContext* App = (AppContext *)lpFiberParameter;
    MSG WindowMessage = {};
    {
        while(PeekMessage(&WindowMessage,NULL,0,0,PM_REMOVE))
        {
            switch(WindowMessage.message)
            {
                case WM_QUIT:
                {
                    App->Running = false;
                }
                break;
                default:
                {
                }
                break;
            }
            TranslateMessage(&WindowMessage);
            DispatchMessage(&WindowMessage);
        }
    }

}



typedef struct
VertexPositionColor
{
    Vec3 Position;
    uint32_t Color;
}
VertexPositionColor;

typedef struct
VertexPositionTexture
{
    Vec3 Position;
    Vec2 TexUV;
}
VertexPositionTexture;

typedef struct
VertexPositionTextureColor
{
    Vec3 Position;
    Vec2 TexUV;
    uint32_t Color;
}
VertexPositionTextureColor;

void mtxOrtho(float* _result, float _left, float _right, float _bottom, float _top, float _near, float _far, float _offset, bool _homogeneousNdc, bool _isLeftHand = true)
{
    const float aa = 2.0f/(_right - _left);
    const float bb = 2.0f/(_top - _bottom);
    const float cc = (_homogeneousNdc ? 2.0f : 1.0f) / (_far - _near);
    const float dd = (_left + _right )/(_left   - _right);
    const float ee = (_top  + _bottom)/(_bottom - _top  );
    const float ff = _homogeneousNdc
        ? (_near + _far)/(_near - _far)
        :  _near        /(_near - _far)
        ;

    zpl_memset(_result, 0, sizeof(float)*16);
    _result[ 0] = aa;
    _result[ 5] = bb;
    _result[10] = _isLeftHand ? cc : -cc;
    _result[12] = dd + _offset;
    _result[13] = ee;
    _result[14] = ff;
    _result[15] = 1.0f;
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,int nShowCmd)
{
    AppContext App = {};
    App.width = 800;
    App.height = 600;
    App.TitleBarHeight = 30;
    App.MinMaxCloseButtonWidth = 60;
    App.MinMaxCloseButtonPadding = 2;
    App.WindowColor = 0xff1c1c1c;
    App.Win32MainThreadFiber = ConvertThreadToFiber(NULL);
    App.Win32MessageProcFiber = CreateFiber(0,&Win32MessageProc,(void *)&App);
    Win32EnableHIDPISupport();
    WNDCLASSEX WNDCLS = {};
    WNDCLS.cbSize = sizeof(WNDCLASSEX);
    WNDCLS.style = CS_HREDRAW | CS_VREDRAW|CS_OWNDC;
    WNDCLS.hInstance = hInstance;
    WNDCLS.hCursor = LoadCursor(NULL,IDC_ARROW);
    WNDCLS.lpszClassName = "testwindowclass";
    WNDCLS.lpfnWndProc = Win32WindowProcedure;
    WNDCLS.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    RegisterClassEx(&WNDCLS);
    HWND hwnd = CreateWindowEx(NULL, WNDCLS.lpszClassName,"test window",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,App.width,App.height,NULL,NULL,WNDCLS.hInstance,NULL);
    //SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE)&~WS_SIZEBOX);
    SetWindowLongPtr(hwnd, GWLP_USERDATA ,(LONG_PTR) &App);
    ShowWindow(hwnd,SW_HIDE);
    {
        bgfx_platform_data_t* BGFXPlatformData = (bgfx_platform_data_t *)calloc(1,sizeof(bgfx_platform_data_t));
        BGFXPlatformData->nwh = (void *)hwnd;
        bgfx_set_platform_data(BGFXPlatformData);
    }
    IMGUI_CHECKVERSION();
    bgfx_init_t BGFXInit = {};
    bgfx_init_ctor(&BGFXInit);
    BGFXInit.type = BGFX_RENDERER_TYPE_DIRECT3D11;
    BGFXInit.vendorId = BGFX_PCI_ID_NONE;
    BGFXInit.resolution.width = App.width;
    BGFXInit.resolution.height = App.height;
    BGFXInit.resolution.reset = BGFX_RESET_NONE;
    bool set = bgfx_init(&BGFXInit);
    //bgfx_set_debug(BGFX_DEBUG_STATS);
    bgfx_set_view_clear(0,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH,0x6495edff,0,0);
    bgfx_set_view_clear(1,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH,0x6495edff,0,0);
    bgfx_set_view_mode(0,BGFX_VIEW_MODE_SEQUENTIAL);
    bgfx_set_view_mode(1,BGFX_VIEW_MODE_SEQUENTIAL);
    bgfx_set_view_mode(255,BGFX_VIEW_MODE_SEQUENTIAL);
    ShowWindow(hwnd,SW_SHOW);
    App.Running = true;
    App.dt = 0;
    LARGE_INTEGER PerfCounter;
    QueryPerformanceCounter(&PerfCounter);
    App.resizebuffer = true;
    App.redraw = false;
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    {
        ImGuiIO* io = &ImGui::GetIO();
        io->Fonts->AddFontDefault();
        unsigned char* pixels;
        int fwidth, fheight, fbpp;
        io->Fonts->GetTexDataAsRGBA32(&pixels, &fwidth, &fheight, &fbpp);
        io->Fonts->TexID = (ImTextureID)bgfx_create_texture_2d(fwidth,fheight,false,1,BGFX_TEXTURE_FORMAT_RGBA8,BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT,bgfx_make_ref(pixels,fwidth*fheight*fbpp)).idx;    
    }
    Effect Shader = LoadShader("vs_texquad.bin","fs_texquad.bin");
    Effect ImGuiShader = LoadShader("vs_imgui.bin","fs_imgui.bin");
    Effect TilemapShader = LoadShader("vs_texquad.bin","fs_textilemap.bin");
    zpl_json_object AtlasJson = ParseJSON("ATLAS.json",false);
    zpl_json_object *AtlasNodes = &AtlasJson;
    VTexHT VirtualTextures = {};
    VTexinit(&VirtualTextures,zpl_heap_allocator());
    if (AtlasNodes != NULL) {
        for (zpl_isize Index = 0; Index < zpl_array_count(AtlasNodes->nodes); Index++) {
            zpl_json_object *AtlasNode = AtlasNodes->nodes + Index;
            zpl_json_object *AtlasLocation = zpl_json_find(AtlasNode, "Name", 0);
            zpl_json_object *TextureNodes = zpl_json_find(AtlasNode, "Images", 0);
            Texture Atlas = LoadPNG(AtlasLocation->string);
            if (TextureNodes) {
                for (zpl_isize TexIndex = 0; TexIndex < zpl_array_count(TextureNodes->nodes); ++TexIndex) {
                    zpl_json_object *TextureNode = TextureNodes->nodes + TexIndex;
                    zpl_json_object *TextureName = zpl_json_find(TextureNode, "Name", 0);
                    zpl_json_object *TextureX = zpl_json_find(TextureNode, "X", 0);
                    zpl_json_object *TextureY = zpl_json_find(TextureNode, "Y", 0);
                    zpl_json_object *TextureWidth = zpl_json_find(TextureNode, "Width", 0);
                    zpl_json_object *TextureHeight = zpl_json_find(TextureNode, "Height", 0);
                    VirtualTexture VTex = {};
                    VTex.Texture = &Atlas;
                    VTex.Rect.X = (int)TextureX->integer;
                    VTex.Rect.Y = (int)TextureY->integer;
                    VTex.Rect.Width = (int)TextureWidth->integer;
                    VTex.Rect.Height = (int)TextureHeight->integer;
                    VTexset(&VirtualTextures,zpl_murmur64(TextureName->string,strlen(TextureName->string)),VTex);
                }
            }        
        }
    }
    int16_t indices[] = {0,1,2,0,1,3};
    uint8_t FrameCount = 0;
    uint32_t WhiteTex[] = {0xffffffff};
    VertexPositionTexture FullScreenVertices[4] = {};
    VertexPositionTexture FullFrameBufferVertices[4] = {};
    char countstr[256] = {};
    sprintf(countstr,"%i",FrameCount);            
    VirtualTexture *Frame = VTexget(&VirtualTextures,zpl_murmur64(countstr,1));
    Texture Atlas = *(Frame->Texture);
    Texture TileTexture = LoadPNG("16x16Tiles.png");
    
    FullScreenVertices[0] = {{0,0,0},{0,240}};
    FullScreenVertices[1] = {{(float)App.width,(float)App.height,0},{320,0}};
    FullScreenVertices[2] = {{0,(float)App.height,0},{0,0}};
    FullScreenVertices[3] = {{(float)App.width,0,0},{320,240}};
    FullFrameBufferVertices[0] = {{0,0,0},{0,(float)TileTexture.Height}};
    FullFrameBufferVertices[1] = {{320,240,0},{(float)TileTexture.Width,0}};
    FullFrameBufferVertices[2] = {{0,240,0},{0,0}};
    FullFrameBufferVertices[3] = {{320,0,0},{(float)TileTexture.Width,(float)TileTexture.Height}};
#include "tilemapdata.cpp"
    bgfx_texture_handle_t LookupTexture = bgfx_create_texture_2d(20,15,false,1,BGFX_TEXTURE_FORMAT_RG8,BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT,NULL);
    bgfx_texture_handle_t WhitePixelTexture = bgfx_create_texture_2d(1,1,false,1,BGFX_TEXTURE_FORMAT_RG8,BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT,bgfx_make_ref(WhiteTex,sizeof(WhiteTex)));
    bgfx_update_texture_2d(LookupTexture,0,0,0,0,20,15,bgfx_make_ref(Tilemap,sizeof(Tilemap)),UINT16_MAX);
    bgfx_uniform_handle_t TextureUniform = bgfx_create_uniform("s_texture",BGFX_UNIFORM_TYPE_SAMPLER,1);
    bgfx_uniform_handle_t LookupTextureUniform = bgfx_create_uniform("l_texture",BGFX_UNIFORM_TYPE_SAMPLER,1);
    bgfx_uniform_handle_t TextureSizeInvUniform = bgfx_create_uniform("tex_sizeinv",BGFX_UNIFORM_TYPE_VEC4,1);
    bgfx_uniform_handle_t TileSizeUniform = bgfx_create_uniform("tile_size",BGFX_UNIFORM_TYPE_VEC4,1);
    bgfx_index_buffer_handle_t IndexBuffer = bgfx_create_index_buffer(bgfx_make_ref(indices,sizeof(indices)),BGFX_BUFFER_NONE);
    bgfx_vertex_layout_t VertexPositionTextureLayout = {};
    bgfx_vertex_layout_begin(&VertexPositionTextureLayout,BGFX_RENDERER_TYPE_NOOP);
    bgfx_vertex_layout_add(&VertexPositionTextureLayout,BGFX_ATTRIB_POSITION,3,BGFX_ATTRIB_TYPE_FLOAT,false,false);
    bgfx_vertex_layout_add(&VertexPositionTextureLayout,BGFX_ATTRIB_TEXCOORD0,2,BGFX_ATTRIB_TYPE_FLOAT,false,false);
    bgfx_vertex_layout_end(&VertexPositionTextureLayout);
    bgfx_vertex_layout_t ImGuiVertexLayout = {};
    bgfx_vertex_layout_begin(&ImGuiVertexLayout,BGFX_RENDERER_TYPE_NOOP);
    bgfx_vertex_layout_add(&ImGuiVertexLayout,BGFX_ATTRIB_POSITION,2,BGFX_ATTRIB_TYPE_FLOAT,false,false);
    bgfx_vertex_layout_add(&ImGuiVertexLayout,BGFX_ATTRIB_TEXCOORD0,2,BGFX_ATTRIB_TYPE_FLOAT,false,false);
    bgfx_vertex_layout_add(&ImGuiVertexLayout,BGFX_ATTRIB_COLOR0,4,BGFX_ATTRIB_TYPE_UINT8,true,false);
    bgfx_vertex_layout_end(&ImGuiVertexLayout);
    bgfx_dynamic_vertex_buffer_handle_t FullScreenVertexBuffer = bgfx_create_dynamic_vertex_buffer(4,&VertexPositionTextureLayout,BGFX_BUFFER_NONE);
    bgfx_dynamic_vertex_buffer_handle_t FullFrameBufferVertexBuffer = bgfx_create_dynamic_vertex_buffer(4,&VertexPositionTextureLayout,BGFX_BUFFER_NONE);
    bgfx_update_dynamic_vertex_buffer(FullFrameBufferVertexBuffer,0,bgfx_make_ref(FullFrameBufferVertices,sizeof(FullFrameBufferVertices)));
    bgfx_update_dynamic_vertex_buffer(FullScreenVertexBuffer,0,bgfx_make_ref(FullScreenVertices,sizeof(FullScreenVertices)));
        
    bgfx_frame_buffer_handle_t FrameBuffer = bgfx_create_frame_buffer(320,240,BGFX_TEXTURE_FORMAT_BGRA8,BGFX_SAMPLER_V_CLAMP | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT);
    float timerduration = (1.0f / 14.0f);
    float timer = timerduration;
    while(App.Running)
    {
        SwitchToFiber(App.Win32MessageProcFiber);
        if(!App.Running) break;
        LARGE_INTEGER NewPerfCounter;
        QueryPerformanceCounter(&NewPerfCounter);
        LARGE_INTEGER PerfFrequency;
        QueryPerformanceFrequency(&PerfFrequency);
        App.dt = ((float)(NewPerfCounter.QuadPart - PerfCounter.QuadPart) )*1000000;
        App.dt /= ((float)PerfFrequency.QuadPart);
        App.dt /= 1000000;
        PerfCounter = NewPerfCounter;
        timer -= App.dt;
        {
            ImGuiIO* io = &ImGui::GetIO();
            io->DisplaySize = ImVec2(App.width, App.height);
            io->DeltaTime   = App.dt;
            io->IniFilename = NULL;
        }
        bgfx_transient_vertex_buffer_t VertexBuffer = {};
        bgfx_alloc_transient_vertex_buffer(&VertexBuffer,4,&VertexPositionTextureLayout);
        if(App.resizebuffer)
        {
            bgfx_reset(App.width,App.height,BGFX_RESET_NONE,BGFXInit.resolution.format);
            FullScreenVertices[0] = {{0,0,0},{0,240}};
            FullScreenVertices[1] = {{(float)App.width,(float)App.height,0},{320,0}};
            FullScreenVertices[2] = {{0,(float)App.height,0},{0,0}};
            FullScreenVertices[3] = {{(float)App.width,0,0},{320,240}};
            bgfx_update_dynamic_vertex_buffer(FullScreenVertexBuffer,0,bgfx_make_ref(FullScreenVertices,sizeof(FullScreenVertices)));
            App.resizebuffer = false;
        }
    
        if(timer <= 0 )
        {
            timer += timerduration;
            {
                FrameCount++;
                if(FrameCount > 7) FrameCount = 0;
            }
        }
        char countstr[256] = {};
        sprintf(countstr,"%i",FrameCount);
        VirtualTexture *Frame = VTexget(&VirtualTextures,zpl_murmur64(countstr,1));
        Rect FrameRect = Frame->Rect;
        VertexPositionTexture* vertices = (VertexPositionTexture*)VertexBuffer.data;
        vertices[0] = {{0,0,0},{(float)FrameRect.X,(float)FrameRect.Y + FrameRect.Height}};
        vertices[1] = {{(float)FrameRect.Width,(float)FrameRect.Height,0},{(float)FrameRect.X +FrameRect.Width,(float)FrameRect.Y}};
        vertices[2] = {{0,(float)FrameRect.Height,0},{(float)FrameRect.X,(float)FrameRect.Y}};
        vertices[3] = {{(float)FrameRect.Width,0,0},{(float)FrameRect.X +FrameRect.Width,(float) FrameRect.Y + FrameRect.Height}};

        vertices[0].Position = AddVec3({100,100,0} ,vertices[0].Position); 
        vertices[1].Position = AddVec3({100,100,0} ,vertices[1].Position); 
        vertices[2].Position = AddVec3({100,100,0} ,vertices[2].Position); 
        vertices[3].Position = AddVec3({100,100,0} ,vertices[3].Position); 
        
        bgfx_set_view_rect(1,0,0,320,240);
        bgfx_set_view_rect(0,0,0,App.width,App.height);
        bgfx_set_view_rect(255,0,0,App.width,App.height);
        bgfx_set_view_frame_buffer(1,FrameBuffer);
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        if(ImGui::Begin("test"))
        {
            ImGui::Text("Hello SANIC");
        }
        ImGui::End();
        ImGui::EndFrame();
        ImGui::Render();
        {
            bgfx_set_dynamic_vertex_buffer(0,FullFrameBufferVertexBuffer,0,4);
            bgfx_set_index_buffer(IndexBuffer,0,6);
            Vec4 AtlasSizeInv = {1.0f /(float)TileTexture.Width,1.0f /(float) TileTexture.Height , (float)TileTexture.Width,(float) TileTexture.Height };
            bgfx_set_uniform(TextureSizeInvUniform,(void *)&AtlasSizeInv,1);
            Vec4 TileSize = {20,15,16,16};
            bgfx_set_uniform(TileSizeUniform,(void *)&TileSize,1);
            bgfx_set_texture(0,TextureUniform,TileTexture.BGFXtexture,UINT32_MAX);
            bgfx_set_texture(1,LookupTextureUniform,LookupTexture,UINT32_MAX);
            bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_BLEND_ALPHA ,0);
            bgfx_submit(1,TilemapShader.ShaderProgram,0,false);
        }
        
        {
            bgfx_set_transient_vertex_buffer(0,&(VertexBuffer),0,4);
            bgfx_set_index_buffer(IndexBuffer,0,6);
            Vec4 AtlasSizeInv = {1.0f /(float)Atlas.Width,1.0f /(float) Atlas.Height , (float)Atlas.Width,(float) Atlas.Height };
            bgfx_set_uniform(TextureSizeInvUniform,(void *)&AtlasSizeInv,1);
            bgfx_set_texture(0,TextureUniform,Atlas.BGFXtexture,UINT32_MAX);
            bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_BLEND_ALPHA ,0);
            bgfx_submit(1,Shader.ShaderProgram,0,false);
        }
        
        {
            bgfx_set_dynamic_vertex_buffer(0,FullScreenVertexBuffer,0,4);
            bgfx_set_index_buffer(IndexBuffer,0,6);
            Vec4 AtlasSizeInv = {1.0f /(float)320,1.0f /(float)240 , (float)320,(float)240 };
            bgfx_set_uniform(TextureSizeInvUniform,(void *)&AtlasSizeInv,1);
            bgfx_set_texture(0,TextureUniform,bgfx_get_texture(FrameBuffer,0),UINT32_MAX);
            bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_BLEND_ALPHA ,0);
            bgfx_submit(0,Shader.ShaderProgram,0,false);        
        }
        {
            ImDrawData* _drawData = ImGui::GetDrawData();
            int viewID = 255; 
            
            const bgfx_caps_t* caps = bgfx_get_caps();
            {
                float ortho[16] = {};
                mtxOrtho(ortho, 0.0f, App.width, App.height, 0.0f, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
                bgfx_set_view_transform(viewID, NULL, ortho);
            }

            // Render command lists
            for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
            {
                bgfx_transient_vertex_buffer_t tvb;
                bgfx_transient_index_buffer_t tib;

                const ImDrawList* drawList = _drawData->CmdLists[ii];
                uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
                uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

                bgfx_alloc_transient_vertex_buffer(&tvb, numVertices, &ImGuiVertexLayout);
                bgfx_alloc_transient_index_buffer(&tib, numIndices);

                ImDrawVert* verts = (ImDrawVert*)tvb.data;
                zpl_memcopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

                ImDrawIdx* indices = (ImDrawIdx*)tib.data;
                zpl_memcopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

                uint32_t offset = 0;
                for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
                {
                    if (cmd->UserCallback)
                    {
                        cmd->UserCallback(drawList, cmd);
                    }
                    else if (cmd->ElemCount > 0)
                    {
                        uint64_t state = 0
                            | BGFX_STATE_WRITE_RGB
                            | BGFX_STATE_WRITE_A
                            | BGFX_STATE_MSAA
                            ;

                        bgfx_texture_handle_t TexHandle  = {};
                      
                        if (cmd->TextureId != NULL)
                        {
                            TexHandle.idx = (uint16_t)cmd->TextureId;
                        }
                        else
                        {
                            TexHandle.idx = WhitePixelTexture.idx;
                        }
                        state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);

                        const uint16_t xx = uint16_t((cmd->ClipRect.x > 0.0f ? cmd->ClipRect.x : 0.0f) );
                        const uint16_t yy = uint16_t((cmd->ClipRect.y > 0.0f ? cmd->ClipRect.y : 0.0f) );
                        bgfx_set_scissor(xx, yy
                                         , uint16_t(( cmd->ClipRect.z > 65535.0f ? 65535.0f : cmd->ClipRect.z )-xx)
                                         , uint16_t(( cmd->ClipRect.w > 65535.0f ? 65535.0f : cmd->ClipRect.w)-yy)
                                         );

                        bgfx_set_state(state,0);
                        bgfx_set_texture(0, TextureUniform, TexHandle,UINT32_MAX);
                        bgfx_set_transient_vertex_buffer(0, &tvb, 0, numVertices);
                        bgfx_set_transient_index_buffer(&tib, offset, cmd->ElemCount);
                        bgfx_submit(viewID, ImGuiShader.ShaderProgram,0,false);
                    }

                    offset += cmd->ElemCount;
                }
            }
        
            bgfx_frame(false);
        }
    }
    ImGui_ImplWin32_Shutdown();
    return 0;
}
