#include "elf/ELFNodeWrapper.h"
#include "elf/ELFFile.h"

ELFElementWrapper::ELFElementWrapper(ELFFile *elf)
    : ExeElementWrapper(elf), m_ELF(elf) 
{ 

}

