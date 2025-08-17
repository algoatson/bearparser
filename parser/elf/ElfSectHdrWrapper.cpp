#include "elf/ElfSectHdrWrapper.h"
#include "elf/ELFFile.h"

ElfSectHdrWrapper::ElfSectHdrWrapper(ELFFile *elfExe)
    : ELFElementWrapper(elfExe), shdrs {} 
    {
        wrap();
        qInfo() << "Section Headers Entry Size:" << getEntrySize();
        qInfo() << "Section Headers Size:" << getSize();

        void *ptr = getPtr();
        qInfo() << "ELF Section Header is located at:" << ptr;
    } 

bool ElfSectHdrWrapper::wrap() {
    // shdrs = m_ELF->getShdrsVariant();
    // return std::visit([](auto *ptr){ return ptr != nullptr; }, shdrs);
    shdrs = m_ELF->getSectionHeaders();
    return shdrs.size() > 0;
}

void *ElfSectHdrWrapper::getPtr() {
    return m_ELF->getContentAt(m_ELF->elfSectHdrOffset(), m_ELF->elfSectHdrSize());
}

bufsize_t ElfSectHdrWrapper::getEntrySize() {
    if (shdrs.isEmpty()) return 0;
    return std::visit([](auto *ptr) {
        using T = std::remove_pointer_t<decltype(ptr)>;
        return sizeof(T);
    }, shdrs[0]);
}

bufsize_t ElfSectHdrWrapper::getSize() {
    return m_ELF->elfSectHdrSize();
}