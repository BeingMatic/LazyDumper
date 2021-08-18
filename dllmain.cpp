#include "Stucture.h"
#include <fstream>

namespace EXT {
    
    PPEB GetCurrentPeb_() {
        return(PPEB)__readgsqword(0x60);
    }
    HMODULE GetCurrentImageBase() {
        return (HMODULE)GetCurrentPeb_()->ImageBaseAddress;
    }
}

void Main() {

    // Get current base addr of process 
    auto BaseAddress = EXT::GetCurrentImageBase();

    // Get DOS Header
    auto DosHeader = (PIMAGE_DOS_HEADER)((uintptr_t)BaseAddress);

    // Get NT Header
    auto NTHeader = (PIMAGE_NT_HEADERS)((uintptr_t)DosHeader->e_lfanew + (uintptr_t)BaseAddress);

    // Get current image size
    auto ImageSize = NTHeader->OptionalHeader.SizeOfImage;

    // Allocate Mem
    char* AllocatedMem = new char[ImageSize];

    // Copy entire binrary to our allocated memory
    memcpy(AllocatedMem, (const void*)BaseAddress, ImageSize);

    // Get DOS header of the allocated dump
    auto DosHeaderNew = (PIMAGE_DOS_HEADER)((uintptr_t)AllocatedMem);

    // Get NT header now of allocated dump, same thing as DOS
    auto NtHeaderNew = (PIMAGE_NT_HEADERS)((uintptr_t)DosHeaderNew->e_lfanew + (uintptr_t)AllocatedMem);

    // Get Section header of our new allocated dump
    PIMAGE_SECTION_HEADER SectionHeader = IMAGE_FIRST_SECTION(NtHeaderNew);

    // Inerate through each section and fix size + address otherwise it would be one big mess
    for (int i = 0; i < NtHeaderNew->FileHeader.NumberOfSections; i++, SectionHeader++)
    {
        SectionHeader->SizeOfRawData = SectionHeader->Misc.VirtualSize;
        SectionHeader->PointerToRawData = SectionHeader->VirtualAddress;
    }

    // Define filename
    char FileName[MAX_PATH];

    // Get current process file name
    GetModuleFileNameA(0, FileName, MAX_PATH);

    // Add _Dump behind original file
    sprintf_s(FileName, "%s_dump.exe", FileName);

    // Write the dumped and fixed executable to a file
    std::ofstream Dump(FileName, std::ios::binary);
    Dump.write((char*)AllocatedMem, ImageSize);
    Dump.close();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    DisableThreadLibraryCalls(hModule);

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        Main();

    return TRUE;
}

