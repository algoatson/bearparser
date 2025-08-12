#pragma once

#include "../ExeNodeWrapper.h"

class ELFFile;

class ELFElementWrapper : public ExeElementWrapper
{
public:
    ELFElementWrapper(ELFFile* elf);
    virtual ~ELFElementWrapper() {}

    ELFFile *getELF() { return m_ELF; }

protected:
    ELFFile *m_ELF;

friend class ELFFile;
};