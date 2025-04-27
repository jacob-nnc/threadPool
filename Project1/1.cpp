#include <cassert>
#define _USE_MATH_DEFINES
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>
#include <easyx.h>
#include "point.h"
#include <fstream>
#include <algorithm>
#include <array>
#include <set>
#include <numeric>
#include <windows.h>
#include "threadPool.h"
using namespace std;


int main1() { 
    // test cross point<4> and print b
    point<3> a[2];
    a[0] = point<3>{ 1, 0, 0 };
    a[1] = point<3>{0,0,  1};
    vector<point<3>> b = cross<3>({ a[0], a[1]});
    cout << "b:" << endl;
    for(auto &i:b){
        cout << i.to_string() << endl;

    }
    return 0;
}

int main2() {
    vector<point<4>> a = { {-0.166769, 0.000000, -10.859960, -0.985996},
        {-0.985996, 0.000000, 0.667687, 0.166769},
        {0.000000, 1.000000, -1.000000, 0.000000},
    };
    auto B = solve<3>(a);

    cout << B.to_string() << endl;
    return 0;
}

int main4(int argc, char *argv[]) {
    array<point<3>,8> cube;
    array<array<int,2>,12> lines;
    for (int i = 0; i < 8; i++) {
        cube[i] = point<3>{2. * (i % 2) - 1, 2. * (i / 2 % 2) - 1,
                           2. * (i / 4 % 2) - 1};
    }

    for (int i = 0,idx=0; i < 7; i++)
    {
        for (int j = i + 1; j < 8; j++)
        {
            if (cube[i].length(cube[j]) <= 2.0001)
            {
                lines[idx] = { i,j };
                idx++;
            }
        }
    }

    point<3> cam[4];
    cam[0] = point<3>{10, 0, 0};                  // cam pos
    cam[1] = point<3>{0, 0, 0};                   // target
    cam[2] = point<3>{0, 0, 1};                   // y
    cam[3] = cross<3>({cam[2], cam[0] - cam[1]})[0]; // x


    initgraph(400, 400);
    double d = 1;
    double w = 2;
    double h = 2;

    BeginBatchDraw();
    for (int i = 0;; i++) {
        
        double theta = i * M_PI / 300;
        double phi = i*M_PI/300;
        cam[0] = 10 * point<3>{sin(theta)*cos(phi), cos(theta)*cos(phi), sin(phi)};
        cam[2] = { cos(theta),-sin(theta),0};
        cam[3] = cross<3>({cam[2], cam[0] - cam[1]})[0]; // x

        cout << "cam:" << endl;
        for (int i = 0; i < 4; i++) {
            cout << cam[i].to_string() << endl;
        }

        auto temp = cam[0].length(cam[1]);
        if (temp < 1e-8)
            continue;
        auto center = (cam[0] * (temp - d) + cam[1] * d) / temp;
        cout << "center:" << endl;
        cout << center.to_string() << endl;

        point<3> A[2];
        A[0] = cam[3].normalize();
        A[1] = cam[2].normalize();


        vector<point<4>> B(3);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 2; j++) {
                B[i].x[j] = A[j].x[i];
            }
        }
        vector<point<3>> po;
        for (int j = 0; j < cube.size(); j++) {
            double t = cam[0].length(cube[j]);

            auto tt = cam[0]-cube[j];
            auto tt1 = cam[0] -center;
            for (int k = 0; k < 3; k++) {
                B[k][2] = tt[k];
                B[k][3] = tt1[k];
            }

            cout << "B:" << endl;
            for (int k = 0; k < 3; k++) {
                cout << B[k].to_string() << endl;
            }

            auto ret = solve<3>(B);

            
            po.push_back(ret);
        }
        Sleep(1);
        cleardevice();
        for (int j = 0; j < po.size(); j++)
        {
            solidcircle(po[j][0]*800+200,po[j][1]*800+200,2);
        }
        for (int j = 0; j < lines.size(); j++)
        {
            auto p1 = po[lines[j][0]];
            auto p2 = po[lines[j][1]];
            setlinecolor(WHITE);
            line(p1[0] * 800 + 200, p1[1] * 800 + 200, p2[0] * 800 + 200, p2[1] * 800 + 200);
        }
        
        FlushBatchDraw();
        //getchar();
    }
    EndBatchDraw();

    return 0;
}

// 获取桌面 WorkerW 窗口
HWND GetWallpaperWindow() {
    HWND progman = FindWindow(L"Progman", NULL);
    SendMessage(progman, 0x052C, 0, 0);  // 通知创建 WorkerW

    HWND wallpaper = nullptr;

    EnumWindows([](HWND topHandle, LPARAM lParam) -> BOOL {
        HWND shellView = FindWindowEx(topHandle, NULL, L"SHELLDLL_DefView", NULL);
        if (shellView != NULL) {
            HWND* pWallpaper = (HWND*)lParam;
            *pWallpaper = FindWindowEx(NULL, topHandle, L"WorkerW", NULL);
        }
        return TRUE;
        }, (LPARAM)&wallpaper);

    return wallpaper;
}

