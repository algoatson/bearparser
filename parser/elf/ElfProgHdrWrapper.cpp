#include "elf/ElfProgHdrWrapper.h"
#include "elf/ELFFile.h"

bool ElfProgHdrWrapper::wrap() {
    phdrs = m_ELF->getPhdrsVariant();
    return std::visit([](auto *ptr){ return ptr != nullptr; }, phdrs);
}

void *ElfProgHdrWrapper::getPtr() {
    return m_ELF->getContentAt(m_ELF->elfProgHdrOffset(), m_ELF->elfProgHdrSize());
}

bufsize_t ElfProgHdrWrapper::getEntrySize() {
    return std::visit([](auto *ptr) {
        using T = std::remove_pointer_t<decltype(ptr)>;
        return sizeof(T);
    }, phdrs);
}

bufsize_t ElfProgHdrWrapper::getSize() {
    return m_ELF->elfProgHdrSize();
}