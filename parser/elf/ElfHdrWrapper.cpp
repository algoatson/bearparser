#include "elf/ElfHdrWrapper.h"
#include "elf/ELFFile.h" // full definition here

bool ElfHdrWrapper::wrap() { 
        hdr = m_ELF->getEhdrVariant();
        return std::visit([](auto *ptr){ return ptr != nullptr; }, hdr); 
    }

void* ElfHdrWrapper::getPtr() {
    return m_ELF->getContent();
}

void* ElfHdrWrapper::getFieldPtr(size_t fieldId, size_t subField) {
    return nullptr;
}

bufsize_t ElfHdrWrapper::getSize() {
    auto hdr = m_ELF->getEhdrVariant();
    
    return std::visit([](auto *ptr) {
        using T = std::remove_pointer_t<decltype(ptr)>;
        return sizeof(T);
    }, hdr);

    // return sizeof(std::remove_pointer_t<decltype(m_ELF->getEhdrVariant())>);
    // return m_ELF->isBit64() ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr);
}