// 设置窗口透明（alpha 取值 0~255）
void SetTransparent(HWND hwnd, BYTE alpha = 180) {
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}

template <size_t n>
void drawPoint(const vector<point<n>>& po,const vector<COLORREF>& colors,const array<int,2>& center,int drawK) {
    for (int j = 0; j < po.size(); j++)
    {
        putpixel(po[j][0] * drawK + center[0], po[j][1] * drawK + center[1],colors[j]);
    }
}

template <size_t n>
void drawLines(const vector<point<n>>& po,const vector<array<int,2>>& lines, const array<int, 2> center, int drawK) {
    for (int j = 0; j < lines.size(); j++)
    {
        auto p1 = po[lines[j][0]];
        auto p2 = po[lines[j][1]];
        setlinecolor(WHITE);
        line(p1[0] * drawK + center[0], p1[1] * drawK + center[1], p2[0] * drawK + center[0], p2[1] * drawK + center[1]);
    }
}

vector<string> split_str(const string &s,char t=' ') {
    int begin = 0;
    vector<string> ret;
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == t) {
            ret.push_back(s.substr(begin, i - begin));
            begin = i + 1;
        }
    }
    if (begin < s.size())ret.push_back(s.substr(begin));
    return ret;
}

void read(string fname,string imgname, vector<point<3>>& dots, vector<array<int, 2>>& lines, vector<COLORREF>& colorin)
{
    set<array<int, 2>> lines_un;
    vector<COLORREF> color;
    ifstream is(fname);

    IMAGE img;
    int wideCharCount = MultiByteToWideChar(CP_ACP, 0, imgname.c_str(), -1, NULL, 0);
    if (wideCharCount == 0) {
        cerr << "Error converting string to wide character." << endl;
        return;
    }

    // 加载图像
    loadimage(&img, wstring(imgname.begin(),imgname.end()).c_str());
    int H=img.getheight();
    int W = img.getwidth();
    SetWorkingImage(&img);
    double maxnum = 0;
    set<array<int,2>> vts;
    while (1) {
        string s;
        if (!getline(is, s) ){
            break;
        }
        if (s.empty())continue;
        auto t = split_str(s);
        if (t[0] == "v") {
            point<3> v;
            for (int i = 0; i < 3; i++)
            {
                v[i] = stod(t[i + 1]);
            }
            dots.push_back(v);
        }
        else if (t[0] == "f") {
            vector<int> idxs;
            for (int i = 1; i < t.size(); i++)
            {
                auto tt = split_str(t[i], '/');
                idxs.push_back(stoi(tt[0])-1);
                vts.insert({ idxs.back(),stoi(tt[1]) - 1 });
            }

            for (int i = 0; i < idxs.size() - 1; i++)
            {
                for (int j = i + 1; j < idxs.size(); j++)
                {
                    if (idxs[i] < idxs[j])
                        lines_un.insert({ idxs[i], idxs[j] });
                    else
                        lines_un.insert({ idxs[j], idxs[i] });
                }
            }
        }
        else if (t[0] == "vt") {
            double x = stod(t[1]);
            double y = stod(t[2]);
            color.push_back(getpixel(x*W,(1-y)*H));
        }
    }
    colorin.resize(dots.size());
    for (auto &i : vts) {
        colorin[i[0]] = color[i[1]];
    }

    point<3> sum;
    
    sum=accumulate(dots.begin(), dots.end(), sum);

    sum = sum / dots.size();
    for (auto& i : dots) {
        i = i - sum;
        for(int j=0;j<3;j++)
            maxnum = max(maxnum, abs(i.x[j]));
    }
    for (auto& i : dots) {
        i = i / maxnum;
    }
    int n=dots.size();
    lines = vector<array<int, 2>>(lines_un.begin(), lines_un.end());
    lines.erase(remove_if(lines.begin(), lines.end(), [n](auto i)->bool {return i[0] >= n || i[1] >= n; }),lines.end());
    SetWorkingImage();
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    
    HANDLE hMutex = CreateMutex(NULL, TRUE, _T("MyUniqueAppMutexName"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // 已经存在一个实例
        return 0;
    }


    // 隐藏控制台窗口
    HWND hwnd1 = GetConsoleWindow();
    ShowWindow(hwnd1, SW_HIDE);

    vector<point<3>> cube;
    vector<array<int, 2>> lines;
    vector<COLORREF> colors;
    read("2.obj","texture_pbr_v128.png", cube, lines,colors);

    point<3> cam[4];
    cam[0] = point<3>{ 3, 0, 0 };                  // cam pos
    cam[1] = point<3>{ 0, 0, 0 };                   // target
    cam[2] = point<3>{ 0, 0, 1 };                   // y
    cam[3] = cross<3>({ cam[2], cam[0] - cam[1] })[0]; // x


    double d = 1;
    double w = 2;
    double h = 2;


    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    // 初始化窗口，使用 SHOWCONSOLE 便于调试，也可以改成 NOCONSOLE
    initgraph(screenW, screenH);

    // 设置 EasyX 窗口句柄
    HWND hwnd = GetHWnd();

    // 嵌入桌面背景
    //HWND desktop = GetWallpaperWindow();
    //if (desktop != nullptr) {
    //    SetParent(hwnd, desktop);
    //}
    //else {
    //    MessageBox(0, L"未能找到桌面背景窗口", L"错误", MB_OK | MB_ICONERROR);
    //    return 1;
    //}

    // 设置窗口为无边框
// 设置窗口为无边框
    LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME);
    SetWindowLong(hwnd, GWL_STYLE, lStyle);

    // 设置窗口为分层透明 + 鼠标穿透
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW;
    exStyle &= ~WS_EX_APPWINDOW;
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    // 设置颜色键透明（RGB值为纯黑：0x000000），这个颜色将变为完全透明
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // SetTransparent(hwnd, 100);

    // 放置在最底层（不影响其他窗口）
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, screenW, screenH, SWP_SHOWWINDOW);

    // setlinestyle(PS_SOLID, 3); // PS_SOLID 为实线，3 为线宽像素数
    setlinecolor(RGB(100, 120, 100));
    // 开始绘制动画

    std::mutex mtx;
    vector<point<3>> po(cube.size());

    BeginBatchDraw();
    for (int i = 0;; i++) {
        double theta = i * M_PI / 300;
        double phi = i * M_PI / 300;
        cam[0] = 1.5 * point<3>{sin(theta)* cos(phi), cos(theta)* cos(phi), sin(phi)};
        cam[2] = { cos(theta),-sin(theta),0 };
        cam[3] = cross<3>({ cam[2], cam[0] - cam[1] })[0]; // x

        //cout << "cam:" << endl;
        //for (int i = 0; i < 4; i++) {
        //    cout << cam[i].to_string() << endl;
        //}

        auto temp = cam[0].length(cam[1]);
        if (temp < 1e-8)
            continue;
        auto center = (cam[0] * (temp - d) + cam[1] * d) / temp;
        //cout << "center:" << endl;
        //cout << center.to_string() << endl;

        point<3> A[3];
        A[0] = cam[3].normalize();
        A[1] = cam[2].normalize();
        A[2] = (center - cam[0]).normalize();

        vector<point<3>> B(3);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                B[i].x[j] = A[j].x[i];
            }
        }

        //cout << "B:" << endl;
        //for (int k = 0; k < 3; k++) {
        //    cout << B[k].to_string() << endl;
        //}

        auto iB = inv(B);

        {
            /*int threadnum = 4;
            threadPool thp(threadnum);
            int batch = ceil((double)cube.size() / threadnum);
            int restbacth = cube.size() - batch * (threadnum - 1);*/
            /*for (int k = 0; k < threadnum; k++) {
                thp.addTask([&cam, &cube, &iB, &po, &mtx,begin=k,batch= (k==threadnum-1? restbacth :batch)]() {*/
            for (int i = 0; i < cube.size(); i++) {
                int idx = i;
                double t = cam[0].length(cube[idx]);

                auto tt = cube[idx] - cam[0];
                point<3> temp;
                for (int jj = 0; jj < 3; jj++)
                {
                    temp[jj] = tt * iB[jj];
                }

                temp[0] = -temp[0] / temp[2];
                temp[1] = temp[1] / temp[2];

                //std::unique_lock<std::mutex> lock(mtx);
                po[idx]=temp;
            }
             /*       });
            }*/
        }
        cleardevice();
        POINT pt;
        if (GetCursorPos(&pt)) {
            // 转换为窗口坐标
            ScreenToClient(GetHWnd(), &pt);

            //drawLines<3>(po, lines, {pt.x,pt.y}, 280);
            // drawPoint<3>(po, colors, {pt.x,pt.y}, 230);

        }

        array<int, 2> cen = { screenW / 2, screenH / 2 };
        drawPoint<3>(po, colors, cen, 300*2);
        // drawLines<3>(po, lines, cen, 2300);

        FlushBatchDraw();
        // Sleep(1);
    }
    EndBatchDraw();

    return 0;
}