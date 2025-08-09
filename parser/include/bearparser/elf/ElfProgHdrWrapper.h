#pragma once

#include "../ExeElementWrapper.h"
#include "elf.h"

class ElfProgHdrWrapper : public ExeElementWrapper
{
public:
    enum FieldID {
        NONE = FIELD_NONE,
        PH_TYPE = 0,
        PH_FLAGS,
        PH_OFFSET,
        PH_VADDR,
        PH_PADDR,
        PH_FILE_SIZE,
        PH_MEM_SIZE,
        PH_ALIGN,
        FIELD_COUNTER
    };

    ElfProgHdrWrapper(Executable *elfExe) : ExeElementWrapper(elfExe) { }

    virtual void* getPtr() { return m_Exe->getContent(); }

    virtual bufsize_t getSize() { 
        return m_Exe->isBit64() ? sizeof(Elf64_Phdr) : sizeof(Elf32_Phdr); 
    }

    virtual QString getName() {
        return "ELF Program Header";
    }

    virtual size_t getFieldsCount() { return FIELD_COUNTER; }

    virtual void* getFieldPtr(size_t fieldId, size_t subField = FIELD_NONE) { }
    virtual QString getFieldName(size_t fieldId) { }
    virtual Executable::addr_type containsAddrType(size_t fieldId, size_t subField = FIELD_NONE) { }
};