#include "elf/ElfSectHdrWrapper.h"
#include "elf/ELFFile.h"

ElfSectHdrWrapper::ElfSectHdrWrapper(ELFFile *elfExe)
    : ELFElementWrapper(elfExe),
      shdrs(static_cast<Elf64_Shdr*>(nullptr)) 
    {
        qInfo() << "Section Headers Entry Size:" << getEntrySize();
        qInfo() << "Section Headers Size:" << getSize();
        wrap();

        void *ptr = getPtr();
        qInfo() << "ELF Section Header is located at:" << ptr;
    } 

bool ElfSectHdrWrapper::wrap() {
    shdrs = m_ELF->getShdrsVariant();
    return std::visit([](auto *ptr){ return ptr != nullptr; }, shdrs);
}

void *ElfSectHdrWrapper::getPtr() {
    return m_ELF->getContentAt(m_ELF->elfSectHdrOffset(), m_ELF->elfSectHdrSize());
}

bufsize_t ElfSectHdrWrapper::getEntrySize() {
    return std::visit([](auto *ptr) {
        using T = std::remove_pointer_t<decltype(ptr)>;
        return sizeof(T);
    }, shdrs);
}

bufsize_t ElfSectHdrWrapper::getSize() {
    return m_ELF->elfSectHdrSize();
}