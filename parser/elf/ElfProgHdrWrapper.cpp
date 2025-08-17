#include "elf/ElfProgHdrWrapper.h"
#include "elf/ELFFile.h"

ElfProgHdrWrapper::ElfProgHdrWrapper(ELFFile *elfExe) 
        : ELFElementWrapper(elfExe), phdrs {}
        {
            wrap();
            qInfo() << "Program Headers Entry Size:" << getEntrySize();
            qInfo() << "Program Headers Size:" << getSize();

            void *ptr = getPtr();
            qInfo() << "ELF Program Header is located at:" << ptr;
        }

bool ElfProgHdrWrapper::wrap() {
    // phdrs = m_ELF->getPhdrsVariant();
    // return std::visit([](auto *ptr){ return ptr != nullptr; }, phdrs);
    phdrs = m_ELF->getProgramHeaders();
    return phdrs.size() > 0;
}

void *ElfProgHdrWrapper::getPtr() {
    return m_ELF->getContentAt(m_ELF->elfProgHdrOffset(), m_ELF->elfProgHdrSize());
}

bufsize_t ElfProgHdrWrapper::getEntrySize() {
    if (phdrs.isEmpty()) return 0;
    return std::visit([](auto *ptr) {
        using T = std::remove_pointer_t<decltype(ptr)>;
        return sizeof(T);
    }, phdrs[0]);
}

bufsize_t ElfProgHdrWrapper::getSize() {
    return m_ELF->elfProgHdrSize();
}