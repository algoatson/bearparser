#include "elf/ElfProgHdrWrapper.h"
#include "elf/ELFFile.h"

bool ElfProgHdrWrapper::wrap() {
    phdrs = m_ELF->getPhdrsVariant();
    return std::visit([](auto *ptr){ return ptr != nullptr; }, phdrs);
}

void *ElfProgHdrWrapper::getPtr() {
    return m_ELF->getContentAt(m_ELF->elfProgHdrOffset(), m_ELF->elfProgHdrSize());
}

bufsize_t ElfProgHdrWrapper::getSize() {
    auto hdr = m_ELF->getPhdrsVariant();
    
    return std::visit([](auto *ptr) {
        using T = std::remove_pointer_t<decltype(ptr)>;
        return sizeof(T);
    }, hdr);
}