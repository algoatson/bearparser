#pragma once

#include "../ExeElementWrapper.h"
#include "../Executable.h"

#include "elf.h"

class ElfHdrWrapper : public ExeElementWrapper
{
public:
    enum FieldID {
        NONE = FIELD_NONE,
        E_IDENT_MAG = 0,
        E_IDENT_CLASS,
        E_IDENT_DATA,
        E_IDENT_VERSION,
        E_IDENT_OSABI,
        E_IDENT_ABIVERSION,
        FIELD_COUNTER
    };

    ElfHdrWrapper(Executable *elfExe) : ExeElementWrapper(elfExe) { }

    virtual void* getPtr() { return m_Exe->getContent(); }

    virtual bufsize_t getSize() {
        return m_Exe->isBit64() ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr);
    }

    virtual QString getName() { return "ELF Hdr"; }
    virtual size_t getFieldsCount() { return FIELD_COUNTER; }

    virtual void* getFieldPtr(size_t fieldId, size_t subField = FIELD_NONE) { }
    virtual QString getFieldName(size_t fieldId) { }
    virtual Executable::addr_type containsAddrType(size_t fieldId, size_t subField = FIELD_NONE) { }
};