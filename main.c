/*
* Application for messaging via the Internet and using IP addresses
* WIN32 API, SOCKET, IP, INTERNET
* Alex_K 11.04.2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winsock2.h>
#include <windows.h>

//#define DEBUG
#define SIZEMAS 250
#define SIZEMASOUT 200
#define PORT 1234
#define CLR_WND_BCG RGB(189,229,253)

HWND hwndParents, ShowMessages, outputMessage, inputName, inputAddress, inputType, ShowStatus1, ShowStatus2;
HWND name, address, type, status;

HWND bt_start, bt_send;

MSG msg = {};

LPDRAWITEMSTRUCT lpds;

char strName[20] = {};
char strAddress[20] = {};
char messageOutput[SIZEMASOUT] = {};
char messageInput[SIZEMAS] = {};
char messageMainInput[5000] = {};
char messageMainOutput[SIZEMAS] = {};

enum {client, server, noShoose} eType = noShoose;
typedef enum {red, green, grey} tColorBt;

BOOL flag_Start = FALSE;
BOOL flag_Send = FALSE;
BOOL flag_Connect = FALSE;

int lineCount = 0;

LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void chat(SOCKET s);
void blockWindows();
void setColorButton(char str[], tColorBt color);
void clearWindows();

int main()
{
    /* WINDOWS */

    const char CLASS_NAME[] = "My Window";
    const char WINDOW_NAME[] = "Messaging";

    BOOL connectClient = FALSE;

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(CLR_WND_BCG);
    RegisterClass(&wc);

    hwndParents = CreateWindow(CLASS_NAME, WINDOW_NAME, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, NULL, NULL);
    ShowMessages = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | ES_READONLY | ES_MULTILINE | WS_VSCROLL,
                             10, 20, 475, 180, hwndParents, NULL, NULL, NULL);
    outputMessage = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | ES_MULTILINE, 10, 220, 475, 55, hwndParents, NULL, NULL, NULL);
    inputName = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | ES_RIGHT, 100, 340, 130, 20, hwndParents, NULL, NULL, NULL);
    inputAddress = CreateWindow("edit", "", WS_VISIBLE | WS_CHILD | ES_RIGHT, 100, 370, 130, 20, hwndParents, NULL, NULL, NULL);
    inputType = CreateWindow("combobox", "", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 100, 400, 130, 100, hwndParents, NULL, NULL, NULL);
        SendMessage(inputType, CB_ADDSTRING, 0, (LPARAM)"client");
        SendMessage(inputType, CB_ADDSTRING, 0, (LPARAM)"server");
    bt_start = CreateWindow("button", "start", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 100, 280, 80, 30, hwndParents, NULL, NULL, NULL);
    bt_send = CreateWindow("button", "send", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 300, 280, 80, 30, hwndParents, NULL, NULL, NULL);
    name = CreateWindow("static", "name", WS_VISIBLE | WS_CHILD | SS_RIGHT, 10, 340, 80, 20, hwndParents, NULL, NULL, NULL);
    address = CreateWindow("static", "address", WS_VISIBLE | WS_CHILD | SS_RIGHT, 10, 370, 80, 20, hwndParents, NULL, NULL, NULL);
    type = CreateWindow("static", "type", WS_VISIBLE | WS_CHILD | SS_RIGHT, 10, 400, 80, 20, hwndParents, NULL, NULL, NULL);
    ShowStatus1 = CreateWindow("static", "", WS_VISIBLE | WS_CHILD | SS_RIGHT, 330, 340, 150, 20, hwndParents, NULL, NULL, NULL);
    ShowStatus2 = CreateWindow("static", "", WS_VISIBLE | WS_CHILD | SS_RIGHT, 330, 370, 150, 20, hwndParents, NULL, NULL, NULL);
    status = CreateWindow("static", "status", WS_VISIBLE | WS_CHILD | SS_RIGHT, 250, 340, 70, 20, hwndParents, NULL, NULL, NULL);

    SetWindowText(ShowStatus1, "No connect");
    SetWindowText(outputMessage, "Enter your message");
    SetWindowText(ShowMessages, "If you want to be a client you need to set the server`s address and shoose the type.\
 If you want to be a server you need to set only type. One person can be only a client and another only a server.\
 You can set your name or not. After that you need to press START and waiting for the connect. Have a good connecting and messaging :-)");

    ShowWindow(hwndParents, SW_NORMAL);

    /* SOCKET */

    WSADATA ws;
    WSAStartup(MAKEWORD(2,2), &ws);

    SOCKET s;
    s = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN sa = {};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);

    while(1)
    {
        if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
            if(msg.message == WM_QUIT)
                break;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (eType == client && flag_Start)
            {
                if(flag_Start)
                {
                    #ifdef DEBUG
                        printf("start client\n");
                    #endif // DEBUG

                    sa.sin_addr.S_un.S_addr = inet_addr(strAddress);

                    int conResult = -1;

                    while(conResult != 0)
                    {
                        conResult = connect(s, (SOCKADDR*)&sa, sizeof(sa));
                        if (conResult == SOCKET_ERROR)
                        {
                            #ifdef DEBUG
                                printf("connect error = %d\n", WSAGetLastError());
                            #endif // DEBUG

                            SetWindowText(ShowStatus1, "connection started");
                        }
                        if (conResult == 0)
                        {
                            #ifdef DEBUG
                                printf("connect OK\n");
                            #endif // DEBUG

                            SetWindowText(ShowStatus1, "connect OK");
                            clearWindows();

                            flag_Start = FALSE;
                            connectClient = TRUE;
                        }
                    }
                }

                if(connectClient)
                {
                    #ifdef DEBUG
                        printf("init client\n");
                    #endif // DEBUG

                    chat(s);
                }
            }

            if (eType == server && flag_Start)
            {
                bind(s, (SOCKADDR*)&sa, sizeof(sa));

                listen(s, 100);

                SetWindowText(ShowStatus1, "connection started");

                SOCKET client_socket;
                SOCKADDR_IN client_addr;
                int client_addr_size = sizeof(client_addr);

                #ifdef DEBUG
                    printf("start connect\n");
                #endif // DEBUG

                while( (client_socket = accept(s, (SOCKADDR*)&client_addr, &client_addr_size)) )
                {
                    #ifdef DEBUG
                        printf("connect OK\n");
                    #endif // DEBUG

                    SetWindowText(ShowStatus1, "connect OK");
                    clearWindows();

                    chat(client_socket);
                }
            }
        }
    }

    closesocket(s);
    WSACleanup();

    return 0;
}

LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_CTLCOLORSTATIC:
            #ifdef DEBUG
                puts("WM_CTLCOLORSTATIC");
            #endif // DEBUG
            if(lParam == (LPARAM)name || lParam == (LPARAM)address || lParam == (LPARAM)type || lParam == (LPARAM)status)
            {
                SetBkMode((HDC)wParam, TRANSPARENT);
                return (INT_PTR)CreateSolidBrush(CLR_WND_BCG);
            }

            if(lParam == (LPARAM)ShowMessages)
            {
                return (INT_PTR)CreateSolidBrush(RGB(255,255,255));
            }

            break;

        case WM_DRAWITEM:
            #ifdef DEBUG
                puts("WM_DRAWITEM");
            #endif // DEBUG
            lpds = (DRAWITEMSTRUCT*)lParam;

            if(lpds->hwndItem == bt_start)
            {
                if(flag_Connect != TRUE)
                    setColorButton("start", green);
                if(flag_Connect)
                    setColorButton("start", grey);
            }

            if(lpds->hwndItem == bt_send)
            {
                if(flag_Connect != TRUE)
                    setColorButton("send", red);
                if(flag_Connect)
                    setColorButton("send", green);
            }

            DrawEdge(lpds->hDC, &lpds->rcItem, (lpds->itemState & ODS_SELECTED ? EDGE_SUNKEN : EDGE_RAISED), BF_RECT);

            return TRUE;

            break;

        case WM_COMMAND:
            #ifdef DEBUG
                puts("WM_COMMAND");
            #endif // DEBUG

            if(lParam == (LPARAM)inputName)
            {
                GetWindowText((HWND)lParam, strName, 20);
                #ifdef DEBUG
                    printf("name: %s\n", strName);
                #endif // DEBUG
            }

            if(lParam == (LPARAM)inputAddress)
            {
                GetWindowText((HWND)lParam, strAddress, 20);
                #ifdef DEBUG
                    printf("address: %s\n", strAddress);
                #endif // DEBUG
            }

            if(lParam == (LPARAM)outputMessage)
            {
                GetWindowText((HWND)lParam, messageOutput, SIZEMASOUT);

                #ifdef DEBUG
                    printf("output Message: %s\n", messageOutput);
                #endif // DEBUG

                int i = 0;
                while(messageOutput[i] != 0)     // send a message after clicking ENTER
                {
                    if(messageOutput[i] == '\r')
                    {
                        flag_Send = TRUE;
                        #ifdef DEBUG
                            printf("Send = TRUE\n");
                        #endif // DEBUG
                        messageOutput[i] = '\0';
                        break;
                    }
                    i++;
                }

                #ifdef DEBUG
                    printf("messageOutput\n");
                    for(int j = 0; j < 30; j++)
                        printf("[%d]=%d ", j, messageOutput[j]);
                    printf("\n");
                #endif // DEBUG
            }

            if(lParam == (LPARAM)inputType)
            {
                if(HIWORD(wParam) == CBN_SELCHANGE)
                    eType = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
            }

            if(lParam == (LPARAM)bt_start)
            {
                flag_Start = TRUE;
            }

            if(lParam == (LPARAM)bt_send)
            {
                flag_Send = TRUE;
            }

            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc (hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void chat(SOCKET s)
{
    flag_Connect = TRUE;
    blockWindows();
    EnableWindow(bt_send, TRUE);

    WSAEventSelect(s, 0, FD_READ);

    while(1)
    {
        if( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
            if(msg.message == WM_QUIT)
            {
                exit(0);
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            while(recv(s, messageInput, sizeof(messageInput), 0) > 0)
            {
                #ifdef DEBUG
                    printf("messageInput= %s\n", messageInput);
                    for(int i = 0; i < 30; i++)
                        printf("[%d]=%d ", i, messageInput[i]);
                    printf("\n");
                #endif // DEBUG

                strcat(messageMainInput, messageInput);
                strcat(messageMainInput, "\r\n");

                SetWindowText(ShowMessages, messageMainInput);

                lineCount += 3;

                SendMessage(ShowMessages, EM_LINESCROLL, 0, lineCount);
            }
            if(flag_Send)
            {
                if(strlen(messageOutput) == 0)
                {
                    flag_Send = FALSE;
                }
                else
                {
                    memset(messageMainOutput, 0, sizeof(messageMainOutput));

                    strcat(messageMainOutput, "message ");
                    strcat(messageMainOutput, strName);
                    strcat(messageMainOutput, ": ");
                    strcat(messageMainOutput, messageOutput);

                    #ifdef DEBUG
                        printf("messageMainOutput\n");
                        for(int i = 0; i < 30; i++)
                            printf("[%d]=%d ", i, messageMainOutput[i]);
                        printf("\n");
                    #endif // DEBUG

                    int snd = send(s, messageMainOutput, sizeof(messageMainOutput), 0);

                    if(snd > 0)
                        SetWindowText(ShowStatus2, "message sent");
                    else
                    {
                        SetWindowText(ShowStatus2, "message didn't send");
                        SetWindowText(ShowStatus1, "No connect");
                        flag_Connect = FALSE;
                    }

                    strcat(messageMainInput, "your message: ");
                    strcat(messageMainInput, messageOutput);
                    strcat(messageMainInput, "\r\n");
                    SetWindowText(ShowMessages, messageMainInput);

                    lineCount += 3;

                    SendMessage(ShowMessages, EM_LINESCROLL, 0, lineCount);

                    SetWindowText(outputMessage, "");
                    memset(messageOutput, 0, sizeof(messageOutput));

                    flag_Send = FALSE;
                }
            }
        }
    }
}

void blockWindows()
{
    EnableWindow(inputName, FALSE);
    EnableWindow(inputAddress, FALSE);
    EnableWindow(inputType, FALSE);
    EnableWindow(bt_start, FALSE);
    EnableWindow(bt_send, FALSE);
}

void setColorButton(char str[], tColorBt color)
{
    SetTextColor(lpds->hDC, RGB(0,0,0));
    if(color == green)
        SetBkColor(lpds->hDC, RGB(32, 244, 67));
    if(color == red)
        SetBkColor(lpds->hDC, RGB(248,67,86));
    if(color == grey)
        SetBkColor(lpds->hDC, RGB(245,245,245));
    ExtTextOut(lpds->hDC, 25, 5, ETO_OPAQUE | ETO_CLIPPED, &lpds->rcItem, str, strlen(str), NULL);
}

void clearWindows()
{
    SetWindowText(outputMessage, "");
    SetWindowText(ShowMessages, "");
}

