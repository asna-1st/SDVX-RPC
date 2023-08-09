// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <detours.h>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <Shlwapi.h>
#include <locale>
#include <codecvt>
#include "discord_rpc.h"
#include "MusicDBCLS.h"
#pragma comment(lib, "detours.lib")

typedef BOOL(WINAPI* PWriteFile)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);

PWriteFile pOriginalWriteFile = nullptr;

typedef BOOL(WINAPI* ReadFileType)(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    );

ReadFileType TrueReadFile;
BOOL WINAPI HookedReadFile(
    HANDLE       hFile,
    LPVOID       lpBuffer,
    DWORD        nNumberOfBytesToRead,
    LPDWORD      lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
);

DiscordState state{};
std::unique_ptr<std::thread> discordThread;
std::wstring songFile;
std::int8_t songStatus = 0, matchScreen = 0;
MusicDBCLS db;

BOOL WINAPI HookedWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    if (hFile == GetStdHandle(STD_OUTPUT_HANDLE))
    {
        std::string output(reinterpret_cast<const char*>(lpBuffer), nNumberOfBytesToWrite);

        if (output.find("avs-ea3: soft id code:") != std::string::npos) {
            if (output.find("KFC") != std::string::npos) {
                MessageBox(NULL, L"This is SDVX", L"Info", MB_ICONINFORMATION);
                rpc_update(state, "SOUND VOLTEX EXCEED GEAR", "Booting the game", "", "", "", "", 1);
            }
        }
        //Music Selection
        else if (output.find("I:Attach: out MUSICSELECT") != std::string::npos) {
            rpc_update(state, "", "In the music selection", "sv6", "SOUND VOLTEX EXCEED GEAR", "", "", 0);
        }
        //Matching
        else if (output.find("I:Attach: out ALTERNATIVE_GAME_SCENE") != std::string::npos) {
            matchScreen = 1;
        }
        else if (songStatus == 2 && matchScreen == 1) {
            size_t startPos = songFile.find_first_not_of(L'0');
            size_t endPos = songFile.find(L"_");
            std::wstring musicIDw = songFile.substr(startPos, endPos - startPos);
            std::string musicID(musicIDw.begin(), musicIDw.end());

            startPos = songFile.rfind(L"_") + 1;
            std::wstring diffw = songFile.substr(startPos);
            std::string diff(diffw.begin(), diffw.end());

            //MessageBox(NULL, musicIDw.c_str(), L"Info", MB_ICONINFORMATION);

            MusicInfo musicInfo;
            if (db.getMusicInfoByID(stoi(musicID), musicInfo)) {
                rpc_update(state, musicInfo.musicTitle, musicInfo.musicArtist + " [" + diff + "]", "sv6", "SOUND VOLTEX EXCEED GEAR", "", "", 1);
            }
            else {
                rpc_update(state, "Error", "", "sv6", "SOUND VOLTEX EXCEED GEAR", "", "", 1);
            }
            songStatus = 0, matchScreen = 0;
        }
        //Result
        else if (output.find("I:Attach: out RESULT_SCENE") != std::string::npos) {
            rpc_update(state, "", "Result Screen", "sv6", "SOUND VOLTEX EXCEED GEAR", "", "", 0);
        }
        //Final Result
        else if (output.find("I:Attach: out T_RESULT_SCENE") != std::string::npos) {
            rpc_update(state, "", "Final Result Screen", "sv6", "SOUND VOLTEX EXCEED GEAR", "", "", 0);
        }
        //Game Over
        else if (output.find("I:Attach: out GAMEOVER") != std::string::npos) {
            rpc_update(state, "", "GAMEOVER Screen", "sv6", "SOUND VOLTEX EXCEED GEAR", "", "", 0);
        }
        //Title Screen
        else if (output.find("I:Attach: out TITLE_SCENE") != std::string::npos) {
            rpc_update(state, "", "Title Screen", "sv6", "SOUND VOLTEX EXCEED GEAR", "", "", 0);
        }
    }

    return pOriginalWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

BOOL WINAPI HookedReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD  nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    if (lpOverlapped == nullptr)
    {
        wchar_t szFileName[MAX_PATH];
        if (GetFinalPathNameByHandle(hFile, szFileName, MAX_PATH, FILE_NAME_NORMALIZED))
        {
            // Check if the file extension is ".vox"
            if (PathMatchSpec(szFileName, L"*.vox"))
            {
                // Log or process the file path
                //MessageBox(NULL, PathFindFileName(szFileName), L"Info", MB_ICONINFORMATION);
                LPCTSTR pszFileName = PathFindFileName(szFileName);
                //songFile = pszFileName;
                if (songFile == pszFileName) {
                    songStatus += 1;
                }
                else {
                    songFile = pszFileName;
                    songStatus += 1;
                    //songStatus = 0;
                }
                //OutputDebugString(szFileName);
            }
        }
    }

    // Call the original function to maintain normal behavior
    return TrueReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

std::wstring GetProgramStartupPath() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);

    std::wstring path = buffer;
    size_t lastSlash = path.find_last_of(L'\\');
    if (lastSlash != std::wstring::npos) {
        path = path.substr(0, lastSlash + 1); // Include the trailing backslash
    }
    //std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    //std::string narrowString = converter.to_bytes(path);

    return path;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Attach the hook
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        TrueReadFile = (ReadFileType)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "ReadFile");
        DetourAttach(&(PVOID&)TrueReadFile, HookedReadFile);
        pOriginalWriteFile = WriteFile;
        DetourAttach(&(PVOID&)pOriginalWriteFile, HookedWriteFile);
        DetourTransactionCommit();
        db = MusicDBCLS("test.db");
        if (!db.openDatabase()) {
            std::cout << "Failed to open the database." << std::endl;
        }
        
        // Initialize Discord Rich Presence
        rpc_init(state);

        std::signal(SIGINT, handleInterruptSignal);

        discordThread = std::make_unique<std::thread>(runDiscordCallbacks, std::ref(state));
        discordThread->detach();


        break;

    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueReadFile, HookedReadFile);
        DetourDetach(&(PVOID&)pOriginalWriteFile, HookedWriteFile);
        DetourTransactionCommit();

        // Cleanup Discord Rich Presence
        rpc_close(state);

        if (discordThread && discordThread->joinable())
            discordThread->join();
        break;

    default:
        break;
    }
    return TRUE;
}
